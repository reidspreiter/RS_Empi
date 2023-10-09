#include "plugin.hpp"


Plugin* pluginInstance;


void init(Plugin* p) {
	pluginInstance = p;

	p->addModel(modelWaitGate);
	p->addModel(modelWaitGate_Zero);
	p->addModel(modelTempinator_Dependence);
	p->addModel(modelWaitGate_Reset);
	p->addModel(modelFlipFlop);
}
