/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "GainProcessor.h"

#include <processor_api/GpuTaskData.h>
#include <processor_api/PortChangedFlags.h>
#include <processor_api/ProcessorSpecification.h>
#include <processor_api/MemoryManager.h>

#include <algorithm>
#include <map>

using namespace GPUA::processor::v2;

#if defined(GPU_AUDIO_MAC)
static constexpr uint32_t g_max_threads_per_block {256u};
#else
static constexpr uint32_t g_max_threads_per_block {512u};
#endif

namespace {
template <typename T>
T divup(T a, T b) {
    return (a + b - 1) / b;
}
} // namespace

Module& GainProcessor::GetModule() const noexcept {
    return m_module;
}

ErrorCode GainProcessor::SetData(void* data, uint32_t data_size) noexcept {
    // make sure we get valid data
    if (data != nullptr && data_size == sizeof(GainConfig::Parameters)) {
        const GainConfig::Parameters* params = reinterpret_cast<const GainConfig::Parameters*>(data);
        // determine the message type - the processor only supports a gain message
        if (params->ThisMessage == params->GainMessage) {
            m_gain_value = params->gain_value;
            return ErrorCode::eSuccess;
        }
    }
    return ErrorCode::eFail;
}

ErrorCode GainProcessor::GetData(void* data, uint32_t& data_size) const noexcept {
    // not implemented; could potentially satisfy user queries
    return ErrorCode::eFail;
}

uint32_t GainProcessor::GetInputPortCount() const noexcept {
    // the gain_processor has one port (for the input data)
    return 1u;
}

ErrorCode GainProcessor::GetInputPort(uint32_t index, InputPort*& port) noexcept {
    // return the requested port
    if (index == 0) {
        port = m_input_port.get();
        return ErrorCode::eSuccess;
    }
    // or nullptr if the index is out-of-bounds (!= 0 in this case)
    port = nullptr;
    return ErrorCode::eOutOfRange;
}

ErrorCode GainProcessor::OnBlueprintRebuild(const ProcessorBlueprint*& blueprint) noexcept {
    // if something changed that requires change to the task configuration
    if (m_changed || m_input_port->m_changed) {
        // the processor requires one block per input channel
        m_gpu_task.block_count = m_input_port->m_channel_count;
        // optimally we have one thread per block; we use multiples of 32 threads up to at most `g_max_threads_per_block`
        m_gpu_task.thread_count = std::min(g_max_threads_per_block, divup(m_input_port->m_max_buffer_size, 32u) * 32u);
        // reset change indicators
        m_changed = m_input_port->m_changed = false;
    }
    blueprint = &m_proc_data;
    return ErrorCode::eSuccess;
}

ErrorCode GainProcessor::PrepareForProcess(const LaunchData& data, uint32_t expected_chunks) noexcept {
    // process the provided user-data
    SetData(data.app_data, data.app_data_size);

    // communicate a blueprint rebuild if anything changed that requires one
    if (m_changed || m_input_port->m_changed)
        return ErrorCode::eBlueprintUpdateNeeded;

    return ErrorCode::eNoChangesNeeded;
}

ErrorCode GainProcessor::PrepareChunk(void* proc_data, void** task_data, uint32_t chunk_id) noexcept {
    // set ProcessorData input for the GPU task in the next launch
    auto proc_params = reinterpret_cast<gain::ProcessorParameter*>(proc_data);
    // number of input channels
    proc_params->channel_count = m_input_port->m_channel_count;
    // maximum number of samples per channel the input buffer can hold
    proc_params->buffer_capacity = m_input_port->m_max_buffer_size;
    // current number of samples per channel in the input buffer (<= buffer_capacity)
    proc_params->buffer_length = m_input_port->m_current_buffer_size;
    // the gain to apply to each sample in each channel of the buffer
    proc_params->gain = m_gain_value;
    return ErrorCode::eSuccess;
}

void GainProcessor::OnProcessingEnd(bool after_fat_transfer) noexcept {
}

ProcessorProfiler* GainProcessor::GetProcessorProfiler() noexcept {
    return nullptr;
}

GainProcessor::GainProcessor(ProcessorSpecification& specification, Module& module) :
    m_module {module},
    m_proc_data {1u, sizeof(gain::ProcessorParameter), ProcessorEndCallback::eNoCallback, 1u, &m_gpu_task},
    m_port_factory {specification.port_factory},
    m_memory_manager {specification.memory_manager} {
    // Get the user-data for processor construction from the ProcessorSpecification
    const GainConfig::Specification* spec = reinterpret_cast<const GainConfig::Specification*>(specification.user_data);
    // make sure the user-data is what we expect it to be, i.e., a GainConfig::Specification
    if (specification.data_size != sizeof(GainConfig::Specification) || spec->ThisType != spec->GainConstructionType) {
        throw std::runtime_error("Error in GainProcessor::GainProcessor: invalid specification provided");
    }
    // use the data provided in the GainConfig::Specification
    m_gain_value = spec->params.gain_value;

    // specify what type of output port the processor has and create it
    PortInfo output_port_info {};
    output_port_info.type = PortType::eRegularPort;
    output_port_info.data_type = PortDataType::eSample32;
    m_output_port = m_port_factory.CreateDataPort(0u, output_port_info);

    // create the processor's input port
    m_input_port = std::make_unique<GainInputPort>(m_output_port.get());

    // the processor only has one task/step and it's index is 0. See `DeclareProcessorStep` in `GainProcessor.cu`
    m_gpu_task.entry_idx = 0u;
    // the task does not need any per-block shared memory
    m_gpu_task.shared_mem_size = 0u;
    // and it does not take task parameters. see `using TaskParameter = void;` in `Properties.h`)
    m_gpu_task.task_param_size = 0u;
    // define dependency relation of blocks (within one task and between tasks)
    m_gpu_task.processing_flags = ::ProcessingFlag::eProcessingFlagBlockForBlockAfterPreviousTask;
}
