#include "plugin.hpp"


struct FlopFlip : Module {

	bool trig1, trig2, trig3, trig4 = false;
	bool flop = false;

	enum ParamId {
		PARAMS_LEN
	};
	enum InputId {
		TRIG1_INPUT,
		TRIG2_INPUT,
		TRIG3_INPUT,
		TRIG4_INPUT,
		FLOP_INPUT,
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

	FlopFlip() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(TRIG1_INPUT, "Trigger 1");
		configInput(TRIG2_INPUT, "Trigger 2");
		configInput(TRIG3_INPUT, "Trigger 3");
		configInput(TRIG4_INPUT, "Trigger 4");
		configInput(FLOP_INPUT, "Flop");
		configInput(FLIP_INPUT, "Flip");
		configOutput(OUT_OUTPUT, "Flopped");
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


struct FlopFlipWidget : ModuleWidget {
	FlopFlipWidget(FlopFlip* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/FlopFlip.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 19.047)), module, FlopFlip::TRIG1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 27.617)), module, FlopFlip::TRIG2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 36.186)), module, FlopFlip::TRIG3_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 44.756)), module, FlopFlip::TRIG4_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 62.831)), module, FlopFlip::FLOP_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 111.777)), module, FlopFlip::FLIP_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.566, 85.468)), module, FlopFlip::OUT_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.566, 79.154)), module, FlopFlip::FLOP_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(7.51, 95.913)), module, FlopFlip::FLIP_LIGHT));
	}
};


Model* modelFlopFlip = createModel<FlopFlip, FlopFlipWidget>("FlopFlip");