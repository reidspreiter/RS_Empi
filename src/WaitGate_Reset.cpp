#include "plugin.hpp"


struct WaitGate_Reset : Module {

	bool gateOpen = false;
	int pulseCounter = 0;
	bool pulse, masterReset, closedReset, openReset = false;

	enum ParamId {
		CLOSEDWAIT_PARAM,
		OPENWAIT_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		CLOSEDRESET_INPUT,
		OPENRESET_INPUT,
		TOTALRESET_INPUT,
		PULSE_INPUT,
		IN_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CLOSED_LIGHT,
		OPEN_LIGHT,
		LIGHTS_LEN
	};

	WaitGate_Reset() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CLOSEDWAIT_PARAM, 1.f, 100.f, 4.f, "Closed for", " pulses");
		configParam(OPENWAIT_PARAM, 0.f, 100.f, 4.f, "Open for", " pulses");
		paramQuantities[CLOSEDWAIT_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT_PARAM]->snapEnabled = true;
		configInput(PULSE_INPUT, "Pulse");
		configInput(TOTALRESET_INPUT, "Reset Module");
		configInput(IN_INPUT, "Signal");
		configInput(CLOSEDRESET_INPUT, "Closed Reset");
		configInput(OPENRESET_INPUT, "Open Reset");
		configOutput(OUT_OUTPUT, "Signal");
		lights[CLOSED_LIGHT].setBrightness(1.f);
	}

	void process(const ProcessArgs& args) override {

		// Check resets
		if (!masterReset && inputs[TOTALRESET_INPUT].getVoltage() > 0) {
			lights[CLOSED_LIGHT].setBrightness(1.f);
			lights[OPEN_LIGHT].setBrightness(0.f);
			masterReset = true;
			pulseCounter = 0;
			gateOpen = false;
		} 
		else if (masterReset && inputs[TOTALRESET_INPUT].getVoltage() == 0) {
			masterReset = false;
		}
		else if (!closedReset && inputs[CLOSEDRESET_INPUT].getVoltage() > 0 && !gateOpen) {
			closedReset = true;
			pulseCounter = 0;
		} 
		else if (closedReset && inputs[CLOSEDRESET_INPUT].getVoltage() == 0) {
			closedReset = false;
		}
		else if (!openReset && inputs[OPENRESET_INPUT].getVoltage() > 0 && gateOpen) {
			openReset = true;
			pulseCounter = 0;
		} 
		else if (openReset && inputs[OPENRESET_INPUT].getVoltage() == 0) {
			openReset = false;
		}
		
		// Count pulses
		if (!pulse && inputs[PULSE_INPUT].getVoltage() == 10) {
			pulseCounter++;
			pulse = true;
		}
		else if (pulse && inputs[PULSE_INPUT].getVoltage() == 0) {
			pulse = false;
		}

		// Open or close gate depending on pulse count
		if (gateOpen) {
			outputs[OUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());

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
	}
};


struct WaitGate_ResetWidget : ModuleWidget {
	WaitGate_ResetWidget(WaitGate_Reset* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/WaitGate_Reset.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.626, 76.76)), module, WaitGate_Reset::CLOSEDWAIT_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 92.995)), module, WaitGate_Reset::OPENWAIT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 16.585)), module, WaitGate_Reset::CLOSEDRESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.691, 25.3)), module, WaitGate_Reset::OPENRESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.691, 34.015)), module, WaitGate_Reset::TOTALRESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 46.543)), module, WaitGate_Reset::PULSE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 63.504)), module, WaitGate_Reset::IN_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 106.089)), module, WaitGate_Reset::OUT_OUTPUT));

		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(7.62, 56.866)), module, WaitGate_Reset::CLOSED_LIGHT));
		addChild(createLightCentered<TinyLight<GreenLight>>(mm2px(Vec(7.62, 86.521)), module, WaitGate_Reset::OPEN_LIGHT));
	}
};


Model* modelWaitGate_Reset = createModel<WaitGate_Reset, WaitGate_ResetWidget>("WaitGate_Reset");