/*
	CDelayLine: implements a delay line of D samples

	Copyright (c) 2017 Will Pirkle
	Free for academic use.
*/
#pragma once

#include <math.h>

// underflow protection
#ifndef FLT_MIN_PLUS
	#define FLT_MIN_PLUS          1.175494351e-38         /* min positive value */
#endif
#ifndef FLT_MIN_MINUS
	#define FLT_MIN_MINUS        -1.175494351e-38         /* min negative value */
#endif

class CDelayLine
{
public:
	// constructor/destructor
	CDelayLine(void);
	virtual ~CDelayLine(void);  // base class MUST declare virtual destructor

protected:
	// member variables
	//
	// --- pointer to our circular buffer
	double* m_pBuffer;
	
	// --- read/write index values for circ buffer
	int m_nReadIndex;
	int m_nWriteIndex;

	// --- max length of buffer
	int m_nBufferSize;

	// --- sample rate (needed for other function)
	int m_nSampleRate;

	// functions for the owner plug-in to call
public:
	// --- declare buffer and zero
	void init(int nDelayLength);

	// --- flush buffer, reset pointers to top
	void resetDelay();

	// --- set functions for Parent Plug In
	void setSampleRate(int nFs){m_nSampleRate = nFs;};

	// --- read the delay at an arbitrary time in mSec
	double readDelay_mSec(double dmSec);

	// --- write the input and increment write index
	void writeDelay(double dDelayInput);
	float m_fPreDelay;
};

