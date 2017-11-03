/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Definition
	Copyright(c) Tritone Systems Inc. 2006-2012

	Your plug-in must implement the constructor,
	destructor and virtual Plug-In API Functions below.
*/

#pragma once

// base class
#include "plugin.h"
#include "CombFilter.h"
#include "Delay.h"
#include "DelayAPF.h"
#include "LPFCombFilter.h"
#include "OnePoleLPF.h"
#include "dSum.h"
#include "KRT.h"
#include "Loop.h"
#include "Comb_APF.h"
#include "ER.h"
#include "DelayLine.h"
#include "WTOscillator.h"
#include "DDLModule.h"

// abstract base class for RackAFX filters
class CReverb : public CPlugIn
{
public:
	// RackAFX Plug-In API Member Methods:
	// The followung 5 methods must be impelemented for a meaningful Plug-In
	//
	// 1. One Time Initialization
	CReverb();

	// 2. One Time Destruction
	virtual ~CReverb(void);

	// 3. The Prepare For Play Function is called just before audio streams
	virtual bool __stdcall prepareForPlay();

	// 4. processAudioFrame() processes an audio input to create an audio output
	virtual bool __stdcall processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels);

	// 5. userInterfaceChange() occurs when the user moves a control.
	virtual bool __stdcall userInterfaceChange(int nControlIndex);


	// OPTIONAL ADVANCED METHODS ------------------------------------------------------------------------------------------------
	// These are more advanced; see the website for more details
	//
	// 6. initialize() is called once just after creation; if you need to use Plug-In -> Host methods
	//				   such as sendUpdateGUI(), you must do them here and NOT in the constructor
	virtual bool __stdcall initialize();

	// 7. joystickControlChange() occurs when the user moves a control.
	virtual bool __stdcall joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix);

	// 8. process buffers instead of Frames:
	// NOTE: set m_bWantBuffers = true to use this function
	virtual bool __stdcall processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels, UINT uBufferSize);

	// 9. rocess buffers instead of Frames:
	// NOTE: set m_bWantVSTBuffers = true to use this function
	virtual bool __stdcall processVSTAudioBuffer(float** ppInputs, float** ppOutputs, UINT uNumChannels, int uNumFrames);

	// 10. MIDI Note On Event
	virtual bool __stdcall midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity);

	// 11. MIDI Note Off Event
	virtual bool __stdcall midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff);


	// 12. MIDI Modulation Wheel uModValue = 0 -> 127
	virtual bool __stdcall midiModWheel(UINT uChannel, UINT uModValue);

	// 13. MIDI Pitch Bend
	//					nActualPitchBendValue = -8192 -> 8191, 0 is center, corresponding to the 14-bit MIDI value
	//					fNormalizedPitchBendValue = -1.0 -> +1.0, 0 is at center by using only -8191 -> +8191
	virtual bool __stdcall midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue);

	// 14. MIDI Timing Clock (Sunk to BPM) function called once per clock
	virtual bool __stdcall midiClock();


	// 15. all MIDI messages -
	// NOTE: set m_bWantAllMIDIMessages true to get everything else (other than note on/off)
	virtual bool __stdcall midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char cData1, unsigned char cData2);

	// 16. initUI() is called only once from the constructor; you do not need to write or call it. Do NOT modify this function
	virtual bool __stdcall initUI();



	



	// Add your code here: ----------------------------------------------------------- //
	// Pre-Delay Block
	CDelay m_PreDelay;

	// ER Generator
	CER m_ER_1;
	
	// input Diffusion
	COnePoleLPF m_InputLPF;
	CDelayAPF m_InputAPF_1;
	CDelayAPF m_InputAPF_2;
	CDelayAPF m_InputAPF_3;
	CDelayAPF m_InputAPF_4;
	CDelayAPF m_InputAPF_5;
	CDelayAPF m_InputAPF_6;
	CDelayAPF m_InputAPF_7;
	CDelayAPF m_InputAPF_8;


	// parallel Comb Bank 1
	CCombFilter m_ParallelCF_1;
	CCombFilter m_ParallelCF_2;
	CCombFilter m_ParallelCF_3;
	CCombFilter m_ParallelCF_4;

	// parallel Comb Bank 2
	CCombFilter m_ParallelCF_5;
	CCombFilter m_ParallelCF_6;
	CCombFilter m_ParallelCF_7;
	CCombFilter m_ParallelCF_8;
	
	// damping
	COnePoleLPF m_DampingLPF1;
	COnePoleLPF m_DampingLPF2;

	//Delay Sum
	CDelay m_LSum1;
	CDelay m_RSum1;
	CDelay m_LSum2;
	CDelay m_RSum2;
	CDelay m_LSum3;
	CDelay m_RSum3;
	CDelay m_LSum4;
	CDelay m_RSum4;
	CDSum m_Tap1;
	CDSum m_Tap2;
	CDSum m_Tap3;
	CDSum m_Tap4;

	// KRT Attenuation
	CKRT m_KRT1;
	CKRT m_KRT2;
	CKRT m_KRT3;
	CKRT m_KRT4;

	CLoop m_Sum;
	CC_A C_A;

	float m_fDelayInSamples;
	float m_fFeedback;
	float m_fWetLevel;

	// maintain our buffer
	float* m_pBuffer;
	int m_nReadIndex;
	int m_nWriteIndex;
	int m_nBufferSize;

	// function to cook all member object's variables at once
	void cookVariables();
	void resetDelay();

	bool  m_bUseExternalFeedback; // flag for enabling/disabling
	float m_fFeedbackIn;		// the user supplied feedback sample value

								// current FB is fb*output
	float getCurrentFeedbackOutput() { return m_fFeedback*m_pBuffer[m_nReadIndex]; }

	// set the feedback sample
	void  setCurrentFeedbackInput(float f) { m_fFeedbackIn = f; }

	// enable/disable external FB source
	void  setUsesExternalFeedback(bool b) { m_bUseExternalFeedback = false; }

	// --- delay lines
	CDelayLine m_LeftDelay;
	CDelayLine m_RightDelay;

	CWTOscillator m_LFO; 	// our LFO
	CDDLModule m_DDL;

	// --- modulated delay boundaries
	float minDelayMod_mSec;
	float maxDelayMod_mSec;

	// --- calculate the mod delay in mSec based on LFO value
	float calculateModDelayTime_mSec(float modValue);

	// --- stolen from DelayIt
	enum { LEFT, RIGHT };

	// --- this is slightly modified from DelayIt
	float doNormalDelay(float xn, float delay_mSec, float feedback, UINT channel);

	float m_fMinDelay_mSec;
	float m_fMaxDelay_mSec;

	// functions to update the member objects
	void updateLFO();
	void updateDDL();

	// cooking function for mod type
	void cookModType();

	// convert a LFO value to a delay offset value
	float calculateDelayOffset(float fLFOSample);

	//delay function

	//bitcrusher/downsampler function
	virtual bool __stdcall bcrush(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels, float downSample, float pitch, float bitdepth, float wet);


	// END OF USER CODE -------------------------------------------------------------- //


	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
	//  **--0x07FD--**

	float m_fPreDelay_mSec;
	float m_fPreDelayAtten_dB;
	float m_fInputLPF_g;
	float m_fAPF_1_Delay_mSec;
	float m_fAPF_1_g;
	float m_fAPF_2_Delay_mSec;
	float m_fAPF_2_g;
	float m_fKRT;
	float m_fRT60;
	float m_fWet_pct;
	float m_fPComb_1_Delay_mSec;
	float m_fPComb_2_Delay_mSec;
	float m_fPComb_3_Delay_mSec;
	float m_fPComb_4_Delay_mSec;
	float m_fAPF_3_Delay_mSec;
	float m_fAPF_3_g;
	float m_fPComb_5_Delay_mSec;
	float m_fPComb_6_Delay_mSec;
	float m_fPComb_7_Delay_mSec;
	float m_fPComb_8_Delay_mSec;
	float m_fAPF_4_Delay_mSec;
	float m_fAPF_4_g;
	float m_fModFrequency_Hz;
	float m_fModDepth_pct;
	float m_fFeedback_pct;
	float m_fPreDelay;
	float m_fChorusOffset;
	float m_fAPF_5_Delay_mSec;
	float m_fAPF_5_g;
	float m_fMTDelay;
	float m_fAPF_6_Delay_mSec;
	float m_fAPF_6_g;
	float m_fAPF_7_Delay_mSec;
	float m_fAPF_7_g;
	float m_fAPF_8_Delay_mSec;
	float m_fAPF_8_g;
	float m_f_Feedback_pct;
	float m_fWetLevel_pct;
	int m_fBitRate;
	int m_fDownSampling;
	int m_fBitDepth;
	UINT m_uSwitch1;
	enum{SWITCH_OFF,SWITCH_ON};
	UINT m_uSwitch2;
	UINT m_uSwitch4;
	UINT m_uModType;
	enum{Flanger,Vibrato,Chorus,Mixed};
	UINT m_uLFOType;
	enum{Triangle,Sine};
	UINT m_uLFOPhase;
	enum{Normal,Quad,Inverted};
	UINT m_uSwitch5;

	// **--0x1A7F--**
	// ------------------------------------------------------------------------------- //

};
