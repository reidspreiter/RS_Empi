#include "plugin.hpp"


struct FloppityFlippity : Module {

	bool floptrig1, floptrig2, fliptrig1, fliptrig2 = false;
	bool flop = false; 

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		FLOP_INPUT,
		FLOPTRIG1_INPUT,
		FLOPTRIG2_INPUT,
		FLIPTRIG1_INPUT,
		FLIPTRIG2_INPUT,
		FLIP_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUT_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		FLOP_LIGHT,
		FLIP_LIGHT,
		LIGHTS_LEN
	};

	FloppityFlippity() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(FLOP_INPUT, "Flop Input");
		configInput(FLOPTRIG1_INPUT, "Flop Trigger 1");
		configInput(FLOPTRIG2_INPUT, "Flop Trigger 2");
		configInput(FLIPTRIG1_INPUT, "Flip Trigger 1");
		configInput(FLIPTRIG2_INPUT, "Flip Trigger 2");
		configInput(FLIP_INPUT, "Flip");
		configOutput(OUT_OUTPUT, "Floppitied");
	}

	void process(const ProcessArgs& args) override {

		// Manage trigger inputs and flip and flop accordingly
		if (!floptrig1 && inputs[FLOPTRIG1_INPUT].getVoltage() > 0) {
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
		else if (!fliptrig1 && inputs[FLIPTRIG1_INPUT].getVoltage() > 0) {
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

		// Update lights and output
		if (flop) {
			outputs[OUT_OUTPUT].setVoltage(inputs[FLOP_INPUT].getVoltage());
			lights[FLOP_LIGHT].setBrightness(1.f);
			lights[FLIP_LIGHT].setBrightness(0.f);
		} 
		else {
			outputs[OUT_OUTPUT].setVoltage(inputs[FLIP_INPUT].getVoltage());
			lights[FLOP_LIGHT].setBrightness(0.f);
			lights[FLIP_LIGHT].setBrightness(1.f);
		}
	}
};


struct FloppityFlippityWidget : ModuleWidget {
	FloppityFlippityWidget(FloppityFlippity* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FloppityFlippity.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 16.793)), module, FloppityFlippity::FLOP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 30.114)), module, FloppityFlippity::FLOPTRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 38.683)), module, FloppityFlippity::FLOPTRIG2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 89.74)), module, FloppityFlippity::FLIPTRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 98.309)), module, FloppityFlippity::FLIPTRIG2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 111.777)), module, FloppityFlippity::FLIP_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.566, 62.185)), module, FloppityFlippity::OUT_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.566, 55.871)), module, FloppityFlippity::FLOP_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.51, 72.629)), module, FloppityFlippity::FLIP_LIGHT));
	}
};


Model* modelFloppityFlippity = createModel<FloppityFlippity, FloppityFlippityWidget>("FloppityFlippity");