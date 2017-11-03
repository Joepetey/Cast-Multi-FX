#include "ER.h"

CER::CER(void)
{
}

CER::~CER(void)
{
}

bool CER::processAudio(float* pInput, float pInput2, float* pOutput)
{

	float Early_Reflection = *pInput + pInput2;

	*pOutput = Early_Reflection;

	// all OK
	return true;
}