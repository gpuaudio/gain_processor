/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef GAIN_GAIN_PROCESSOR_CUH
#define GAIN_GAIN_PROCESSOR_CUH

#include "Properties.h"

#include <platform/Abstraction.h>

template <typename TSample>
class GainProcessorDevice {
public:
    // mandatory explicit defined constructor
    __device_fct GainProcessorDevice() __device_addr {
    }

    // mandatory explicitly defined destructor
    __device_fct ~GainProcessorDevice() __device_addr {
    }

    // mandatory init function; can be used to initialize processor data members
    template <class Context>
    __device_fct void init(Context context, unsigned int max_buffer_length) __device_addr {
    }

    // Every task of the processor must match the following interface:
    // ```
    // template <class Context>
    // __device_fct void process(Context context, __device_addr gain::ProcessorParameter* processor_param, __device_addr gain::TaskParameter* task_param,
    //                           __device_addr float* __device_addr* input, __device_addr float* __device_addr* output) __device_addr;
    // ```
    //
    // - `Context` provides all the call specific information, like `threadId`, `blockId`, `callId`, shared memory, synchronization ...
    //    - Depending on the scheduling environment and the task type, we specialize the `Context` as needed this is why it is a template parameter
    //
    // - `ProcessorParameter* processor_param` is a parameter that can be set per call of the processor
    //
    // - `TaskParameter* task_param` is a parameter specific to the task and is set for every task individually
    //    - All tasks get the same `ProcessorParameter` and individual `TaskParameters`
    //    - The host interface has to provide the sizes required for the parameters (see GainProcessor::m_proc_data and GainProcessor::m_gpu_task)
    //
    // - `float** input` points to the input. input[p][s] is sample s of port p.
    //    Layout: all samples of the first channel, all samples of the second channel, ...
    // - `float** output` points to allocated device memory for the output. output[p][s] is sample s of port p.
    //    Layout: all samples of the first channel, all samples of the second channel, ...
    //
    // ================================
    // Basic functionalities of `Context` are:
    //    - `call()` to get the call id : [0, GainProcessor::m_proc_data::num_calls - 1]
    //    - `blockId()` to get the blockId : [0, GainProcessor::m_gpu_task::block_count - 1]
    //    - `threadId()` to get the threadId : [0, GainProcessor::m_gpu_task::thread_count - 1]
    //    - `blockDim()` to get the blockSize : GainProcessor::m_gpu_task::thread_count
    //    - `smem()` to get the registered shared memory : GainProcessor::m_gpu_task::shared_mem_size bytes
    //    - `synchronize()` to synchronize all threads in the block
    //
    // Note:
    //  - exclusively use `context.synchronize()` for synchronization of a block. Platform specific sync operations might hang
    //  - use `context.blockId()`, `context.threadId()`, `context.blockDim()` instead of platform specific alternatives, which might be wrong
    //  - you can use multiple blocks,e.g., one or multiple per channel - Make sure in the host processor that enough blocks are requested (GainProcessor::m_gpu_task::block_count)
    //  - you can use multiple calls to split the processing of a longer buffer into smaller grain-sized portions. You can also do that with a loop inside the task.
    //    However, when you use multiple calls execution can parallelize better if there are multiple processors in the chain, as we can already execute the next processor
    //    when parts of its input, i.e., the current processors output, are available. It will guarantee that, within a processor, a grain-sized portion of the input
    //    will only be processed when the previous portion has been processed.

    template <class Context>
    __device_fct void process(Context context, __device_addr gain::ProcessorParameter* processor_param, __device_addr gain::TaskParameter* task_param,
        __device_addr float* __device_addr* input, __device_addr float* __device_addr* output) __device_addr {
        // sanity check, that not more blocks enter than we have channels. If host code is correct, i.e.,
        // GainProcessor::m_gpu_task::block_count == GainProcessor::m_input_port->m_channel_count, this is obsolete.
        if (context.blockId() < processor_param->channel_count) {
            // get the offset to the first sample of the blocks channel
            uint32_t const channel_offset = context.blockId() * processor_param->buffer_capacity;
            // pointer to the first input sample in the block's channel
            __device_addr TSample const* channel_input = input[0] + channel_offset;
            // pointer to the first output sample in the block's channel
            __device_addr TSample* channel_output = output[0] + channel_offset;
            // iterate over the buffer_length <= buffer_capacity samples of the block's channel; one thread per sample
            for (uint32_t s = context.threadId(); s < processor_param->buffer_length; s += context.blockDim()) {
                // apply the gain and write to output
                channel_output[s] = channel_input[s] * processor_param->gain;
            }
        }
    }
};

#endif // GAIN_GAIN_PROCESSOR_CUH
