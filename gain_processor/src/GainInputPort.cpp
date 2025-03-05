/*
 * Copyright (c) 2024 Braingines SA - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "GainInputPort.h"

#include <processor_api/PortDescription.h>

GainInputPort::GainInputPort(GPUA::processor::v2::OutputPort* output_port) :
    m_output_port {output_port} {
}

GPUA::processor::v2::PortId GainInputPort::GetPortId() noexcept {
    return m_output_port->GetPortId();
}

GPUA::processor::v2::ErrorCode GainInputPort::Connect(const GPUA::processor::v2::OutputPort& data_port) noexcept {
    using namespace GPUA::processor::v2;

    // get info of port that wants to connect to `this`
    auto& input_port = data_port.GetPortInfo();

    // make sure the port is compatible and can be connected
    if (input_port.type != PortType::eRegularPort ||
        input_port.data_type != PortDataType::eSample32) {
        return ErrorCode::eUnsupported;
    }

    // store properties of input port to configure the GPU task (see GainProcessor::PrepareChunk)
    m_max_buffer_size = input_port.capacity_in_bytes / sizeof(float);
    m_current_buffer_size = input_port.size_in_bytes / sizeof(float);
    m_channel_count = input_port.channel_count;

    // configure the output port according to the input-port's properties
    auto& output_port = m_output_port->GetPortInfo();
    output_port = input_port;
    output_port.grain = input_port.capacity_in_bytes;
    output_port.transfer_to_cpu = false;
    output_port.is_produced = true;

    // signal the output port to reset with the new properties
    m_output_port->Changed(PortChangedFlags::eReset);

    // indicate that the change to trigger a re-build of the blueprint
    // before the next launch (see GainProcessor::PrepareForProcess)
    m_changed = true;

    return ErrorCode::eSuccess;
}

GPUA::processor::v2::ErrorCode GainInputPort::Disconnect() noexcept {
    using namespace GPUA::processor::v2;

    // clear properties
    m_max_buffer_size = m_current_buffer_size = 0;
    m_channel_count = 0;

    // clear the output port properties
    auto& output_port = m_output_port->GetPortInfo();
    output_port.capacity_in_bytes = 0;
    output_port.size_in_bytes = 0;
    output_port.channel_count = 0;
    output_port.grain = 0;
    output_port.offset = 0;
    output_port.oversampling_ratio = 0;
    output_port.is_produced = false;
    output_port.transfer_to_cpu = false;

    // signal the output port to reset with the cleared properties
    m_output_port->Changed(PortChangedFlags::eReset);

    return ErrorCode::eSuccess;
}

GPUA::processor::v2::ErrorCode GainInputPort::InputPortUpdated(GPUA::processor::v2::PortChangedFlags flags, const GPUA::processor::v2::OutputPort& data_port) noexcept {
    using namespace GPUA::processor::v2;

    // make sure the update is supported
    auto& input_port = data_port.GetPortInfo();
    if (input_port.type != PortType::eRegularPort ||
        input_port.data_type != PortDataType::eSample32) {
        Disconnect();
        return ErrorCode::eUnsupported;
    }

    PortChangedFlags new_flags = flags & (~(PortChangedFlags::eGrainChanged | PortChangedFlags::eTransferToCpuChanged | PortChangedFlags::eProduceInfoChanged));
    if (static_cast<uint32_t>(new_flags) == 0) {
        return ErrorCode::eSuccess;
    }

    constexpr PortChangedFlags need_rebuild_flags = PortChangedFlags::eCapacityChanged | PortChangedFlags::eChannelCountChanged;

    if ((flags % need_rebuild_flags)) {
        m_changed = true;
    }

    auto& output_port = m_output_port->GetPortInfo();

    output_port = input_port;
    output_port.grain = input_port.capacity_in_bytes;
    output_port.transfer_to_cpu = false;
    output_port.is_produced = true;

    m_max_buffer_size = input_port.capacity_in_bytes / sizeof(float);
    m_current_buffer_size = input_port.size_in_bytes / sizeof(float);
    m_channel_count = input_port.channel_count;
    if (flags == PortChangedFlags::eReset || flags == PortChangedFlags::eTypeChanged) {
        m_output_port->Changed(PortChangedFlags::eReset);
    }
    else {
        m_output_port->Changed(new_flags);
    }
    return ErrorCode::eSuccess;
}

uint32_t GainInputPort::GetInputGrain() const noexcept {
    return m_max_buffer_size * sizeof(float);
}

GPUA::processor::v2::ErrorCode GainInputPort::GetPortDescription(const GPUA::processor::v2::PortDescription*& description) const noexcept {
    static GPUA::processor::v2::PortDescription desc {L"Input Port", L"Port forwarded to the output"};
    description = &desc;
    return GPUA::processor::v2::ErrorCode::eSuccess;
}
