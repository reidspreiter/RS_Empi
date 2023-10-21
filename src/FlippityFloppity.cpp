#include "plugin.hpp"


struct FlippityFloppity : Module {

	bool fliptrig1, fliptrig2, floptrig1, floptrig2 = false;
	bool flop = false;
	
	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		FLIPTRIG1_INPUT,
		FLIPTRIG2_INPUT,
		IN_INPUT,
		FLOPTRIG1_INPUT,
		FLOPTRIG2_INPUT,
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

	FlippityFloppity() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(FLIPTRIG1_INPUT, "Flip Trigger 1");
		configInput(FLIPTRIG2_INPUT, "Flip Trigger 2");
		configInput(IN_INPUT, "Flippity");
		configInput(FLOPTRIG1_INPUT, "Flop Trigger 1");
		configInput(FLOPTRIG2_INPUT, "Flop Trigger 2");
		configOutput(FLIPOUT_OUTPUT, "Flip");
		configOutput(FLOPOUT_OUTPUT, "Flop");
	}

	void process(const ProcessArgs& args) override {

		// Manage trigger inputs and flip and flop accordingly
		if (!fliptrig1 && inputs[FLIPTRIG1_INPUT].getVoltage() > 0) {
			flop = false;
			fliptrig1 = true;
		}
		else if (fliptrig1 && inputs[FLIPTRIG1_INPUT].getVoltage() == 0) {
			fliptrig1 = false;
		}
		else if (!fliptrig2 && inputs[FLIPTRIG2_INPUT].getVoltage() > 0) {
			flop = false;
			fliptrig2 = true;
		}
		else if (fliptrig2 && inputs[FLIPTRIG2_INPUT].getVoltage() == 0) {
			fliptrig2 = false;
		}
		else if (!floptrig1 && inputs[FLOPTRIG1_INPUT].getVoltage() > 0) {
			flop = true;
			floptrig1 = true;
		}
		else if (floptrig1 && inputs[FLOPTRIG1_INPUT].getVoltage() == 0) {
			floptrig1 = false;
		}
		else if (!floptrig2 && inputs[FLOPTRIG2_INPUT].getVoltage() > 0) {
			flop = true;
			floptrig2 = true;
		}
		else if (floptrig2 && inputs[FLOPTRIG2_INPUT].getVoltage() == 0) {
			floptrig2 = false;
		}

		// Update lights and output
		if (flop) {
			outputs[FLOPOUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());
			outputs[FLIPOUT_OUTPUT].setVoltage(0.f);
			lights[FLOP_LIGHT].setBrightness(1.f);
			lights[FLIP_LIGHT].setBrightness(0.f);
		} else {
			outputs[FLIPOUT_OUTPUT].setVoltage(inputs[IN_INPUT].getVoltage());
			outputs[FLOPOUT_OUTPUT].setVoltage(0.f);
			lights[FLOP_LIGHT].setBrightness(0.f);
			lights[FLIP_LIGHT].setBrightness(1.f);
		}

	}
};


struct FlippityFloppityWidget : ModuleWidget {
	FlippityFloppityWidget(FlippityFloppity* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FlippityFloppity.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 38.051)), module, FlippityFloppity::FLIPTRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 46.621)), module, FlippityFloppity::FLIPTRIG2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 62.074)), module, FlippityFloppity::IN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 81.802)), module, FlippityFloppity::FLOPTRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 90.372)), module, FlippityFloppity::FLOPTRIG2_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 23.993)), module, FlippityFloppity::FLIPOUT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 100.268)), module, FlippityFloppity::FLOPOUT_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.62, 18.087)), module, FlippityFloppity::FLIP_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.62, 110.566)), module, FlippityFloppity::FLOP_LIGHT));
	}
};


Model* modelFlippityFloppity = createModel<FlippityFloppity, FlippityFloppityWidget>("FlippityFloppity");