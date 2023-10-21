#include "plugin.hpp"


struct Accent : Module {

	int count = 0;
	bool pulse = false;
	int light = 0;

	enum ParamId {
		LENGTH_PARAM,
		ACCENT1_PARAM,
		ACCENT2_PARAM,
		ACCENT3_PARAM,
		ACCENT4_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		PULSE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		DOWNBEAT_OUTPUT,
		ACCENTS_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ACC1_LIGHT,
		ACC2_LIGHT,
		ACC3_LIGHT,
		ACC4_LIGHT,
		LIGHTS_LEN
	};

	Accent() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(LENGTH_PARAM, 2.f, 99.f, 4.f, "Beats per measure");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;
		configParam(ACCENT1_PARAM, 0.f, 98.f, 0.f, "Accent ", " beats after downbeat");
		paramQuantities[ACCENT1_PARAM]->snapEnabled = true;
		configParam(ACCENT2_PARAM, 0.f, 98.f, 0.f, "Accent ", " beats after downbeat");
		paramQuantities[ACCENT2_PARAM]->snapEnabled = true;
		configParam(ACCENT3_PARAM, 0.f, 98.f, 0.f, "Accent ", " beats after downbeat");
		paramQuantities[ACCENT3_PARAM]->snapEnabled = true;
		configParam(ACCENT4_PARAM, 0.f, 98.f, 0.f, "Accent ", " beats after downbeat");
		paramQuantities[ACCENT4_PARAM]->snapEnabled = true;
		configInput(PULSE_INPUT, "Pulse");
		configOutput(DOWNBEAT_OUTPUT, "Downbeat");
		configOutput(ACCENTS_OUTPUT, "Accents");
	}

	void process(const ProcessArgs& args) override {

		// Count pulses
		if (!pulse && inputs[PULSE_INPUT].getVoltage() > 0.f) {
			pulse = true;
			lights[LightId(light)].setBrightness(0.f);
			count = (count + 1 == params[LENGTH_PARAM].getValue() ? 0 : count + 1);
		}
		else if (pulse && inputs[PULSE_INPUT].getVoltage() == 0.f) {
			pulse = false;
		}

		// Apply accents
		if (count == 0) {
			outputs[DOWNBEAT_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
		}
		else {
			outputs[DOWNBEAT_OUTPUT].setVoltage(0.f);

			if (count == params[ACCENT1_PARAM].getValue()) {
				outputs[ACCENTS_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
				lights[ACC1_LIGHT].setBrightness(1.f);
				light = 0;
			} 
			else if (count == params[ACCENT2_PARAM].getValue()) {
				outputs[ACCENTS_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
				lights[ACC2_LIGHT].setBrightness(1.f);
				light = 1;
			} 
			else if (count == params[ACCENT3_PARAM].getValue()) {
				outputs[ACCENTS_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
				lights[ACC3_LIGHT].setBrightness(1.f);
				light = 2;
			} 
			else if (count == params[ACCENT4_PARAM].getValue()) {
				outputs[ACCENTS_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
				lights[ACC4_LIGHT].setBrightness(1.f);
				light = 3;
			}
			else {
				outputs[ACCENTS_OUTPUT].setVoltage(0.f);
			}
		}
	}
};


struct AccentWidget : ModuleWidget {
	AccentWidget(Accent* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Accent.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.626, 30.551)), module, Accent::LENGTH_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.564, 46.011)), module, Accent::ACCENT1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 55.65)), module, Accent::ACCENT2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 65.288)), module, Accent::ACCENT3_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 74.927)), module, Accent::ACCENT4_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.626, 17.466)), module, Accent::PULSE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 91.242)), module, Accent::DOWNBEAT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 106.089)), module, Accent::ACCENTS_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.833, 42.546)), module, Accent::ACC1_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.833, 52.184)), module, Accent::ACC2_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.833, 61.823)), module, Accent::ACC3_LIGHT));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(3.833, 71.462)), module, Accent::ACC4_LIGHT));
	}
};


Model* modelAccent = createModel<Accent, AccentWidget>("Accent");