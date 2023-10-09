#include "plugin.hpp"


struct WaitGate : Module {

	bool gateOpen = false;
	int pulseCounter = 0;
	bool countedPulse = false;

	enum ParamId {
		CLOSEDWAIT_PARAM,
		OPENWAIT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PULSE_INPUT,
		SIGNAL_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		SIGNAL_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CLOSED_LIGHT,
		OPEN_LIGHT,
		LIGHTS_LEN
	};

	WaitGate() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CLOSEDWAIT_PARAM, 0.f, 100.f, 0.f, "Closed for", " pulses");
		configParam(OPENWAIT_PARAM, 0.f, 100.f, 0.f, "Open for", " pulses");
		paramQuantities[CLOSEDWAIT_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT_PARAM]->snapEnabled = true;
		configInput(PULSE_INPUT, "Pulse");
		configInput(SIGNAL_INPUT, "Signal");
		configOutput(SIGNAL_OUTPUT, "Signal");
	}

	void process(const ProcessArgs& args) override {

		// Count number of pulses
		if (!countedPulse && inputs[PULSE_INPUT].getVoltage() == 10) {
			pulseCounter++;
			countedPulse = true;
		}
		else if (countedPulse && inputs[PULSE_INPUT].getVoltage() == 0) {
			countedPulse = false;
		}

		// Open or close gate depending on pulse count
		if (gateOpen && params[OPENWAIT_PARAM].getValue() == 0) {
			;
		}
		else if (!gateOpen && pulseCounter >= params[CLOSEDWAIT_PARAM].getValue()) {
			pulseCounter = 0;
			gateOpen = true;
		}
		else if (gateOpen && pulseCounter >= params[OPENWAIT_PARAM].getValue()) {
			pulseCounter = 0;
			gateOpen = false;
		}

		// Allow input to pass through an open gate
		if (gateOpen) {
			outputs[SIGNAL_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage());
		}

		//adjust lighting
		lights[CLOSED_LIGHT].setBrightness(gateOpen ? 0.f : 1.f);
		lights[OPEN_LIGHT].setBrightness(gateOpen ? 1.f : 0.f);
	}
};


struct WaitGateWidget : ModuleWidget {
	WaitGateWidget(WaitGate* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/WaitGate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.626, 64.59)), module, WaitGate::CLOSEDWAIT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 93.525)), module, WaitGate::OPENWAIT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 24.847)), module, WaitGate::PULSE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 50.804)), module, WaitGate::SIGNAL_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 108.255)), module, WaitGate::SIGNAL_OUTPUT));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(7.62, 38.979)), module, WaitGate::CLOSED_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(7.62, 80.804)), module, WaitGate::OPEN_LIGHT));
	}
};


Model* modelWaitGate = createModel<WaitGate, WaitGateWidget>("WaitGate");