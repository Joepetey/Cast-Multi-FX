#pragma once
#include "Delay.h"

// derived class of CDelay
class CDSum : public CDelay
{
public:
	// constructor/destructor
	CDSum(void);
	~CDSum(void);

	// members
protected:

public:

	// overrides
	bool processAudio(float* pInput, float* pInput2, float* pOutput);

};
