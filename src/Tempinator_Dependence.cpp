#include "plugin.hpp"


// Stores sequence step information
struct Step {
    bool on = false; 
    int dep = 17;
    float depbeats = 1.f;
    float newbeats = 1.f;
    float hold = 4.f;
    float rate = 1.f;
    float cycleTime;

    float getBaseCycleTime() {
        return depbeats / newbeats / rate;
    }
};


struct Tempinator_Dependence : Module {

	// Stores the sequence of steps
	Step steps[8];

	int seqStep = 0;
	int viewStep = 0;
	int hold = 0;

	float depClock = 0.f;
	float masterClock = 0.f;
	float masterCycleTime = 0.f;

	bool onOffButton = false;
	bool moveButton = false;
	bool depButton = false;
	bool resetButton = false;

	enum ParamId {
		MASTER_PARAM,
		MOVELEFT_PARAM,
		ACTIVE_PARAM,
		MOVERIGHT_PARAM,
		DEPBEATS_PARAM,
		NEWBEATS_PARAM,
		DEP_PARAM,
		HOLD_PARAM,
		RATE_PARAM,
		RESET_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		BPM_OUTPUT,
		CLOCK_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		SEQ1_LIGHT, //0
		SEQ2_LIGHT,
		SEQ3_LIGHT,
		SEQ4_LIGHT,
		SEQ5_LIGHT,
		SEQ6_LIGHT,
		SEQ7_LIGHT,
		SEQ8_LIGHT, //7
		POS1_LIGHT, //8
		POS2_LIGHT,
		POS3_LIGHT,
		POS4_LIGHT,
		POS5_LIGHT,
		POS6_LIGHT,
		POS7_LIGHT,
		POS8_LIGHT, //15
		ACTIVE_LIGHT,
		DEPMASTER_LIGHT, //17
		DEPPREV_LIGHT,
		DEP2PREV_LIGHT, //19
		LIGHTS_LEN
	};

	Tempinator_Dependence() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MASTER_PARAM, 20.f, 400.f, 60.f, "Tempo", "bpm");
		paramQuantities[MASTER_PARAM]->snapEnabled = true;
		configParam(MOVELEFT_PARAM, 0.f, 1.f, 0.f, "Step left");
		configParam(ACTIVE_PARAM, 0.f, 1.f, 0.f, "Turn step on or off");
		configParam(MOVERIGHT_PARAM, 0.f, 1.f, 0.f, "Step right");
		configParam(DEPBEATS_PARAM, 1.f, 100.f, 1.f, "Beats in old tempo");
		paramQuantities[DEPBEATS_PARAM]->snapEnabled = true;
		configParam(NEWBEATS_PARAM, 1.f, 100.f, 1.f, "Beats in new tempo");
		paramQuantities[NEWBEATS_PARAM]->snapEnabled = true;
		configSwitch(DEP_PARAM, 0.f, 1.f, 0.f, "Dependence - BPM, previous step, or previous previous step");
		configParam(HOLD_PARAM, 0.f, 900.f, 4.f, "Continue for", " beats");
		paramQuantities[HOLD_PARAM]->snapEnabled = true;
		configParam(RATE_PARAM, 0.01f, 10.f, 1.f, "Speed", "X");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset sequence");
		configInput(RESET_INPUT, "Reset sequence");
		configOutput(BPM_OUTPUT, "BPM");
		configOutput(CLOCK_OUTPUT, "Sequence tempo");
		lights[SEQ1_LIGHT].setBrightness(1.f);
		steps[0].on = true;
	}

	// Loads viewStep information onto panel
	void loadViewStep(Step s) {
		params[DEPBEATS_PARAM].setValue(s.depbeats);
		params[NEWBEATS_PARAM].setValue(s.newbeats);
		params[HOLD_PARAM].setValue(s.hold);
		params[RATE_PARAM].setValue(s.rate);
		lights[ACTIVE_LIGHT].setBrightness(s.on ? 1.f : 0.f);
		lights[LightId(viewStep + 8)].setBrightness(1.f);
		int dep = s.dep;
		lights[DEPMASTER_LIGHT].setBrightness(dep == 17 ? 1.f : 0.f);
		lights[DEPPREV_LIGHT].setBrightness(dep == 18 ? 1.f : 0.f);
		lights[DEP2PREV_LIGHT].setBrightness(dep == 19 ? 1.f : 0.f);
	}

	// Stores viewStep information in sequence array
	void storeViewStep() {
		Step s = steps[viewStep];
		s.depbeats = params[DEPBEATS_PARAM].getValue();
		s.newbeats = params[NEWBEATS_PARAM].getValue();
		s.hold = params[HOLD_PARAM].getValue();
		s.rate = params[RATE_PARAM].getValue();
		steps[viewStep] = s;
	}

	// Resets sequence 
	void reset() {
		if (resetButton) {
			if (params[RESET_PARAM].getValue() == 0 && inputs[RESET_INPUT].getVoltage() == 0) {
				resetButton = false;
			}
		}
		else if (params[RESET_PARAM].getValue() == 1 || inputs[RESET_INPUT].getVoltage() > 0) {
			resetButton = true;
			lights[LightId(seqStep)].setBrightness(0.2f);
			seqStep = masterClock = depClock = hold = 0;
			lights[SEQ1_LIGHT].setBrightness(1.f);
		}
	}

	// Manages dependency menu
	void dependency() {
		if (depButton) {
			if (params[DEP_PARAM].getValue() == 0) {
				depButton = false;
			}
		}
		else if (params[DEP_PARAM].getValue() == 1) {
			depButton = true;
			int dep = steps[viewStep].dep;
			if (dep == 17 && viewStep > 0) {
				dep = 18;
			}
			else if (dep == 18 && viewStep > 1) {
				dep = 19;
			}
			else {
				dep = 17;
			}
			steps[viewStep].dep = dep;
			lights[DEPMASTER_LIGHT].setBrightness(dep == 17 ? 1.f : 0.f);
			lights[DEPPREV_LIGHT].setBrightness(dep == 18 ? 1.f : 0.f);
			lights[DEP2PREV_LIGHT].setBrightness(dep == 19 ? 1.f : 0.f);
		}
	}

	// Manages on/off button
	void onOff() {
		if (params[ACTIVE_PARAM].getValue() == 1 && viewStep != 0) {
			if (!onOffButton) {
				onOffButton = true;
				bool on = steps[viewStep].on;
				on = !on;
				steps[viewStep].on = on;
				lights[LightId(viewStep)].setBrightness(on ? 0.2f : 0.f);
				lights[ACTIVE_LIGHT].setBrightness(on ? 1.f : 0.f);
			}
		}
		else {
			onOffButton = false;
		}
	}

	// Moves viewStep left or right and loads corresponding information
	void moveViewStep() {
		if (moveButton) {
			if (params[MOVERIGHT_PARAM].getValue() == 0 && params[MOVELEFT_PARAM].getValue() == 0) {
				moveButton = false;
			}
		}
		else if (params[MOVELEFT_PARAM].getValue() == 1) {
			moveButton = true;
			lights[LightId(viewStep + 8)].setBrightness(0.f);
			(viewStep == 0) ? viewStep = 7 : viewStep--;
		}
		else if (params[MOVERIGHT_PARAM].getValue() == 1) {
			moveButton = true;
			lights[LightId(viewStep + 8)].setBrightness(0.f);
			viewStep = (viewStep + 1) % 8;
		}
		loadViewStep(steps[viewStep]);
	}

	// Main process of module
	void process(const ProcessArgs& args) override {

		// Get current step in sequence
		Step currStep = steps[seqStep];

		// Track passed time
		depClock += args.sampleTime;
		masterClock += args.sampleTime;

		// Output master tempo
		masterCycleTime = 1 / (params[MASTER_PARAM].getValue() / 60.f);
		if (masterClock >= masterCycleTime) {
			masterClock -= masterCycleTime;
		}
		outputs[BPM_OUTPUT].setVoltage(masterClock < (masterCycleTime/2.0) ? 10.f : 0.f);

		// Calculate step tempo
		float stepCycleTime = currStep.getBaseCycleTime();
		if (currStep.dep == 17) {
			stepCycleTime *= masterCycleTime;
		}
		else if (currStep.dep == 18) {
			stepCycleTime *= steps[seqStep - 1].cycleTime;
		} 
		else {
			stepCycleTime *= steps[seqStep - 2].cycleTime;
		}

		// Output step tempo
		steps[seqStep].cycleTime = stepCycleTime;
		if (depClock >= stepCycleTime) {
			currStep.hold > 0 ? hold++ : hold = 0;
			depClock -= stepCycleTime;
		}
		outputs[CLOCK_OUTPUT].setVoltage(depClock < (stepCycleTime/2.0) ? 10.f : 0.f);

		// Move to next seqStep if necessary
		if (hold >= currStep.hold && hold != 0) {
			hold = 0;
			lights[LightId(seqStep)].setBrightness(currStep.on ? 0.2f : 0.f);
			do {
				seqStep = (seqStep + 1) % 8;
			} while (!steps[seqStep].on);
			lights[LightId(seqStep)].setBrightness(1.f);
		}

		// Manage buttons and viewStep values
		reset();
		dependency();
		onOff();
		storeViewStep();
		moveViewStep();
	}

	json_t* dataToJson() override {

		json_t* rootJ = json_object();
		json_t* stepsJ = json_array();

		for (size_t i = 0; i < 8; i++) {
			Step s = steps[i];
			json_t* objectJ = json_object();
			json_object_set_new(objectJ, "on", json_boolean(s.on));
			json_object_set_new(objectJ, "dep", json_integer(s.dep));
			json_object_set_new(objectJ, "depbeats", json_real(s.depbeats));
			json_object_set_new(objectJ, "newbeats", json_real(s.newbeats));
			json_object_set_new(objectJ, "hold", json_real(s.hold));
			json_object_set_new(objectJ, "rate", json_real(s.rate));
			json_object_set_new(objectJ, "cycleTime", json_real(s.cycleTime));
			json_array_insert_new(stepsJ, i, objectJ);
		}
		json_object_set_new(rootJ, "steps", stepsJ);
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

		json_t* stepsJ = json_object_get(rootJ, "steps");
		if (stepsJ) {
			for (int i = 0; i < 8; i++) {
				json_t* stepJ = json_array_get(stepsJ, i);
				if (stepJ) {
					json_t* onJ = json_object_get(stepJ, "on");
					if (onJ) {
						bool on = json_boolean_value(onJ);
						if (on && i > 0) {
							lights[LightId(i)].setBrightness(0.2f);
						}
						else if (on) {
							lights[SEQ1_LIGHT].setBrightness(1.f);
						}
						steps[i].on = on;
					}
					json_t* depJ = json_object_get(stepJ, "dep");
					if (depJ) {
						steps[i].dep = json_integer_value(depJ);
					}
					json_t* depbeatsJ = json_object_get(stepJ, "depbeats");
					if (depbeatsJ) {
						steps[i].depbeats = json_number_value(depbeatsJ);
					}
					json_t* newbeatsJ = json_object_get(stepJ, "newbeats");
					if (newbeatsJ) {
						steps[i].newbeats = json_number_value(newbeatsJ);
					}
					json_t* holdJ = json_object_get(stepJ, "hold");
					if (holdJ) {
						steps[i].hold = json_number_value(holdJ);
					}
					json_t* rateJ = json_object_get(stepJ, "rate");
					if (rateJ) {
						steps[i].rate = json_number_value(rateJ);
					}
					json_t* cycleTimeJ = json_object_get(stepJ, "cycleTime");
					if (cycleTimeJ) {
						steps[i].cycleTime = json_number_value(cycleTimeJ);
					}
				}
			}	
		}
	}
};


struct Tempinator_DependenceWidget : ModuleWidget {
	Tempinator_DependenceWidget(Tempinator_Dependence* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tempinator_Dependence.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 17.554)), module, Tempinator_Dependence::MASTER_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(8.182, 43.833)), module, Tempinator_Dependence::MOVELEFT_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(15.24, 43.833)), module, Tempinator_Dependence::ACTIVE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(22.298, 43.833)), module, Tempinator_Dependence::MOVERIGHT_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 58.344)), module, Tempinator_Dependence::DEPBEATS_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(23.902, 58.344)), module, Tempinator_Dependence::NEWBEATS_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(23.902, 76.202)), module, Tempinator_Dependence::DEP_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 88.763)), module, Tempinator_Dependence::HOLD_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(23.911, 88.763)), module, Tempinator_Dependence::RATE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(14.142, 108.255)), module, Tempinator_Dependence::RESET_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.608, 108.255)), module, Tempinator_Dependence::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.902, 17.554)), module, Tempinator_Dependence::BPM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.911, 108.255)), module, Tempinator_Dependence::CLOCK_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(5.359, 32.865)), module, Tempinator_Dependence::SEQ1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(8.182, 32.865)), module, Tempinator_Dependence::SEQ2_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(11.005, 32.865)), module, Tempinator_Dependence::SEQ3_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.828, 32.865)), module, Tempinator_Dependence::SEQ4_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(16.652, 32.865)), module, Tempinator_Dependence::SEQ5_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(19.475, 32.865)), module, Tempinator_Dependence::SEQ6_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(22.298, 32.865)), module, Tempinator_Dependence::SEQ7_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(25.121, 32.865)), module, Tempinator_Dependence::SEQ8_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(5.359, 35.733)), module, Tempinator_Dependence::POS1_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(8.182, 35.733)), module, Tempinator_Dependence::POS2_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(11.005, 35.733)), module, Tempinator_Dependence::POS3_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(13.828, 35.733)), module, Tempinator_Dependence::POS4_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(16.652, 35.733)), module, Tempinator_Dependence::POS5_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(19.475, 35.733)), module, Tempinator_Dependence::POS6_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(22.298, 35.733)), module, Tempinator_Dependence::POS7_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(25.121, 35.733)), module, Tempinator_Dependence::POS8_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(15.24, 39.142)), module, Tempinator_Dependence::ACTIVE_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.476, 68.501)), module, Tempinator_Dependence::DEPMASTER_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(6.608, 68.501)), module, Tempinator_Dependence::DEPPREV_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(9.74, 68.551)), module, Tempinator_Dependence::DEP2PREV_LIGHT));
	}
};


Model* modelTempinator_Dependence = createModel<Tempinator_Dependence, Tempinator_DependenceWidget>("Tempinator_Dependence");