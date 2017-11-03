#pragma once
#include "Delay.h"

// derived class of CDelay
class CER : public CDelay
{
public:
	// constructor/destructor
	CER(void);
	~CER(void);

	// members
protected:

public:

	// overrides
	bool processAudio(float* pInput, float pInput2, float* pOutput);

};
