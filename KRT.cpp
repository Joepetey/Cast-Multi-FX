#include "KRT.h"

CKRT::CKRT(void)
{
}

CKRT::~CKRT(void)
{
}

bool CKRT::processAudio(float* pInput, float krt, float* pOutput)
{

	float KRT = *pInput * (krt);

	*pOutput = KRT;

	return true;
}