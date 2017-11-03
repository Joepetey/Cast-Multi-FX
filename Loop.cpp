#include "Loop.h"

CLoop::CLoop(void)
{
}

CLoop::~CLoop(void)
{
}

bool CLoop::processAudio(float* pInput, float* pOutput)
{

	float sum = *pInput + *pOutput;

	*pOutput = sum;

	return true;
}