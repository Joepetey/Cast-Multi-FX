#include "dSum.h"

CDSum::CDSum(void)
{
}

CDSum::~CDSum(void)
{
}

bool CDSum::processAudio(float* pInput, float* pInput2, float* pOutput)
{
	
	float L_RSum = *pInput + *pInput2;

	*pOutput = L_RSum;

	// all OK
	return true;
}
