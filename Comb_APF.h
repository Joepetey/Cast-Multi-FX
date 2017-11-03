#pragma once
#include "Delay.h"

// derived class of CDelay
class CC_A : public CDelay
{
public:
	// constructor/destructor
	CC_A(void);
	~CC_A(void);

	// members
protected:
	float C_A;

public:

	// overrides
	bool processAudio(float* pInput, float* pInput2, float* pOutput);

};