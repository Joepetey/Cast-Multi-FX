/*
CDelayLine: implements a delay line of D samples

Copyright (c) 2017 Will Pirkle
Free for academic use.
*/

#include "DelayLine.h"
#include "pluginconstants.h"

CDelayLine::CDelayLine(void)
{
	// --- zero everything
	m_pBuffer = NULL;
	m_nSampleRate = 0;

	// --- reset
	resetDelay();
}

CDelayLine::~CDelayLine(void)
{
	if(m_pBuffer)
		delete m_pBuffer;

	m_pBuffer = NULL;
}

void CDelayLine::init(int nDelayLength)
{
	// --- save for later
	m_nBufferSize = nDelayLength;

	// --- delete if existing
	if(m_pBuffer)
		delete m_pBuffer;

	// --- create
	m_pBuffer = new double[m_nBufferSize];

	// --- flush buffer
	memset(m_pBuffer, 0, m_nBufferSize*sizeof(double));
}

void CDelayLine::resetDelay()
{
	// --- flush buffer
	if(m_pBuffer)
		memset(m_pBuffer, 0, m_nBufferSize*sizeof(double));

	// --- init read/write indices
	m_nWriteIndex = 0; 
	m_nReadIndex = 0; 
}

// --- read delay from an arbitrary location given in mSec
double CDelayLine::readDelay_mSec(double dmSec)
{
	// --- local variales
	double dDelayInSamples = dmSec*((float)m_nSampleRate)/1000.0;

	// --- subtract to make read index
	int nReadIndex = m_nWriteIndex - (int)dDelayInSamples;

	// --- wrap if needed
	if (nReadIndex < 0)
		nReadIndex += m_nBufferSize;	// amount of wrap is Read + Length

	// --- if you get clicks or pops, set a breakpoint on this trap
	if (nReadIndex < 0 || nReadIndex >= m_nBufferSize)
 		int trap = 0; // <--- trap

	//---  Read the output of the delay at m_nReadIndex
	double yn = m_pBuffer[nReadIndex];

	// --- Read the location ONE BEHIND yn at y(n-1)
	int nReadIndex_1 = nReadIndex - 1;
	if(nReadIndex_1 < 0)
		nReadIndex_1 = m_nBufferSize-1; // m_nBufferSize-1 is last location

	// -- get y(n-1)
	double yn_1 = m_pBuffer[nReadIndex_1];

	// --- get the fractional component
	double dFracDelay = dDelayInSamples - (int)dDelayInSamples;
	
	// --- interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
	return dLinTerp(0, 1, yn, yn_1, dFracDelay); // interp frac between them
}

// --- write delay an dincrement the delay index value
void CDelayLine::writeDelay(double dDelayInput)
{
	// --- write to the delay line
	m_pBuffer[m_nWriteIndex] = dDelayInput; // external feedback sample

	// --- increment the pointers and wrap if necessary
	m_nWriteIndex++;
	if(m_nWriteIndex >= m_nBufferSize)
		m_nWriteIndex = 0;
}


