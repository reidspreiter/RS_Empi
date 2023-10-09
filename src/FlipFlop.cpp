#include "plugin.hpp"


struct FlipFlop : Module {

	bool flop = true;
	bool trig1, trig2, trig3, trig4 = false;

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		SIGNAL_INPUT,
		TRIG1_INPUT,
		TRIG2_INPUT,
		TRIG3_INPUT,
		TRIG4_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		FLIPOUT_OUTPUT,
		FLOPOUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		FLIP_LIGHT,
		FLOP_LIGHT,
		LIGHTS_LEN
	};

	FlipFlop() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(SIGNAL_INPUT, "Input");
		configInput(TRIG1_INPUT, "Trigger 1");
		configInput(TRIG2_INPUT, "Trigger 2");
		configInput(TRIG3_INPUT, "Trigger 3");
		configInput(TRIG4_INPUT, "Trigger 4");
		configOutput(FLIPOUT_OUTPUT, "Flip Output");
		configOutput(FLOPOUT_OUTPUT, "Flop Output");
	}

	void process(const ProcessArgs& args) override {

		// Manage trigger inputs and flip and flop accordingly
		if (!trig1 && inputs[TRIG1_INPUT].getVoltage() > 0) {
			flop = !flop;
			trig1 = true;
		}
		else if (trig1 && inputs[TRIG1_INPUT].getVoltage() == 0) {
			trig1 = false;
		}
		else if (!trig2 && inputs[TRIG2_INPUT].getVoltage() > 0) {
			flop = !flop;
			trig2 = true;
		}
		else if (trig2 && inputs[TRIG2_INPUT].getVoltage() == 0) {
			trig2 = false;
		}
		else if (!trig3 && inputs[TRIG3_INPUT].getVoltage() > 0) {
			flop = !flop;
			trig3 = true;
		}
		else if (trig3 && inputs[TRIG3_INPUT].getVoltage() == 0) {
			trig3 = false;
		}
		else if (!trig4 && inputs[TRIG4_INPUT].getVoltage() > 0) {
			flop = !flop;
			trig4 = true;
		}
		else if (trig4 && inputs[TRIG4_INPUT].getVoltage() == 0) {
			trig4 = false;
		}

		// Update lights and output
		if (flop) {
			outputs[FLOPOUT_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage());
			lights[FLOP_LIGHT].setBrightness(1.f);
			lights[FLIP_LIGHT].setBrightness(0.f);
		} 
		else {
			outputs[FLIPOUT_OUTPUT].setVoltage(inputs[SIGNAL_INPUT].getVoltage());
			lights[FLOP_LIGHT].setBrightness(0.f);
			lights[FLIP_LIGHT].setBrightness(1.f);
		}
	}
};


struct FlipFlopWidget : ModuleWidget {
	FlipFlopWidget(FlipFlop* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FlipFlop.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 40.75)), module, FlipFlop::SIGNAL_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 54.086)), module, FlipFlop::TRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.622, 62.133)), module, FlipFlop::TRIG2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 70.18)), module, FlipFlop::TRIG3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 78.227)), module, FlipFlop::TRIG4_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 21.98)), module, FlipFlop::FLIPOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 97.143)), module, FlipFlop::FLOPOUT_OUTPUT));

		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(7.62, 32.665)), module, FlipFlop::FLIP_LIGHT));
		addChild(createLightCentered<SmallLight<WhiteLight>>(mm2px(Vec(7.62, 107.828)), module, FlipFlop::FLOP_LIGHT));
	}
};


Model* modelFlipFlop = createModel<FlipFlop, FlipFlopWidget>("FlipFlop");