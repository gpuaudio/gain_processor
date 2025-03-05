# Gain Processor

The `gain_processor` is a simple processor that applies a configurable gain to an input buffer.
It is well suited to understand the components of a processor and to explore the possibilities.
This project also lends itself as a template to start implementing your own GPU-powered processors.

# Host Code Components

## GainModule
Implements the interface for the engine to create and destroy the processor.

## GainDeviceCodeProvider
Provides the engine access to the device binary, which is embedded in the processor binary.

## GainModuleInfoProvider
Implements the interfaces to query properties of the processor like name, id, version and supported GPU platforms.
Also provides the engines with the names of the processor device funtions.

## GainModuleLibrary
Defines the module export functions to create and destroy the GainModule, GainDeviceCodeProvider and GainModuleInfoProvider.

## GainInputPort
Implements the input to the processor. Provides functionality to connect, disconnect or update inputs tot the processor.

## GainProcessor
This is the host-side of the processor and implements the processor interface. Configures the execution of the processor
and provides parameters for the GPU taks.

# Device Code Components

## Properties
Defines the names for device function substitution to avoid name conflicts between processors.
Also contains user-defined of parameter structs, which are passed to the processor functions during processing.

## GainProcessor.cuh
The device side implementation of the processor. Defines the GPU processor and its tasks, i.e., the processing functions.

## GainProcessor.cuh
Declares the GPU tasks and the GPU processor using pre-defined macros.
