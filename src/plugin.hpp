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
extern Model* modelTempinator;
extern Model* modelFloppityFlippity;
extern Model* modelFlopFlip;
extern Model* modelFlippityFloppity;
extern Model* modelAccent;
extern Model* modelWaitGate_Duo;
extern Model* modelTempinator_DepAcc;
extern Model* modelAccent_Separate;