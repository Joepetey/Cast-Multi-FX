#pragma once
/*
CTrivOscillator: implements a trivial oscillator with several waveform choices

Copyright (c) 2017 Will Pirkle
Free for academic use.
*/

#include "synthfunctions.h"

class CTrivOscillator
{
public:
	CTrivOscillator();
	virtual ~CTrivOscillator(void);

	// --- public functions for owner to call (interface)
	//
	// --- setters
	void setOscFrequency(float frequency)
	{
		oscFrequency = frequency;
		cookVariables();
	}
	void setSampleRate(float fs)
	{
		sampleRate = fs;
		cookVariables();
	}
	void setOscWaveform(UINT waveform) { oscWaveform = waveform; }

	// --- reset
	void reset()
	{
		modCounter = 0.f;
		phaseInc = 0.f;
		modCounterQP = 0.25f;
		cookVariables();
	}

	// --- make a new osc sample NOTE: bipolar [-1, +1] output
	//     use helper functions if you need to convert unipolarToBipolar()
	inline void doOscillate(float& output, float& quadPhaseOutput)
	{
		float oscOut = 0.f;
		float oscOutQP = 0.f;

		// --- always first
		checkWrapModulos();

		// --- sawtooth
		if (oscWaveform == TRI)
		{
			// --- saw
			oscOut = unipolarToBipolar(modCounter);

			// --- convert to bipolar triagle
			oscOut = 2.0*fabs(oscOut) - 1.0;

			// -- quad phase
			oscOutQP = unipolarToBipolar(modCounterQP);

			// bipolar triagle
			oscOutQP = 2.0*fabs(oscOutQP) - 1.0;
		}
		else if(oscWaveform == SINE)
		{
			double dAngle = modCounter*2.0*(double)pi - (double)pi;
			double dAngleQP = modCounterQP*2.0*(double)pi - (double)pi;

			// call the parabolicSine approximator
			oscOut = parabolicSine(-1.0*dAngle);
			oscOutQP = parabolicSine(-1.0*dAngleQP);
		}
		else if (oscWaveform == UPSAW)
		{
			oscOut = unipolarToBipolar(modCounter);
			oscOutQP = unipolarToBipolar(modCounterQP);
		}
		else if (oscWaveform == DNSAW)
		{
			oscOut = -unipolarToBipolar(modCounter);
			oscOutQP = -unipolarToBipolar(modCounterQP);
		}

		// --- add phase inc, always last
		modCounter += phaseInc;
		modCounterQP += phaseInc;

		// --- write outputs
		output = oscOut;
		quadPhaseOutput = oscOutQP;
	}

protected:
	// --- oscillator frequency 
	float oscFrequency;
	float sampleRate;

	// --- NOTE: make identical copy in plugin
	enum { TRI, SINE, UPSAW, DNSAW };
	UINT oscWaveform;

	// --- cooker
	void cookVariables()
	{
		// --- phase inc = fo/fs
		phaseInc = oscFrequency / sampleRate;
	}

	// --- check for wrap
	inline void checkWrapModulos()
	{
		// --- check and wrap if needed
		if (phaseInc > 0 && modCounter >= 1.0)
			modCounter -= 1.0;

		if (phaseInc > 0 && modCounterQP >= 1.0)
			modCounterQP -= 1.0;
	}

private:
	float modCounter;		// --- modulo counter 0->1
	float phaseInc;			// --- phase inc = fo/fs
	float modCounterQP;		// --- Quad Phase modulo counter 0->1

};

