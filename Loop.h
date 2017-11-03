#pragma once
#include "Delay.h"

// derived class of CDelay
class CLoop : public CDelay
{
public:
	// constructor/destructor
	CLoop(void);
	~CLoop(void);

	// members
protected:
	float sum;

public:

	// function
	bool processAudio(float* pInput, float* pOutput);

};
