#pragma once
#include "Delay.h"

// derived class of CDelay
class CKRT : public CDelay
{
public:
	// constructor/destructor
	CKRT(void);
	~CKRT(void);

	// members
protected:

public:

	// overrides
	bool processAudio(float* pInput, float RT60, float* pOutput);

};