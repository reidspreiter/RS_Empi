#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	p->addModel(modelWaitGate);
	p->addModel(modelWaitGate_Zero);
	p->addModel(modelTempinator_Dependence);
	p->addModel(modelWaitGate_Reset);
	p->addModel(modelFlipFlop);
	p->addModel(modelTempinator);
	p->addModel(modelFloppityFlippity);
	p->addModel(modelFlopFlip);
	p->addModel(modelFlippityFloppity);
	p->addModel(modelAccent);
	p->addModel(modelWaitGate_Duo);
	p->addModel(modelTempinator_DepAcc);
	p->addModel(modelAccent_Separate);
}
