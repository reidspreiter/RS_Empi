#include "plugin.hpp"


struct Step {
	bool on = false;
	float bpm = 30.f;
	float hold = 4.f;
};


struct Tempinator : Module {

	Step steps[12];

	int seqStep = 0;
	int viewStep = 0;
	int hold = 0;

	float clock = 0.f;

	bool onOffButton = false;
	bool moveButton = false;
	bool resetButton = false;

	enum ParamId {
		RESET_PARAM,
		ONOFF_PARAM,
		UP_PARAM,
		DOWN_PARAM,
		BPM_PARAM,
		BEATS_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(SEQ_LIGHTS, 12),
		ENUMS(POS_LIGHTS, 12),
		ACTIVE_LIGHT,
		LIGHTS_LEN
	};

	Tempinator() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(ONOFF_PARAM, 0.f, 1.f, 0.f, "On/Off");
		configParam(UP_PARAM, 0.f, 1.f, 0.f, "Step up");
		configParam(DOWN_PARAM, 0.f, 1.f, 0.f, "Step down");
		configParam(BPM_PARAM, 1.f, 999.f, 30.f, "Set step tempo", " bpm");
		paramQuantities[BPM_PARAM]->snapEnabled = true;
		configParam(BEATS_PARAM, 0.f, 999.f, 4.f, "Set step duration", " beats");
		paramQuantities[BEATS_PARAM]->snapEnabled = true;
		configInput(RESET_INPUT, "Reset");
		configOutput(OUT_OUTPUT, "Clock");
		lights[SEQ_LIGHTS + 0].setBrightness(1.f);
		steps[0].on = true;
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
			lights[SEQ_LIGHTS + seqStep].setBrightness(0.2f);
			seqStep = clock = hold = 0;
			lights[SEQ_LIGHTS + 0].setBrightness(1.f);
		}
	}

	// Manages I/O button
	void onOff() {
		if (params[ONOFF_PARAM].getValue() == 1 && viewStep != 0) {
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

	// Stores viewStep information in sequence array
	void storeViewStep() {
		Step s = steps[viewStep];
		s.bpm = params[BPM_PARAM].getValue();
		s.hold = params[BEATS_PARAM].getValue();
		steps[viewStep] = s;
	}

	// Moves viewStep up or down and loads corresponding information
	void moveViewStep() {
		if (moveButton) {
			if (params[DOWN_PARAM].getValue() == 0 && params[UP_PARAM].getValue() == 0) {
				moveButton = false;
			}
		}
		else if (params[UP_PARAM].getValue() == 1) {
			moveButton = true;
			lights[POS_LIGHTS + viewStep].setBrightness(0.f);
			(viewStep == 0) ? viewStep = 11 : viewStep--;
		}
		else if (params[DOWN_PARAM].getValue() == 1) {
			moveButton = true;
			lights[POS_LIGHTS + viewStep].setBrightness(0.f);
			viewStep = (viewStep + 1) % 12;
		}
		loadViewStep(steps[viewStep]);
	}

	// Loads viewStep information onto panel
	void loadViewStep(Step s) {
		params[BPM_PARAM].setValue(s.bpm);
		params[BEATS_PARAM].setValue(s.hold);
		lights[ACTIVE_LIGHT].setBrightness(s.on ? 1.f : 0.f);
		lights[POS_LIGHTS + viewStep].setBrightness(1.f);
	}

	// Main process of module
	void process(const ProcessArgs& args) override {

		// Get current step in sequence
		Step currStep = steps[seqStep];

		// Output step tempo
		clock += args.sampleTime;
		float stepCycleTime = 1 / (currStep.bpm / 60.f);
		if (clock >= stepCycleTime) {
			currStep.hold > 0 ? hold++ : hold = 0;
			clock -= stepCycleTime;
		}
		outputs[OUT_OUTPUT].setVoltage(clock < (stepCycleTime/2.0) ? 10.f : 0.f);

		// Move to next seqStep if necessary
		if (hold >= currStep.hold && hold != 0 ) {
			hold = 0;
			lights[SEQ_LIGHTS + seqStep].setBrightness(currStep.on ? 0.2f : 0.f);
			do {
				seqStep = (seqStep + 1) % 12;
			} while (!steps[seqStep].on);
			lights[SEQ_LIGHTS + seqStep].setBrightness(1.f);
		}

		// Manage buttons and viewStep values
		reset();
		onOff();
		storeViewStep();
		moveViewStep();
	}

	json_t* dataToJson() override {
		json_t* rootJ = json_object();
		json_t* stepsJ = json_array();

		for (size_t i = 0; i < 12; i++) {
			Step s = steps[i];
			json_t* objectJ = json_object();
			json_object_set_new(objectJ, "on", json_boolean(s.on));
			json_object_set_new(objectJ, "bpm", json_real(s.bpm));
			json_object_set_new(objectJ, "hold", json_real(s.hold));
			json_array_insert_new(stepsJ, i, objectJ);
		}
		json_object_set_new(rootJ, "steps", stepsJ);
		return rootJ;
	}

	void dataFromJson(json_t* rootJ) override {

		json_t* stepsJ = json_object_get(rootJ, "steps");
		if (stepsJ) {
			for (int i = 0; i < 12; i++) {
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
					json_t* bpmJ = json_object_get(stepJ, "bpm");
					if (bpmJ) {
						steps[i].bpm = json_number_value(bpmJ);
					}
					json_t* holdJ = json_object_get(stepJ, "hold");
					if (holdJ) {
						steps[i].hold = json_number_value(holdJ);
					}
				}
			}
		}
	}
};


struct TempinatorWidget : ModuleWidget {
	TempinatorWidget(Tempinator* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Tempinator.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.622, 35.188)), module, Tempinator::RESET_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(7.622, 50.045)), module, Tempinator::ONOFF_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(4.327, 64.155)), module, Tempinator::UP_PARAM));
		addParam(createParamCentered<VCVButton>(mm2px(Vec(10.917, 64.155)), module, Tempinator::DOWN_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.626, 77.29)), module, Tempinator::BPM_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 91.408)), module, Tempinator::BEATS_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 21.142)), module, Tempinator::RESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 106.089)), module, Tempinator::OUT_OUTPUT));

		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 26.274)), module, Tempinator::SEQ_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 26.274)), module, Tempinator::POS_LIGHTS + 0));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 29.245)), module, Tempinator::SEQ_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 29.245)), module, Tempinator::POS_LIGHTS + 1));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 32.216)), module, Tempinator::SEQ_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 32.216)), module, Tempinator::POS_LIGHTS + 2));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 35.188)), module, Tempinator::SEQ_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 35.188)), module, Tempinator::POS_LIGHTS + 3));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 38.159)), module, Tempinator::SEQ_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 38.159)), module, Tempinator::POS_LIGHTS + 4));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 41.131)), module, Tempinator::SEQ_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 41.131)), module, Tempinator::POS_LIGHTS + 5));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 44.102)), module, Tempinator::SEQ_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 44.102)), module, Tempinator::POS_LIGHTS + 6));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 47.074)), module, Tempinator::SEQ_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 47.074)), module, Tempinator::POS_LIGHTS + 7));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 50.045)), module, Tempinator::SEQ_LIGHTS + 8));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 50.045)), module, Tempinator::POS_LIGHTS + 8));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 53.016)), module, Tempinator::SEQ_LIGHTS + 9));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 53.016)), module, Tempinator::POS_LIGHTS + 9));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 55.988)), module, Tempinator::SEQ_LIGHTS + 10));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 55.988)), module, Tempinator::POS_LIGHTS + 10));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(2.74, 58.959)), module, Tempinator::SEQ_LIGHTS + 11));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(7.622, 58.959)), module, Tempinator::ACTIVE_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(12.505, 58.959)), module, Tempinator::POS_LIGHTS + 11));
	}
};


Model* modelTempinator = createModel<Tempinator, TempinatorWidget>("Tempinator");