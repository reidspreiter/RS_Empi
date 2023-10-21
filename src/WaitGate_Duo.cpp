#include "plugin.hpp"


struct WaitGate_Duo : Module {

	bool gateOpen, countedPulse1, countedPulse2, reset = false;
	int pulseCounter1, pulseCounter2 = 0;

	enum ParamId {
		RESET_PARAM,
		CLOSEDWAIT1_PARAM,
		CLOSEDWAIT2_PARAM,
		OPENWAIT1_PARAM,
		OPENWAIT2_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		PULSE1_INPUT,
		PULSE2_INPUT,
		IN1_INPUT,
		IN2_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT1_OUTPUT,
		OUT2_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CLOSEDLOAD1_LIGHT,
		CLOSED_LIGHT,
		CLOSEDLOAD2_LIGHT,
		OPENLOAD1_LIGHT,
		OPEN_LIGHT,
		OPENLOAD2_LIGHT,
		LIGHTS_LEN
	};

	WaitGate_Duo() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(RESET_PARAM, 0.f, 1.f, 0.f, "Reset");
		configParam(CLOSEDWAIT1_PARAM, 1.f, 100.f, 4.f, "Closed for", " pulses");
		configParam(CLOSEDWAIT2_PARAM, 1.f, 100.f, 4.f, "Closed for", " pulses");
		configParam(OPENWAIT1_PARAM, 0.f, 100.f, 4.f, "Open for", " pulses");
		configParam(OPENWAIT2_PARAM, 0.f, 100.f, 4.f, "Open for", " pulses");
		paramQuantities[CLOSEDWAIT1_PARAM]->snapEnabled = true;
		paramQuantities[CLOSEDWAIT2_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT1_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT2_PARAM]->snapEnabled = true;
		configInput(RESET_INPUT, "Reset");
		configInput(PULSE1_INPUT, "Pulse 1");
		configInput(PULSE2_INPUT, "Pulse 2");
		configInput(IN1_INPUT, "Signal 1");
		configInput(IN2_INPUT, "Signal 2");
		configOutput(OUT1_OUTPUT, "Signal 1");
		configOutput(OUT2_OUTPUT, "Signal 2");
		lights[CLOSED_LIGHT].setBrightness(1.f);
	}

	void process(const ProcessArgs& args) override {

		// Count number of pulses
		if (!countedPulse1 && inputs[PULSE1_INPUT].getVoltage() > 0) {
			pulseCounter1++;
			countedPulse1 = true;
		}
		else if (countedPulse1 && inputs[PULSE1_INPUT].getVoltage() == 0) {
			countedPulse1 = false;
		}

		if (!countedPulse2 && inputs[PULSE2_INPUT].getVoltage() > 0) {
			pulseCounter2++;
			countedPulse2 = true;
		}
		else if (countedPulse2 && inputs[PULSE2_INPUT].getVoltage() == 0) {
			countedPulse2 = false;
		}

		// Open or close gate depending on pulse counts
		if (gateOpen) {
			outputs[OUT1_OUTPUT].setVoltage(inputs[IN1_INPUT].getVoltage());
			outputs[OUT2_OUTPUT].setVoltage(inputs[IN2_INPUT].getVoltage());

			if (params[OPENWAIT1_PARAM].getValue() == 0 && params[OPENWAIT1_PARAM].getValue() == 0) {
				pulseCounter1 = 0;
				pulseCounter2 = 0;
			}
			else if (pulseCounter1 >= params[OPENWAIT1_PARAM].getValue() && pulseCounter2 >= params[OPENWAIT2_PARAM].getValue()) {
				pulseCounter1 = 0;
				pulseCounter2 = 0;
				lights[CLOSED_LIGHT].setBrightness(1.f);
				lights[OPEN_LIGHT].setBrightness(0.f);
				lights[OPENLOAD1_LIGHT].setBrightness(0.f);
				lights[OPENLOAD2_LIGHT].setBrightness(0.f);
				gateOpen = false;
			}
			else if (pulseCounter1 >= params[OPENWAIT1_PARAM].getValue()) {
				lights[OPENLOAD1_LIGHT].setBrightness(1.f);
			}
			else if (pulseCounter2 >= params[OPENWAIT2_PARAM].getValue()) {
				lights[OPENLOAD2_LIGHT].setBrightness(1.f);
			}
		}
		else if (pulseCounter1 >= params[CLOSEDWAIT1_PARAM].getValue() && pulseCounter2 >= params[CLOSEDWAIT2_PARAM].getValue()) {
			pulseCounter1 = 0;
			pulseCounter2 = 0;
			lights[CLOSED_LIGHT].setBrightness(0.f);
			lights[OPEN_LIGHT].setBrightness(1.f);
			lights[CLOSEDLOAD1_LIGHT].setBrightness(0.f);
			lights[CLOSEDLOAD2_LIGHT].setBrightness(0.f);
			gateOpen = true;
		}
		else if (pulseCounter1 >= params[CLOSEDWAIT1_PARAM].getValue()) {
			lights[CLOSEDLOAD1_LIGHT].setBrightness(1.f);
		}
		else if (pulseCounter2 >= params[CLOSEDWAIT2_PARAM].getValue()) {
			lights[CLOSEDLOAD2_LIGHT].setBrightness(1.f);
		}

		// Reset
		if (!reset && (inputs[RESET_INPUT].getVoltage() > 0 || params[RESET_PARAM].getValue() > 0)) {
			pulseCounter1 = 0;
			pulseCounter2 = 0;
			gateOpen = false;
			outputs[OUT1_OUTPUT].setVoltage(0.f);
			outputs[OUT2_OUTPUT].setVoltage(0.f);
			lights[CLOSED_LIGHT].setBrightness(1.f);
			lights[CLOSEDLOAD1_LIGHT].setBrightness(0.f);
			lights[CLOSEDLOAD2_LIGHT].setBrightness(0.f);
			lights[OPEN_LIGHT].setBrightness(0.f);
			lights[OPENLOAD1_LIGHT].setBrightness(0.f);
			lights[OPENLOAD2_LIGHT].setBrightness(0.f);
			reset = true;
		}
		else if (reset && (inputs[RESET_INPUT].getVoltage() == 0 && params[RESET_PARAM].getValue() == 0)) {
			reset = false;
		}
	}
};


struct WaitGate_DuoWidget : ModuleWidget {
	WaitGate_DuoWidget(WaitGate_Duo* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/WaitGate_Duo.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<VCVButton>(mm2px(Vec(18.326, 16.91)), module, WaitGate_Duo::RESET_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.075, 68.294)), module, WaitGate_Duo::CLOSEDWAIT1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.326, 68.294)), module, WaitGate_Duo::CLOSEDWAIT2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.075, 91.938)), module, WaitGate_Duo::OPENWAIT1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.567, 91.938)), module, WaitGate_Duo::OPENWAIT2_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.075, 16.91)), module, WaitGate_Duo::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.075, 30.312)), module, WaitGate_Duo::PULSE1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.326, 30.312)), module, WaitGate_Duo::PULSE2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.075, 54.508)), module, WaitGate_Duo::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(18.326, 54.508)), module, WaitGate_Duo::IN2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.074, 106.089)), module, WaitGate_Duo::OUT1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.567, 106.089)), module, WaitGate_Duo::OUT2_OUTPUT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(7.075, 42.682)), module, WaitGate_Duo::CLOSEDLOAD1_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(12.7, 42.682)), module, WaitGate_Duo::CLOSED_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(18.326, 42.682)), module, WaitGate_Duo::CLOSEDLOAD2_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(7.075, 80.804)), module, WaitGate_Duo::OPENLOAD1_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(12.7, 80.804)), module, WaitGate_Duo::OPEN_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(18.326, 80.804)), module, WaitGate_Duo::OPENLOAD2_LIGHT));
	}
};


Model* modelWaitGate_Duo = createModel<WaitGate_Duo, WaitGate_DuoWidget>("WaitGate_Duo");