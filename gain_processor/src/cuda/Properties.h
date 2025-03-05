/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef GAIN_PROPERTIES_H
#define GAIN_PROPERTIES_H

// The entries in GPUFUNCTIONS_SCRAMBLED are used to replace the processor device function names
// during compilation to avoid name conflicts between processors.
// clang-format off
#define GPUFUNCTIONS_SCRAMBLED \
xZlXkJ6ZTgY7z38wxBdV, \
i7ZmbrK1JkN41qLWphlQ, \
SekraDIjk20sSSc8nVpl, \
ntDCDmE7S72GfkGCz8Wt, \
Ih48E1XSlsHhYKE49X9v, \
lmcqTgaYdZb3MBszdVtM, \
FHbFSrlQkldT3gl0bCzY, \
MARWu9r33xCAGYwfOPDP, \
IU1Gi6vsluEbesDFt2wT, \
gJsr8J2Jg46tQmjSsINe
// clang-format on

#if !defined(GPU_AUDIO_MAC)
#include <cstdint>
#endif

// DO NOT REMOVE! Contains macros for device function name substitution.
#include <scheduler/common_macros.h>

// parameter struct passed to each task (members are set in GainProcessor::PrepareChunk)
namespace gain {
struct ProcessorParameter {
    uint32_t channel_count;
    uint32_t buffer_capacity;
    uint32_t buffer_length;
    float gain;
};

// per task parameter struct. could be different for each task if the processor
// had more than one. unused in gain_processor.
using TaskParameter = void;
} // namespace gain

#endif // GAIN_PROPERTIES_H
