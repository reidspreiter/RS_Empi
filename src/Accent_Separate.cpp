#include "plugin.hpp"


struct Accent_Separate : Module {

	int count = 0;
	bool pulse = false;

	enum ParamId {
		LENGTH_PARAM,
		ENUMS(ACC_PARAMS, 7),
		PARAMS_LEN
	};
	enum InputId {
		PULSE_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		ENUMS(ACC_OUTPUTS, 7),
		DOWNBEAT_OUTPUT,
		ACCS_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		ENUMS(ACC_LIGHTS, 7),
		LIGHTS_LEN
	};

	Accent_Separate() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(LENGTH_PARAM, 2.f, 200.f, 0.f, "Beats per measure");
		paramQuantities[LENGTH_PARAM]->snapEnabled = true;
		configParam(ACC_PARAMS + 0, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 1, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 2, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 3, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 4, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 5, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		configParam(ACC_PARAMS + 6, 0.f, 199.f, 0.f, "Accent", " beats after downbeat");
		paramQuantities[ACC_PARAMS + 0]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 1]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 2]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 3]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 4]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 5]->snapEnabled = true;
		paramQuantities[ACC_PARAMS + 6]->snapEnabled = true;
		configInput(PULSE_INPUT, "Pulse");
		configOutput(ACC_OUTPUTS + 0, "Accent");
		configOutput(ACC_OUTPUTS + 1, "Accent");
		configOutput(ACC_OUTPUTS + 2, "Accent");
		configOutput(ACC_OUTPUTS + 3, "Accent");
		configOutput(ACC_OUTPUTS + 4, "Accent");
		configOutput(ACC_OUTPUTS + 5, "Accent");
		configOutput(ACC_OUTPUTS + 6, "Accent");
		configOutput(DOWNBEAT_OUTPUT, "Downbeat");
		configOutput(ACCS_OUTPUT, "Accents");
	}

	void process(const ProcessArgs& args) override {

		// Count pulses
		if (!pulse && inputs[PULSE_INPUT].getVoltage() > 0.f) {
			pulse = true;
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

			for (int i = 0; i < 7; i++) {
				if (count == params[ACC_PARAMS + i].getValue()) {
					if (pulse) {
						outputs[ACCS_OUTPUT].setVoltage(inputs[PULSE_INPUT].getVoltage());
						outputs[ACC_OUTPUTS + i].setVoltage(inputs[PULSE_INPUT].getVoltage());
						lights[ACC_LIGHTS + i].setBrightness(1.f);
					}
					else {
						outputs[ACCS_OUTPUT].setVoltage(0.f);
						outputs[ACC_OUTPUTS + i].setVoltage(0.f);
						lights[ACC_LIGHTS + i].setBrightness(0.f);
					}
					break;
				} 
			}
		}
	}
};


struct Accent_SeparateWidget : ModuleWidget {
	Accent_SeparateWidget(Accent_Separate* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Accent_Separate.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(18.871, 17.552)), module, Accent_Separate::LENGTH_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.277, 33.052)), module, Accent_Separate::ACC_PARAMS + 0));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.333, 42.691)), module, Accent_Separate::ACC_PARAMS + 1));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.333, 52.33)), module, Accent_Separate::ACC_PARAMS + 2));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.333, 61.969)), module, Accent_Separate::ACC_PARAMS + 3));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.389, 71.607)), module, Accent_Separate::ACC_PARAMS + 4));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.389, 81.246)), module, Accent_Separate::ACC_PARAMS + 5));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(6.389, 90.885)), module, Accent_Separate::ACC_PARAMS + 6));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(6.389, 17.553)), module, Accent_Separate::PULSE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.877, 33.052)), module, Accent_Separate::ACC_OUTPUTS + 0));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 42.691)), module, Accent_Separate::ACC_OUTPUTS + 1));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 52.33)), module, Accent_Separate::ACC_OUTPUTS + 2));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 61.969)), module, Accent_Separate::ACC_OUTPUTS + 3));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 71.607)), module, Accent_Separate::ACC_OUTPUTS + 4));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 81.246)), module, Accent_Separate::ACC_OUTPUTS + 5));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 90.885)), module, Accent_Separate::ACC_OUTPUTS + 6));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(6.389, 106.089)), module, Accent_Separate::DOWNBEAT_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(18.871, 106.089)), module, Accent_Separate::ACCS_OUTPUT));

		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.546, 29.587)), module, Accent_Separate::ACC_LIGHTS + 0));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.546, 39.226)), module, Accent_Separate::ACC_LIGHTS + 1));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.546, 48.864)), module, Accent_Separate::ACC_LIGHTS + 2));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.546, 58.503)), module, Accent_Separate::ACC_LIGHTS + 3));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.602, 68.142)), module, Accent_Separate::ACC_LIGHTS + 4));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.602, 77.781)), module, Accent_Separate::ACC_LIGHTS + 5));
		addChild(createLightCentered<TinyLight<WhiteLight>>(mm2px(Vec(2.602, 87.419)), module, Accent_Separate::ACC_LIGHTS + 6));
	}
};


Model* modelAccent_Separate = createModel<Accent_Separate, Accent_SeparateWidget>("Accent_Separate");