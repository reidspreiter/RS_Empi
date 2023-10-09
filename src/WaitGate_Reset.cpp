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
		PULSE_INPUT,
		MASTERRESET_INPUT,
		SIGNAL_INPUT,
		CLOSEDRESET_INPUT,
		OPENRESET_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTSIGNAL_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		CLOSED_LIGHT,
		CLOSEDLOAD_LIGHT,
		OPEN_LIGHT,
		OPENLOAD_LIGHT,
		LIGHTS_LEN
	};

	WaitGate_Reset() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(CLOSEDWAIT_PARAM, 0.f, 100.f, 0.f, "Closed for", " pulses");
		configParam(OPENWAIT_PARAM, 0.f, 100.f, 0.f, "Open for", " pulses");
		paramQuantities[CLOSEDWAIT_PARAM]->snapEnabled = true;
		paramQuantities[OPENWAIT_PARAM]->snapEnabled = true;
		configInput(PULSE_INPUT, "Pulse");
		configInput(MASTERRESET_INPUT, "Reset Module");
		configInput(SIGNAL_INPUT, "Signal");
		configInput(CLOSEDRESET_INPUT, "Reset Closed");
		configInput(OPENRESET_INPUT, "Reset Open");
		configOutput(OUTSIGNAL_OUTPUT, "Signal");
	}

	void process(const ProcessArgs& args) override {
		
		// Check master reset
		if (!masterReset && inputs[MASTERRESET_INPUT].getVoltage() > 0) {
			masterReset = true;
			pulseCounter = 0;
			gateOpen = false;
		} 
		else if (masterReset && inputs[MASTERRESET_INPUT].getVoltage() == 0) {
			masterReset = false;
		}
		// Check closed reset
		else if (!closedReset && inputs[CLOSEDRESET_INPUT].getVoltage() > 0 && !gateOpen) {
			closedReset = true;
			pulseCounter = 0;
		} 
		else if (closedReset && inputs[CLOSEDRESET_INPUT].getVoltage() == 0) {
			closedReset = false;
		}
		// Check open reset
		else if (!openReset && inputs[OPENRESET_INPUT].getVoltage() > 0 && gateOpen) {
			openReset = true;
			pulseCounter = 0;
		} 
		else if (openReset && inputs[OPENRESET_INPUT].getVoltage() == 0) {
			openReset = false;
		}
		
		// Count number of pulses
		if (!pulse && inputs[PULSE_INPUT].getVoltage() == 10) {
			pulseCounter++;
			pulse = true;
		}
		else if (pulse && inputs[PULSE_INPUT].getVoltage() == 0) {
			pulse = false;
		}

		float openWait = params[OPENWAIT_PARAM].getValue();
		float closedWait = params[CLOSEDWAIT_PARAM].getValue();

		if (gateOpen) {
			// Close gate if correct conditions
			if (openWait != 0 && pulseCounter >= openWait) {
				pulseCounter = 0;
				gateOpen = false;
			}

			// Allow input to pass through an open gate
			outputs[OUTSIGNAL_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage());

			lights[CLOSED_LIGHT].setBrightness(0.f);
			lights[CLOSEDLOAD_LIGHT].setBrightness(0.f);
			lights[OPEN_LIGHT].setBrightness(1.f);
			lights[OPENLOAD_LIGHT].setBrightness(pulseCounter / openWait);
		} 
		else {
			// Open gate if correct conditions
			if (pulseCounter >= closedWait) {
				pulseCounter = 0;
				gateOpen = true;
			}
			
			lights[CLOSED_LIGHT].setBrightness(1.f);
			lights[CLOSEDLOAD_LIGHT].setBrightness(pulseCounter / closedWait);
			lights[OPEN_LIGHT].setBrightness(0.f);
			lights[OPENLOAD_LIGHT].setBrightness(0.f);
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

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.626, 64.59)), module, WaitGate_Reset::CLOSEDWAIT_PARAM));
		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(7.62, 93.525)), module, WaitGate_Reset::OPENWAIT_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 25.049)), module, WaitGate_Reset::PULSE_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.359, 25.049)), module, WaitGate_Reset::MASTERRESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 50.804)), module, WaitGate_Reset::SIGNAL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.359, 50.804)), module, WaitGate_Reset::CLOSEDRESET_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.359, 93.525)), module, WaitGate_Reset::OPENRESET_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 108.255)), module, WaitGate_Reset::OUTSIGNAL_OUTPUT));

		addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(15.24, 38.979)), module, WaitGate_Reset::CLOSED_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(22.359, 64.59)), module, WaitGate_Reset::CLOSEDLOAD_LIGHT));
		addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(15.24, 80.804)), module, WaitGate_Reset::OPEN_LIGHT));
		addChild(createLightCentered<LargeLight<WhiteLight>>(mm2px(Vec(22.359, 108.255)), module, WaitGate_Reset::OPENLOAD_LIGHT));
	}
};


Model* modelWaitGate_Reset = createModel<WaitGate_Reset, WaitGate_ResetWidget>("WaitGate_Reset");