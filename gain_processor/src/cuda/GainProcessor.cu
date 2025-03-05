/*
 * Copyright (c) 2022 Braingines SA/GPU Audio INC - All Rights Reserved
 * Unauthorized copying of this file is strictly prohibited
 * Proprietary and confidential
 */

#include "GainProcessor.cuh"

#include <scheduler/device/processor.cuh>

// - `DeclareProcessorStep` must come before `DeclareProcessor`
// - `DeclareProcessorStep` parameters are:
//    - full processor name (with namespace and template parameters)
//    - an increasing integer to mark the tasks 0, 1, 2... (this integer is used to extract the actual task)
//    - the name of the method (currently no additional template parameters are supported)
//    - the processor parameter type (must match the function definition and has to be the same for all tasks)
//    - the task parameter type (must match the function definition and can be different for each task)
// - `DeclareProcessor` parameters are:
//    - full processor name (with namespace and template parameters)
//    - the number of tasks (must match the increasing integer from DeclareProcessorStep)

DeclareProcessorStep(GainProcessorDevice<float>, 0, process, float, gain::ProcessorParameter, gain::TaskParameter);
DeclareProcessor(GainProcessorDevice<float>, 1);
