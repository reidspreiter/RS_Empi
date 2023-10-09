#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelWaitGate;
extern Model* modelWaitGate_Zero;
extern Model* modelTempinator_Dependence;
extern Model* modelWaitGate_Reset;
extern Model* modelFlipFlop;