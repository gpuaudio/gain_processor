/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#ifndef GAIN_GAIN_MODULE_H
#define GAIN_GAIN_MODULE_H

#include <processor_api/ModuleBase.h>
#include <processor_api/ModuleInfoProvider.h>
#include <processor_api/ModuleSpecification.h>

class GainModule : public GPUA::processor::v2::ModuleBase {
public:
    explicit GainModule(const GPUA::processor::v2::ModuleSpecification& specification);
    ~GainModule() override = default;

    // Copy ctor and copy assignment are deleted along with move assignment operator deletion
    GainModule& operator=(GainModule&&) = delete;

    ////////////////////////////////
    // GPUA::processor::v2::Module methods
    GPUA::processor::v2::ErrorCode CreateProcessor(GPUA::processor::v2::ProcessorSpecification& specification, GPUA::processor::v2::Processor*& processor) noexcept override;
    GPUA::processor::v2::ErrorCode DeleteProcessor(GPUA::processor::v2::Processor* processor) noexcept override;
    // GPUA::processor::v2::Module methods
    ////////////////////////////////
};

#endif // GAIN_GAIN_MODULE_H
