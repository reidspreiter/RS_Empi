#include "plugin.hpp"


// Stores sequence step information
struct Step {
    bool on = false; 
    int dep = 0;
    float depbeats = 1.f;
    float newbeats = 1.f;
    float hold = 4.f;
    float rate = 1.f;
	float cycleTime = 1.f;
	float length = 2.f;
	float acc1 = 0.f;
	float acc2 = 0.f;
	float acc3 = 0.f;
	float acc4 = 0.f;

    float getBaseCycleTime() {
        return depbeats / newbeats / rate;
    }
};


struct Tempinator_DepAcc : Module {

	Step steps[8];

	int seqStep = 0;
	int viewStep = 0;
	int hold = 0;
	int masterCount = 0;
	int stepCount = 0;

	float depClock = 0.f;
	float masterClock = 0.f;
	float masterCycleTime = 0.f;

	bool onOffButton = false;
	bool moveButton = false;
	bool depButton = false;
	bool resetButton = false;
	bool stepPulse = false;

	enum ParamId {
		MASTER_PARAM,
		MASTERLENGTH_PARAM,
		LENGTH_PARAM,
		MOVELEFT_PARAM,
		ACTIVE_PARAM,
		MOVERIGHT_PARAM,
		ACCENT1_PARAM,
		ACCENT2_PARAM,
		DEPBEATS_PARAM,
		NEWBEATS_PARAM,
		ACCENT3_PARAM,
		DEP_PARAM,
		ACCENT4_PARAM,
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
		MASTERDOWNBEAT_OUTPUT,
		DOWNBEAT_OUTPUT,
		CLOCK_OUTPUT,
		ACCENTS_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(SEQ_LIGHTS, 8),
		ENUMS(POS_LIGHTS, 8),
		ENUMS(DEP_LIGHTS, 3),
		ACTIVE_LIGHT,
		DEPMASTER_LIGHT,
		DEPPREV_LIGHT,
		DEP2PREV_LIGHT,
		LIGHTS_LEN
	};

	Tempinator_DepAcc() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(MASTER_PARAM, 20.f, 400.f, 60.f, "Tempo", "bpm");
		paramQuantities[MASTER_PARAM]->snapEnabled = true;
		configParam(MASTERLENGTH_PARAM, 2.f, 100.f, 4.f, "Beats per measure");
		paramQuantities[MASTERLENGTH_PARAM]->snapEnabled = true;
		configParam(LENGTH_PARAM, 2.f, 100.f, 4.f, "Step beats per measure");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;
		configParam(MOVELEFT_PARAM, 0.f, 1.f, 0.f, "Step left");
		configParam(ACTIVE_PARAM, 0.f, 1.f, 0.f, "Turn step on or off");
		configParam(MOVERIGHT_PARAM, 0.f, 1.f, 0.f, "Step right");
		configParam(ACCENT1_PARAM, 0.f, 99.f, 0.f, "Accent", " beats after downbeat");
		paramQuantities[ACCENT1_PARAM]->snapEnabled = true;
		configParam(ACCENT2_PARAM, 0.f, 99.f, 0.f, "Accent", " beats after downbeat");
		paramQuantities[ACCENT2_PARAM]->snapEnabled = true;
		configParam(DEPBEATS_PARAM, 1.f, 100.f, 1.f, "Beats in ond tempo");
		paramQuantities[DEPBEATS_PARAM]->snapEnabled = true;
		configParam(NEWBEATS_PARAM, 1.f, 100.f, 1.f, "Beats in new tempo");
		paramQuantities[NEWBEATS_PARAM]->snapEnabled = true;
		configParam(ACCENT3_PARAM, 0.f, 99.f, 0.f, "Accent", " beats after downbeat");
		paramQuantities[ACCENT3_PARAM]->snapEnabled = true;
		configParam(DEP_PARAM, 0.f, 1.f, 0.f, "Dependence - BPM, previous step, or previous previous step");
		configParam(ACCENT4_PARAM, 0.f, 99.f, 0.f, "Accent", " beats after downbeat");
		paramQuantities[ACCENT4_PARAM]->snapEnabled = true;
		configParam(HOLD_PARAM, 0.f, 900.f, 4.f, "Continue for", " beats");
		paramQuantities[HOLD_PARAM]->snapEnabled = true;
		configParam(RATE_PARAM, 0.01f, 10.f, 1.f, "Speed", "X");
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset sequence");
		configInput(RESET_INPUT, "Reset sequence");
		configOutput(BPM_OUTPUT, "BPM");
		configOutput(MASTERDOWNBEAT_OUTPUT, "Downbeat");
		configOutput(DOWNBEAT_OUTPUT, "Step Downbeat");
		configOutput(CLOCK_OUTPUT, "Sequence tempo");
		configOutput(ACCENTS_OUTPUT, "Accents");
		lights[SEQ_LIGHTS + 0].setBrightness(1.f);
		steps[0].on = true;
	}

	// Loads viewStep information onto panel
	void loadViewStep(Step s) {
		params[DEPBEATS_PARAM].setValue(s.depbeats);
		params[NEWBEATS_PARAM].setValue(s.newbeats);
		params[HOLD_PARAM].setValue(s.hold);
		params[RATE_PARAM].setValue(s.rate);
		params[LENGTH_PARAM].setValue(s.length);
		params[ACCENT1_PARAM].setValue(s.acc1);
		params[ACCENT2_PARAM].setValue(s.acc2);
		params[ACCENT3_PARAM].setValue(s.acc3);
		params[ACCENT4_PARAM].setValue(s.acc4);
		lights[ACTIVE_LIGHT].setBrightness(s.on ? 1.f : 0.f);
		lights[POS_LIGHTS + viewStep].setBrightness(1.f);
		lights[DEP_LIGHTS + s.dep].setBrightness(1.f);
	}

	// Stores viewStep information in sequence array
	void storeViewStep() {
		Step s = steps[viewStep];
		s.depbeats = params[DEPBEATS_PARAM].getValue();
		s.newbeats = params[NEWBEATS_PARAM].getValue();
		s.hold = params[HOLD_PARAM].getValue();
		s.rate = params[RATE_PARAM].getValue();
		s.length = params[LENGTH_PARAM].getValue();
		s.acc1 = params[ACCENT1_PARAM].getValue();
		s.acc2 = params[ACCENT2_PARAM].getValue();
		s.acc3 = params[ACCENT3_PARAM].getValue();
		s.acc4 = params[ACCENT4_PARAM].getValue();
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
			stepPulse = false;
			seqStep = masterClock = depClock = hold = 0;
			masterCount = stepCount = 0;
			outputs[ACCENTS_OUTPUT].setVoltage(0.f);
			lights[SEQ_LIGHTS + seqStep].setBrightness(0.2f);
			lights[SEQ_LIGHTS + 0].setBrightness(1.f);
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
			lights[DEP_LIGHTS + dep].setBrightness(0.f);
			if (dep == 0 && viewStep > 0) {
				dep = 1;
			}
			else if (dep == 1 && viewStep > 1) {
				dep = 2;
			}
			else {
				dep = 0;
			}
			lights[DEP_LIGHTS + dep].setBrightness(1.f);
			steps[viewStep].dep = dep;
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
				lights[SEQ_LIGHTS + viewStep].setBrightness(on ? 0.2f : 0.f);
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
			lights[POS_LIGHTS + viewStep].setBrightness(0.f);
			lights[DEP_LIGHTS + steps[viewStep].dep].setBrightness(0.f);
			(viewStep == 0) ? viewStep = 7 : viewStep--;
			loadViewStep(steps[viewStep]);
		}
		else if (params[MOVERIGHT_PARAM].getValue() == 1) {
			moveButton = true;
			lights[POS_LIGHTS + viewStep].setBrightness(0.f);
			lights[DEP_LIGHTS + steps[viewStep].dep].setBrightness(0.f);
			viewStep = (viewStep + 1) % 8;
			loadViewStep(steps[viewStep]);
		}
	}

	// Main process of module
	void process(const ProcessArgs& args) override {

		Step currStep = steps[seqStep];

		depClock += args.sampleTime;
		masterClock += args.sampleTime;

		// Output master tempo
		masterCycleTime = 1 / (params[MASTER_PARAM].getValue() / 60.f);
		if (masterClock >= masterCycleTime) {
			masterClock -= masterCycleTime;
			masterCount = (masterCount + 1 == params[MASTERLENGTH_PARAM].getValue() ? 0 : masterCount + 1);
		}
		outputs[BPM_OUTPUT].setVoltage(masterClock < (masterCycleTime/2.0) ? 10.f : 0.f);
		if (masterCount == 0) {
			outputs[MASTERDOWNBEAT_OUTPUT].setVoltage(masterClock < (masterCycleTime/2.0) ? 10.f : 0.f);
		}

		// Calculate step tempo
		float stepCycleTime = currStep.getBaseCycleTime();
		if (currStep.dep == 0) {
			stepCycleTime *= masterCycleTime;
		}
		else if (currStep.dep == 1) {
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
			stepCount = (stepCount + 1 == currStep.length ? 0 : stepCount + 1);
		}
		if (depClock < (stepCycleTime/2.0)) {
			stepPulse = true;
			outputs[CLOCK_OUTPUT].setVoltage(10.f);
		} 
		else {
			stepPulse = false;
			outputs[CLOCK_OUTPUT].setVoltage(0.f);
		}

		// Perform accents
		if (stepCount == 0) {
			outputs[DOWNBEAT_OUTPUT].setVoltage(stepPulse ? 10.f : 0.f);
		}
		else if (stepCount == currStep.acc1 || stepCount == currStep.acc2 || stepCount == currStep.acc3 || stepCount == currStep.acc4) {
				outputs[ACCENTS_OUTPUT].setVoltage(stepPulse ? 10.f : 0.f);
		}

		// Move to next seqStep if necessary
		if (hold >= currStep.hold && hold != 0) {
			hold = 0;
			lights[SEQ_LIGHTS + seqStep].setBrightness(currStep.on ? 0.2f : 0.f);
			do {
				seqStep = (seqStep + 1) % 8;
			} while (!steps[seqStep].on);
			lights[SEQ_LIGHTS + seqStep].setBrightness(1.f);
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
			json_object_set_new(objectJ, "length", json_real(s.length));
			json_object_set_new(objectJ, "acc1", json_real(s.acc1));
			json_object_set_new(objectJ, "acc2", json_real(s.acc2));
			json_object_set_new(objectJ, "acc3", json_real(s.acc3));
			json_object_set_new(objectJ, "acc4", json_real(s.acc4));
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
							lights[SEQ_LIGHTS + i].setBrightness(0.2f);
						}
						else if (on) {
							lights[SEQ_LIGHTS + 0].setBrightness(1.f);
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
					json_t* lengthJ = json_object_get(stepJ, "length");
					if (lengthJ) {
						steps[i].length = json_number_value(lengthJ);
					}
					json_t* acc1J = json_object_get(stepJ, "acc1");
					if (acc1J) {
						steps[i].acc1 = json_number_value(acc1J);
					}
					json_t* acc2J = json_object_get(stepJ, "acc2");
					if (acc2J) {
						steps[i].acc2 = json_number_value(acc2J);
					}
					json_t* acc3J = json_object_get(stepJ, "acc3");
					if (acc3J) {
						steps[i].acc3 = json_number_value(acc3J);
					}
					json_t* acc4J = json_object_get(stepJ, "acc4");
					if (acc4J) {
						steps[i].acc4 = json_number_value(acc4J);
					}
				}
			}	
		}
		loadViewStep(steps[0]);
	}
};


struct Tempinator_DepAccWidget : ModuleWidget {
	Tempinator_DepAccWidget(Tempinator_DepAcc* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tempinator_DepAcc.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 17.554)), module, Tempinator_DepAcc::MASTER_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(27.821, 17.554)), module, Tempinator_DepAcc::MASTERLENGTH_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.428, 33.365)), module, Tempinator_DepAcc::LENGTH_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(8.182, 43.833)), module, Tempinator_DepAcc::MOVELEFT_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(15.24, 43.833)), module, Tempinator_DepAcc::ACTIVE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(22.298, 43.833)), module, Tempinator_DepAcc::MOVERIGHT_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.366, 47.766)), module, Tempinator_DepAcc::ACCENT1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.422, 57.404)), module, Tempinator_DepAcc::ACCENT2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 58.344)), module, Tempinator_DepAcc::DEPBEATS_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(23.902, 58.344)), module, Tempinator_DepAcc::NEWBEATS_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.422, 67.043)), module, Tempinator_DepAcc::ACCENT3_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(23.902, 76.202)), module, Tempinator_DepAcc::DEP_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(38.422, 76.682)), module, Tempinator_DepAcc::ACCENT4_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.608, 87.175)), module, Tempinator_DepAcc::HOLD_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(23.911, 87.175)), module, Tempinator_DepAcc::RATE_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(14.142, 106.088)), module, Tempinator_DepAcc::RESET_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.608, 106.088)), module, Tempinator_DepAcc::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(17.214, 17.554)), module, Tempinator_DepAcc::BPM_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.428, 17.554)), module, Tempinator_DepAcc::MASTERDOWNBEAT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.422, 91.409)), module, Tempinator_DepAcc::DOWNBEAT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(23.911, 106.089)), module, Tempinator_DepAcc::CLOCK_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(38.422, 106.078)), module, Tempinator_DepAcc::ACCENTS_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(5.359, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(8.182, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(11.005, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(13.828, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(16.652, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(19.475, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(22.298, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(25.121, 32.865)), module, Tempinator_DepAcc::SEQ_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(5.359, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(8.182, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(11.005, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(13.828, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(16.652, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(19.475, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(22.298, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(25.121, 35.733)), module, Tempinator_DepAcc::POS_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(15.24, 39.142)), module, Tempinator_DepAcc::ACTIVE_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(3.476, 68.501)), module, Tempinator_DepAcc::DEP_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(6.608, 68.501)), module, Tempinator_DepAcc::DEP_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(9.74, 68.551)), module, Tempinator_DepAcc::DEP_LIGHTS + 2));	
	}
};


Model* modelTempinator_DepAcc = createModel<Tempinator_DepAcc, Tempinator_DepAccWidget>("Tempinator_DepAcc");