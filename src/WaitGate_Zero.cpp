#include "plugin.hpp"


struct WaitGate_Zero : Module {

	bool gateOpen, countedPulse, reset = false;
	int pulseCounter = 0;

	enum ParamId {
		CLOSEDWAIT_PARAM,
		OPENWAIT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		RESET_INPUT,
		SIGNAL_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SIGNAL_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ZERO_LIGHT,
		CLOSED_LIGHT,
		OPEN_LIGHT,
		LIGHTS_LEN
	};

	WaitGate_Zero() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CLOSEDWAIT_PARAM, 1.f, 100.f, 4.f, "Closed for", " pulses");
		configParam(OPENWAIT_PARAM, 0.f, 100.f, 4.f, "Open for", " pulses");
		paramQuantities[CLOSEDWAIT_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT_PARAM]->snapEnabled = true;
		configInput(SIGNAL_INPUT, "Signal");
		configOutput(SIGNAL_OUTPUT, "Signal");
		lights[CLOSED_LIGHT].setBrightness(1.f);
	}

	void process(const ProcessArgs& args) override {

		// Count number of pulses
		if (!countedPulse && inputs[SIGNAL_INPUT].getVoltage() == 0) {
			lights[ZERO_LIGHT].setBrightness(1.f);
			pulseCounter++;
			countedPulse = true;
		}
		else if (countedPulse && inputs[SIGNAL_INPUT].getVoltage() > 0) {
			lights[ZERO_LIGHT].setBrightness(0.f);
			countedPulse = false;
		}

		// Open or close gate depending on pulse count
		if (gateOpen) {
			outputs[SIGNAL_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage());

			if (params[OPENWAIT_PARAM].getValue() == 0) {
				pulseCounter = 0;
			}
			else if (pulseCounter >= params[OPENWAIT_PARAM].getValue()) {
				pulseCounter = 0;
				lights[CLOSED_LIGHT].setBrightness(1.f);
				lights[OPEN_LIGHT].setBrightness(0.f);
				gateOpen = false;
			}
		}
		else if (pulseCounter >= params[CLOSEDWAIT_PARAM].getValue()) {
			pulseCounter = 0;
			lights[CLOSED_LIGHT].setBrightness(0.f);
			lights[OPEN_LIGHT].setBrightness(1.f);
			gateOpen = true;
		}

		// Reset
		if (!reset && inputs[RESET_INPUT].getVoltage() > 0) {
			pulseCounter = 0;
			gateOpen = false;
			outputs[SIGNAL_OUTPUT].setVoltage(0.f);
			lights[CLOSED_LIGHT].setBrightness(1.f);
			lights[OPEN_LIGHT].setBrightness(0.f);
			reset = true;
		}
		else if (reset && inputs[RESET_INPUT].getVoltage() == 0) {
			reset = false;
		}
	}
};


struct WaitGate_ZeroWidget : ModuleWidget {
	WaitGate_ZeroWidget(WaitGate_Zero* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/WaitGate_Zero.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.555, 68.294)), module, WaitGate_Zero::CLOSEDWAIT_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.549, 91.937)), module, WaitGate_Zero::OPENWAIT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 30.312)), module, WaitGate_Zero::RESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.555, 54.508)), module, WaitGate_Zero::SIGNAL_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 106.089)), module, WaitGate_Zero::SIGNAL_OUTPUT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(7.626, 16.846)), module, WaitGate_Zero::ZERO_LIGHT));
		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(7.549, 42.683)), module, WaitGate_Zero::CLOSED_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(7.549, 80.804)), module, WaitGate_Zero::OPEN_LIGHT));
	}
};


Model* modelWaitGate_Zero = createModel<WaitGate_Zero, WaitGate_ZeroWidget>("WaitGate_Zero");