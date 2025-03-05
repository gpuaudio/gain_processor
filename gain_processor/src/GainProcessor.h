/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef GAIN_GAIN_PROCESSOR_H
#define GAIN_GAIN_PROCESSOR_H

#include "GainInputPort.h"
#include "Properties.h"

#include <gain_processor/GainSpecification.h>

#include <processor_api/GpuTaskData.h>
#include <processor_api/MemoryManager.h>
#include <processor_api/ModuleBase.h>
#include <processor_api/LaunchData.h>
#include <processor_api/Processor.h>
#include <processor_api/ProcessorBlueprint.h>
#include <processor_api/ProcessorProfiler.h>
#include <processor_api/PortFactory.h>

#include <cstdint>
#include <fstream>
#include <map>

class GainProcessor : public GPUA::processor::v2::Processor {
public:
    explicit GainProcessor(GPUA::processor::v2::ProcessorSpecification& specification, GPUA::processor::v2::Module& module);
    ~GainProcessor() = default;

    // Copy ctor and copy assignment are deleted along with move assignment operator deletion
    GainProcessor& operator=(GainProcessor&&) = delete;

    ////////////////////////////////
    // GPUA::processor::v2::Processor methods
    GPUA::processor::v2::Module& GetModule() const noexcept override;

    GPUA::processor::v2::ErrorCode SetData(void* data, uint32_t data_size) noexcept override;
    GPUA::processor::v2::ErrorCode GetData(void* data, uint32_t& data_size) const noexcept override;

    uint32_t GetInputPortCount() const noexcept override;
    GPUA::processor::v2::ErrorCode GetInputPort(uint32_t index, GPUA::processor::v2::InputPort*& port) noexcept override;

    GPUA::processor::v2::ErrorCode OnBlueprintRebuild(const GPUA::processor::v2::ProcessorBlueprint*& blueprint) noexcept override;
    GPUA::processor::v2::ErrorCode PrepareForProcess(const GPUA::processor::v2::LaunchData& data, uint32_t expected_chunks) noexcept override;
    GPUA::processor::v2::ErrorCode PrepareChunk(void* proc_data, void** task_data, uint32_t chunk_id) noexcept override;
    void OnProcessingEnd(bool after_fat_transfer) noexcept override;

    GPUA::processor::v2::ProcessorProfiler* GetProcessorProfiler() noexcept override;
    // GPUA::processor::v2::Processor methods
    ////////////////////////////////

private:
    GPUA::processor::v2::Module& m_module;
    GPUA::processor::v2::PortFactory& m_port_factory;
    GPUA::processor::v2::MemoryManager& m_memory_manager;

    GPUA::processor::v2::GpuTaskData m_gpu_task;
    GPUA::processor::v2::ProcessorBlueprint m_proc_data;

    std::unique_ptr<GainInputPort> m_input_port;
    GPUA::processor::v2::OutputPortPointer m_output_port {0, 0};

    float m_gain_value {};

    bool m_changed {true};
};

#endif // GAIN_GAIN_PROCESSOR_H
