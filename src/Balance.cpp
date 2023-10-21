#include "plugin.hpp"


struct Balance : Module {

	int count1, count2, count3 = 0;
	bool pulse1, pulse2, pulse3 = false;
	bool block1, block2, block3 = false;

	enum ParamId {
		BAL1_PARAM,
		BAL2_PARAM,
		BAL3_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		IN1_INPUT,
		IN2_INPUT,
		IN3_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		O1_OUTPUT,
		O2_OUTPUT,
		O3_OUTPUT,
		OUTPUTS_LEN
	};
	enum LightId {
		IN1_LIGHT,
		IN2_LIGHT,
		IN3_LIGHT,
		LIGHTS_LEN
	};

	Balance() {
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configParam(BAL1_PARAM, 1.f, 200.f, 1.f, "Balance 1");
		configParam(BAL2_PARAM, 1.f, 200.f, 1.f, "Balance 2");
		configParam(BAL3_PARAM, 0.f, 200.f, 1.f, "Balance 3");
		paramQuantities[BAL1_PARAM]->snapEnabled = true;
		paramQuantities[BAL2_PARAM]->snapEnabled = true;
		paramQuantities[BAL3_PARAM]->snapEnabled = true;
		configInput(IN1_INPUT, "In 1");
		configInput(IN2_INPUT, "In 2");
		configInput(IN3_INPUT, "In 3");
		configOutput(O1_OUTPUT, "Out 1");
		configOutput(O2_OUTPUT, "Out 2");
		configOutput(O3_OUTPUT, "Out 3");
	}

	void process(const ProcessArgs& args) override {

		// Count pulses
		if (!pulse1 && inputs[IN1_INPUT].getVoltage() > 0.f && !block1) {
			pulse1 = true;
		}
		else if (pulse1 && inputs[IN1_INPUT].getVoltage() == 0.f) {
			pulse1 = false;
			if (count1 + 1 < params[BAL1_PARAM].getValue()) {
				count1++;
			}
			else {
				lights[IN1_LIGHT].setBrightness(1.f);
				block1 = true;
			}
		}

		if (!pulse2 && inputs[IN2_INPUT].getVoltage() > 0.f && !block2) {
			pulse2 = true;
		}
		else if (pulse2 && inputs[IN2_INPUT].getVoltage() == 0.f) {
			pulse2 = false;
			if (count2 + 1 < params[BAL2_PARAM].getValue()) {
				count2++;
			}
			else {
				lights[IN2_LIGHT].setBrightness(1.f);
				block2 = true;
			}
		}

		if (params[BAL3_PARAM].getValue() == 0.f) {
			block3 = true;
		}
		else if (!pulse3 && inputs[IN3_INPUT].getVoltage() > 0.f && !block3) {
			pulse3 = true;
		}
		else if (pulse3 && inputs[IN3_INPUT].getVoltage() == 0.f) {
			if (count3 + 1 < params[BAL3_PARAM].getValue()) {
				count3++;
			}
			else {
				lights[IN3_LIGHT].setBrightness(1.f);
				block3 = true;
			}
			pulse3 = false;
		}

		// Balance
		outputs[O1_OUTPUT].setVoltage(block1 ? 0.f : inputs[IN1_INPUT].getVoltage());
		outputs[O2_OUTPUT].setVoltage(block2 ? 0.f : inputs[IN2_INPUT].getVoltage());
		outputs[O3_OUTPUT].setVoltage(block3 ? 0.f : inputs[IN3_INPUT].getVoltage());

		if (block1 && block2 && block3) {
			block1 = block2 = block3 = false;
			count1 = count2 = count3 = 0;
			lights[IN1_LIGHT].setBrightness(0.f);
			lights[IN2_LIGHT].setBrightness(0.f);
			lights[IN3_LIGHT].setBrightness(0.f);
		}
	}
};


struct BalanceWidget : ModuleWidget {
	BalanceWidget(Balance* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/Balance.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.612, 53.014)), module, Balance::BAL1_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.612, 62.127)), module, Balance::BAL2_PARAM));
		addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(7.62, 71.241)), module, Balance::BAL3_PARAM));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 18.014)), module, Balance::IN1_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.62, 27.128)), module, Balance::IN2_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(7.628, 36.242)), module, Balance::IN3_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 87.881)), module, Balance::O1_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 96.994)), module, Balance::O2_OUTPUT));
		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(7.62, 106.089)), module, Balance::O3_OUTPUT));

		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(3.937, 14.332)), module, Balance::IN1_LIGHT));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(3.937, 23.445)), module, Balance::IN2_LIGHT));
		addChild(createLightCentered<TinyLight<RedLight>>(mm2px(Vec(3.945, 32.559)), module, Balance::IN3_LIGHT));
	}
};


Model* modelBalance = createModel<Balance, BalanceWidget>("Balance");