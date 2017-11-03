#include "Comb_APF.h"

CC_A::CC_A(void)
{
}

CC_A::~CC_A(void)
{
}

bool CC_A::processAudio(float* pInput,float* pInput2, float* pOutput)
{

	float C_A = *pInput + *pInput2;

	*pOutput = C_A;

	// all OK
	return true;
}
