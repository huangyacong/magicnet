#include "SeList.h"
#include <stdio.h>
#include "SeHash.h"
#include "SeTime.h"
#include <stdbool.h>
#include "SeMagicNet.h"

int main()
{
	struct SEMAGICNETS kTest;
	SeMagicNetSInit(&kTest, "gate.log", 1000 * 30, 100, 8888, 8887);
	while (1) { SeMagicNetSProcess(&kTest); }
	SeMagicNetSFin(&kTest);
	return 0;
}