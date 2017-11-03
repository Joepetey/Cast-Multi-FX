/*
	RackAFX(TM)
	Applications Programming Interface
	Derived Class Object Implementation
*/


#include "Reverb.h"

#define MIN_CHORUS_DELAY_MSEC 10.f
#define MAX_CHORUS_DELAY_MSEC 60.f

#define MIN_FLANGER_DELAY_MSEC 0.01f
#define MAX_FLANGER_DELAY_MSEC 7.f


/* constructor()
	You can initialize variables here.
	You can also allocate memory here as long is it does not
	require the plugin to be fully instantiated. If so, allocate in init()

*/
CReverb::CReverb()
{
	// Added by RackAFX - DO NOT REMOVE
	//
	// initUI() for GUI controls: this must be called before initializing/using any GUI variables
	initUI();
	// END initUI()

	// built in initialization
	m_PlugInName = "Cast Multi-FX";

	// Default to Stereo Operation:
	// Change this if you want to support more/less channels
	m_uMaxInputChannels = 2;
	m_uMaxOutputChannels = 2;

	// use of MIDI controllers to adjust sliders/knobs
	m_bEnableMIDIControl = true;		// by default this is enabled

	// custom GUI stuff
	m_bLinkGUIRowsAndButtons = false;	// change this if you want to force-link

	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUseCustomVSTGUI = false;

	// for a user (not RackAFX) generated GUI - advanced you must compile your own resources
	// DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bUserCustomGUI = false;

	// output only - SYNTH - plugin DO NOT CHANGE let RackAFX change it for you; use Edit Project to alter
	m_bOutputOnlyPlugIn = false;

	// Finish initializations here
	m_fDelayInSamples = 0;
	m_fFeedback = 0;
	m_f_Feedback_pct = 0;
	m_fWetLevel = 0;
	// reset indices
	m_nReadIndex = 0;
	m_nWriteIndex = 0;

	// no buffer yet because we don�t have a sample rate yet
	m_pBuffer = NULL;
	m_nBufferSize = 0;

	//FLANGER INIT
	minDelayMod_mSec = MIN_FLANGER_DELAY_MSEC;
	maxDelayMod_mSec = MAX_FLANGER_DELAY_MSEC;

	m_fMinDelay_mSec = 0.0;
	m_fMaxDelay_mSec = 0.0;
	m_fChorusOffset = 0.0;

	m_LFO.m_fFrequency_Hz = 0;
	m_LFO.m_uOscType = 2; // triangle	enum{sine,saw,tri,square};

	m_DDL.m_bUseExternalFeedback = false;
	m_DDL.m_fDelay_ms = 0;

	// init the cooked varaiables
	cookVariables();
}


/* destructor()
	Destroy variables allocated in the contructor()

*/
CReverb::~CReverb(void)
{
	if (m_pBuffer)
		delete[] m_pBuffer;

}

/*
initialize()
	Called by the client after creation; the parent window handle is now valid
	so you can use the Plug-In -> Host functions here (eg sendUpdateUI())

	NOTE: This function is called whenever the plug-in gets a new Client UI
	      e.g. loading and unloading User Plug-Ins.

	      Therefore, do not place One Time Initialization code here, place it in
	      the END of the constructor.

	      This function is really designed only for letting you communicate back
	      with the Host GUI via sendUpdateUI()
	See the website www.willpirkle.com for more details
*/
bool __stdcall CReverb::initialize()
{
	// Add your code here

	return true;
}
void CReverb::updateLFO()
{
	// set raw data
	m_LFO.m_fFrequency_Hz = m_fModFrequency_Hz;

	// typo in book; this re-maps OUR LFO type to the WTOscillator's LFO
	// Oscillator:  enum{sine,saw,tri,square}; sine = 0, saw = 1, tri = 2
	// this module: enum{tri,sin}; tri = 0, sin = 1
	// so, if type == 0, map to oscillator type 2 (tri), else map to 0
	m_LFO.m_uOscType = m_uLFOType == 0 ? 2 : 0;

	// cook it
	m_LFO.cookFrequency();
}

void CReverb::updateDDL()
{
	// test and set if needed
	if (m_uModType != Vibrato)
		m_DDL.m_fFeedback_pct = m_fFeedback_pct;

	// cook it
	m_DDL.cookVariables();
}
void CReverb::cookModType()
{
	switch (m_uModType)
	{
	case Flanger:
	{
		m_fMinDelay_mSec = 0 + m_fPreDelay;
		m_fMaxDelay_mSec = 7 + m_fPreDelay;
		m_DDL.m_fWetLevel_pct = 50.0;
		m_DDL.m_fFeedback_pct = m_fFeedback_pct;
		break;
	}

	case Vibrato:
	{
		m_fMinDelay_mSec = 0 + m_fPreDelay;
		m_fMaxDelay_mSec = 7 + m_fPreDelay;
		m_DDL.m_fWetLevel_pct = 100.0;
		m_DDL.m_fFeedback_pct = 0.0; // NOTE! no FB for vibrato
		break;
	}

	case Chorus:
	{
		m_fMinDelay_mSec = 5 + m_fPreDelay;
		m_fMaxDelay_mSec = 30 + m_fPreDelay;
		m_DDL.m_fWetLevel_pct = 50.0;
		//			m_DDL.m_fInputAttenuation = 1.0;
		break;
	}
	case Mixed:
	{
		m_fMinDelay_mSec = 0 + m_fPreDelay;
		m_fMaxDelay_mSec = 30 + m_fPreDelay;
		m_DDL.m_fWetLevel_pct = 75;
		m_DDL.m_fFeedback_pct = m_fFeedback_pct;
		
	}

	default: // is Flanger
	{
		m_fMinDelay_mSec = 0 + m_fPreDelay;
		m_fMaxDelay_mSec = 7 + m_fPreDelay;
		m_DDL.m_fWetLevel_pct = 50.0;
		m_DDL.m_fFeedback_pct = m_fFeedback_pct;
		break;
	}
	}
}


// function to cook all variables at once
void CReverb::cookVariables()
{	
	// Pre-Delay
	m_PreDelay.setDelay_mSec(m_fPreDelay_mSec);
	m_PreDelay.setOutputAttenuation_dB(m_fPreDelayAtten_dB);
	m_PreDelay.setSampleRate(m_nSampleRate); // Added WP

	// input diffusion
	m_InputAPF_1.setDelay_mSec(m_fAPF_1_Delay_mSec);
	m_InputAPF_1.setAPF_g(m_fAPF_1_g);
	m_InputAPF_1.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_2.setDelay_mSec(m_fAPF_2_Delay_mSec);
	m_InputAPF_2.setAPF_g(m_fAPF_2_g);
	m_InputAPF_2.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_3.setDelay_mSec(m_fAPF_3_Delay_mSec);
	m_InputAPF_3.setAPF_g(m_fAPF_3_g);
	m_InputAPF_3.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_4.setDelay_mSec(m_fAPF_4_Delay_mSec);
	m_InputAPF_4.setAPF_g(m_fAPF_4_g);
	m_InputAPF_4.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_5.setDelay_mSec(m_fAPF_5_Delay_mSec);
	m_InputAPF_5.setAPF_g(m_fAPF_5_g);
	m_InputAPF_5.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_6.setDelay_mSec(m_fAPF_6_Delay_mSec);
	m_InputAPF_6.setAPF_g(m_fAPF_6_g);
	m_InputAPF_6.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_7.setDelay_mSec(m_fAPF_7_Delay_mSec);
	m_InputAPF_7.setAPF_g(m_fAPF_7_g);
	m_InputAPF_7.setSampleRate(m_nSampleRate); // Added WP

	m_InputAPF_8.setDelay_mSec(m_fAPF_8_Delay_mSec);
	m_InputAPF_8.setAPF_g(m_fAPF_8_g);
	m_InputAPF_8.setSampleRate(m_nSampleRate); // Added WP

	// DelaySum
	m_LSum1.setDelay_mSec(m_fMTDelay);
	m_LSum1.setSampleRate(m_nSampleRate);
	m_RSum1.setDelay_mSec(m_fMTDelay);
	m_RSum1.setSampleRate(m_nSampleRate);
	m_LSum2.setDelay_mSec(m_fMTDelay/2);
	m_LSum2.setSampleRate(m_nSampleRate);
	m_RSum2.setDelay_mSec(m_fMTDelay/2);
	m_RSum2.setSampleRate(m_nSampleRate);
	m_LSum3.setDelay_mSec(m_fMTDelay/3);
	m_LSum3.setSampleRate(m_nSampleRate);
	m_RSum3.setDelay_mSec(m_fMTDelay/3);
	m_RSum3.setSampleRate(m_nSampleRate);
	m_LSum4.setDelay_mSec(m_fMTDelay/4);
	m_LSum4.setSampleRate(m_nSampleRate);
	m_RSum4.setDelay_mSec(m_fMTDelay/4);
	m_RSum4.setSampleRate(m_nSampleRate);
	m_Tap1.setDelay_mSec(m_fMTDelay);
	m_Tap1.setSampleRate(m_nSampleRate);
	m_Tap2.setDelay_mSec(m_fMTDelay);
	m_Tap2.setSampleRate(m_nSampleRate);
	m_Tap3.setDelay_mSec(m_fMTDelay);
	m_Tap3.setSampleRate(m_nSampleRate);
	m_Tap4.setDelay_mSec(m_fMTDelay);
	m_Tap4.setSampleRate(m_nSampleRate);

	// Comb Filters
	// set sample rate // added WP
	m_ParallelCF_1.setSampleRate(m_nSampleRate);
	m_ParallelCF_2.setSampleRate(m_nSampleRate);
	m_ParallelCF_3.setSampleRate(m_nSampleRate);
	m_ParallelCF_4.setSampleRate(m_nSampleRate);
	m_ParallelCF_5.setSampleRate(m_nSampleRate);
	m_ParallelCF_6.setSampleRate(m_nSampleRate);
	m_ParallelCF_7.setSampleRate(m_nSampleRate);
	m_ParallelCF_8.setSampleRate(m_nSampleRate);

	// set delays first...
	m_ParallelCF_1.setDelay_mSec(m_fPComb_1_Delay_mSec);
	m_ParallelCF_2.setDelay_mSec(m_fPComb_2_Delay_mSec);
	m_ParallelCF_3.setDelay_mSec(m_fPComb_3_Delay_mSec);
	m_ParallelCF_4.setDelay_mSec(m_fPComb_4_Delay_mSec);
	m_ParallelCF_5.setDelay_mSec(m_fPComb_5_Delay_mSec);
	m_ParallelCF_6.setDelay_mSec(m_fPComb_6_Delay_mSec);
	m_ParallelCF_7.setDelay_mSec(m_fPComb_7_Delay_mSec);
	m_ParallelCF_8.setDelay_mSec(m_fPComb_8_Delay_mSec);

	// ...then calcualte comb g's from RT60:
	m_ParallelCF_1.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_2.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_3.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_4.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_5.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_6.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_7.setComb_g_with_RTSixty(m_fRT60);
	m_ParallelCF_8.setComb_g_with_RTSixty(m_fRT60);

	// LPFs
    m_InputLPF.setLPF_g(m_fInputLPF_g);
	
	//bitcrusher stuff
	m_fFeedback = m_f_Feedback_pct / 100.0;
	m_fWetLevel = m_fWetLevel_pct / 100.0;
	m_fDelayInSamples = ((float)m_nSampleRate / (m_fBitRate));

	// subtract to make read index
	m_nReadIndex = m_nWriteIndex - (int)m_fDelayInSamples; // cast as int!

														   //  the check and wrap BACKWARDS if the index is negative
	if (m_nReadIndex < 0)
		m_nReadIndex += m_nBufferSize;	// amount of wrap is Read + Length

}

float CReverb::calculateDelayOffset(float fLFOSample)
{
	if (m_uModType == Flanger || m_uModType == Vibrato)
	{
		// flanger 0->1 gets mapped to 0->maxdelay
		return (m_fModDepth_pct / 100.0)*(fLFOSample*(m_fMaxDelay_mSec - m_fMinDelay_mSec)) + m_fMinDelay_mSec;
	}
	else if (m_uModType == Chorus)
	{
		// chorus adds starting offset to move delay range
		float fStart = m_fMinDelay_mSec + m_fChorusOffset;

		return (m_fModDepth_pct / 100.0)*(fLFOSample*(m_fMaxDelay_mSec - m_fMinDelay_mSec)) + fStart;
	}
	else if (m_uModType == Mixed)
	{
		float fStart = m_fMinDelay_mSec + m_fChorusOffset;
		return (m_fModDepth_pct / 100.0)*(fLFOSample*(m_fMaxDelay_mSec - m_fMinDelay_mSec)) + m_fMinDelay_mSec;
	}
}

void CReverb::resetDelay()
{
	// flush buffer	
	if (m_pBuffer)
		memset(m_pBuffer, 0, m_nBufferSize * sizeof(float));

	// init read/write indices
	m_nWriteIndex = 0; // reset the Write index to top
	m_nReadIndex = 0; // reset the Read index to top
}

/* prepareForPlay()
	Called by the client after Play() is initiated but before audio streams

	You can perform buffer flushes and per-run intializations.
	You can check the following variables and use them if needed:

	m_nNumWAVEChannels;
	m_nSampleRate;
	m_nBitDepth;

	NOTE: the above values are only valid during prepareForPlay() and
		  processAudioFrame() because the user might change to another wave file,
		  or use the sound card, oscillators, or impulse response mechanisms

    NOTE: if you allocte memory in this function, destroy it in ::destroy() above
*/
bool __stdcall CReverb::prepareForPlay()
{
	// Add your code here:
	// up to 2 seconds predelay
	m_PreDelay.init(2.0*(m_nSampleRate));

	// init up to 100 mSec
	m_InputAPF_1.init(0.1*(m_nSampleRate));
	m_InputAPF_2.init(0.1*(m_nSampleRate));
	m_InputAPF_3.init(0.1*(m_nSampleRate));
	m_InputAPF_4.init(0.1*(m_nSampleRate));
	m_InputAPF_5.init(0.1*(m_nSampleRate));
	m_InputAPF_6.init(0.1*(m_nSampleRate));
	m_InputAPF_7.init(0.1*(m_nSampleRate));
	m_InputAPF_8.init(0.1*(m_nSampleRate));

	// 2 seconds max
	m_LSum1.init(2.0*(m_nSampleRate));
	m_RSum1.init(2.0*(m_nSampleRate));
	m_LSum2.init(2.0*(m_nSampleRate));
	m_RSum2.init(2.0*(m_nSampleRate));
	m_LSum3.init(2.0*(m_nSampleRate));
	m_RSum3.init(2.0*(m_nSampleRate));
	m_LSum4.init(2.0*(m_nSampleRate));
	m_RSum4.init(2.0*(m_nSampleRate));
	m_Tap1.init(2.0*(m_nSampleRate));
	m_Tap2.init(2.0*(m_nSampleRate));
	m_Tap3.init(2.0*(m_nSampleRate));
	m_Tap4.init(2.0*(m_nSampleRate));


	// 100 mSec each max
	m_ParallelCF_1.init(0.1*(m_nSampleRate));
	m_ParallelCF_2.init(0.1*(m_nSampleRate));
	m_ParallelCF_3.init(0.1*(m_nSampleRate));
	m_ParallelCF_4.init(0.1*(m_nSampleRate));
	m_ParallelCF_5.init(0.1*(m_nSampleRate));
	m_ParallelCF_6.init(0.1*(m_nSampleRate));
	m_ParallelCF_7.init(0.1*(m_nSampleRate));
	m_ParallelCF_8.init(0.1*(m_nSampleRate));

	// init the three LPFs
	m_InputLPF.init();

	// Call all delay resets
	m_PreDelay.resetDelay();
	m_InputAPF_1.resetDelay();
	m_InputAPF_2.resetDelay();

	m_ParallelCF_1.resetDelay();
	m_ParallelCF_2.resetDelay();
	m_ParallelCF_3.resetDelay();
	m_ParallelCF_4.resetDelay();
	m_ParallelCF_5.resetDelay();
	m_ParallelCF_6.resetDelay();
	m_ParallelCF_7.resetDelay();
	m_ParallelCF_8.resetDelay();

	// set sample rates on combs (needed to calc g values)
	m_ParallelCF_1.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_2.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_3.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_4.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_5.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_6.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_7.setSampleRate(this->m_nSampleRate);
	m_ParallelCF_8.setSampleRate(this->m_nSampleRate);

	// flush buffers
	m_InputLPF.init();
	m_DampingLPF1.init();
	m_DampingLPF2.init();

	// cook everything
	cookVariables();

	// setup our delay line
	m_nBufferSize = (2 * m_fDelayInSamples);	// 2 seconds delay @ fs

												// delete it if it exists
	if (m_pBuffer)
		delete[] m_pBuffer;

	// create the new buffer
	m_pBuffer = new float[m_nBufferSize];

	// --- init the delay lines for 300mSec max delay - note this is sample-rate-dependent
	//     see the function - this will also flush the delay line
	m_LeftDelay.init(0.300 * m_nSampleRate);
	m_RightDelay.init(0.300 * m_nSampleRate);

	// --- set the sample rate
	m_LeftDelay.setSampleRate(m_nSampleRate);
	m_RightDelay.setSampleRate(m_nSampleRate);

	m_LFO.prepareForPlay();

	// DDL needs to know sample rate to initialize its buffer
	m_DDL.m_nSampleRate = m_nSampleRate;
	m_DDL.prepareForPlay();

	m_LFO.m_uPolarity = 1;   // 0 = bipolar, 1 = unipolar
	m_LFO.m_uTableMode = 0;  // normal, no band limiting
	m_LFO.reset();		  // reset it

						  // initialize
	cookModType();
	updateLFO();
	updateDDL();

	// start the LFO!
	m_LFO.m_bNoteOn = true;

	// reset 
	resetDelay();

	// cook
	cookVariables();



	return CPlugIn::prepareForPlay(); // --- this sets up parameter smoothing, if enabled in RackAFX - DO NOT ALTER
}

float CReverb::calculateModDelayTime_mSec(float modValue)
{
	return modValue*(maxDelayMod_mSec - minDelayMod_mSec) + minDelayMod_mSec;
}

float CReverb::doNormalDelay(float xn, float delay_mSec, float feedback, UINT channel)
{
	// --- 1) read delay decode channel
	float delay = channel == LEFT ? m_LeftDelay.readDelay_mSec(delay_mSec) : m_RightDelay.readDelay_mSec(delay_mSec);

	// --- 2) form feedback = input + fb*delay
	float delayInput = xn + (feedback / 100.f)*delay;

	// --- 3) write input to delay line
	if (channel == LEFT)
		m_LeftDelay.writeDelay(delayInput);
	else
		m_RightDelay.writeDelay(delayInput);

	return delay;
}

//bitcrusher
bool __stdcall CReverb::bcrush(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels, float downSample, float pitch, float bitdepth, float wet) 
{
	float xn = pInputBuffer[0];
	float f_xnL2 = pInputBuffer[0];
	float f_xnR2 = pInputBuffer[1];

	float ynL2 = 0.0;
	float ynR2 = 0.0;

	float ynR = 0.0;

	// Read the output of the delay at m_nReadIndex
	float yn = m_pBuffer[m_nReadIndex];

	// if delay < 1 sample, interpolate between input x(n) and x(n-1)
	if (m_nReadIndex == m_nWriteIndex && m_fDelayInSamples < 1.00)
	{
		// interpolate x(n) with x(n-1), set yn = xn
		yn = xn;
	}

	// Read the location ONE BEHIND yn at y(n-1)
	int nReadIndex_1 = m_nReadIndex - downSample;
	if (nReadIndex_1 < 0)
		nReadIndex_1 = m_nBufferSize - downSample; // m_nBufferSize-1 is last location

														// get y(n-1)
	float yn_1 = m_pBuffer[nReadIndex_1];

	// interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
	float fFracDelay = m_fDelayInSamples - (int)m_fDelayInSamples;

	// linerp: x1, x2, y1, y2, x
	float fInterp = dLinTerp(0, 1, yn, yn_1, fFracDelay); // interp frac between them

														  // if zero delay, just pass the input to output
	if (m_fDelayInSamples == 0)
		yn = xn;
	else
		yn = fInterp;

	// write the input to the delay
	if (!m_bUseExternalFeedback)
		m_pBuffer[m_nWriteIndex] = xn + pitch*yn; // normal
	else
		m_pBuffer[m_nWriteIndex] = xn + m_fFeedbackIn; // external feedback sample

													   // write the input to the delay
	m_pBuffer[m_nWriteIndex] = xn + pitch*yn;
	if (bitdepth == 32) {
		yn = yn;
	}
	else
	{
		double k = pow(2.0, bitdepth) - 1;
		float q = (long)(f_xnL2 * k);
		float q2 = (long)(f_xnR2 * k);
		ynL2 = (float)q / k;
		ynR2 = (float)q2 / k;
		yn = (long)(yn *k);
		yn = (float)yn / k;
		pOutputBuffer[0] = yn;
	}
	return true;
}


/* processAudioFrame

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

LEFT INPUT = pInputBuffer[0];
RIGHT INPUT = pInputBuffer[1]

LEFT INPUT = pInputBuffer[0]
LEFT OUTPUT = pOutputBuffer[1]

*/
bool __stdcall CReverb::processAudioFrame(float* pInputBuffer, float* pOutputBuffer, UINT uNumInputChannels, UINT uNumOutputChannels)
{
	// --- smooth params (if enabled) DO NOT REMOVE THIS CODE
	smoothParameterValues();
	if (m_uSwitch1 == SWITCH_ON)
	{
		if (m_uSwitch4 == SWITCH_ON && m_uSwitch2 == SWITCH_ON) //bcrush and delay and reverb
		{
			float xn = pInputBuffer[0];
			float f_xnL2 = pInputBuffer[0];
			float f_xnR2 = pInputBuffer[1];

			float ynL2 = 0.0;
			float ynR2 = 0.0;

			float ynR = 0.0;

			// Read the output of the delay at m_nReadIndex
			float yn = m_pBuffer[m_nReadIndex];

			// if delay < 1 sample, interpolate between input x(n) and x(n-1)
			if (m_nReadIndex == m_nWriteIndex && m_fDelayInSamples < 1.00)
			{
				// interpolate x(n) with x(n-1), set yn = xn
				yn = xn;
			}

			// Read the location ONE BEHIND yn at y(n-1)
			int nReadIndex_1 = m_nReadIndex - m_fDownSampling;
			if (nReadIndex_1 < 0)
				nReadIndex_1 = m_nBufferSize - m_fDownSampling; // m_nBufferSize-1 is last location

																// get y(n-1)
			float yn_1 = m_pBuffer[nReadIndex_1];

			// interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
			float fFracDelay = m_fDelayInSamples - (int)m_fDelayInSamples;

			// linerp: x1, x2, y1, y2, x
			float fInterp = dLinTerp(0, 1, yn, yn_1, fFracDelay); // interp frac between them

																  // if zero delay, just pass the input to output
			if (m_fDelayInSamples == 0)
				yn = xn;
			else
				yn = fInterp;

			// write the input to the delay
			if (!m_bUseExternalFeedback)
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn; // normal
			else
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedbackIn; // external feedback sample

															   // write the input to the delay
			m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn;
			if (m_fBitDepth == 32) {
				yn = yn;
			}
			else
			{
				double k = pow(2.0, m_fBitDepth) - 1;
				float q = (long)(f_xnL2 * k);
				float q2 = (long)(f_xnR2 * k);
				ynL2 = (float)q / k;
				ynR2 = (float)q2 / k;
				yn = (long)(yn *k);
				yn = (float)yn / k;
				pInputBuffer[0] = ynL2;
				pInputBuffer[1] = ynR2;
			}
			pInputBuffer[0] = yn;
			pInputBuffer[1] = yn;
			float fYn = 0;
			float fYqn = 0;
			m_LFO.doOscillate(&fYn, &fYqn);

			// 2. calculate delay offset
			float fDelay = 0.0;
			if (m_uLFOPhase == Quad)
				fDelay = calculateDelayOffset(fYqn); // quadrature LFO
			else
				fDelay = calculateDelayOffset(fYn); // normal LFO

													// 3. set the delay & cook
			m_DDL.m_fDelay_ms = fDelay;
			m_DDL.cookVariables();

			// 4. get the delay output one channel in/one channel out
			m_DDL.processAudioFrame(&pInputBuffer[0], &pOutputBuffer[0], 1, 1);
			m_DDL.processAudioFrame(&pInputBuffer[1], &pOutputBuffer[1], 1, 1);

			float B_D1 = 0;
			float B_D2 = 0;
			B_D1 = pOutputBuffer[0];
			B_D2 = pOutputBuffer[1];

			// begin the series/parallel signal push
			// needed for starting push
			float krt4 = 0;

			//output->input
			float Out_In = 0;
			m_Sum.processAudio(&krt4, &Out_In);

			// Pre-Delay
			float fPreDelayOut = 0;
			m_PreDelay.processAudio(&B_D1, &fPreDelayOut);
			fPreDelayOut = fPreDelayOut + Out_In;

			// Pre-Delay Out -> fAPF_1_Out
			float fAPF_1_Out = 0;
			m_InputAPF_1.processAudio(&fPreDelayOut, &fAPF_1_Out);

			// fAPF_1_Out -> fAPF_2_Out
			float fAPF_2_Out = 0;
			m_InputAPF_2.processAudio(&fAPF_1_Out, &fAPF_2_Out);

			// fAPF_2_Out -> fInputLPF
			float fInputLPF = 0;
			m_InputLPF.processAudio(&fAPF_2_Out, &fInputLPF);

			// comb filter bank
			// variables for each output
			float fPC_1_Out = 0;
			float fPC_2_Out = 0;
			float fPC_3_Out = 0;
			float fPC_4_Out = 0;
			float fPC_5_Out = 0;
			float fPC_6_Out = 0;
			float fPC_7_Out = 0;
			float fPC_8_Out = 0;
			float fC1_Out = 0;
			float fC2_Out = 0;

			// fInputLPF -> fPC_1_Out, fPC_2_Out, fPC_3_Out, fPC_4_Out
			m_ParallelCF_1.processAudio(&fInputLPF, &fPC_1_Out);
			m_ParallelCF_2.processAudio(&fInputLPF, &fPC_2_Out);
			m_ParallelCF_3.processAudio(&fInputLPF, &fPC_3_Out);
			m_ParallelCF_4.processAudio(&fInputLPF, &fPC_4_Out);

			// fInputLPF -> fPC_5_Out, fPC_6_Out, fPC_7_Out, fPC_8_Out
			m_ParallelCF_5.processAudio(&fInputLPF, &fPC_5_Out);
			m_ParallelCF_6.processAudio(&fInputLPF, &fPC_6_Out);
			m_ParallelCF_7.processAudio(&fInputLPF, &fPC_7_Out);
			m_ParallelCF_8.processAudio(&fInputLPF, &fPC_8_Out);

			// form outputs: note attenuation by 0.25 for each and alternating signs
			fC1_Out = 0.25*fPC_1_Out - 0.25*fPC_2_Out + 0.25*fPC_3_Out - 0.25*fPC_4_Out;
			fC2_Out = 0.25*fPC_5_Out - 0.25*fPC_6_Out + 0.25*fPC_7_Out - 0.25*fPC_8_Out;

			//APF_2_Out -> CombFilters
			float fFiltered1 = 0;
			C_A.processAudio(&fInputLPF, &fC1_Out, &fFiltered1);

			// fAPF_2_Out -> dSum
			float fdSumL1 = 0;
			float fdSumR1 = 0;
			float fdSum1 = 0;
			m_LSum1.processAudio(&fFiltered1, &fdSumL1);
			m_RSum1.processAudio(&fFiltered1, &fdSumR1);
			m_Tap1.processAudio(&fdSumL1, &fdSumR1, &fdSum1);

			//dSum -> KRT(RT60)
			float krt1 = 0;
			m_KRT1.processAudio(&fdSum1, m_fKRT, &krt1);

			// KRT1 -> APF3
			float fAPF_3_Out = 0;
			m_InputAPF_3.processAudio(&krt1, &fAPF_3_Out);

			// fAPF_3_Out -> fAPF_4_Out
			float fAPF_4_Out = 0;
			m_InputAPF_4.processAudio(&fAPF_3_Out, &fAPF_4_Out);

			//APF_2_Out -> CombFilters
			float fFiltered2 = 0;
			C_A.processAudio(&fAPF_4_Out, &fC2_Out, &fFiltered2);

			// fAPF_4_Out -> dSum
			float fdSumL2 = 0;
			float fdSumR2 = 0;
			float fdSum2 = 0;
			m_LSum2.processAudio(&fFiltered2, &fdSumL2);
			m_RSum2.processAudio(&fFiltered2, &fdSumR2);
			m_Tap1.processAudio(&fdSumL2, &fdSumR2, &fdSum2);

			//dSum2 -> KRT2(RT60)
			float krt2 = 0;
			m_KRT1.processAudio(&fdSum2, m_fKRT, &krt2);

			// KRT2 -> APF5
			float fAPF_5_Out = 0;
			m_InputAPF_5.processAudio(&krt2, &fAPF_5_Out);

			// fAPF_5_Out -> fAPF_6_Out
			float fAPF_6_Out = 0;
			m_InputAPF_6.processAudio(&fAPF_5_Out, &fAPF_6_Out);

			//APF_6_Out -> CombFilters
			float fFiltered3 = 0;
			C_A.processAudio(&fAPF_6_Out, &fC1_Out, &fFiltered3);

			// fAPF_4_Out -> dSum
			float fdSumL3 = 0;
			float fdSumR3 = 0;
			float fdSum3 = 0;
			m_LSum2.processAudio(&fFiltered3, &fdSumL3);
			m_RSum2.processAudio(&fFiltered3, &fdSumR3);
			m_Tap1.processAudio(&fdSumL3, &fdSumR3, &fdSum3);

			//dSum3 -> KRT3(RT60)
			float krt3 = 0;
			m_KRT1.processAudio(&fdSum3, m_fKRT, &krt3);

			// KRT3 -> APF7
			float fAPF_7_Out = 0;
			m_InputAPF_7.processAudio(&krt3, &fAPF_7_Out);

			// fAPF_7_Out -> fAPF_8_Out
			float fAPF_8_Out = 0;
			m_InputAPF_8.processAudio(&fAPF_7_Out, &fAPF_8_Out);

			//APF_8_Out -> CombFilters
			float fFiltered4 = 0;
			C_A.processAudio(&fAPF_8_Out, &fC1_Out, &fFiltered4);

			// fAPF_8_Out -> dSum
			float fdSumL4 = 0;
			float fdSumR4 = 0;
			float fdSum4 = 0;
			m_LSum2.processAudio(&fFiltered4, &fdSumL4);
			m_RSum2.processAudio(&fFiltered4, &fdSumR4);
			m_Tap1.processAudio(&fdSumL4, &fdSumR4, &fdSum4);

			//dSum3 -> KRT3(RT60)
			krt4 = 0;
			m_KRT1.processAudio(&fdSum4, m_fKRT, &krt4);

			float L_Output = (0.25 * fdSumL1) + (0.25 * fdSumL2) + (0.25 * fdSumL3) + (0.25 * fdSumL4);
			float R_Output = (0.25 * fdSumR1) + (0.25 * fdSumR2) + (0.25 * fdSumR3) + (0.25 * fdSumR4);
			float M_Output = (0.25 * fdSum1) + (0.25 * fdSum2) + (0.25 * fdSum3) + (0.25 * fdSum4);

			// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_3_Out
			pOutputBuffer[0] = ((100.0 - m_fWet_pct) / 100.0)*B_D1 +
				(m_fWet_pct / 100.0)*(L_Output);

			// Do RIGHT Channel if there is one
			if (uNumOutputChannels == 2)
			{
				// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_4_Out
				pOutputBuffer[1] = ((100.0 - m_fWet_pct) / 100.0)*B_D2 +
					(m_fWet_pct / 100.0)*(R_Output);
				m_nWriteIndex++;
				if (m_nWriteIndex >= m_nBufferSize)
					m_nWriteIndex = 0;

				m_nReadIndex++;
				if (m_nReadIndex >= m_nBufferSize)
					m_nReadIndex = 0;
			}
			return true;
		}
		else if (m_uSwitch4 == SWITCH_ON) //delay and bitcrusher
		{
			float xn = pInputBuffer[0];
			float f_xnL2 = pInputBuffer[0];
			float f_xnR2 = pInputBuffer[1];

			float ynL2 = 0.0;
			float ynR2 = 0.0;

			float ynR = 0.0;

			// Read the output of the delay at m_nReadIndex
			float yn = m_pBuffer[m_nReadIndex];

			// if delay < 1 sample, interpolate between input x(n) and x(n-1)
			if (m_nReadIndex == m_nWriteIndex && m_fDelayInSamples < 1.00)
			{
				// interpolate x(n) with x(n-1), set yn = xn
				yn = xn;
			}

			// Read the location ONE BEHIND yn at y(n-1)
			int nReadIndex_1 = m_nReadIndex - m_fDownSampling;
			if (nReadIndex_1 < 0)
				nReadIndex_1 = m_nBufferSize - m_fDownSampling; // m_nBufferSize-1 is last location

																// get y(n-1)
			float yn_1 = m_pBuffer[nReadIndex_1];

			// interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
			float fFracDelay = m_fDelayInSamples - (int)m_fDelayInSamples;

			// linerp: x1, x2, y1, y2, x
			float fInterp = dLinTerp(0, 1, yn, yn_1, fFracDelay); // interp frac between them

																  // if zero delay, just pass the input to output
			if (m_fDelayInSamples == 0)
				yn = xn;
			else
				yn = fInterp;

			// write the input to the delay
			if (!m_bUseExternalFeedback)
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn; // normal
			else
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedbackIn; // external feedback sample

															   // write the input to the delay
			m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn;
			if (m_fBitDepth == 32) {
				yn = yn;
			}
			else
			{
				double k = pow(2.0, m_fBitDepth) - 1;
				float q = (long)(f_xnL2 * k);
				float q2 = (long)(f_xnR2 * k);
				ynL2 = (float)q / k;
				ynR2 = (float)q2 / k;
				yn = (long)(yn *k);
				yn = (float)yn / k;
			}
			pInputBuffer[0] = yn;
			pInputBuffer[1] = yn;
			float fYn = 0;
			float fYqn = 0;
			m_LFO.doOscillate(&fYn, &fYqn);

			// 2. calculate delay offset
			float fDelay = 0.0;
			if (m_uLFOPhase == Quad)
				fDelay = calculateDelayOffset(fYqn); // quadrature LFO
			else
				fDelay = calculateDelayOffset(fYn); // normal LFO

													// 3. set the delay & cook
			m_DDL.m_fDelay_ms = fDelay;
			m_DDL.cookVariables();

			// 4. get the delay output one channel in/one channel out
			m_DDL.processAudioFrame(&pInputBuffer[0], &pOutputBuffer[0], 1, 1);

			// Mono-In, Stereo-Out (AUX Effect)
			if (uNumInputChannels == 1 && uNumOutputChannels == 2)
				pOutputBuffer[1] = pOutputBuffer[0];

			// Stereo-In, Stereo-Out (INSERT Effect)
			if (uNumInputChannels == 2 && uNumOutputChannels == 2)
				m_DDL.processAudioFrame(&pInputBuffer[1], &pOutputBuffer[1], 1, 1);
			pOutputBuffer[1] = pOutputBuffer[1];

			m_nWriteIndex++;
			if (m_nWriteIndex >= m_nBufferSize)
				m_nWriteIndex = 0;

			m_nReadIndex++;
			if (m_nReadIndex >= m_nBufferSize)
				m_nReadIndex = 0;

			return true;
		}
		if (m_uSwitch2 == SWITCH_ON) // bCrush and Reverb
		{
			float xn = pInputBuffer[0];
			float f_xnL2 = pInputBuffer[0];
			float f_xnR2 = pInputBuffer[1];

			float ynL2 = 0.0;
			float ynR2 = 0.0;

			float ynR = 0.0;

			// Read the output of the delay at m_nReadIndex
			float yn = m_pBuffer[m_nReadIndex];

			// if delay < 1 sample, interpolate between input x(n) and x(n-1)
			if (m_nReadIndex == m_nWriteIndex && m_fDelayInSamples < 1.00)
			{
				// interpolate x(n) with x(n-1), set yn = xn
				yn = xn;
			}

			// Read the location ONE BEHIND yn at y(n-1)
			int nReadIndex_1 = m_nReadIndex - m_fDownSampling;
			if (nReadIndex_1 < 0)
				nReadIndex_1 = m_nBufferSize - m_fDownSampling; // m_nBufferSize-1 is last location

														   // get y(n-1)
			float yn_1 = m_pBuffer[nReadIndex_1];

			// interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
			float fFracDelay = m_fDelayInSamples - (int)m_fDelayInSamples;

			// linerp: x1, x2, y1, y2, x
			float fInterp = dLinTerp(0, 1, yn, yn_1, fFracDelay); // interp frac between them

																  // if zero delay, just pass the input to output
			if (m_fDelayInSamples == 0)
				yn = xn;
			else
				yn = fInterp;

			// write the input to the delay
	
			
			if (!m_bUseExternalFeedback)
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn; // normal
			else
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedbackIn; // external feedback sample

															   // write the input to the delay
			m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn;

															   // write the input to the del
			if (m_fBitDepth == 32) {
				yn = yn;
			}
			else
			{
				double k = pow(2.0, m_fBitDepth) - 1;
				float q = (long)(f_xnL2 * k);
				float q2 = (long)(f_xnR2 * k);
				ynL2 = (float)q / k;
				ynR2 = (float)q2 / k;
				yn = (long)(yn *k);
				yn = (float)yn / k;

			}

			// create the wet/dry mix and write to the output buffer
			// dry = 1 - wet

			float B_R = 0;
			B_R = yn;

			// incremnent the pointers and wrap if necessary

			// begin the series/parallel signal push
			// needed for starting push
			float krt4 = 0;

			//output->input
			float Out_In = 0;
			m_Sum.processAudio(&krt4, &Out_In);
			Out_In = B_R + Out_In;
			B_R = 0;

			// Pre-Delay
			float fPreDelayOut = 0;
			m_PreDelay.processAudio(&Out_In, &fPreDelayOut);
			fPreDelayOut = fPreDelayOut + Out_In;

			// Pre-Delay Out -> fAPF_1_Out
			float fAPF_1_Out = 0;
			m_InputAPF_1.processAudio(&fPreDelayOut, &fAPF_1_Out);

			// fAPF_1_Out -> fAPF_2_Out
			float fAPF_2_Out = 0;
			m_InputAPF_2.processAudio(&fAPF_1_Out, &fAPF_2_Out);

			// fAPF_2_Out -> fInputLPF
			float fInputLPF = 0;
			m_InputLPF.processAudio(&fAPF_2_Out, &fInputLPF);

			// comb filter bank
			// variables for each output
			float fPC_1_Out = 0;
			float fPC_2_Out = 0;
			float fPC_3_Out = 0;
			float fPC_4_Out = 0;
			float fPC_5_Out = 0;
			float fPC_6_Out = 0;
			float fPC_7_Out = 0;
			float fPC_8_Out = 0;
			float fC1_Out = 0;
			float fC2_Out = 0;

			// fInputLPF -> fPC_1_Out, fPC_2_Out, fPC_3_Out, fPC_4_Out
			m_ParallelCF_1.processAudio(&fInputLPF, &fPC_1_Out);
			m_ParallelCF_2.processAudio(&fInputLPF, &fPC_2_Out);
			m_ParallelCF_3.processAudio(&fInputLPF, &fPC_3_Out);
			m_ParallelCF_4.processAudio(&fInputLPF, &fPC_4_Out);

			// fInputLPF -> fPC_5_Out, fPC_6_Out, fPC_7_Out, fPC_8_Out
			m_ParallelCF_5.processAudio(&fInputLPF, &fPC_5_Out);
			m_ParallelCF_6.processAudio(&fInputLPF, &fPC_6_Out);
			m_ParallelCF_7.processAudio(&fInputLPF, &fPC_7_Out);
			m_ParallelCF_8.processAudio(&fInputLPF, &fPC_8_Out);

			// form outputs: note attenuation by 0.25 for each and alternating signs
			fC1_Out = 0.25*fPC_1_Out - 0.25*fPC_2_Out + 0.25*fPC_3_Out - 0.25*fPC_4_Out;
			fC2_Out = 0.25*fPC_5_Out - 0.25*fPC_6_Out + 0.25*fPC_7_Out - 0.25*fPC_8_Out;

			//APF_2_Out -> CombFilters
			float fFiltered1 = 0;
			C_A.processAudio(&fInputLPF, &fC1_Out, &fFiltered1);

			// fAPF_2_Out -> dSum
			float fdSumL1 = 0;
			float fdSumR1 = 0;
			float fdSum1 = 0;
			m_LSum1.processAudio(&fFiltered1, &fdSumL1);
			m_RSum1.processAudio(&fFiltered1, &fdSumR1);
			m_Tap1.processAudio(&fdSumL1, &fdSumR1, &fdSum1);

			//dSum -> KRT(RT60)
			float krt1 = 0;
			m_KRT1.processAudio(&fdSum1, m_fKRT, &krt1);

			// KRT1 -> APF3
			float fAPF_3_Out = 0;
			m_InputAPF_3.processAudio(&krt1, &fAPF_3_Out);

			// fAPF_3_Out -> fAPF_4_Out
			float fAPF_4_Out = 0;
			m_InputAPF_4.processAudio(&fAPF_3_Out, &fAPF_4_Out);

			//APF_2_Out -> CombFilters
			float fFiltered2 = 0;
			C_A.processAudio(&fAPF_4_Out, &fC2_Out, &fFiltered2);

			// fAPF_4_Out -> dSum
			float fdSumL2 = 0;
			float fdSumR2 = 0;
			float fdSum2 = 0;
			m_LSum2.processAudio(&fFiltered2, &fdSumL2);
			m_RSum2.processAudio(&fFiltered2, &fdSumR2);
			m_Tap1.processAudio(&fdSumL2, &fdSumR2, &fdSum2);

			//dSum2 -> KRT2(RT60)
			float krt2 = 0;
			m_KRT1.processAudio(&fdSum2, m_fKRT, &krt2);

			// KRT2 -> APF5
			float fAPF_5_Out = 0;
			m_InputAPF_5.processAudio(&krt2, &fAPF_5_Out);

			// fAPF_5_Out -> fAPF_6_Out
			float fAPF_6_Out = 0;
			m_InputAPF_6.processAudio(&fAPF_5_Out, &fAPF_6_Out);

			//APF_6_Out -> CombFilters
			float fFiltered3 = 0;
			C_A.processAudio(&fAPF_6_Out, &fC1_Out, &fFiltered3);

			// fAPF_4_Out -> dSum
			float fdSumL3 = 0;
			float fdSumR3 = 0;
			float fdSum3 = 0;
			m_LSum2.processAudio(&fFiltered3, &fdSumL3);
			m_RSum2.processAudio(&fFiltered3, &fdSumR3);
			m_Tap1.processAudio(&fdSumL3, &fdSumR3, &fdSum3);

			//dSum3 -> KRT3(RT60)
			float krt3 = 0;
			m_KRT1.processAudio(&fdSum3, m_fKRT, &krt3);

			// KRT3 -> APF7
			float fAPF_7_Out = 0;
			m_InputAPF_7.processAudio(&krt3, &fAPF_7_Out);

			// fAPF_7_Out -> fAPF_8_Out
			float fAPF_8_Out = 0;
			m_InputAPF_8.processAudio(&fAPF_7_Out, &fAPF_8_Out);

			//APF_8_Out -> CombFilters
			float fFiltered4 = 0;
			C_A.processAudio(&fAPF_8_Out, &fC1_Out, &fFiltered4);

			// fAPF_8_Out -> dSum
			float fdSumL4 = 0;
			float fdSumR4 = 0;
			float fdSum4 = 0;
			m_LSum2.processAudio(&fFiltered4, &fdSumL4);
			m_RSum2.processAudio(&fFiltered4, &fdSumR4);
			m_Tap1.processAudio(&fdSumL4, &fdSumR4, &fdSum4);

			//dSum3 -> KRT3(RT60)
			krt4 = 0;
			m_KRT1.processAudio(&fdSum4, m_fKRT, &krt4);

			float L_Output = (0.25 * fdSumL1) + (0.25 * fdSumL2) + (0.25 * fdSumL3) + (0.25 * fdSumL4);
			float R_Output = (0.25 * fdSumR1) + (0.25 * fdSumR2) + (0.25 * fdSumR3) + (0.25 * fdSumR4);
			float M_Output = (0.25 * fdSum1) + (0.25 * fdSum2) + (0.25 * fdSum3) + (0.25 * fdSum4);

			// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_3_Out
			pOutputBuffer[0] = ((100.0 - m_fWet_pct) / 100.0)*yn +
				(m_fWet_pct / 100.0)*(L_Output);

			// Do RIGHT Channel if there is one
			if (uNumOutputChannels == 2)
			{
				// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_4_Out
				pOutputBuffer[1] = ((100.0 - m_fWet_pct) / 100.0)*yn +
					(m_fWet_pct / 100.0)*(R_Output);
			}
				m_nWriteIndex++;
				if (m_nWriteIndex >= m_nBufferSize)
					m_nWriteIndex = 0;

				m_nReadIndex++;
				if (m_nReadIndex >= m_nBufferSize)
					m_nReadIndex = 0;
			return true;
		}
		else
		{
			float xn = pInputBuffer[0];
			float f_xnL2 = pInputBuffer[0];
			float f_xnR2 = pInputBuffer[1];

			float ynL2 = 0.0;
			float ynR2 = 0.0;

			float ynR = 0.0;

			// Read the output of the delay at m_nReadIndex
			float yn = m_pBuffer[m_nReadIndex];

			// if delay < 1 sample, interpolate between input x(n) and x(n-1)
			if (m_nReadIndex == m_nWriteIndex && m_fDelayInSamples < 1.00)
			{
				// interpolate x(n) with x(n-1), set yn = xn
				yn = xn;
			}

			// Read the location ONE BEHIND yn at y(n-1)
			int nReadIndex_1 = m_nReadIndex - m_fDownSampling;
			if (nReadIndex_1 < 0)
				nReadIndex_1 = m_nBufferSize - m_fDownSampling; // m_nBufferSize-1 is last location

																// get y(n-1)
			float yn_1 = m_pBuffer[nReadIndex_1];

			// interpolate: (0, yn) and (1, yn_1) by the amount fracDelay
			float fFracDelay = m_fDelayInSamples - (int)m_fDelayInSamples;

			// linerp: x1, x2, y1, y2, x
			float fInterp = dLinTerp(0, 1, yn, yn_1, fFracDelay); // interp frac between them

																  // if zero delay, just pass the input to output
			if (m_fDelayInSamples == 0)
				yn = xn;
			else
				yn = fInterp;

			// write the input to the delay
			if (!m_bUseExternalFeedback)
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn; // normal
			else
				m_pBuffer[m_nWriteIndex] = xn + m_fFeedbackIn; // external feedback sample

															   // write the input to the delay
			m_pBuffer[m_nWriteIndex] = xn + m_fFeedback*yn;
			if (m_fBitDepth == 32) {
				yn = yn;
			}
			else
			{
				double k = pow(2.0, m_fBitDepth) - 1;
				float q = (long)(f_xnL2 * k);
				float q2 = (long)(f_xnR2 * k);
				ynL2 = (float)q / k;
				ynR2 = (float)q2 / k;
				yn = (long)(yn *k);
				yn = (float)yn / k;
			}


			// create the wet/dry mix and write to the output buffer
			// dry = 1 - wet

			float fInputLPF = m_fWetLevel*yn + (1.0 - m_fWetLevel)*xn;
			float out = 0;
			m_InputLPF.processAudio(&fInputLPF, &out);
			pOutputBuffer[0] = out;

			// incremnent the pointers and wrap if necessary
			m_nWriteIndex++;
			if (m_nWriteIndex >= m_nBufferSize)
				m_nWriteIndex = 0;

			m_nReadIndex++;
			if (m_nReadIndex >= m_nBufferSize)
				m_nReadIndex = 0;

			// Mono-In, Stereo-Out (AUX Effect)
			if (uNumInputChannels == 1 && uNumOutputChannels == 2)
				pOutputBuffer[1] = pOutputBuffer[0]; // copy MONO!

													 // DDL Module is MONO - just do a copy here too
													 // Stereo-In, Stereo-Out (INSERT Effect)
			if (uNumInputChannels == 2 && uNumOutputChannels == 2)
				pOutputBuffer[1] = pOutputBuffer[0];// copy MONO!

			return true;

		}
	}
	else if (m_uSwitch2 == SWITCH_ON) //reverb
	{
		if (m_uSwitch4 == SWITCH_ON) //delay and reverb
		{
			float fYn = 0;
			float fYqn = 0;
			m_LFO.doOscillate(&fYn, &fYqn);
			// 2. calculate delay offset
			float fDelay = 0.0;
			if (m_uLFOPhase == Quad)
				fDelay = calculateDelayOffset(fYqn); // quadrature LFO
			else
				fDelay = calculateDelayOffset(fYn); // normal LFO

													// 3. set the delay & cook
			m_DDL.m_fDelay_ms = fDelay;
			m_DDL.cookVariables();

			// 4. get the delay output one channel in/one channel out
			m_DDL.processAudioFrame(&pInputBuffer[0], &pOutputBuffer[0], 1, 1);
			m_DDL.processAudioFrame(&pInputBuffer[1], &pOutputBuffer[1], 1, 1);

			float B_D1 = 0;
			float B_D2 = 0;
			B_D1 = pOutputBuffer[0];
			B_D2 = pOutputBuffer[1];

			// begin the series/parallel signal push
			// needed for starting push
			float krt4 = 0;

			//output->input
			float Out_In = 0;
			m_Sum.processAudio(&krt4, &Out_In);

			// Pre-Delay
			float fPreDelayOut = 0;
			m_PreDelay.processAudio(&B_D1, &fPreDelayOut);
			fPreDelayOut = fPreDelayOut + Out_In;

			// Pre-Delay Out -> fAPF_1_Out
			float fAPF_1_Out = 0;
			m_InputAPF_1.processAudio(&fPreDelayOut, &fAPF_1_Out);

			// fAPF_1_Out -> fAPF_2_Out
			float fAPF_2_Out = 0;
			m_InputAPF_2.processAudio(&fAPF_1_Out, &fAPF_2_Out);

			// fAPF_2_Out -> fInputLPF
			float fInputLPF = 0;
			m_InputLPF.processAudio(&fAPF_2_Out, &fInputLPF);

			// comb filter bank
			// variables for each output
			float fPC_1_Out = 0;
			float fPC_2_Out = 0;
			float fPC_3_Out = 0;
			float fPC_4_Out = 0;
			float fPC_5_Out = 0;
			float fPC_6_Out = 0;
			float fPC_7_Out = 0;
			float fPC_8_Out = 0;
			float fC1_Out = 0;
			float fC2_Out = 0;

			// fInputLPF -> fPC_1_Out, fPC_2_Out, fPC_3_Out, fPC_4_Out
			m_ParallelCF_1.processAudio(&fInputLPF, &fPC_1_Out);
			m_ParallelCF_2.processAudio(&fInputLPF, &fPC_2_Out);
			m_ParallelCF_3.processAudio(&fInputLPF, &fPC_3_Out);
			m_ParallelCF_4.processAudio(&fInputLPF, &fPC_4_Out);

			// fInputLPF -> fPC_5_Out, fPC_6_Out, fPC_7_Out, fPC_8_Out
			m_ParallelCF_5.processAudio(&fInputLPF, &fPC_5_Out);
			m_ParallelCF_6.processAudio(&fInputLPF, &fPC_6_Out);
			m_ParallelCF_7.processAudio(&fInputLPF, &fPC_7_Out);
			m_ParallelCF_8.processAudio(&fInputLPF, &fPC_8_Out);

			// form outputs: note attenuation by 0.25 for each and alternating signs
			fC1_Out = 0.25*fPC_1_Out - 0.25*fPC_2_Out + 0.25*fPC_3_Out - 0.25*fPC_4_Out;
			fC2_Out = 0.25*fPC_5_Out - 0.25*fPC_6_Out + 0.25*fPC_7_Out - 0.25*fPC_8_Out;

			//APF_2_Out -> CombFilters
			float fFiltered1 = 0;
			C_A.processAudio(&fInputLPF, &fC1_Out, &fFiltered1);

			// fAPF_2_Out -> dSum
			float fdSumL1 = 0;
			float fdSumR1 = 0;
			float fdSum1 = 0;
			m_LSum1.processAudio(&fFiltered1, &fdSumL1);
			m_RSum1.processAudio(&fFiltered1, &fdSumR1);
			m_Tap1.processAudio(&fdSumL1, &fdSumR1, &fdSum1);

			//dSum -> KRT(RT60)
			float krt1 = 0;
			m_KRT1.processAudio(&fdSum1, m_fKRT, &krt1);

			// KRT1 -> APF3
			float fAPF_3_Out = 0;
			m_InputAPF_3.processAudio(&krt1, &fAPF_3_Out);

			// fAPF_3_Out -> fAPF_4_Out
			float fAPF_4_Out = 0;
			m_InputAPF_4.processAudio(&fAPF_3_Out, &fAPF_4_Out);

			//APF_2_Out -> CombFilters
			float fFiltered2 = 0;
			C_A.processAudio(&fAPF_4_Out, &fC2_Out, &fFiltered2);

			// fAPF_4_Out -> dSum
			float fdSumL2 = 0;
			float fdSumR2 = 0;
			float fdSum2 = 0;
			m_LSum2.processAudio(&fFiltered2, &fdSumL2);
			m_RSum2.processAudio(&fFiltered2, &fdSumR2);
			m_Tap1.processAudio(&fdSumL2, &fdSumR2, &fdSum2);

			//dSum2 -> KRT2(RT60)
			float krt2 = 0;
			m_KRT1.processAudio(&fdSum2, m_fKRT, &krt2);

			// KRT2 -> APF5
			float fAPF_5_Out = 0;
			m_InputAPF_5.processAudio(&krt2, &fAPF_5_Out);

			// fAPF_5_Out -> fAPF_6_Out
			float fAPF_6_Out = 0;
			m_InputAPF_6.processAudio(&fAPF_5_Out, &fAPF_6_Out);

			//APF_6_Out -> CombFilters
			float fFiltered3 = 0;
			C_A.processAudio(&fAPF_6_Out, &fC1_Out, &fFiltered3);

			// fAPF_4_Out -> dSum
			float fdSumL3 = 0;
			float fdSumR3 = 0;
			float fdSum3 = 0;
			m_LSum2.processAudio(&fFiltered3, &fdSumL3);
			m_RSum2.processAudio(&fFiltered3, &fdSumR3);
			m_Tap1.processAudio(&fdSumL3, &fdSumR3, &fdSum3);

			//dSum3 -> KRT3(RT60)
			float krt3 = 0;
			m_KRT1.processAudio(&fdSum3, m_fKRT, &krt3);

			// KRT3 -> APF7
			float fAPF_7_Out = 0;
			m_InputAPF_7.processAudio(&krt3, &fAPF_7_Out);

			// fAPF_7_Out -> fAPF_8_Out
			float fAPF_8_Out = 0;
			m_InputAPF_8.processAudio(&fAPF_7_Out, &fAPF_8_Out);

			//APF_8_Out -> CombFilters
			float fFiltered4 = 0;
			C_A.processAudio(&fAPF_8_Out, &fC1_Out, &fFiltered4);

			// fAPF_8_Out -> dSum
			float fdSumL4 = 0;
			float fdSumR4 = 0;
			float fdSum4 = 0;
			m_LSum2.processAudio(&fFiltered4, &fdSumL4);
			m_RSum2.processAudio(&fFiltered4, &fdSumR4);
			m_Tap1.processAudio(&fdSumL4, &fdSumR4, &fdSum4);

			//dSum3 -> KRT3(RT60)
			krt4 = 0;
			m_KRT1.processAudio(&fdSum4, m_fKRT, &krt4);

			float L_Output = (0.25 * fdSumL1) + (0.25 * fdSumL2) + (0.25 * fdSumL3) + (0.25 * fdSumL4);
			float R_Output = (0.25 * fdSumR1) + (0.25 * fdSumR2) + (0.25 * fdSumR3) + (0.25 * fdSumR4);
			float M_Output = (0.25 * fdSum1) + (0.25 * fdSum2) + (0.25 * fdSum3) + (0.25 * fdSum4);

			// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_3_Out
			pOutputBuffer[0] = ((100.0 - m_fWet_pct) / 100.0)*B_D1 +
				(m_fWet_pct / 100.0)*(L_Output);

			// Do RIGHT Channel if there is one
			if (uNumOutputChannels == 2)
			{
				// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_4_Out
				pOutputBuffer[1] = ((100.0 - m_fWet_pct) / 100.0)*B_D2 +
					(m_fWet_pct / 100.0)*(R_Output);
				m_nWriteIndex++;
				if (m_nWriteIndex >= m_nBufferSize)
					m_nWriteIndex = 0;

				m_nReadIndex++;
				if (m_nReadIndex >= m_nBufferSize)
					m_nReadIndex = 0;
			}
			return true;
		}
		else
		{
			float fInputSample = pInputBuffer[0];
			if (uNumInputChannels == 2)
			{
				// mix
				fInputSample += pInputBuffer[1];

				// attenuate by 0.5
				fInputSample *= 0.5;
			}

			// begin the series/parallel signal push
			// needed for starting push
			float krt4 = 0;

			//output->input
			float Out_In = 0;
			m_Sum.processAudio(&krt4, &Out_In);

			// Pre-Delay
			float fPreDelayOut = 0;
			m_PreDelay.processAudio(&fInputSample, &fPreDelayOut);
			fPreDelayOut = fPreDelayOut + Out_In;

			// Pre-Delay Out -> fAPF_1_Out
			float fAPF_1_Out = 0;
			m_InputAPF_1.processAudio(&fPreDelayOut, &fAPF_1_Out);

			// fAPF_1_Out -> fAPF_2_Out
			float fAPF_2_Out = 0;
			m_InputAPF_2.processAudio(&fAPF_1_Out, &fAPF_2_Out);

			// fAPF_2_Out -> fInputLPF
			float fInputLPF = 0;
			m_InputLPF.processAudio(&fAPF_2_Out, &fInputLPF);

			// comb filter bank
			// variables for each output
			float fPC_1_Out = 0;
			float fPC_2_Out = 0;
			float fPC_3_Out = 0;
			float fPC_4_Out = 0;
			float fPC_5_Out = 0;
			float fPC_6_Out = 0;
			float fPC_7_Out = 0;
			float fPC_8_Out = 0;
			float fC1_Out = 0;
			float fC2_Out = 0;

			// fInputLPF -> fPC_1_Out, fPC_2_Out, fPC_3_Out, fPC_4_Out
			m_ParallelCF_1.processAudio(&fInputLPF, &fPC_1_Out);
			m_ParallelCF_2.processAudio(&fInputLPF, &fPC_2_Out);
			m_ParallelCF_3.processAudio(&fInputLPF, &fPC_3_Out);
			m_ParallelCF_4.processAudio(&fInputLPF, &fPC_4_Out);

			// fInputLPF -> fPC_5_Out, fPC_6_Out, fPC_7_Out, fPC_8_Out
			m_ParallelCF_5.processAudio(&fInputLPF, &fPC_5_Out);
			m_ParallelCF_6.processAudio(&fInputLPF, &fPC_6_Out);
			m_ParallelCF_7.processAudio(&fInputLPF, &fPC_7_Out);
			m_ParallelCF_8.processAudio(&fInputLPF, &fPC_8_Out);

			// form outputs: note attenuation by 0.25 for each and alternating signs
			fC1_Out = 0.25*fPC_1_Out - 0.25*fPC_2_Out + 0.25*fPC_3_Out - 0.25*fPC_4_Out;
			fC2_Out = 0.25*fPC_5_Out - 0.25*fPC_6_Out + 0.25*fPC_7_Out - 0.25*fPC_8_Out;

			//APF_2_Out -> CombFilters
			float fFiltered1 = 0;
			C_A.processAudio(&fInputLPF, &fC1_Out, &fFiltered1);

			// fAPF_2_Out -> dSum
			float fdSumL1 = 0;
			float fdSumR1 = 0;
			float fdSum1 = 0;
			m_LSum1.processAudio(&fFiltered1, &fdSumL1);
			m_RSum1.processAudio(&fFiltered1, &fdSumR1);
			m_Tap1.processAudio(&fdSumL1, &fdSumR1, &fdSum1);

			//dSum -> KRT(RT60)
			float krt1 = 0;
			m_KRT1.processAudio(&fdSum1, m_fKRT, &krt1);

			// KRT1 -> APF3
			float fAPF_3_Out = 0;
			m_InputAPF_3.processAudio(&krt1, &fAPF_3_Out);

			// fAPF_3_Out -> fAPF_4_Out
			float fAPF_4_Out = 0;
			m_InputAPF_4.processAudio(&fAPF_3_Out, &fAPF_4_Out);

			//APF_2_Out -> CombFilters
			float fFiltered2 = 0;
			C_A.processAudio(&fAPF_4_Out, &fC2_Out, &fFiltered2);

			// fAPF_4_Out -> dSum
			float fdSumL2 = 0;
			float fdSumR2 = 0;
			float fdSum2 = 0;
			m_LSum2.processAudio(&fFiltered2, &fdSumL2);
			m_RSum2.processAudio(&fFiltered2, &fdSumR2);
			m_Tap1.processAudio(&fdSumL2, &fdSumR2, &fdSum2);

			//dSum2 -> KRT2(RT60)
			float krt2 = 0;
			m_KRT1.processAudio(&fdSum2, m_fKRT, &krt2);

			// KRT2 -> APF5
			float fAPF_5_Out = 0;
			m_InputAPF_5.processAudio(&krt2, &fAPF_5_Out);

			// fAPF_5_Out -> fAPF_6_Out
			float fAPF_6_Out = 0;
			m_InputAPF_6.processAudio(&fAPF_5_Out, &fAPF_6_Out);

			//APF_6_Out -> CombFilters
			float fFiltered3 = 0;
			C_A.processAudio(&fAPF_6_Out, &fC1_Out, &fFiltered3);

			// fAPF_4_Out -> dSum
			float fdSumL3 = 0;
			float fdSumR3 = 0;
			float fdSum3 = 0;
			m_LSum2.processAudio(&fFiltered3, &fdSumL3);
			m_RSum2.processAudio(&fFiltered3, &fdSumR3);
			m_Tap1.processAudio(&fdSumL3, &fdSumR3, &fdSum3);

			//dSum3 -> KRT3(RT60)
			float krt3 = 0;
			m_KRT1.processAudio(&fdSum3, m_fKRT, &krt3);

			// KRT3 -> APF7
			float fAPF_7_Out = 0;
			m_InputAPF_7.processAudio(&krt3, &fAPF_7_Out);

			// fAPF_7_Out -> fAPF_8_Out
			float fAPF_8_Out = 0;
			m_InputAPF_8.processAudio(&fAPF_7_Out, &fAPF_8_Out);

			//APF_8_Out -> CombFilters
			float fFiltered4 = 0;
			C_A.processAudio(&fAPF_8_Out, &fC1_Out, &fFiltered4);

			// fAPF_8_Out -> dSum
			float fdSumL4 = 0;
			float fdSumR4 = 0;
			float fdSum4 = 0;
			m_LSum2.processAudio(&fFiltered4, &fdSumL4);
			m_RSum2.processAudio(&fFiltered4, &fdSumR4);
			m_Tap1.processAudio(&fdSumL4, &fdSumR4, &fdSum4);

			//dSum3 -> KRT3(RT60)
			krt4 = 0;
			m_KRT1.processAudio(&fdSum4, m_fKRT, &krt4);

			float L_Output = (0.25 * fdSumL1) + (0.25 * fdSumL2) + (0.25 * fdSumL3) + (0.25 * fdSumL4);
			float R_Output = (0.25 * fdSumR1) + (0.25 * fdSumR2) + (0.25 * fdSumR3) + (0.25 * fdSumR4);
			float M_Output = (0.25 * fdSum1) + (0.25 * fdSum2) + (0.25 * fdSum3) + (0.25 * fdSum4);

			// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_3_Out
			pOutputBuffer[0] = ((100.0 - m_fWet_pct) / 100.0)*fInputSample +
				(m_fWet_pct / 100.0)*(L_Output);

			// Do RIGHT Channel if there is one
			if (uNumOutputChannels == 2)
			{
				// form output = (100-Wet)/100*x(n) + (Wet/100)*fAPF_4_Out
				pOutputBuffer[1] = ((100.0 - m_fWet_pct) / 100.0)*fInputSample +
					(m_fWet_pct / 100.0)*(R_Output);
			}


			return true;
		}
	}
	else if (m_uSwitch4 == SWITCH_ON) //delay
	{
		float fYn = 0;
		float fYqn = 0;
		m_LFO.doOscillate(&fYn, &fYqn);
		// 2. calculate delay offset
		float fDelay = 0.0;
		if (m_uLFOPhase == Quad)
			fDelay = calculateDelayOffset(fYqn); // quadrature LFO
		else
			fDelay = calculateDelayOffset(fYn); // normal LFO

												// 3. set the delay & cook
		m_DDL.m_fDelay_ms = fDelay;
		m_DDL.cookVariables();

		// 4. get the delay output one channel in/one channel out
		m_DDL.processAudioFrame(&pInputBuffer[0], &pOutputBuffer[0], 1, 1);
		m_DDL.processAudioFrame(&pInputBuffer[1], &pOutputBuffer[1], 1, 1);
		// Mono-In, Stereo-Out (AUX Effect)
		if (uNumInputChannels == 1 && uNumOutputChannels == 2)
			pOutputBuffer[1] = pOutputBuffer[0];

		// Stereo-In, Stereo-Out (INSERT Effect)
		if (uNumInputChannels == 2 && uNumOutputChannels == 2)
			pOutputBuffer[1] = pOutputBuffer[0];

		return true;
	}
	else if ((m_uSwitch1 == SWITCH_OFF && m_uSwitch4 == SWITCH_OFF) || m_uSwitch5 == SWITCH_ON) {
		pOutputBuffer[0] = pInputBuffer[0];
		if (uNumInputChannels == 2 && uNumOutputChannels == 2)
			pOutputBuffer[1] = pInputBuffer[1];
	}
}


/* ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ----------------------------------- //
   	**--0x2983--**

UIList Index	Variable Name					Control Index		
-------------------------------------------------------------------
0				m_fPreDelay_mSec                  0
1				m_fPreDelayAtten_dB               1
2				m_fInputLPF_g                     2
3				m_fAPF_1_Delay_mSec               3
4				m_fAPF_1_g                        4
5				m_fAPF_2_Delay_mSec               5
6				m_fAPF_2_g                        6
7				m_fKRT                            7
8				m_fRT60                           8
9				m_fWet_pct                        9
10				m_fPComb_1_Delay_mSec             12
11				m_fPComb_2_Delay_mSec             13
12				m_fPComb_3_Delay_mSec             14
13				m_fPComb_4_Delay_mSec             15
14				m_fAPF_3_Delay_mSec               18
15				m_fAPF_3_g                        19
16				m_fPComb_5_Delay_mSec             22
17				m_fPComb_6_Delay_mSec             23
18				m_fPComb_7_Delay_mSec             24
19				m_fPComb_8_Delay_mSec             25
20				m_fAPF_4_Delay_mSec               28
21				m_fAPF_4_g                        29
22				m_fModFrequency_Hz                30
23				m_fModDepth_pct                   31
24				m_fFeedback_pct                   32
25				m_fPreDelay                       33
26				m_fChorusOffset                   34
27				m_fAPF_5_Delay_mSec               38
28				m_fAPF_5_g                        39
29				m_fMTDelay                        102
30				m_fAPF_6_Delay_mSec               108
31				m_fAPF_6_g                        109
32				m_fAPF_7_Delay_mSec               118
33				m_fAPF_7_g                        119
34				m_fAPF_8_Delay_mSec               128
35				m_fAPF_8_g                        129
36				m_f_Feedback_pct                  130
37				m_fWetLevel_pct                   131
38				m_fBitRate                        132
39				m_fDownSampling                   133
40				m_fBitDepth                       134
41				m_uSwitch1                        45
42				m_uSwitch2                        46
43				m_uSwitch4                        48
44				m_uModType                        41
45				m_uLFOType                        42
46				m_uLFOPhase                       43
47				m_uSwitch5                        3072

	Assignable Buttons               Index
-----------------------------------------------
	B1                                50
	B2                                51
	B3                                52

-----------------------------------------------
	Joystick List Boxes (Classic)    Index
-----------------------------------------------
	Drop List A                       60
	Drop List B                       61
	Drop List C                       62
	Drop List D                       63

-----------------------------------------------

	**--0xFFDD--**
// ------------------------------------------------------------------------------- */
// Add your UI Handler code here ------------------------------------------------- //
//
bool __stdcall CReverb::userInterfaceChange(int nControlIndex)
{
	// change the min/max limits; set wet/dry and Feedback 
	if (nControlIndex == 41) // 41 is mod type switch
		cookModType();

	// else just call the other updates which handle all the rest
	//
	// frequency and LFO type
	updateLFO();

	// Wet/Dry and Feedback
	updateDDL();

	return true;
}



/* joystickControlChange

	Indicates the user moved the joystick point; the variables are the relative mixes
	of each axis; the values will add up to 1.0

			B
			|
		A -	x -	C
			|
			D

	The point in the very center (x) would be:
	fControlA = 0.25
	fControlB = 0.25
	fControlC = 0.25
	fControlD = 0.25

	AC Mix = projection on X Axis (0 -> 1)
	BD Mix = projection on Y Axis (0 -> 1)
*/
bool __stdcall CReverb::joystickControlChange(float fControlA, float fControlB, float fControlC, float fControlD, float fACMix, float fBDMix)
{
	// add your code here

	return true;
}



/* processAudioBuffer

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	For advanced users only!!
*/
bool __stdcall CReverb::processRackAFXAudioBuffer(float* pInputBuffer, float* pOutputBuffer,
													   UINT uNumInputChannels, UINT uNumOutputChannels,
													   UINT uBufferSize)
{
	/* --- OLD Project Found:
		   if you want to enable parameter smoothing, add the single line of code:

		   smoothParameterValues();

		   inside your sample loop processing so that it is called exactly once per sample period.
		   Then, edit your project settings in RackAFX and check Enable Parameter Smoothing.*/


	for(UINT i=0; i<uBufferSize; i++)
	{
		// pass through code
		pOutputBuffer[i] = pInputBuffer[i];
	}


	return true;
}



/* processVSTAudioBuffer

// ALL VALUES IN AND OUT ON THE RANGE OF -1.0 TO + 1.0

	The I/O buffers are interleaved depending on the number of channels. If uNumChannels = 2, then the
	buffer is L/R/L/R/L/R etc...

	if uNumChannels = 6 then the buffer is L/R/C/Sub/BL/BR etc...

	It is up to you to decode and de-interleave the data.

	The VST input and output buffers are pointers-to-pointers. The pp buffers are the same depth as uNumChannels, so
	if uNumChannels = 2, then ppInputs would contain two pointers,

		ppInputs[0] = a pointer to the LEFT buffer of data
		ppInputs[1] = a pointer to the RIGHT buffer of data

	Similarly, ppOutputs would have 2 pointers, one for left and one for right.

*/
bool __stdcall CReverb::processVSTAudioBuffer(float** ppInputs, float** ppOutputs,
													UINT uNumChannels, int uNumFrames)
{
	/* --- OLD Project Found:
		   if you want to enable parameter smoothing, add the single line of code:

		   smoothParameterValues();

		   inside your sample loop processing so that it is called exactly once per sample period.
		   Then, edit your project settings in RackAFX and check Enable Parameter Smoothing.*/

	// PASS Through example for Stereo interleaved data

	// MONO First
    float* in1  =  ppInputs[0];
    float* out1 =  ppOutputs[0];
  	float* in2;
    float* out2;

 	// if STEREO,
   	if(uNumChannels == 2)
   	{
    	in2  =  ppInputs[1];
     	out2 = ppOutputs[1];
	}

	// loop and pass input to output by de-referencing ptrs
	while (--uNumFrames >= 0)
    {
        (*out1++) = (*in1++);

        if(uNumChannels == 2)
        	(*out2++) = (*in2++);
    }

	// all OK
	return true;
}

bool __stdcall CReverb::midiNoteOn(UINT uChannel, UINT uMIDINote, UINT uVelocity)
{
	return true;
}

bool __stdcall CReverb::midiNoteOff(UINT uChannel, UINT uMIDINote, UINT uVelocity, bool bAllNotesOff)
{
	return true;
}

// uModValue = 0->127
bool __stdcall CReverb::midiModWheel(UINT uChannel, UINT uModValue)
{
	return true;
}

// nActualPitchBendValue 		= -8192 -> +8191, 0 at center
// fNormalizedPitchBendValue 	= -1.0  -> +1.0,  0 at center
bool __stdcall CReverb::midiPitchBend(UINT uChannel, int nActualPitchBendValue, float fNormalizedPitchBendValue)
{
	return true;
}

// MIDI Clock
// http://home.roadrunner.com/~jgglatt/tech/midispec/clock.htm
/* There are 24 MIDI Clocks in every quarter note. (12 MIDI Clocks in an eighth note, 6 MIDI Clocks in a 16th, etc).
   Therefore, when a slave device counts down the receipt of 24 MIDI Clock messages, it knows that one quarter note
   has passed. When the slave counts off another 24 MIDI Clock messages, it knows that another quarter note has passed.
   Etc. Of course, the rate that the master sends these messages is based upon the master's tempo.

   For example, for a tempo of 120 BPM (ie, there are 120 quarter notes in every minute), the master sends a MIDI clock
   every 20833 microseconds. (ie, There are 1,000,000 microseconds in a second. Therefore, there are 60,000,000
   microseconds in a minute. At a tempo of 120 BPM, there are 120 quarter notes per minute. There are 24 MIDI clocks
   in each quarter note. Therefore, there should be 24 * 120 MIDI Clocks per minute.
   So, each MIDI Clock is sent at a rate of 60,000,000/(24 * 120) microseconds).
*/
bool __stdcall CReverb::midiClock()
{

	return true;
}

// any midi message other than note on, note off, pitchbend, mod wheel or clock
bool __stdcall CReverb::midiMessage(unsigned char cChannel, unsigned char cStatus, unsigned char
						   				  cData1, unsigned char cData2)
{
	return true;
}


// DO NOT DELETE THIS FUNCTION --------------------------------------------------- //
bool __stdcall CReverb::initUI()
{
	// ADDED BY RACKAFX -- DO NOT EDIT THIS CODE!!! ------------------------------ //
	if(m_UIControlList.count() > 0)
		return true;

// **--0xDEA7--**

	int nIndexer = 0;
	m_fPreDelay_mSec = 40.000000;
	CUICtrl* ui0 = new CUICtrl;
	ui0->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui0->uControlId = 0;
	ui0->bLogSlider = false;
	ui0->bExpSlider = false;
	ui0->fUserDisplayDataLoLimit = 0.000000;
	ui0->fUserDisplayDataHiLimit = 100.000000;
	ui0->uUserDataType = floatData;
	ui0->fInitUserIntValue = 0;
	ui0->fInitUserFloatValue = 40.000000;
	ui0->fInitUserDoubleValue = 0;
	ui0->fInitUserUINTValue = 0;
	ui0->m_pUserCookedIntData = NULL;
	ui0->m_pUserCookedFloatData = &m_fPreDelay_mSec;
	ui0->m_pUserCookedDoubleData = NULL;
	ui0->m_pUserCookedUINTData = NULL;
	ui0->cControlUnits = "mSec";
	ui0->cVariableName = "m_fPreDelay_mSec";
	ui0->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui0->dPresetData[0] = 40.000000;ui0->dPresetData[1] = 100.000000;ui0->dPresetData[2] = 0.000000;ui0->dPresetData[3] = 0.000000;ui0->dPresetData[4] = 0.000000;ui0->dPresetData[5] = 0.000000;ui0->dPresetData[6] = 0.000000;ui0->dPresetData[7] = 0.000000;ui0->dPresetData[8] = 0.000000;ui0->dPresetData[9] = 0.000000;ui0->dPresetData[10] = 0.000000;ui0->dPresetData[11] = 0.000000;ui0->dPresetData[12] = 0.000000;ui0->dPresetData[13] = 0.000000;ui0->dPresetData[14] = 0.000000;ui0->dPresetData[15] = 0.000000;
	ui0->cControlName = "Pre Delay Reverb";
	ui0->bOwnerControl = false;
	ui0->bMIDIControl = false;
	ui0->uMIDIControlCommand = 176;
	ui0->uMIDIControlName = 3;
	ui0->uMIDIControlChannel = 0;
	ui0->nGUIRow = nIndexer++;
	ui0->nGUIColumn = -1;
	ui0->bEnableParamSmoothing = false;
	ui0->fSmoothingTimeInMs = 100.00;
	ui0->uControlTheme[0] = 0; ui0->uControlTheme[1] = 0; ui0->uControlTheme[2] = 0; ui0->uControlTheme[3] = 0; ui0->uControlTheme[4] = 0; ui0->uControlTheme[5] = 0; ui0->uControlTheme[6] = 0; ui0->uControlTheme[7] = 0; ui0->uControlTheme[8] = 0; ui0->uControlTheme[9] = 0; ui0->uControlTheme[10] = 0; ui0->uControlTheme[11] = 0; ui0->uControlTheme[12] = 0; ui0->uControlTheme[13] = 0; ui0->uControlTheme[14] = 0; ui0->uControlTheme[15] = 0; ui0->uControlTheme[16] = 2; ui0->uControlTheme[17] = 0; ui0->uControlTheme[18] = 0; ui0->uControlTheme[19] = 0; ui0->uControlTheme[20] = 0; ui0->uControlTheme[21] = 0; ui0->uControlTheme[22] = 0; ui0->uControlTheme[23] = 0; ui0->uControlTheme[24] = 0; ui0->uControlTheme[25] = 0; ui0->uControlTheme[26] = 0; ui0->uControlTheme[27] = 0; ui0->uControlTheme[28] = 0; ui0->uControlTheme[29] = 0; ui0->uControlTheme[30] = 0; ui0->uControlTheme[31] = 0; 
	ui0->uFluxCapControl[0] = 0; ui0->uFluxCapControl[1] = 0; ui0->uFluxCapControl[2] = 0; ui0->uFluxCapControl[3] = 0; ui0->uFluxCapControl[4] = 0; ui0->uFluxCapControl[5] = 0; ui0->uFluxCapControl[6] = 0; ui0->uFluxCapControl[7] = 0; ui0->uFluxCapControl[8] = 0; ui0->uFluxCapControl[9] = 0; ui0->uFluxCapControl[10] = 0; ui0->uFluxCapControl[11] = 0; ui0->uFluxCapControl[12] = 0; ui0->uFluxCapControl[13] = 0; ui0->uFluxCapControl[14] = 0; ui0->uFluxCapControl[15] = 0; ui0->uFluxCapControl[16] = 0; ui0->uFluxCapControl[17] = 0; ui0->uFluxCapControl[18] = 0; ui0->uFluxCapControl[19] = 0; ui0->uFluxCapControl[20] = 0; ui0->uFluxCapControl[21] = 0; ui0->uFluxCapControl[22] = 0; ui0->uFluxCapControl[23] = 0; ui0->uFluxCapControl[24] = 0; ui0->uFluxCapControl[25] = 0; ui0->uFluxCapControl[26] = 0; ui0->uFluxCapControl[27] = 0; ui0->uFluxCapControl[28] = 0; ui0->uFluxCapControl[29] = 0; ui0->uFluxCapControl[30] = 0; ui0->uFluxCapControl[31] = 0; ui0->uFluxCapControl[32] = 0; ui0->uFluxCapControl[33] = 0; ui0->uFluxCapControl[34] = 0; ui0->uFluxCapControl[35] = 0; ui0->uFluxCapControl[36] = 0; ui0->uFluxCapControl[37] = 0; ui0->uFluxCapControl[38] = 0; ui0->uFluxCapControl[39] = 0; ui0->uFluxCapControl[40] = 0; ui0->uFluxCapControl[41] = 0; ui0->uFluxCapControl[42] = 0; ui0->uFluxCapControl[43] = 0; ui0->uFluxCapControl[44] = 0; ui0->uFluxCapControl[45] = 0; ui0->uFluxCapControl[46] = 0; ui0->uFluxCapControl[47] = 0; ui0->uFluxCapControl[48] = 0; ui0->uFluxCapControl[49] = 0; ui0->uFluxCapControl[50] = 0; ui0->uFluxCapControl[51] = 0; ui0->uFluxCapControl[52] = 0; ui0->uFluxCapControl[53] = 0; ui0->uFluxCapControl[54] = 0; ui0->uFluxCapControl[55] = 0; ui0->uFluxCapControl[56] = 0; ui0->uFluxCapControl[57] = 0; ui0->uFluxCapControl[58] = 0; ui0->uFluxCapControl[59] = 0; ui0->uFluxCapControl[60] = 0; ui0->uFluxCapControl[61] = 0; ui0->uFluxCapControl[62] = 0; ui0->uFluxCapControl[63] = 0; 
	ui0->fFluxCapData[0] = 0.000000; ui0->fFluxCapData[1] = 0.000000; ui0->fFluxCapData[2] = 0.000000; ui0->fFluxCapData[3] = 0.000000; ui0->fFluxCapData[4] = 0.000000; ui0->fFluxCapData[5] = 0.000000; ui0->fFluxCapData[6] = 0.000000; ui0->fFluxCapData[7] = 0.000000; ui0->fFluxCapData[8] = 0.000000; ui0->fFluxCapData[9] = 0.000000; ui0->fFluxCapData[10] = 0.000000; ui0->fFluxCapData[11] = 0.000000; ui0->fFluxCapData[12] = 0.000000; ui0->fFluxCapData[13] = 0.000000; ui0->fFluxCapData[14] = 0.000000; ui0->fFluxCapData[15] = 0.000000; ui0->fFluxCapData[16] = 0.000000; ui0->fFluxCapData[17] = 0.000000; ui0->fFluxCapData[18] = 0.000000; ui0->fFluxCapData[19] = 0.000000; ui0->fFluxCapData[20] = 0.000000; ui0->fFluxCapData[21] = 0.000000; ui0->fFluxCapData[22] = 0.000000; ui0->fFluxCapData[23] = 0.000000; ui0->fFluxCapData[24] = 0.000000; ui0->fFluxCapData[25] = 0.000000; ui0->fFluxCapData[26] = 0.000000; ui0->fFluxCapData[27] = 0.000000; ui0->fFluxCapData[28] = 0.000000; ui0->fFluxCapData[29] = 0.000000; ui0->fFluxCapData[30] = 0.000000; ui0->fFluxCapData[31] = 0.000000; ui0->fFluxCapData[32] = 0.000000; ui0->fFluxCapData[33] = 0.000000; ui0->fFluxCapData[34] = 0.000000; ui0->fFluxCapData[35] = 0.000000; ui0->fFluxCapData[36] = 0.000000; ui0->fFluxCapData[37] = 0.000000; ui0->fFluxCapData[38] = 0.000000; ui0->fFluxCapData[39] = 0.000000; ui0->fFluxCapData[40] = 0.000000; ui0->fFluxCapData[41] = 0.000000; ui0->fFluxCapData[42] = 0.000000; ui0->fFluxCapData[43] = 0.000000; ui0->fFluxCapData[44] = 0.000000; ui0->fFluxCapData[45] = 0.000000; ui0->fFluxCapData[46] = 0.000000; ui0->fFluxCapData[47] = 0.000000; ui0->fFluxCapData[48] = 0.000000; ui0->fFluxCapData[49] = 0.000000; ui0->fFluxCapData[50] = 0.000000; ui0->fFluxCapData[51] = 0.000000; ui0->fFluxCapData[52] = 0.000000; ui0->fFluxCapData[53] = 0.000000; ui0->fFluxCapData[54] = 0.000000; ui0->fFluxCapData[55] = 0.000000; ui0->fFluxCapData[56] = 0.000000; ui0->fFluxCapData[57] = 0.000000; ui0->fFluxCapData[58] = 0.000000; ui0->fFluxCapData[59] = 0.000000; ui0->fFluxCapData[60] = 0.000000; ui0->fFluxCapData[61] = 0.000000; ui0->fFluxCapData[62] = 0.000000; ui0->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui0);
	delete ui0;


	m_fPreDelayAtten_dB = 0.000000;
	CUICtrl* ui1 = new CUICtrl;
	ui1->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui1->uControlId = 1;
	ui1->bLogSlider = false;
	ui1->bExpSlider = false;
	ui1->fUserDisplayDataLoLimit = -96.000000;
	ui1->fUserDisplayDataHiLimit = 0.000000;
	ui1->uUserDataType = floatData;
	ui1->fInitUserIntValue = 0;
	ui1->fInitUserFloatValue = 0.000000;
	ui1->fInitUserDoubleValue = 0;
	ui1->fInitUserUINTValue = 0;
	ui1->m_pUserCookedIntData = NULL;
	ui1->m_pUserCookedFloatData = &m_fPreDelayAtten_dB;
	ui1->m_pUserCookedDoubleData = NULL;
	ui1->m_pUserCookedUINTData = NULL;
	ui1->cControlUnits = "dB";
	ui1->cVariableName = "m_fPreDelayAtten_dB";
	ui1->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui1->dPresetData[0] = 0.000000;ui1->dPresetData[1] = 0.000000;ui1->dPresetData[2] = 0.000000;ui1->dPresetData[3] = 0.000000;ui1->dPresetData[4] = 0.000000;ui1->dPresetData[5] = 0.000000;ui1->dPresetData[6] = 0.000000;ui1->dPresetData[7] = 0.000000;ui1->dPresetData[8] = 0.000000;ui1->dPresetData[9] = 0.000000;ui1->dPresetData[10] = 0.000000;ui1->dPresetData[11] = 0.000000;ui1->dPresetData[12] = 0.000000;ui1->dPresetData[13] = 0.000000;ui1->dPresetData[14] = 0.000000;ui1->dPresetData[15] = 0.000000;
	ui1->cControlName = "PD Atten";
	ui1->bOwnerControl = false;
	ui1->bMIDIControl = false;
	ui1->uMIDIControlCommand = 176;
	ui1->uMIDIControlName = 3;
	ui1->uMIDIControlChannel = 0;
	ui1->nGUIRow = nIndexer++;
	ui1->nGUIColumn = -1;
	ui1->bEnableParamSmoothing = false;
	ui1->fSmoothingTimeInMs = 100.00;
	ui1->uControlTheme[0] = 0; ui1->uControlTheme[1] = 0; ui1->uControlTheme[2] = 0; ui1->uControlTheme[3] = 0; ui1->uControlTheme[4] = 0; ui1->uControlTheme[5] = 0; ui1->uControlTheme[6] = 0; ui1->uControlTheme[7] = 0; ui1->uControlTheme[8] = 0; ui1->uControlTheme[9] = 0; ui1->uControlTheme[10] = 0; ui1->uControlTheme[11] = 0; ui1->uControlTheme[12] = 0; ui1->uControlTheme[13] = 0; ui1->uControlTheme[14] = 0; ui1->uControlTheme[15] = 0; ui1->uControlTheme[16] = 2; ui1->uControlTheme[17] = 0; ui1->uControlTheme[18] = 0; ui1->uControlTheme[19] = 0; ui1->uControlTheme[20] = 0; ui1->uControlTheme[21] = 0; ui1->uControlTheme[22] = 0; ui1->uControlTheme[23] = 0; ui1->uControlTheme[24] = 0; ui1->uControlTheme[25] = 0; ui1->uControlTheme[26] = 0; ui1->uControlTheme[27] = 0; ui1->uControlTheme[28] = 0; ui1->uControlTheme[29] = 0; ui1->uControlTheme[30] = 0; ui1->uControlTheme[31] = 0; 
	ui1->uFluxCapControl[0] = 0; ui1->uFluxCapControl[1] = 0; ui1->uFluxCapControl[2] = 0; ui1->uFluxCapControl[3] = 0; ui1->uFluxCapControl[4] = 0; ui1->uFluxCapControl[5] = 0; ui1->uFluxCapControl[6] = 0; ui1->uFluxCapControl[7] = 0; ui1->uFluxCapControl[8] = 0; ui1->uFluxCapControl[9] = 0; ui1->uFluxCapControl[10] = 0; ui1->uFluxCapControl[11] = 0; ui1->uFluxCapControl[12] = 0; ui1->uFluxCapControl[13] = 0; ui1->uFluxCapControl[14] = 0; ui1->uFluxCapControl[15] = 0; ui1->uFluxCapControl[16] = 0; ui1->uFluxCapControl[17] = 0; ui1->uFluxCapControl[18] = 0; ui1->uFluxCapControl[19] = 0; ui1->uFluxCapControl[20] = 0; ui1->uFluxCapControl[21] = 0; ui1->uFluxCapControl[22] = 0; ui1->uFluxCapControl[23] = 0; ui1->uFluxCapControl[24] = 0; ui1->uFluxCapControl[25] = 0; ui1->uFluxCapControl[26] = 0; ui1->uFluxCapControl[27] = 0; ui1->uFluxCapControl[28] = 0; ui1->uFluxCapControl[29] = 0; ui1->uFluxCapControl[30] = 0; ui1->uFluxCapControl[31] = 0; ui1->uFluxCapControl[32] = 0; ui1->uFluxCapControl[33] = 0; ui1->uFluxCapControl[34] = 0; ui1->uFluxCapControl[35] = 0; ui1->uFluxCapControl[36] = 0; ui1->uFluxCapControl[37] = 0; ui1->uFluxCapControl[38] = 0; ui1->uFluxCapControl[39] = 0; ui1->uFluxCapControl[40] = 0; ui1->uFluxCapControl[41] = 0; ui1->uFluxCapControl[42] = 0; ui1->uFluxCapControl[43] = 0; ui1->uFluxCapControl[44] = 0; ui1->uFluxCapControl[45] = 0; ui1->uFluxCapControl[46] = 0; ui1->uFluxCapControl[47] = 0; ui1->uFluxCapControl[48] = 0; ui1->uFluxCapControl[49] = 0; ui1->uFluxCapControl[50] = 0; ui1->uFluxCapControl[51] = 0; ui1->uFluxCapControl[52] = 0; ui1->uFluxCapControl[53] = 0; ui1->uFluxCapControl[54] = 0; ui1->uFluxCapControl[55] = 0; ui1->uFluxCapControl[56] = 0; ui1->uFluxCapControl[57] = 0; ui1->uFluxCapControl[58] = 0; ui1->uFluxCapControl[59] = 0; ui1->uFluxCapControl[60] = 0; ui1->uFluxCapControl[61] = 0; ui1->uFluxCapControl[62] = 0; ui1->uFluxCapControl[63] = 0; 
	ui1->fFluxCapData[0] = 0.000000; ui1->fFluxCapData[1] = 0.000000; ui1->fFluxCapData[2] = 0.000000; ui1->fFluxCapData[3] = 0.000000; ui1->fFluxCapData[4] = 0.000000; ui1->fFluxCapData[5] = 0.000000; ui1->fFluxCapData[6] = 0.000000; ui1->fFluxCapData[7] = 0.000000; ui1->fFluxCapData[8] = 0.000000; ui1->fFluxCapData[9] = 0.000000; ui1->fFluxCapData[10] = 0.000000; ui1->fFluxCapData[11] = 0.000000; ui1->fFluxCapData[12] = 0.000000; ui1->fFluxCapData[13] = 0.000000; ui1->fFluxCapData[14] = 0.000000; ui1->fFluxCapData[15] = 0.000000; ui1->fFluxCapData[16] = 0.000000; ui1->fFluxCapData[17] = 0.000000; ui1->fFluxCapData[18] = 0.000000; ui1->fFluxCapData[19] = 0.000000; ui1->fFluxCapData[20] = 0.000000; ui1->fFluxCapData[21] = 0.000000; ui1->fFluxCapData[22] = 0.000000; ui1->fFluxCapData[23] = 0.000000; ui1->fFluxCapData[24] = 0.000000; ui1->fFluxCapData[25] = 0.000000; ui1->fFluxCapData[26] = 0.000000; ui1->fFluxCapData[27] = 0.000000; ui1->fFluxCapData[28] = 0.000000; ui1->fFluxCapData[29] = 0.000000; ui1->fFluxCapData[30] = 0.000000; ui1->fFluxCapData[31] = 0.000000; ui1->fFluxCapData[32] = 0.000000; ui1->fFluxCapData[33] = 0.000000; ui1->fFluxCapData[34] = 0.000000; ui1->fFluxCapData[35] = 0.000000; ui1->fFluxCapData[36] = 0.000000; ui1->fFluxCapData[37] = 0.000000; ui1->fFluxCapData[38] = 0.000000; ui1->fFluxCapData[39] = 0.000000; ui1->fFluxCapData[40] = 0.000000; ui1->fFluxCapData[41] = 0.000000; ui1->fFluxCapData[42] = 0.000000; ui1->fFluxCapData[43] = 0.000000; ui1->fFluxCapData[44] = 0.000000; ui1->fFluxCapData[45] = 0.000000; ui1->fFluxCapData[46] = 0.000000; ui1->fFluxCapData[47] = 0.000000; ui1->fFluxCapData[48] = 0.000000; ui1->fFluxCapData[49] = 0.000000; ui1->fFluxCapData[50] = 0.000000; ui1->fFluxCapData[51] = 0.000000; ui1->fFluxCapData[52] = 0.000000; ui1->fFluxCapData[53] = 0.000000; ui1->fFluxCapData[54] = 0.000000; ui1->fFluxCapData[55] = 0.000000; ui1->fFluxCapData[56] = 0.000000; ui1->fFluxCapData[57] = 0.000000; ui1->fFluxCapData[58] = 0.000000; ui1->fFluxCapData[59] = 0.000000; ui1->fFluxCapData[60] = 0.000000; ui1->fFluxCapData[61] = 0.000000; ui1->fFluxCapData[62] = 0.000000; ui1->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui1);
	delete ui1;


	m_fInputLPF_g = 0.500000;
	CUICtrl* ui2 = new CUICtrl;
	ui2->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui2->uControlId = 2;
	ui2->bLogSlider = false;
	ui2->bExpSlider = false;
	ui2->fUserDisplayDataLoLimit = 0.000000;
	ui2->fUserDisplayDataHiLimit = 1.000000;
	ui2->uUserDataType = floatData;
	ui2->fInitUserIntValue = 0;
	ui2->fInitUserFloatValue = 0.500000;
	ui2->fInitUserDoubleValue = 0;
	ui2->fInitUserUINTValue = 0;
	ui2->m_pUserCookedIntData = NULL;
	ui2->m_pUserCookedFloatData = &m_fInputLPF_g;
	ui2->m_pUserCookedDoubleData = NULL;
	ui2->m_pUserCookedUINTData = NULL;
	ui2->cControlUnits = "";
	ui2->cVariableName = "m_fInputLPF_g";
	ui2->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui2->dPresetData[0] = 0.500000;ui2->dPresetData[1] = 0.000000;ui2->dPresetData[2] = 0.000000;ui2->dPresetData[3] = 0.000000;ui2->dPresetData[4] = 0.000000;ui2->dPresetData[5] = 0.000000;ui2->dPresetData[6] = 0.000000;ui2->dPresetData[7] = 0.000000;ui2->dPresetData[8] = 0.000000;ui2->dPresetData[9] = 0.000000;ui2->dPresetData[10] = 0.000000;ui2->dPresetData[11] = 0.000000;ui2->dPresetData[12] = 0.000000;ui2->dPresetData[13] = 0.000000;ui2->dPresetData[14] = 0.000000;ui2->dPresetData[15] = 0.000000;
	ui2->cControlName = "Bndwidth";
	ui2->bOwnerControl = false;
	ui2->bMIDIControl = false;
	ui2->uMIDIControlCommand = 176;
	ui2->uMIDIControlName = 3;
	ui2->uMIDIControlChannel = 0;
	ui2->nGUIRow = nIndexer++;
	ui2->nGUIColumn = -1;
	ui2->bEnableParamSmoothing = false;
	ui2->fSmoothingTimeInMs = 100.00;
	ui2->uControlTheme[0] = 0; ui2->uControlTheme[1] = 0; ui2->uControlTheme[2] = 0; ui2->uControlTheme[3] = 0; ui2->uControlTheme[4] = 0; ui2->uControlTheme[5] = 0; ui2->uControlTheme[6] = 0; ui2->uControlTheme[7] = 0; ui2->uControlTheme[8] = 0; ui2->uControlTheme[9] = 0; ui2->uControlTheme[10] = 0; ui2->uControlTheme[11] = 0; ui2->uControlTheme[12] = 0; ui2->uControlTheme[13] = 0; ui2->uControlTheme[14] = 0; ui2->uControlTheme[15] = 0; ui2->uControlTheme[16] = 2; ui2->uControlTheme[17] = 0; ui2->uControlTheme[18] = 0; ui2->uControlTheme[19] = 0; ui2->uControlTheme[20] = 0; ui2->uControlTheme[21] = 0; ui2->uControlTheme[22] = 0; ui2->uControlTheme[23] = 0; ui2->uControlTheme[24] = 0; ui2->uControlTheme[25] = 0; ui2->uControlTheme[26] = 0; ui2->uControlTheme[27] = 0; ui2->uControlTheme[28] = 0; ui2->uControlTheme[29] = 0; ui2->uControlTheme[30] = 0; ui2->uControlTheme[31] = 0; 
	ui2->uFluxCapControl[0] = 0; ui2->uFluxCapControl[1] = 0; ui2->uFluxCapControl[2] = 0; ui2->uFluxCapControl[3] = 0; ui2->uFluxCapControl[4] = 0; ui2->uFluxCapControl[5] = 0; ui2->uFluxCapControl[6] = 0; ui2->uFluxCapControl[7] = 0; ui2->uFluxCapControl[8] = 0; ui2->uFluxCapControl[9] = 0; ui2->uFluxCapControl[10] = 0; ui2->uFluxCapControl[11] = 0; ui2->uFluxCapControl[12] = 0; ui2->uFluxCapControl[13] = 0; ui2->uFluxCapControl[14] = 0; ui2->uFluxCapControl[15] = 0; ui2->uFluxCapControl[16] = 0; ui2->uFluxCapControl[17] = 0; ui2->uFluxCapControl[18] = 0; ui2->uFluxCapControl[19] = 0; ui2->uFluxCapControl[20] = 0; ui2->uFluxCapControl[21] = 0; ui2->uFluxCapControl[22] = 0; ui2->uFluxCapControl[23] = 0; ui2->uFluxCapControl[24] = 0; ui2->uFluxCapControl[25] = 0; ui2->uFluxCapControl[26] = 0; ui2->uFluxCapControl[27] = 0; ui2->uFluxCapControl[28] = 0; ui2->uFluxCapControl[29] = 0; ui2->uFluxCapControl[30] = 0; ui2->uFluxCapControl[31] = 0; ui2->uFluxCapControl[32] = 0; ui2->uFluxCapControl[33] = 0; ui2->uFluxCapControl[34] = 0; ui2->uFluxCapControl[35] = 0; ui2->uFluxCapControl[36] = 0; ui2->uFluxCapControl[37] = 0; ui2->uFluxCapControl[38] = 0; ui2->uFluxCapControl[39] = 0; ui2->uFluxCapControl[40] = 0; ui2->uFluxCapControl[41] = 0; ui2->uFluxCapControl[42] = 0; ui2->uFluxCapControl[43] = 0; ui2->uFluxCapControl[44] = 0; ui2->uFluxCapControl[45] = 0; ui2->uFluxCapControl[46] = 0; ui2->uFluxCapControl[47] = 0; ui2->uFluxCapControl[48] = 0; ui2->uFluxCapControl[49] = 0; ui2->uFluxCapControl[50] = 0; ui2->uFluxCapControl[51] = 0; ui2->uFluxCapControl[52] = 0; ui2->uFluxCapControl[53] = 0; ui2->uFluxCapControl[54] = 0; ui2->uFluxCapControl[55] = 0; ui2->uFluxCapControl[56] = 0; ui2->uFluxCapControl[57] = 0; ui2->uFluxCapControl[58] = 0; ui2->uFluxCapControl[59] = 0; ui2->uFluxCapControl[60] = 0; ui2->uFluxCapControl[61] = 0; ui2->uFluxCapControl[62] = 0; ui2->uFluxCapControl[63] = 0; 
	ui2->fFluxCapData[0] = 0.000000; ui2->fFluxCapData[1] = 0.000000; ui2->fFluxCapData[2] = 0.000000; ui2->fFluxCapData[3] = 0.000000; ui2->fFluxCapData[4] = 0.000000; ui2->fFluxCapData[5] = 0.000000; ui2->fFluxCapData[6] = 0.000000; ui2->fFluxCapData[7] = 0.000000; ui2->fFluxCapData[8] = 0.000000; ui2->fFluxCapData[9] = 0.000000; ui2->fFluxCapData[10] = 0.000000; ui2->fFluxCapData[11] = 0.000000; ui2->fFluxCapData[12] = 0.000000; ui2->fFluxCapData[13] = 0.000000; ui2->fFluxCapData[14] = 0.000000; ui2->fFluxCapData[15] = 0.000000; ui2->fFluxCapData[16] = 0.000000; ui2->fFluxCapData[17] = 0.000000; ui2->fFluxCapData[18] = 0.000000; ui2->fFluxCapData[19] = 0.000000; ui2->fFluxCapData[20] = 0.000000; ui2->fFluxCapData[21] = 0.000000; ui2->fFluxCapData[22] = 0.000000; ui2->fFluxCapData[23] = 0.000000; ui2->fFluxCapData[24] = 0.000000; ui2->fFluxCapData[25] = 0.000000; ui2->fFluxCapData[26] = 0.000000; ui2->fFluxCapData[27] = 0.000000; ui2->fFluxCapData[28] = 0.000000; ui2->fFluxCapData[29] = 0.000000; ui2->fFluxCapData[30] = 0.000000; ui2->fFluxCapData[31] = 0.000000; ui2->fFluxCapData[32] = 0.000000; ui2->fFluxCapData[33] = 0.000000; ui2->fFluxCapData[34] = 0.000000; ui2->fFluxCapData[35] = 0.000000; ui2->fFluxCapData[36] = 0.000000; ui2->fFluxCapData[37] = 0.000000; ui2->fFluxCapData[38] = 0.000000; ui2->fFluxCapData[39] = 0.000000; ui2->fFluxCapData[40] = 0.000000; ui2->fFluxCapData[41] = 0.000000; ui2->fFluxCapData[42] = 0.000000; ui2->fFluxCapData[43] = 0.000000; ui2->fFluxCapData[44] = 0.000000; ui2->fFluxCapData[45] = 0.000000; ui2->fFluxCapData[46] = 0.000000; ui2->fFluxCapData[47] = 0.000000; ui2->fFluxCapData[48] = 0.000000; ui2->fFluxCapData[49] = 0.000000; ui2->fFluxCapData[50] = 0.000000; ui2->fFluxCapData[51] = 0.000000; ui2->fFluxCapData[52] = 0.000000; ui2->fFluxCapData[53] = 0.000000; ui2->fFluxCapData[54] = 0.000000; ui2->fFluxCapData[55] = 0.000000; ui2->fFluxCapData[56] = 0.000000; ui2->fFluxCapData[57] = 0.000000; ui2->fFluxCapData[58] = 0.000000; ui2->fFluxCapData[59] = 0.000000; ui2->fFluxCapData[60] = 0.000000; ui2->fFluxCapData[61] = 0.000000; ui2->fFluxCapData[62] = 0.000000; ui2->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui2);
	delete ui2;


	m_fAPF_1_Delay_mSec = 13.280000;
	CUICtrl* ui3 = new CUICtrl;
	ui3->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui3->uControlId = 3;
	ui3->bLogSlider = false;
	ui3->bExpSlider = false;
	ui3->fUserDisplayDataLoLimit = 0.000000;
	ui3->fUserDisplayDataHiLimit = 100.000000;
	ui3->uUserDataType = floatData;
	ui3->fInitUserIntValue = 0;
	ui3->fInitUserFloatValue = 13.280000;
	ui3->fInitUserDoubleValue = 0;
	ui3->fInitUserUINTValue = 0;
	ui3->m_pUserCookedIntData = NULL;
	ui3->m_pUserCookedFloatData = &m_fAPF_1_Delay_mSec;
	ui3->m_pUserCookedDoubleData = NULL;
	ui3->m_pUserCookedUINTData = NULL;
	ui3->cControlUnits = "mSec";
	ui3->cVariableName = "m_fAPF_1_Delay_mSec";
	ui3->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui3->dPresetData[0] = 13.280000;ui3->dPresetData[1] = 15.780001;ui3->dPresetData[2] = 15.780001;ui3->dPresetData[3] = 6.280001;ui3->dPresetData[4] = 0.000000;ui3->dPresetData[5] = 15.780001;ui3->dPresetData[6] = 0.000000;ui3->dPresetData[7] = 0.000000;ui3->dPresetData[8] = 0.000000;ui3->dPresetData[9] = 0.000000;ui3->dPresetData[10] = 0.000000;ui3->dPresetData[11] = 0.000000;ui3->dPresetData[12] = 0.000000;ui3->dPresetData[13] = 0.000000;ui3->dPresetData[14] = 0.000000;ui3->dPresetData[15] = 0.000000;
	ui3->cControlName = "APF1 Dly";
	ui3->bOwnerControl = false;
	ui3->bMIDIControl = false;
	ui3->uMIDIControlCommand = 176;
	ui3->uMIDIControlName = 3;
	ui3->uMIDIControlChannel = 0;
	ui3->nGUIRow = nIndexer++;
	ui3->nGUIColumn = -1;
	ui3->bEnableParamSmoothing = false;
	ui3->fSmoothingTimeInMs = 100.00;
	ui3->uControlTheme[0] = 0; ui3->uControlTheme[1] = 0; ui3->uControlTheme[2] = 0; ui3->uControlTheme[3] = 0; ui3->uControlTheme[4] = 0; ui3->uControlTheme[5] = 0; ui3->uControlTheme[6] = 0; ui3->uControlTheme[7] = 0; ui3->uControlTheme[8] = 0; ui3->uControlTheme[9] = 0; ui3->uControlTheme[10] = 0; ui3->uControlTheme[11] = 0; ui3->uControlTheme[12] = 0; ui3->uControlTheme[13] = 0; ui3->uControlTheme[14] = 0; ui3->uControlTheme[15] = 0; ui3->uControlTheme[16] = 2; ui3->uControlTheme[17] = 0; ui3->uControlTheme[18] = 0; ui3->uControlTheme[19] = 0; ui3->uControlTheme[20] = 0; ui3->uControlTheme[21] = 0; ui3->uControlTheme[22] = 0; ui3->uControlTheme[23] = 0; ui3->uControlTheme[24] = 0; ui3->uControlTheme[25] = 0; ui3->uControlTheme[26] = 0; ui3->uControlTheme[27] = 0; ui3->uControlTheme[28] = 0; ui3->uControlTheme[29] = 0; ui3->uControlTheme[30] = 0; ui3->uControlTheme[31] = 0; 
	ui3->uFluxCapControl[0] = 0; ui3->uFluxCapControl[1] = 0; ui3->uFluxCapControl[2] = 0; ui3->uFluxCapControl[3] = 0; ui3->uFluxCapControl[4] = 0; ui3->uFluxCapControl[5] = 0; ui3->uFluxCapControl[6] = 0; ui3->uFluxCapControl[7] = 0; ui3->uFluxCapControl[8] = 0; ui3->uFluxCapControl[9] = 0; ui3->uFluxCapControl[10] = 0; ui3->uFluxCapControl[11] = 0; ui3->uFluxCapControl[12] = 0; ui3->uFluxCapControl[13] = 0; ui3->uFluxCapControl[14] = 0; ui3->uFluxCapControl[15] = 0; ui3->uFluxCapControl[16] = 0; ui3->uFluxCapControl[17] = 0; ui3->uFluxCapControl[18] = 0; ui3->uFluxCapControl[19] = 0; ui3->uFluxCapControl[20] = 0; ui3->uFluxCapControl[21] = 0; ui3->uFluxCapControl[22] = 0; ui3->uFluxCapControl[23] = 0; ui3->uFluxCapControl[24] = 0; ui3->uFluxCapControl[25] = 0; ui3->uFluxCapControl[26] = 0; ui3->uFluxCapControl[27] = 0; ui3->uFluxCapControl[28] = 0; ui3->uFluxCapControl[29] = 0; ui3->uFluxCapControl[30] = 0; ui3->uFluxCapControl[31] = 0; ui3->uFluxCapControl[32] = 0; ui3->uFluxCapControl[33] = 0; ui3->uFluxCapControl[34] = 0; ui3->uFluxCapControl[35] = 0; ui3->uFluxCapControl[36] = 0; ui3->uFluxCapControl[37] = 0; ui3->uFluxCapControl[38] = 0; ui3->uFluxCapControl[39] = 0; ui3->uFluxCapControl[40] = 0; ui3->uFluxCapControl[41] = 0; ui3->uFluxCapControl[42] = 0; ui3->uFluxCapControl[43] = 0; ui3->uFluxCapControl[44] = 0; ui3->uFluxCapControl[45] = 0; ui3->uFluxCapControl[46] = 0; ui3->uFluxCapControl[47] = 0; ui3->uFluxCapControl[48] = 0; ui3->uFluxCapControl[49] = 0; ui3->uFluxCapControl[50] = 0; ui3->uFluxCapControl[51] = 0; ui3->uFluxCapControl[52] = 0; ui3->uFluxCapControl[53] = 0; ui3->uFluxCapControl[54] = 0; ui3->uFluxCapControl[55] = 0; ui3->uFluxCapControl[56] = 0; ui3->uFluxCapControl[57] = 0; ui3->uFluxCapControl[58] = 0; ui3->uFluxCapControl[59] = 0; ui3->uFluxCapControl[60] = 0; ui3->uFluxCapControl[61] = 0; ui3->uFluxCapControl[62] = 0; ui3->uFluxCapControl[63] = 0; 
	ui3->fFluxCapData[0] = 0.000000; ui3->fFluxCapData[1] = 0.000000; ui3->fFluxCapData[2] = 0.000000; ui3->fFluxCapData[3] = 0.000000; ui3->fFluxCapData[4] = 0.000000; ui3->fFluxCapData[5] = 0.000000; ui3->fFluxCapData[6] = 0.000000; ui3->fFluxCapData[7] = 0.000000; ui3->fFluxCapData[8] = 0.000000; ui3->fFluxCapData[9] = 0.000000; ui3->fFluxCapData[10] = 0.000000; ui3->fFluxCapData[11] = 0.000000; ui3->fFluxCapData[12] = 0.000000; ui3->fFluxCapData[13] = 0.000000; ui3->fFluxCapData[14] = 0.000000; ui3->fFluxCapData[15] = 0.000000; ui3->fFluxCapData[16] = 0.000000; ui3->fFluxCapData[17] = 0.000000; ui3->fFluxCapData[18] = 0.000000; ui3->fFluxCapData[19] = 0.000000; ui3->fFluxCapData[20] = 0.000000; ui3->fFluxCapData[21] = 0.000000; ui3->fFluxCapData[22] = 0.000000; ui3->fFluxCapData[23] = 0.000000; ui3->fFluxCapData[24] = 0.000000; ui3->fFluxCapData[25] = 0.000000; ui3->fFluxCapData[26] = 0.000000; ui3->fFluxCapData[27] = 0.000000; ui3->fFluxCapData[28] = 0.000000; ui3->fFluxCapData[29] = 0.000000; ui3->fFluxCapData[30] = 0.000000; ui3->fFluxCapData[31] = 0.000000; ui3->fFluxCapData[32] = 0.000000; ui3->fFluxCapData[33] = 0.000000; ui3->fFluxCapData[34] = 0.000000; ui3->fFluxCapData[35] = 0.000000; ui3->fFluxCapData[36] = 0.000000; ui3->fFluxCapData[37] = 0.000000; ui3->fFluxCapData[38] = 0.000000; ui3->fFluxCapData[39] = 0.000000; ui3->fFluxCapData[40] = 0.000000; ui3->fFluxCapData[41] = 0.000000; ui3->fFluxCapData[42] = 0.000000; ui3->fFluxCapData[43] = 0.000000; ui3->fFluxCapData[44] = 0.000000; ui3->fFluxCapData[45] = 0.000000; ui3->fFluxCapData[46] = 0.000000; ui3->fFluxCapData[47] = 0.000000; ui3->fFluxCapData[48] = 0.000000; ui3->fFluxCapData[49] = 0.000000; ui3->fFluxCapData[50] = 0.000000; ui3->fFluxCapData[51] = 0.000000; ui3->fFluxCapData[52] = 0.000000; ui3->fFluxCapData[53] = 0.000000; ui3->fFluxCapData[54] = 0.000000; ui3->fFluxCapData[55] = 0.000000; ui3->fFluxCapData[56] = 0.000000; ui3->fFluxCapData[57] = 0.000000; ui3->fFluxCapData[58] = 0.000000; ui3->fFluxCapData[59] = 0.000000; ui3->fFluxCapData[60] = 0.000000; ui3->fFluxCapData[61] = 0.000000; ui3->fFluxCapData[62] = 0.000000; ui3->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui3);
	delete ui3;


	m_fAPF_1_g = 0.700000;
	CUICtrl* ui4 = new CUICtrl;
	ui4->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui4->uControlId = 4;
	ui4->bLogSlider = false;
	ui4->bExpSlider = false;
	ui4->fUserDisplayDataLoLimit = -1.000000;
	ui4->fUserDisplayDataHiLimit = 1.000000;
	ui4->uUserDataType = floatData;
	ui4->fInitUserIntValue = 0;
	ui4->fInitUserFloatValue = 0.700000;
	ui4->fInitUserDoubleValue = 0;
	ui4->fInitUserUINTValue = 0;
	ui4->m_pUserCookedIntData = NULL;
	ui4->m_pUserCookedFloatData = &m_fAPF_1_g;
	ui4->m_pUserCookedDoubleData = NULL;
	ui4->m_pUserCookedUINTData = NULL;
	ui4->cControlUnits = "";
	ui4->cVariableName = "m_fAPF_1_g";
	ui4->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui4->dPresetData[0] = 0.700000;ui4->dPresetData[1] = -0.490000;ui4->dPresetData[2] = -0.490000;ui4->dPresetData[3] = -0.490000;ui4->dPresetData[4] = 0.000000;ui4->dPresetData[5] = -0.490000;ui4->dPresetData[6] = 0.000000;ui4->dPresetData[7] = 0.000000;ui4->dPresetData[8] = 0.000000;ui4->dPresetData[9] = 0.000000;ui4->dPresetData[10] = 0.000000;ui4->dPresetData[11] = 0.000000;ui4->dPresetData[12] = 0.000000;ui4->dPresetData[13] = 0.000000;ui4->dPresetData[14] = 0.000000;ui4->dPresetData[15] = 0.000000;
	ui4->cControlName = "APF1 g";
	ui4->bOwnerControl = false;
	ui4->bMIDIControl = false;
	ui4->uMIDIControlCommand = 176;
	ui4->uMIDIControlName = 3;
	ui4->uMIDIControlChannel = 0;
	ui4->nGUIRow = nIndexer++;
	ui4->nGUIColumn = -1;
	ui4->bEnableParamSmoothing = false;
	ui4->fSmoothingTimeInMs = 100.00;
	ui4->uControlTheme[0] = 0; ui4->uControlTheme[1] = 0; ui4->uControlTheme[2] = 0; ui4->uControlTheme[3] = 0; ui4->uControlTheme[4] = 0; ui4->uControlTheme[5] = 0; ui4->uControlTheme[6] = 0; ui4->uControlTheme[7] = 0; ui4->uControlTheme[8] = 0; ui4->uControlTheme[9] = 0; ui4->uControlTheme[10] = 0; ui4->uControlTheme[11] = 0; ui4->uControlTheme[12] = 0; ui4->uControlTheme[13] = 0; ui4->uControlTheme[14] = 0; ui4->uControlTheme[15] = 0; ui4->uControlTheme[16] = 2; ui4->uControlTheme[17] = 0; ui4->uControlTheme[18] = 0; ui4->uControlTheme[19] = 0; ui4->uControlTheme[20] = 0; ui4->uControlTheme[21] = 0; ui4->uControlTheme[22] = 0; ui4->uControlTheme[23] = 0; ui4->uControlTheme[24] = 0; ui4->uControlTheme[25] = 0; ui4->uControlTheme[26] = 0; ui4->uControlTheme[27] = 0; ui4->uControlTheme[28] = 0; ui4->uControlTheme[29] = 0; ui4->uControlTheme[30] = 0; ui4->uControlTheme[31] = 0; 
	ui4->uFluxCapControl[0] = 0; ui4->uFluxCapControl[1] = 0; ui4->uFluxCapControl[2] = 0; ui4->uFluxCapControl[3] = 0; ui4->uFluxCapControl[4] = 0; ui4->uFluxCapControl[5] = 0; ui4->uFluxCapControl[6] = 0; ui4->uFluxCapControl[7] = 0; ui4->uFluxCapControl[8] = 0; ui4->uFluxCapControl[9] = 0; ui4->uFluxCapControl[10] = 0; ui4->uFluxCapControl[11] = 0; ui4->uFluxCapControl[12] = 0; ui4->uFluxCapControl[13] = 0; ui4->uFluxCapControl[14] = 0; ui4->uFluxCapControl[15] = 0; ui4->uFluxCapControl[16] = 0; ui4->uFluxCapControl[17] = 0; ui4->uFluxCapControl[18] = 0; ui4->uFluxCapControl[19] = 0; ui4->uFluxCapControl[20] = 0; ui4->uFluxCapControl[21] = 0; ui4->uFluxCapControl[22] = 0; ui4->uFluxCapControl[23] = 0; ui4->uFluxCapControl[24] = 0; ui4->uFluxCapControl[25] = 0; ui4->uFluxCapControl[26] = 0; ui4->uFluxCapControl[27] = 0; ui4->uFluxCapControl[28] = 0; ui4->uFluxCapControl[29] = 0; ui4->uFluxCapControl[30] = 0; ui4->uFluxCapControl[31] = 0; ui4->uFluxCapControl[32] = 0; ui4->uFluxCapControl[33] = 0; ui4->uFluxCapControl[34] = 0; ui4->uFluxCapControl[35] = 0; ui4->uFluxCapControl[36] = 0; ui4->uFluxCapControl[37] = 0; ui4->uFluxCapControl[38] = 0; ui4->uFluxCapControl[39] = 0; ui4->uFluxCapControl[40] = 0; ui4->uFluxCapControl[41] = 0; ui4->uFluxCapControl[42] = 0; ui4->uFluxCapControl[43] = 0; ui4->uFluxCapControl[44] = 0; ui4->uFluxCapControl[45] = 0; ui4->uFluxCapControl[46] = 0; ui4->uFluxCapControl[47] = 0; ui4->uFluxCapControl[48] = 0; ui4->uFluxCapControl[49] = 0; ui4->uFluxCapControl[50] = 0; ui4->uFluxCapControl[51] = 0; ui4->uFluxCapControl[52] = 0; ui4->uFluxCapControl[53] = 0; ui4->uFluxCapControl[54] = 0; ui4->uFluxCapControl[55] = 0; ui4->uFluxCapControl[56] = 0; ui4->uFluxCapControl[57] = 0; ui4->uFluxCapControl[58] = 0; ui4->uFluxCapControl[59] = 0; ui4->uFluxCapControl[60] = 0; ui4->uFluxCapControl[61] = 0; ui4->uFluxCapControl[62] = 0; ui4->uFluxCapControl[63] = 0; 
	ui4->fFluxCapData[0] = 0.000000; ui4->fFluxCapData[1] = 0.000000; ui4->fFluxCapData[2] = 0.000000; ui4->fFluxCapData[3] = 0.000000; ui4->fFluxCapData[4] = 0.000000; ui4->fFluxCapData[5] = 0.000000; ui4->fFluxCapData[6] = 0.000000; ui4->fFluxCapData[7] = 0.000000; ui4->fFluxCapData[8] = 0.000000; ui4->fFluxCapData[9] = 0.000000; ui4->fFluxCapData[10] = 0.000000; ui4->fFluxCapData[11] = 0.000000; ui4->fFluxCapData[12] = 0.000000; ui4->fFluxCapData[13] = 0.000000; ui4->fFluxCapData[14] = 0.000000; ui4->fFluxCapData[15] = 0.000000; ui4->fFluxCapData[16] = 0.000000; ui4->fFluxCapData[17] = 0.000000; ui4->fFluxCapData[18] = 0.000000; ui4->fFluxCapData[19] = 0.000000; ui4->fFluxCapData[20] = 0.000000; ui4->fFluxCapData[21] = 0.000000; ui4->fFluxCapData[22] = 0.000000; ui4->fFluxCapData[23] = 0.000000; ui4->fFluxCapData[24] = 0.000000; ui4->fFluxCapData[25] = 0.000000; ui4->fFluxCapData[26] = 0.000000; ui4->fFluxCapData[27] = 0.000000; ui4->fFluxCapData[28] = 0.000000; ui4->fFluxCapData[29] = 0.000000; ui4->fFluxCapData[30] = 0.000000; ui4->fFluxCapData[31] = 0.000000; ui4->fFluxCapData[32] = 0.000000; ui4->fFluxCapData[33] = 0.000000; ui4->fFluxCapData[34] = 0.000000; ui4->fFluxCapData[35] = 0.000000; ui4->fFluxCapData[36] = 0.000000; ui4->fFluxCapData[37] = 0.000000; ui4->fFluxCapData[38] = 0.000000; ui4->fFluxCapData[39] = 0.000000; ui4->fFluxCapData[40] = 0.000000; ui4->fFluxCapData[41] = 0.000000; ui4->fFluxCapData[42] = 0.000000; ui4->fFluxCapData[43] = 0.000000; ui4->fFluxCapData[44] = 0.000000; ui4->fFluxCapData[45] = 0.000000; ui4->fFluxCapData[46] = 0.000000; ui4->fFluxCapData[47] = 0.000000; ui4->fFluxCapData[48] = 0.000000; ui4->fFluxCapData[49] = 0.000000; ui4->fFluxCapData[50] = 0.000000; ui4->fFluxCapData[51] = 0.000000; ui4->fFluxCapData[52] = 0.000000; ui4->fFluxCapData[53] = 0.000000; ui4->fFluxCapData[54] = 0.000000; ui4->fFluxCapData[55] = 0.000000; ui4->fFluxCapData[56] = 0.000000; ui4->fFluxCapData[57] = 0.000000; ui4->fFluxCapData[58] = 0.000000; ui4->fFluxCapData[59] = 0.000000; ui4->fFluxCapData[60] = 0.000000; ui4->fFluxCapData[61] = 0.000000; ui4->fFluxCapData[62] = 0.000000; ui4->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui4);
	delete ui4;


	m_fAPF_2_Delay_mSec = 28.129999;
	CUICtrl* ui5 = new CUICtrl;
	ui5->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui5->uControlId = 5;
	ui5->bLogSlider = false;
	ui5->bExpSlider = false;
	ui5->fUserDisplayDataLoLimit = 0.000000;
	ui5->fUserDisplayDataHiLimit = 100.000000;
	ui5->uUserDataType = floatData;
	ui5->fInitUserIntValue = 0;
	ui5->fInitUserFloatValue = 28.129999;
	ui5->fInitUserDoubleValue = 0;
	ui5->fInitUserUINTValue = 0;
	ui5->m_pUserCookedIntData = NULL;
	ui5->m_pUserCookedFloatData = &m_fAPF_2_Delay_mSec;
	ui5->m_pUserCookedDoubleData = NULL;
	ui5->m_pUserCookedUINTData = NULL;
	ui5->cControlUnits = "mSec";
	ui5->cVariableName = "m_fAPF_2_Delay_mSec";
	ui5->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui5->dPresetData[0] = 28.129997;ui5->dPresetData[1] = 28.129997;ui5->dPresetData[2] = 28.129997;ui5->dPresetData[3] = 10.629998;ui5->dPresetData[4] = 0.000000;ui5->dPresetData[5] = 28.129997;ui5->dPresetData[6] = 0.000000;ui5->dPresetData[7] = 0.000000;ui5->dPresetData[8] = 0.000000;ui5->dPresetData[9] = 0.000000;ui5->dPresetData[10] = 0.000000;ui5->dPresetData[11] = 0.000000;ui5->dPresetData[12] = 0.000000;ui5->dPresetData[13] = 0.000000;ui5->dPresetData[14] = 0.000000;ui5->dPresetData[15] = 0.000000;
	ui5->cControlName = "APF2 Dly";
	ui5->bOwnerControl = false;
	ui5->bMIDIControl = false;
	ui5->uMIDIControlCommand = 176;
	ui5->uMIDIControlName = 3;
	ui5->uMIDIControlChannel = 0;
	ui5->nGUIRow = nIndexer++;
	ui5->nGUIColumn = -1;
	ui5->bEnableParamSmoothing = false;
	ui5->fSmoothingTimeInMs = 100.00;
	ui5->uControlTheme[0] = 0; ui5->uControlTheme[1] = 0; ui5->uControlTheme[2] = 0; ui5->uControlTheme[3] = 0; ui5->uControlTheme[4] = 0; ui5->uControlTheme[5] = 0; ui5->uControlTheme[6] = 0; ui5->uControlTheme[7] = 0; ui5->uControlTheme[8] = 0; ui5->uControlTheme[9] = 0; ui5->uControlTheme[10] = 0; ui5->uControlTheme[11] = 0; ui5->uControlTheme[12] = 0; ui5->uControlTheme[13] = 0; ui5->uControlTheme[14] = 0; ui5->uControlTheme[15] = 0; ui5->uControlTheme[16] = 2; ui5->uControlTheme[17] = 0; ui5->uControlTheme[18] = 0; ui5->uControlTheme[19] = 0; ui5->uControlTheme[20] = 0; ui5->uControlTheme[21] = 0; ui5->uControlTheme[22] = 0; ui5->uControlTheme[23] = 0; ui5->uControlTheme[24] = 0; ui5->uControlTheme[25] = 0; ui5->uControlTheme[26] = 0; ui5->uControlTheme[27] = 0; ui5->uControlTheme[28] = 0; ui5->uControlTheme[29] = 0; ui5->uControlTheme[30] = 0; ui5->uControlTheme[31] = 0; 
	ui5->uFluxCapControl[0] = 0; ui5->uFluxCapControl[1] = 0; ui5->uFluxCapControl[2] = 0; ui5->uFluxCapControl[3] = 0; ui5->uFluxCapControl[4] = 0; ui5->uFluxCapControl[5] = 0; ui5->uFluxCapControl[6] = 0; ui5->uFluxCapControl[7] = 0; ui5->uFluxCapControl[8] = 0; ui5->uFluxCapControl[9] = 0; ui5->uFluxCapControl[10] = 0; ui5->uFluxCapControl[11] = 0; ui5->uFluxCapControl[12] = 0; ui5->uFluxCapControl[13] = 0; ui5->uFluxCapControl[14] = 0; ui5->uFluxCapControl[15] = 0; ui5->uFluxCapControl[16] = 0; ui5->uFluxCapControl[17] = 0; ui5->uFluxCapControl[18] = 0; ui5->uFluxCapControl[19] = 0; ui5->uFluxCapControl[20] = 0; ui5->uFluxCapControl[21] = 0; ui5->uFluxCapControl[22] = 0; ui5->uFluxCapControl[23] = 0; ui5->uFluxCapControl[24] = 0; ui5->uFluxCapControl[25] = 0; ui5->uFluxCapControl[26] = 0; ui5->uFluxCapControl[27] = 0; ui5->uFluxCapControl[28] = 0; ui5->uFluxCapControl[29] = 0; ui5->uFluxCapControl[30] = 0; ui5->uFluxCapControl[31] = 0; ui5->uFluxCapControl[32] = 0; ui5->uFluxCapControl[33] = 0; ui5->uFluxCapControl[34] = 0; ui5->uFluxCapControl[35] = 0; ui5->uFluxCapControl[36] = 0; ui5->uFluxCapControl[37] = 0; ui5->uFluxCapControl[38] = 0; ui5->uFluxCapControl[39] = 0; ui5->uFluxCapControl[40] = 0; ui5->uFluxCapControl[41] = 0; ui5->uFluxCapControl[42] = 0; ui5->uFluxCapControl[43] = 0; ui5->uFluxCapControl[44] = 0; ui5->uFluxCapControl[45] = 0; ui5->uFluxCapControl[46] = 0; ui5->uFluxCapControl[47] = 0; ui5->uFluxCapControl[48] = 0; ui5->uFluxCapControl[49] = 0; ui5->uFluxCapControl[50] = 0; ui5->uFluxCapControl[51] = 0; ui5->uFluxCapControl[52] = 0; ui5->uFluxCapControl[53] = 0; ui5->uFluxCapControl[54] = 0; ui5->uFluxCapControl[55] = 0; ui5->uFluxCapControl[56] = 0; ui5->uFluxCapControl[57] = 0; ui5->uFluxCapControl[58] = 0; ui5->uFluxCapControl[59] = 0; ui5->uFluxCapControl[60] = 0; ui5->uFluxCapControl[61] = 0; ui5->uFluxCapControl[62] = 0; ui5->uFluxCapControl[63] = 0; 
	ui5->fFluxCapData[0] = 0.000000; ui5->fFluxCapData[1] = 0.000000; ui5->fFluxCapData[2] = 0.000000; ui5->fFluxCapData[3] = 0.000000; ui5->fFluxCapData[4] = 0.000000; ui5->fFluxCapData[5] = 0.000000; ui5->fFluxCapData[6] = 0.000000; ui5->fFluxCapData[7] = 0.000000; ui5->fFluxCapData[8] = 0.000000; ui5->fFluxCapData[9] = 0.000000; ui5->fFluxCapData[10] = 0.000000; ui5->fFluxCapData[11] = 0.000000; ui5->fFluxCapData[12] = 0.000000; ui5->fFluxCapData[13] = 0.000000; ui5->fFluxCapData[14] = 0.000000; ui5->fFluxCapData[15] = 0.000000; ui5->fFluxCapData[16] = 0.000000; ui5->fFluxCapData[17] = 0.000000; ui5->fFluxCapData[18] = 0.000000; ui5->fFluxCapData[19] = 0.000000; ui5->fFluxCapData[20] = 0.000000; ui5->fFluxCapData[21] = 0.000000; ui5->fFluxCapData[22] = 0.000000; ui5->fFluxCapData[23] = 0.000000; ui5->fFluxCapData[24] = 0.000000; ui5->fFluxCapData[25] = 0.000000; ui5->fFluxCapData[26] = 0.000000; ui5->fFluxCapData[27] = 0.000000; ui5->fFluxCapData[28] = 0.000000; ui5->fFluxCapData[29] = 0.000000; ui5->fFluxCapData[30] = 0.000000; ui5->fFluxCapData[31] = 0.000000; ui5->fFluxCapData[32] = 0.000000; ui5->fFluxCapData[33] = 0.000000; ui5->fFluxCapData[34] = 0.000000; ui5->fFluxCapData[35] = 0.000000; ui5->fFluxCapData[36] = 0.000000; ui5->fFluxCapData[37] = 0.000000; ui5->fFluxCapData[38] = 0.000000; ui5->fFluxCapData[39] = 0.000000; ui5->fFluxCapData[40] = 0.000000; ui5->fFluxCapData[41] = 0.000000; ui5->fFluxCapData[42] = 0.000000; ui5->fFluxCapData[43] = 0.000000; ui5->fFluxCapData[44] = 0.000000; ui5->fFluxCapData[45] = 0.000000; ui5->fFluxCapData[46] = 0.000000; ui5->fFluxCapData[47] = 0.000000; ui5->fFluxCapData[48] = 0.000000; ui5->fFluxCapData[49] = 0.000000; ui5->fFluxCapData[50] = 0.000000; ui5->fFluxCapData[51] = 0.000000; ui5->fFluxCapData[52] = 0.000000; ui5->fFluxCapData[53] = 0.000000; ui5->fFluxCapData[54] = 0.000000; ui5->fFluxCapData[55] = 0.000000; ui5->fFluxCapData[56] = 0.000000; ui5->fFluxCapData[57] = 0.000000; ui5->fFluxCapData[58] = 0.000000; ui5->fFluxCapData[59] = 0.000000; ui5->fFluxCapData[60] = 0.000000; ui5->fFluxCapData[61] = 0.000000; ui5->fFluxCapData[62] = 0.000000; ui5->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui5);
	delete ui5;


	m_fAPF_2_g = -0.500000;
	CUICtrl* ui6 = new CUICtrl;
	ui6->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui6->uControlId = 6;
	ui6->bLogSlider = false;
	ui6->bExpSlider = false;
	ui6->fUserDisplayDataLoLimit = -1.000000;
	ui6->fUserDisplayDataHiLimit = 1.000000;
	ui6->uUserDataType = floatData;
	ui6->fInitUserIntValue = 0;
	ui6->fInitUserFloatValue = -0.500000;
	ui6->fInitUserDoubleValue = 0;
	ui6->fInitUserUINTValue = 0;
	ui6->m_pUserCookedIntData = NULL;
	ui6->m_pUserCookedFloatData = &m_fAPF_2_g;
	ui6->m_pUserCookedDoubleData = NULL;
	ui6->m_pUserCookedUINTData = NULL;
	ui6->cControlUnits = "";
	ui6->cVariableName = "m_fAPF_2_g";
	ui6->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui6->dPresetData[0] = -0.500000;ui6->dPresetData[1] = 0.500000;ui6->dPresetData[2] = 0.500000;ui6->dPresetData[3] = 0.500000;ui6->dPresetData[4] = 0.000000;ui6->dPresetData[5] = 0.500000;ui6->dPresetData[6] = 0.000000;ui6->dPresetData[7] = 0.000000;ui6->dPresetData[8] = 0.000000;ui6->dPresetData[9] = 0.000000;ui6->dPresetData[10] = 0.000000;ui6->dPresetData[11] = 0.000000;ui6->dPresetData[12] = 0.000000;ui6->dPresetData[13] = 0.000000;ui6->dPresetData[14] = 0.000000;ui6->dPresetData[15] = 0.000000;
	ui6->cControlName = "APF2 g";
	ui6->bOwnerControl = false;
	ui6->bMIDIControl = false;
	ui6->uMIDIControlCommand = 176;
	ui6->uMIDIControlName = 3;
	ui6->uMIDIControlChannel = 0;
	ui6->nGUIRow = nIndexer++;
	ui6->nGUIColumn = -1;
	ui6->bEnableParamSmoothing = false;
	ui6->fSmoothingTimeInMs = 100.00;
	ui6->uControlTheme[0] = 0; ui6->uControlTheme[1] = 0; ui6->uControlTheme[2] = 0; ui6->uControlTheme[3] = 0; ui6->uControlTheme[4] = 0; ui6->uControlTheme[5] = 0; ui6->uControlTheme[6] = 0; ui6->uControlTheme[7] = 0; ui6->uControlTheme[8] = 0; ui6->uControlTheme[9] = 0; ui6->uControlTheme[10] = 0; ui6->uControlTheme[11] = 0; ui6->uControlTheme[12] = 0; ui6->uControlTheme[13] = 0; ui6->uControlTheme[14] = 0; ui6->uControlTheme[15] = 0; ui6->uControlTheme[16] = 2; ui6->uControlTheme[17] = 0; ui6->uControlTheme[18] = 0; ui6->uControlTheme[19] = 0; ui6->uControlTheme[20] = 0; ui6->uControlTheme[21] = 0; ui6->uControlTheme[22] = 0; ui6->uControlTheme[23] = 0; ui6->uControlTheme[24] = 0; ui6->uControlTheme[25] = 0; ui6->uControlTheme[26] = 0; ui6->uControlTheme[27] = 0; ui6->uControlTheme[28] = 0; ui6->uControlTheme[29] = 0; ui6->uControlTheme[30] = 0; ui6->uControlTheme[31] = 0; 
	ui6->uFluxCapControl[0] = 0; ui6->uFluxCapControl[1] = 0; ui6->uFluxCapControl[2] = 0; ui6->uFluxCapControl[3] = 0; ui6->uFluxCapControl[4] = 0; ui6->uFluxCapControl[5] = 0; ui6->uFluxCapControl[6] = 0; ui6->uFluxCapControl[7] = 0; ui6->uFluxCapControl[8] = 0; ui6->uFluxCapControl[9] = 0; ui6->uFluxCapControl[10] = 0; ui6->uFluxCapControl[11] = 0; ui6->uFluxCapControl[12] = 0; ui6->uFluxCapControl[13] = 0; ui6->uFluxCapControl[14] = 0; ui6->uFluxCapControl[15] = 0; ui6->uFluxCapControl[16] = 0; ui6->uFluxCapControl[17] = 0; ui6->uFluxCapControl[18] = 0; ui6->uFluxCapControl[19] = 0; ui6->uFluxCapControl[20] = 0; ui6->uFluxCapControl[21] = 0; ui6->uFluxCapControl[22] = 0; ui6->uFluxCapControl[23] = 0; ui6->uFluxCapControl[24] = 0; ui6->uFluxCapControl[25] = 0; ui6->uFluxCapControl[26] = 0; ui6->uFluxCapControl[27] = 0; ui6->uFluxCapControl[28] = 0; ui6->uFluxCapControl[29] = 0; ui6->uFluxCapControl[30] = 0; ui6->uFluxCapControl[31] = 0; ui6->uFluxCapControl[32] = 0; ui6->uFluxCapControl[33] = 0; ui6->uFluxCapControl[34] = 0; ui6->uFluxCapControl[35] = 0; ui6->uFluxCapControl[36] = 0; ui6->uFluxCapControl[37] = 0; ui6->uFluxCapControl[38] = 0; ui6->uFluxCapControl[39] = 0; ui6->uFluxCapControl[40] = 0; ui6->uFluxCapControl[41] = 0; ui6->uFluxCapControl[42] = 0; ui6->uFluxCapControl[43] = 0; ui6->uFluxCapControl[44] = 0; ui6->uFluxCapControl[45] = 0; ui6->uFluxCapControl[46] = 0; ui6->uFluxCapControl[47] = 0; ui6->uFluxCapControl[48] = 0; ui6->uFluxCapControl[49] = 0; ui6->uFluxCapControl[50] = 0; ui6->uFluxCapControl[51] = 0; ui6->uFluxCapControl[52] = 0; ui6->uFluxCapControl[53] = 0; ui6->uFluxCapControl[54] = 0; ui6->uFluxCapControl[55] = 0; ui6->uFluxCapControl[56] = 0; ui6->uFluxCapControl[57] = 0; ui6->uFluxCapControl[58] = 0; ui6->uFluxCapControl[59] = 0; ui6->uFluxCapControl[60] = 0; ui6->uFluxCapControl[61] = 0; ui6->uFluxCapControl[62] = 0; ui6->uFluxCapControl[63] = 0; 
	ui6->fFluxCapData[0] = 0.000000; ui6->fFluxCapData[1] = 0.000000; ui6->fFluxCapData[2] = 0.000000; ui6->fFluxCapData[3] = 0.000000; ui6->fFluxCapData[4] = 0.000000; ui6->fFluxCapData[5] = 0.000000; ui6->fFluxCapData[6] = 0.000000; ui6->fFluxCapData[7] = 0.000000; ui6->fFluxCapData[8] = 0.000000; ui6->fFluxCapData[9] = 0.000000; ui6->fFluxCapData[10] = 0.000000; ui6->fFluxCapData[11] = 0.000000; ui6->fFluxCapData[12] = 0.000000; ui6->fFluxCapData[13] = 0.000000; ui6->fFluxCapData[14] = 0.000000; ui6->fFluxCapData[15] = 0.000000; ui6->fFluxCapData[16] = 0.000000; ui6->fFluxCapData[17] = 0.000000; ui6->fFluxCapData[18] = 0.000000; ui6->fFluxCapData[19] = 0.000000; ui6->fFluxCapData[20] = 0.000000; ui6->fFluxCapData[21] = 0.000000; ui6->fFluxCapData[22] = 0.000000; ui6->fFluxCapData[23] = 0.000000; ui6->fFluxCapData[24] = 0.000000; ui6->fFluxCapData[25] = 0.000000; ui6->fFluxCapData[26] = 0.000000; ui6->fFluxCapData[27] = 0.000000; ui6->fFluxCapData[28] = 0.000000; ui6->fFluxCapData[29] = 0.000000; ui6->fFluxCapData[30] = 0.000000; ui6->fFluxCapData[31] = 0.000000; ui6->fFluxCapData[32] = 0.000000; ui6->fFluxCapData[33] = 0.000000; ui6->fFluxCapData[34] = 0.000000; ui6->fFluxCapData[35] = 0.000000; ui6->fFluxCapData[36] = 0.000000; ui6->fFluxCapData[37] = 0.000000; ui6->fFluxCapData[38] = 0.000000; ui6->fFluxCapData[39] = 0.000000; ui6->fFluxCapData[40] = 0.000000; ui6->fFluxCapData[41] = 0.000000; ui6->fFluxCapData[42] = 0.000000; ui6->fFluxCapData[43] = 0.000000; ui6->fFluxCapData[44] = 0.000000; ui6->fFluxCapData[45] = 0.000000; ui6->fFluxCapData[46] = 0.000000; ui6->fFluxCapData[47] = 0.000000; ui6->fFluxCapData[48] = 0.000000; ui6->fFluxCapData[49] = 0.000000; ui6->fFluxCapData[50] = 0.000000; ui6->fFluxCapData[51] = 0.000000; ui6->fFluxCapData[52] = 0.000000; ui6->fFluxCapData[53] = 0.000000; ui6->fFluxCapData[54] = 0.000000; ui6->fFluxCapData[55] = 0.000000; ui6->fFluxCapData[56] = 0.000000; ui6->fFluxCapData[57] = 0.000000; ui6->fFluxCapData[58] = 0.000000; ui6->fFluxCapData[59] = 0.000000; ui6->fFluxCapData[60] = 0.000000; ui6->fFluxCapData[61] = 0.000000; ui6->fFluxCapData[62] = 0.000000; ui6->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui6);
	delete ui6;


	m_fKRT = 0.000001;
	CUICtrl* ui7 = new CUICtrl;
	ui7->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui7->uControlId = 7;
	ui7->bLogSlider = false;
	ui7->bExpSlider = false;
	ui7->fUserDisplayDataLoLimit = 0.000001;
	ui7->fUserDisplayDataHiLimit = 0.800000;
	ui7->uUserDataType = floatData;
	ui7->fInitUserIntValue = 0;
	ui7->fInitUserFloatValue = 0.000001;
	ui7->fInitUserDoubleValue = 0;
	ui7->fInitUserUINTValue = 0;
	ui7->m_pUserCookedIntData = NULL;
	ui7->m_pUserCookedFloatData = &m_fKRT;
	ui7->m_pUserCookedDoubleData = NULL;
	ui7->m_pUserCookedUINTData = NULL;
	ui7->cControlUnits = "Multiplier";
	ui7->cVariableName = "m_fKRT";
	ui7->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui7->dPresetData[0] = 0.000001;ui7->dPresetData[1] = 0.796000;ui7->dPresetData[2] = 0.500000;ui7->dPresetData[3] = 0.500000;ui7->dPresetData[4] = 0.000000;ui7->dPresetData[5] = 0.500000;ui7->dPresetData[6] = 0.000000;ui7->dPresetData[7] = 0.000000;ui7->dPresetData[8] = 0.000000;ui7->dPresetData[9] = 0.000000;ui7->dPresetData[10] = 0.000000;ui7->dPresetData[11] = 0.000000;ui7->dPresetData[12] = 0.000000;ui7->dPresetData[13] = 0.000000;ui7->dPresetData[14] = 0.000000;ui7->dPresetData[15] = 0.000000;
	ui7->cControlName = "KRT";
	ui7->bOwnerControl = false;
	ui7->bMIDIControl = false;
	ui7->uMIDIControlCommand = 176;
	ui7->uMIDIControlName = 3;
	ui7->uMIDIControlChannel = 0;
	ui7->nGUIRow = nIndexer++;
	ui7->nGUIColumn = -1;
	ui7->bEnableParamSmoothing = false;
	ui7->fSmoothingTimeInMs = 100.00;
	ui7->uControlTheme[0] = 0; ui7->uControlTheme[1] = 0; ui7->uControlTheme[2] = 0; ui7->uControlTheme[3] = 0; ui7->uControlTheme[4] = 0; ui7->uControlTheme[5] = 0; ui7->uControlTheme[6] = 0; ui7->uControlTheme[7] = 0; ui7->uControlTheme[8] = 0; ui7->uControlTheme[9] = 0; ui7->uControlTheme[10] = 0; ui7->uControlTheme[11] = 0; ui7->uControlTheme[12] = 0; ui7->uControlTheme[13] = 0; ui7->uControlTheme[14] = 0; ui7->uControlTheme[15] = 0; ui7->uControlTheme[16] = 2; ui7->uControlTheme[17] = 0; ui7->uControlTheme[18] = 0; ui7->uControlTheme[19] = 0; ui7->uControlTheme[20] = 0; ui7->uControlTheme[21] = 0; ui7->uControlTheme[22] = 0; ui7->uControlTheme[23] = 0; ui7->uControlTheme[24] = 0; ui7->uControlTheme[25] = 0; ui7->uControlTheme[26] = 0; ui7->uControlTheme[27] = 0; ui7->uControlTheme[28] = 0; ui7->uControlTheme[29] = 0; ui7->uControlTheme[30] = 0; ui7->uControlTheme[31] = 0; 
	ui7->uFluxCapControl[0] = 0; ui7->uFluxCapControl[1] = 0; ui7->uFluxCapControl[2] = 0; ui7->uFluxCapControl[3] = 0; ui7->uFluxCapControl[4] = 0; ui7->uFluxCapControl[5] = 0; ui7->uFluxCapControl[6] = 0; ui7->uFluxCapControl[7] = 0; ui7->uFluxCapControl[8] = 0; ui7->uFluxCapControl[9] = 0; ui7->uFluxCapControl[10] = 0; ui7->uFluxCapControl[11] = 0; ui7->uFluxCapControl[12] = 0; ui7->uFluxCapControl[13] = 0; ui7->uFluxCapControl[14] = 0; ui7->uFluxCapControl[15] = 0; ui7->uFluxCapControl[16] = 0; ui7->uFluxCapControl[17] = 0; ui7->uFluxCapControl[18] = 0; ui7->uFluxCapControl[19] = 0; ui7->uFluxCapControl[20] = 0; ui7->uFluxCapControl[21] = 0; ui7->uFluxCapControl[22] = 0; ui7->uFluxCapControl[23] = 0; ui7->uFluxCapControl[24] = 0; ui7->uFluxCapControl[25] = 0; ui7->uFluxCapControl[26] = 0; ui7->uFluxCapControl[27] = 0; ui7->uFluxCapControl[28] = 0; ui7->uFluxCapControl[29] = 0; ui7->uFluxCapControl[30] = 0; ui7->uFluxCapControl[31] = 0; ui7->uFluxCapControl[32] = 0; ui7->uFluxCapControl[33] = 0; ui7->uFluxCapControl[34] = 0; ui7->uFluxCapControl[35] = 0; ui7->uFluxCapControl[36] = 0; ui7->uFluxCapControl[37] = 0; ui7->uFluxCapControl[38] = 0; ui7->uFluxCapControl[39] = 0; ui7->uFluxCapControl[40] = 0; ui7->uFluxCapControl[41] = 0; ui7->uFluxCapControl[42] = 0; ui7->uFluxCapControl[43] = 0; ui7->uFluxCapControl[44] = 0; ui7->uFluxCapControl[45] = 0; ui7->uFluxCapControl[46] = 0; ui7->uFluxCapControl[47] = 0; ui7->uFluxCapControl[48] = 0; ui7->uFluxCapControl[49] = 0; ui7->uFluxCapControl[50] = 0; ui7->uFluxCapControl[51] = 0; ui7->uFluxCapControl[52] = 0; ui7->uFluxCapControl[53] = 0; ui7->uFluxCapControl[54] = 0; ui7->uFluxCapControl[55] = 0; ui7->uFluxCapControl[56] = 0; ui7->uFluxCapControl[57] = 0; ui7->uFluxCapControl[58] = 0; ui7->uFluxCapControl[59] = 0; ui7->uFluxCapControl[60] = 0; ui7->uFluxCapControl[61] = 0; ui7->uFluxCapControl[62] = 0; ui7->uFluxCapControl[63] = 0; 
	ui7->fFluxCapData[0] = 0.000000; ui7->fFluxCapData[1] = 0.000000; ui7->fFluxCapData[2] = 0.000000; ui7->fFluxCapData[3] = 0.000000; ui7->fFluxCapData[4] = 0.000000; ui7->fFluxCapData[5] = 0.000000; ui7->fFluxCapData[6] = 0.000000; ui7->fFluxCapData[7] = 0.000000; ui7->fFluxCapData[8] = 0.000000; ui7->fFluxCapData[9] = 0.000000; ui7->fFluxCapData[10] = 0.000000; ui7->fFluxCapData[11] = 0.000000; ui7->fFluxCapData[12] = 0.000000; ui7->fFluxCapData[13] = 0.000000; ui7->fFluxCapData[14] = 0.000000; ui7->fFluxCapData[15] = 0.000000; ui7->fFluxCapData[16] = 0.000000; ui7->fFluxCapData[17] = 0.000000; ui7->fFluxCapData[18] = 0.000000; ui7->fFluxCapData[19] = 0.000000; ui7->fFluxCapData[20] = 0.000000; ui7->fFluxCapData[21] = 0.000000; ui7->fFluxCapData[22] = 0.000000; ui7->fFluxCapData[23] = 0.000000; ui7->fFluxCapData[24] = 0.000000; ui7->fFluxCapData[25] = 0.000000; ui7->fFluxCapData[26] = 0.000000; ui7->fFluxCapData[27] = 0.000000; ui7->fFluxCapData[28] = 0.000000; ui7->fFluxCapData[29] = 0.000000; ui7->fFluxCapData[30] = 0.000000; ui7->fFluxCapData[31] = 0.000000; ui7->fFluxCapData[32] = 0.000000; ui7->fFluxCapData[33] = 0.000000; ui7->fFluxCapData[34] = 0.000000; ui7->fFluxCapData[35] = 0.000000; ui7->fFluxCapData[36] = 0.000000; ui7->fFluxCapData[37] = 0.000000; ui7->fFluxCapData[38] = 0.000000; ui7->fFluxCapData[39] = 0.000000; ui7->fFluxCapData[40] = 0.000000; ui7->fFluxCapData[41] = 0.000000; ui7->fFluxCapData[42] = 0.000000; ui7->fFluxCapData[43] = 0.000000; ui7->fFluxCapData[44] = 0.000000; ui7->fFluxCapData[45] = 0.000000; ui7->fFluxCapData[46] = 0.000000; ui7->fFluxCapData[47] = 0.000000; ui7->fFluxCapData[48] = 0.000000; ui7->fFluxCapData[49] = 0.000000; ui7->fFluxCapData[50] = 0.000000; ui7->fFluxCapData[51] = 0.000000; ui7->fFluxCapData[52] = 0.000000; ui7->fFluxCapData[53] = 0.000000; ui7->fFluxCapData[54] = 0.000000; ui7->fFluxCapData[55] = 0.000000; ui7->fFluxCapData[56] = 0.000000; ui7->fFluxCapData[57] = 0.000000; ui7->fFluxCapData[58] = 0.000000; ui7->fFluxCapData[59] = 0.000000; ui7->fFluxCapData[60] = 0.000000; ui7->fFluxCapData[61] = 0.000000; ui7->fFluxCapData[62] = 0.000000; ui7->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui7);
	delete ui7;


	m_fRT60 = 1800.000000;
	CUICtrl* ui8 = new CUICtrl;
	ui8->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui8->uControlId = 8;
	ui8->bLogSlider = false;
	ui8->bExpSlider = false;
	ui8->fUserDisplayDataLoLimit = 0.000000;
	ui8->fUserDisplayDataHiLimit = 5000.000000;
	ui8->uUserDataType = floatData;
	ui8->fInitUserIntValue = 0;
	ui8->fInitUserFloatValue = 1800.000000;
	ui8->fInitUserDoubleValue = 0;
	ui8->fInitUserUINTValue = 0;
	ui8->m_pUserCookedIntData = NULL;
	ui8->m_pUserCookedFloatData = &m_fRT60;
	ui8->m_pUserCookedDoubleData = NULL;
	ui8->m_pUserCookedUINTData = NULL;
	ui8->cControlUnits = "mSec";
	ui8->cVariableName = "m_fRT60";
	ui8->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui8->dPresetData[0] = 1800.000122;ui8->dPresetData[1] = 5000.000000;ui8->dPresetData[2] = 1450.000000;ui8->dPresetData[3] = 1324.999878;ui8->dPresetData[4] = 0.000000;ui8->dPresetData[5] = 1450.000000;ui8->dPresetData[6] = 0.000000;ui8->dPresetData[7] = 0.000000;ui8->dPresetData[8] = 0.000000;ui8->dPresetData[9] = 0.000000;ui8->dPresetData[10] = 0.000000;ui8->dPresetData[11] = 0.000000;ui8->dPresetData[12] = 0.000000;ui8->dPresetData[13] = 0.000000;ui8->dPresetData[14] = 0.000000;ui8->dPresetData[15] = 0.000000;
	ui8->cControlName = "RT60";
	ui8->bOwnerControl = false;
	ui8->bMIDIControl = false;
	ui8->uMIDIControlCommand = 176;
	ui8->uMIDIControlName = 3;
	ui8->uMIDIControlChannel = 0;
	ui8->nGUIRow = nIndexer++;
	ui8->nGUIColumn = -1;
	ui8->bEnableParamSmoothing = false;
	ui8->fSmoothingTimeInMs = 100.00;
	ui8->uControlTheme[0] = 0; ui8->uControlTheme[1] = 0; ui8->uControlTheme[2] = 0; ui8->uControlTheme[3] = 0; ui8->uControlTheme[4] = 0; ui8->uControlTheme[5] = 0; ui8->uControlTheme[6] = 0; ui8->uControlTheme[7] = 0; ui8->uControlTheme[8] = 0; ui8->uControlTheme[9] = 0; ui8->uControlTheme[10] = 0; ui8->uControlTheme[11] = 0; ui8->uControlTheme[12] = 0; ui8->uControlTheme[13] = 0; ui8->uControlTheme[14] = 0; ui8->uControlTheme[15] = 0; ui8->uControlTheme[16] = 2; ui8->uControlTheme[17] = 0; ui8->uControlTheme[18] = 0; ui8->uControlTheme[19] = 0; ui8->uControlTheme[20] = 0; ui8->uControlTheme[21] = 0; ui8->uControlTheme[22] = 0; ui8->uControlTheme[23] = 0; ui8->uControlTheme[24] = 0; ui8->uControlTheme[25] = 0; ui8->uControlTheme[26] = 0; ui8->uControlTheme[27] = 0; ui8->uControlTheme[28] = 0; ui8->uControlTheme[29] = 0; ui8->uControlTheme[30] = 0; ui8->uControlTheme[31] = 0; 
	ui8->uFluxCapControl[0] = 0; ui8->uFluxCapControl[1] = 0; ui8->uFluxCapControl[2] = 0; ui8->uFluxCapControl[3] = 0; ui8->uFluxCapControl[4] = 0; ui8->uFluxCapControl[5] = 0; ui8->uFluxCapControl[6] = 0; ui8->uFluxCapControl[7] = 0; ui8->uFluxCapControl[8] = 0; ui8->uFluxCapControl[9] = 0; ui8->uFluxCapControl[10] = 0; ui8->uFluxCapControl[11] = 0; ui8->uFluxCapControl[12] = 0; ui8->uFluxCapControl[13] = 0; ui8->uFluxCapControl[14] = 0; ui8->uFluxCapControl[15] = 0; ui8->uFluxCapControl[16] = 0; ui8->uFluxCapControl[17] = 0; ui8->uFluxCapControl[18] = 0; ui8->uFluxCapControl[19] = 0; ui8->uFluxCapControl[20] = 0; ui8->uFluxCapControl[21] = 0; ui8->uFluxCapControl[22] = 0; ui8->uFluxCapControl[23] = 0; ui8->uFluxCapControl[24] = 0; ui8->uFluxCapControl[25] = 0; ui8->uFluxCapControl[26] = 0; ui8->uFluxCapControl[27] = 0; ui8->uFluxCapControl[28] = 0; ui8->uFluxCapControl[29] = 0; ui8->uFluxCapControl[30] = 0; ui8->uFluxCapControl[31] = 0; ui8->uFluxCapControl[32] = 0; ui8->uFluxCapControl[33] = 0; ui8->uFluxCapControl[34] = 0; ui8->uFluxCapControl[35] = 0; ui8->uFluxCapControl[36] = 0; ui8->uFluxCapControl[37] = 0; ui8->uFluxCapControl[38] = 0; ui8->uFluxCapControl[39] = 0; ui8->uFluxCapControl[40] = 0; ui8->uFluxCapControl[41] = 0; ui8->uFluxCapControl[42] = 0; ui8->uFluxCapControl[43] = 0; ui8->uFluxCapControl[44] = 0; ui8->uFluxCapControl[45] = 0; ui8->uFluxCapControl[46] = 0; ui8->uFluxCapControl[47] = 0; ui8->uFluxCapControl[48] = 0; ui8->uFluxCapControl[49] = 0; ui8->uFluxCapControl[50] = 0; ui8->uFluxCapControl[51] = 0; ui8->uFluxCapControl[52] = 0; ui8->uFluxCapControl[53] = 0; ui8->uFluxCapControl[54] = 0; ui8->uFluxCapControl[55] = 0; ui8->uFluxCapControl[56] = 0; ui8->uFluxCapControl[57] = 0; ui8->uFluxCapControl[58] = 0; ui8->uFluxCapControl[59] = 0; ui8->uFluxCapControl[60] = 0; ui8->uFluxCapControl[61] = 0; ui8->uFluxCapControl[62] = 0; ui8->uFluxCapControl[63] = 0; 
	ui8->fFluxCapData[0] = 0.000000; ui8->fFluxCapData[1] = 0.000000; ui8->fFluxCapData[2] = 0.000000; ui8->fFluxCapData[3] = 0.000000; ui8->fFluxCapData[4] = 0.000000; ui8->fFluxCapData[5] = 0.000000; ui8->fFluxCapData[6] = 0.000000; ui8->fFluxCapData[7] = 0.000000; ui8->fFluxCapData[8] = 0.000000; ui8->fFluxCapData[9] = 0.000000; ui8->fFluxCapData[10] = 0.000000; ui8->fFluxCapData[11] = 0.000000; ui8->fFluxCapData[12] = 0.000000; ui8->fFluxCapData[13] = 0.000000; ui8->fFluxCapData[14] = 0.000000; ui8->fFluxCapData[15] = 0.000000; ui8->fFluxCapData[16] = 0.000000; ui8->fFluxCapData[17] = 0.000000; ui8->fFluxCapData[18] = 0.000000; ui8->fFluxCapData[19] = 0.000000; ui8->fFluxCapData[20] = 0.000000; ui8->fFluxCapData[21] = 0.000000; ui8->fFluxCapData[22] = 0.000000; ui8->fFluxCapData[23] = 0.000000; ui8->fFluxCapData[24] = 0.000000; ui8->fFluxCapData[25] = 0.000000; ui8->fFluxCapData[26] = 0.000000; ui8->fFluxCapData[27] = 0.000000; ui8->fFluxCapData[28] = 0.000000; ui8->fFluxCapData[29] = 0.000000; ui8->fFluxCapData[30] = 0.000000; ui8->fFluxCapData[31] = 0.000000; ui8->fFluxCapData[32] = 0.000000; ui8->fFluxCapData[33] = 0.000000; ui8->fFluxCapData[34] = 0.000000; ui8->fFluxCapData[35] = 0.000000; ui8->fFluxCapData[36] = 0.000000; ui8->fFluxCapData[37] = 0.000000; ui8->fFluxCapData[38] = 0.000000; ui8->fFluxCapData[39] = 0.000000; ui8->fFluxCapData[40] = 0.000000; ui8->fFluxCapData[41] = 0.000000; ui8->fFluxCapData[42] = 0.000000; ui8->fFluxCapData[43] = 0.000000; ui8->fFluxCapData[44] = 0.000000; ui8->fFluxCapData[45] = 0.000000; ui8->fFluxCapData[46] = 0.000000; ui8->fFluxCapData[47] = 0.000000; ui8->fFluxCapData[48] = 0.000000; ui8->fFluxCapData[49] = 0.000000; ui8->fFluxCapData[50] = 0.000000; ui8->fFluxCapData[51] = 0.000000; ui8->fFluxCapData[52] = 0.000000; ui8->fFluxCapData[53] = 0.000000; ui8->fFluxCapData[54] = 0.000000; ui8->fFluxCapData[55] = 0.000000; ui8->fFluxCapData[56] = 0.000000; ui8->fFluxCapData[57] = 0.000000; ui8->fFluxCapData[58] = 0.000000; ui8->fFluxCapData[59] = 0.000000; ui8->fFluxCapData[60] = 0.000000; ui8->fFluxCapData[61] = 0.000000; ui8->fFluxCapData[62] = 0.000000; ui8->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui8);
	delete ui8;


	m_fWet_pct = 50.000000;
	CUICtrl* ui9 = new CUICtrl;
	ui9->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui9->uControlId = 9;
	ui9->bLogSlider = false;
	ui9->bExpSlider = false;
	ui9->fUserDisplayDataLoLimit = 0.000000;
	ui9->fUserDisplayDataHiLimit = 100.000000;
	ui9->uUserDataType = floatData;
	ui9->fInitUserIntValue = 0;
	ui9->fInitUserFloatValue = 50.000000;
	ui9->fInitUserDoubleValue = 0;
	ui9->fInitUserUINTValue = 0;
	ui9->m_pUserCookedIntData = NULL;
	ui9->m_pUserCookedFloatData = &m_fWet_pct;
	ui9->m_pUserCookedDoubleData = NULL;
	ui9->m_pUserCookedUINTData = NULL;
	ui9->cControlUnits = "%";
	ui9->cVariableName = "m_fWet_pct";
	ui9->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui9->dPresetData[0] = 50.000000;ui9->dPresetData[1] = 100.000000;ui9->dPresetData[2] = 53.000004;ui9->dPresetData[3] = 47.500004;ui9->dPresetData[4] = 0.000000;ui9->dPresetData[5] = 53.000004;ui9->dPresetData[6] = 0.000000;ui9->dPresetData[7] = 0.000000;ui9->dPresetData[8] = 0.000000;ui9->dPresetData[9] = 0.000000;ui9->dPresetData[10] = 0.000000;ui9->dPresetData[11] = 0.000000;ui9->dPresetData[12] = 0.000000;ui9->dPresetData[13] = 0.000000;ui9->dPresetData[14] = 0.000000;ui9->dPresetData[15] = 0.000000;
	ui9->cControlName = "Dry/Wet";
	ui9->bOwnerControl = false;
	ui9->bMIDIControl = true;
	ui9->uMIDIControlCommand = 176;
	ui9->uMIDIControlName = 3;
	ui9->uMIDIControlChannel = 0;
	ui9->nGUIRow = nIndexer++;
	ui9->nGUIColumn = -1;
	ui9->bEnableParamSmoothing = false;
	ui9->fSmoothingTimeInMs = 100.00;
	ui9->uControlTheme[0] = 0; ui9->uControlTheme[1] = 0; ui9->uControlTheme[2] = 0; ui9->uControlTheme[3] = 0; ui9->uControlTheme[4] = 0; ui9->uControlTheme[5] = 0; ui9->uControlTheme[6] = 0; ui9->uControlTheme[7] = 0; ui9->uControlTheme[8] = 0; ui9->uControlTheme[9] = 0; ui9->uControlTheme[10] = 0; ui9->uControlTheme[11] = 0; ui9->uControlTheme[12] = 0; ui9->uControlTheme[13] = 0; ui9->uControlTheme[14] = 0; ui9->uControlTheme[15] = 0; ui9->uControlTheme[16] = 2; ui9->uControlTheme[17] = 0; ui9->uControlTheme[18] = 0; ui9->uControlTheme[19] = 0; ui9->uControlTheme[20] = 0; ui9->uControlTheme[21] = 0; ui9->uControlTheme[22] = 0; ui9->uControlTheme[23] = 0; ui9->uControlTheme[24] = 0; ui9->uControlTheme[25] = 0; ui9->uControlTheme[26] = 0; ui9->uControlTheme[27] = 0; ui9->uControlTheme[28] = 0; ui9->uControlTheme[29] = 0; ui9->uControlTheme[30] = 0; ui9->uControlTheme[31] = 0; 
	ui9->uFluxCapControl[0] = 0; ui9->uFluxCapControl[1] = 0; ui9->uFluxCapControl[2] = 0; ui9->uFluxCapControl[3] = 0; ui9->uFluxCapControl[4] = 0; ui9->uFluxCapControl[5] = 0; ui9->uFluxCapControl[6] = 0; ui9->uFluxCapControl[7] = 0; ui9->uFluxCapControl[8] = 0; ui9->uFluxCapControl[9] = 0; ui9->uFluxCapControl[10] = 0; ui9->uFluxCapControl[11] = 0; ui9->uFluxCapControl[12] = 0; ui9->uFluxCapControl[13] = 0; ui9->uFluxCapControl[14] = 0; ui9->uFluxCapControl[15] = 0; ui9->uFluxCapControl[16] = 0; ui9->uFluxCapControl[17] = 0; ui9->uFluxCapControl[18] = 0; ui9->uFluxCapControl[19] = 0; ui9->uFluxCapControl[20] = 0; ui9->uFluxCapControl[21] = 0; ui9->uFluxCapControl[22] = 0; ui9->uFluxCapControl[23] = 0; ui9->uFluxCapControl[24] = 0; ui9->uFluxCapControl[25] = 0; ui9->uFluxCapControl[26] = 0; ui9->uFluxCapControl[27] = 0; ui9->uFluxCapControl[28] = 0; ui9->uFluxCapControl[29] = 0; ui9->uFluxCapControl[30] = 0; ui9->uFluxCapControl[31] = 0; ui9->uFluxCapControl[32] = 0; ui9->uFluxCapControl[33] = 0; ui9->uFluxCapControl[34] = 0; ui9->uFluxCapControl[35] = 0; ui9->uFluxCapControl[36] = 0; ui9->uFluxCapControl[37] = 0; ui9->uFluxCapControl[38] = 0; ui9->uFluxCapControl[39] = 0; ui9->uFluxCapControl[40] = 0; ui9->uFluxCapControl[41] = 0; ui9->uFluxCapControl[42] = 0; ui9->uFluxCapControl[43] = 0; ui9->uFluxCapControl[44] = 0; ui9->uFluxCapControl[45] = 0; ui9->uFluxCapControl[46] = 0; ui9->uFluxCapControl[47] = 0; ui9->uFluxCapControl[48] = 0; ui9->uFluxCapControl[49] = 0; ui9->uFluxCapControl[50] = 0; ui9->uFluxCapControl[51] = 0; ui9->uFluxCapControl[52] = 0; ui9->uFluxCapControl[53] = 0; ui9->uFluxCapControl[54] = 0; ui9->uFluxCapControl[55] = 0; ui9->uFluxCapControl[56] = 0; ui9->uFluxCapControl[57] = 0; ui9->uFluxCapControl[58] = 0; ui9->uFluxCapControl[59] = 0; ui9->uFluxCapControl[60] = 0; ui9->uFluxCapControl[61] = 0; ui9->uFluxCapControl[62] = 0; ui9->uFluxCapControl[63] = 0; 
	ui9->fFluxCapData[0] = 0.000000; ui9->fFluxCapData[1] = 0.000000; ui9->fFluxCapData[2] = 0.000000; ui9->fFluxCapData[3] = 0.000000; ui9->fFluxCapData[4] = 0.000000; ui9->fFluxCapData[5] = 0.000000; ui9->fFluxCapData[6] = 0.000000; ui9->fFluxCapData[7] = 0.000000; ui9->fFluxCapData[8] = 0.000000; ui9->fFluxCapData[9] = 0.000000; ui9->fFluxCapData[10] = 0.000000; ui9->fFluxCapData[11] = 0.000000; ui9->fFluxCapData[12] = 0.000000; ui9->fFluxCapData[13] = 0.000000; ui9->fFluxCapData[14] = 0.000000; ui9->fFluxCapData[15] = 0.000000; ui9->fFluxCapData[16] = 0.000000; ui9->fFluxCapData[17] = 0.000000; ui9->fFluxCapData[18] = 0.000000; ui9->fFluxCapData[19] = 0.000000; ui9->fFluxCapData[20] = 0.000000; ui9->fFluxCapData[21] = 0.000000; ui9->fFluxCapData[22] = 0.000000; ui9->fFluxCapData[23] = 0.000000; ui9->fFluxCapData[24] = 0.000000; ui9->fFluxCapData[25] = 0.000000; ui9->fFluxCapData[26] = 0.000000; ui9->fFluxCapData[27] = 0.000000; ui9->fFluxCapData[28] = 0.000000; ui9->fFluxCapData[29] = 0.000000; ui9->fFluxCapData[30] = 0.000000; ui9->fFluxCapData[31] = 0.000000; ui9->fFluxCapData[32] = 0.000000; ui9->fFluxCapData[33] = 0.000000; ui9->fFluxCapData[34] = 0.000000; ui9->fFluxCapData[35] = 0.000000; ui9->fFluxCapData[36] = 0.000000; ui9->fFluxCapData[37] = 0.000000; ui9->fFluxCapData[38] = 0.000000; ui9->fFluxCapData[39] = 0.000000; ui9->fFluxCapData[40] = 0.000000; ui9->fFluxCapData[41] = 0.000000; ui9->fFluxCapData[42] = 0.000000; ui9->fFluxCapData[43] = 0.000000; ui9->fFluxCapData[44] = 0.000000; ui9->fFluxCapData[45] = 0.000000; ui9->fFluxCapData[46] = 0.000000; ui9->fFluxCapData[47] = 0.000000; ui9->fFluxCapData[48] = 0.000000; ui9->fFluxCapData[49] = 0.000000; ui9->fFluxCapData[50] = 0.000000; ui9->fFluxCapData[51] = 0.000000; ui9->fFluxCapData[52] = 0.000000; ui9->fFluxCapData[53] = 0.000000; ui9->fFluxCapData[54] = 0.000000; ui9->fFluxCapData[55] = 0.000000; ui9->fFluxCapData[56] = 0.000000; ui9->fFluxCapData[57] = 0.000000; ui9->fFluxCapData[58] = 0.000000; ui9->fFluxCapData[59] = 0.000000; ui9->fFluxCapData[60] = 0.000000; ui9->fFluxCapData[61] = 0.000000; ui9->fFluxCapData[62] = 0.000000; ui9->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui9);
	delete ui9;


	m_fPComb_1_Delay_mSec = 31.709999;
	CUICtrl* ui10 = new CUICtrl;
	ui10->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui10->uControlId = 12;
	ui10->bLogSlider = false;
	ui10->bExpSlider = false;
	ui10->fUserDisplayDataLoLimit = 0.000000;
	ui10->fUserDisplayDataHiLimit = 100.000000;
	ui10->uUserDataType = floatData;
	ui10->fInitUserIntValue = 0;
	ui10->fInitUserFloatValue = 31.709999;
	ui10->fInitUserDoubleValue = 0;
	ui10->fInitUserUINTValue = 0;
	ui10->m_pUserCookedIntData = NULL;
	ui10->m_pUserCookedFloatData = &m_fPComb_1_Delay_mSec;
	ui10->m_pUserCookedDoubleData = NULL;
	ui10->m_pUserCookedUINTData = NULL;
	ui10->cControlUnits = "mSec                            Comb Bank 1";
	ui10->cVariableName = "m_fPComb_1_Delay_mSec";
	ui10->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui10->dPresetData[0] = 31.709999;ui10->dPresetData[1] = 31.709999;ui10->dPresetData[2] = 31.709999;ui10->dPresetData[3] = 6.710000;ui10->dPresetData[4] = 0.000000;ui10->dPresetData[5] = 31.709999;ui10->dPresetData[6] = 0.000000;ui10->dPresetData[7] = 0.000000;ui10->dPresetData[8] = 0.000000;ui10->dPresetData[9] = 0.000000;ui10->dPresetData[10] = 0.000000;ui10->dPresetData[11] = 0.000000;ui10->dPresetData[12] = 0.000000;ui10->dPresetData[13] = 0.000000;ui10->dPresetData[14] = 0.000000;ui10->dPresetData[15] = 0.000000;
	ui10->cControlName = "PComb1 Dly";
	ui10->bOwnerControl = false;
	ui10->bMIDIControl = false;
	ui10->uMIDIControlCommand = 176;
	ui10->uMIDIControlName = 3;
	ui10->uMIDIControlChannel = 0;
	ui10->nGUIRow = nIndexer++;
	ui10->nGUIColumn = -1;
	ui10->bEnableParamSmoothing = false;
	ui10->fSmoothingTimeInMs = 100.00;
	ui10->uControlTheme[0] = 0; ui10->uControlTheme[1] = 0; ui10->uControlTheme[2] = 0; ui10->uControlTheme[3] = 0; ui10->uControlTheme[4] = 0; ui10->uControlTheme[5] = 0; ui10->uControlTheme[6] = 0; ui10->uControlTheme[7] = 0; ui10->uControlTheme[8] = 0; ui10->uControlTheme[9] = 0; ui10->uControlTheme[10] = 0; ui10->uControlTheme[11] = 0; ui10->uControlTheme[12] = 0; ui10->uControlTheme[13] = 0; ui10->uControlTheme[14] = 0; ui10->uControlTheme[15] = 0; ui10->uControlTheme[16] = 2; ui10->uControlTheme[17] = 0; ui10->uControlTheme[18] = 0; ui10->uControlTheme[19] = 0; ui10->uControlTheme[20] = 0; ui10->uControlTheme[21] = 0; ui10->uControlTheme[22] = 0; ui10->uControlTheme[23] = 0; ui10->uControlTheme[24] = 0; ui10->uControlTheme[25] = 0; ui10->uControlTheme[26] = 0; ui10->uControlTheme[27] = 0; ui10->uControlTheme[28] = 0; ui10->uControlTheme[29] = 0; ui10->uControlTheme[30] = 0; ui10->uControlTheme[31] = 0; 
	ui10->uFluxCapControl[0] = 0; ui10->uFluxCapControl[1] = 0; ui10->uFluxCapControl[2] = 0; ui10->uFluxCapControl[3] = 0; ui10->uFluxCapControl[4] = 0; ui10->uFluxCapControl[5] = 0; ui10->uFluxCapControl[6] = 0; ui10->uFluxCapControl[7] = 0; ui10->uFluxCapControl[8] = 0; ui10->uFluxCapControl[9] = 0; ui10->uFluxCapControl[10] = 0; ui10->uFluxCapControl[11] = 0; ui10->uFluxCapControl[12] = 0; ui10->uFluxCapControl[13] = 0; ui10->uFluxCapControl[14] = 0; ui10->uFluxCapControl[15] = 0; ui10->uFluxCapControl[16] = 0; ui10->uFluxCapControl[17] = 0; ui10->uFluxCapControl[18] = 0; ui10->uFluxCapControl[19] = 0; ui10->uFluxCapControl[20] = 0; ui10->uFluxCapControl[21] = 0; ui10->uFluxCapControl[22] = 0; ui10->uFluxCapControl[23] = 0; ui10->uFluxCapControl[24] = 0; ui10->uFluxCapControl[25] = 0; ui10->uFluxCapControl[26] = 0; ui10->uFluxCapControl[27] = 0; ui10->uFluxCapControl[28] = 0; ui10->uFluxCapControl[29] = 0; ui10->uFluxCapControl[30] = 0; ui10->uFluxCapControl[31] = 0; ui10->uFluxCapControl[32] = 0; ui10->uFluxCapControl[33] = 0; ui10->uFluxCapControl[34] = 0; ui10->uFluxCapControl[35] = 0; ui10->uFluxCapControl[36] = 0; ui10->uFluxCapControl[37] = 0; ui10->uFluxCapControl[38] = 0; ui10->uFluxCapControl[39] = 0; ui10->uFluxCapControl[40] = 0; ui10->uFluxCapControl[41] = 0; ui10->uFluxCapControl[42] = 0; ui10->uFluxCapControl[43] = 0; ui10->uFluxCapControl[44] = 0; ui10->uFluxCapControl[45] = 0; ui10->uFluxCapControl[46] = 0; ui10->uFluxCapControl[47] = 0; ui10->uFluxCapControl[48] = 0; ui10->uFluxCapControl[49] = 0; ui10->uFluxCapControl[50] = 0; ui10->uFluxCapControl[51] = 0; ui10->uFluxCapControl[52] = 0; ui10->uFluxCapControl[53] = 0; ui10->uFluxCapControl[54] = 0; ui10->uFluxCapControl[55] = 0; ui10->uFluxCapControl[56] = 0; ui10->uFluxCapControl[57] = 0; ui10->uFluxCapControl[58] = 0; ui10->uFluxCapControl[59] = 0; ui10->uFluxCapControl[60] = 0; ui10->uFluxCapControl[61] = 0; ui10->uFluxCapControl[62] = 0; ui10->uFluxCapControl[63] = 0; 
	ui10->fFluxCapData[0] = 0.000000; ui10->fFluxCapData[1] = 0.000000; ui10->fFluxCapData[2] = 0.000000; ui10->fFluxCapData[3] = 0.000000; ui10->fFluxCapData[4] = 0.000000; ui10->fFluxCapData[5] = 0.000000; ui10->fFluxCapData[6] = 0.000000; ui10->fFluxCapData[7] = 0.000000; ui10->fFluxCapData[8] = 0.000000; ui10->fFluxCapData[9] = 0.000000; ui10->fFluxCapData[10] = 0.000000; ui10->fFluxCapData[11] = 0.000000; ui10->fFluxCapData[12] = 0.000000; ui10->fFluxCapData[13] = 0.000000; ui10->fFluxCapData[14] = 0.000000; ui10->fFluxCapData[15] = 0.000000; ui10->fFluxCapData[16] = 0.000000; ui10->fFluxCapData[17] = 0.000000; ui10->fFluxCapData[18] = 0.000000; ui10->fFluxCapData[19] = 0.000000; ui10->fFluxCapData[20] = 0.000000; ui10->fFluxCapData[21] = 0.000000; ui10->fFluxCapData[22] = 0.000000; ui10->fFluxCapData[23] = 0.000000; ui10->fFluxCapData[24] = 0.000000; ui10->fFluxCapData[25] = 0.000000; ui10->fFluxCapData[26] = 0.000000; ui10->fFluxCapData[27] = 0.000000; ui10->fFluxCapData[28] = 0.000000; ui10->fFluxCapData[29] = 0.000000; ui10->fFluxCapData[30] = 0.000000; ui10->fFluxCapData[31] = 0.000000; ui10->fFluxCapData[32] = 0.000000; ui10->fFluxCapData[33] = 0.000000; ui10->fFluxCapData[34] = 0.000000; ui10->fFluxCapData[35] = 0.000000; ui10->fFluxCapData[36] = 0.000000; ui10->fFluxCapData[37] = 0.000000; ui10->fFluxCapData[38] = 0.000000; ui10->fFluxCapData[39] = 0.000000; ui10->fFluxCapData[40] = 0.000000; ui10->fFluxCapData[41] = 0.000000; ui10->fFluxCapData[42] = 0.000000; ui10->fFluxCapData[43] = 0.000000; ui10->fFluxCapData[44] = 0.000000; ui10->fFluxCapData[45] = 0.000000; ui10->fFluxCapData[46] = 0.000000; ui10->fFluxCapData[47] = 0.000000; ui10->fFluxCapData[48] = 0.000000; ui10->fFluxCapData[49] = 0.000000; ui10->fFluxCapData[50] = 0.000000; ui10->fFluxCapData[51] = 0.000000; ui10->fFluxCapData[52] = 0.000000; ui10->fFluxCapData[53] = 0.000000; ui10->fFluxCapData[54] = 0.000000; ui10->fFluxCapData[55] = 0.000000; ui10->fFluxCapData[56] = 0.000000; ui10->fFluxCapData[57] = 0.000000; ui10->fFluxCapData[58] = 0.000000; ui10->fFluxCapData[59] = 0.000000; ui10->fFluxCapData[60] = 0.000000; ui10->fFluxCapData[61] = 0.000000; ui10->fFluxCapData[62] = 0.000000; ui10->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui10);
	delete ui10;


	m_fPComb_2_Delay_mSec = 37.110001;
	CUICtrl* ui11 = new CUICtrl;
	ui11->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui11->uControlId = 13;
	ui11->bLogSlider = false;
	ui11->bExpSlider = false;
	ui11->fUserDisplayDataLoLimit = 0.000000;
	ui11->fUserDisplayDataHiLimit = 100.000000;
	ui11->uUserDataType = floatData;
	ui11->fInitUserIntValue = 0;
	ui11->fInitUserFloatValue = 37.110001;
	ui11->fInitUserDoubleValue = 0;
	ui11->fInitUserUINTValue = 0;
	ui11->m_pUserCookedIntData = NULL;
	ui11->m_pUserCookedFloatData = &m_fPComb_2_Delay_mSec;
	ui11->m_pUserCookedDoubleData = NULL;
	ui11->m_pUserCookedUINTData = NULL;
	ui11->cControlUnits = "mSec";
	ui11->cVariableName = "m_fPComb_2_Delay_mSec";
	ui11->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui11->dPresetData[0] = 37.110001;ui11->dPresetData[1] = 37.110001;ui11->dPresetData[2] = 37.110001;ui11->dPresetData[3] = 8.000000;ui11->dPresetData[4] = 0.000000;ui11->dPresetData[5] = 37.110001;ui11->dPresetData[6] = 0.000000;ui11->dPresetData[7] = 0.000000;ui11->dPresetData[8] = 0.000000;ui11->dPresetData[9] = 0.000000;ui11->dPresetData[10] = 0.000000;ui11->dPresetData[11] = 0.000000;ui11->dPresetData[12] = 0.000000;ui11->dPresetData[13] = 0.000000;ui11->dPresetData[14] = 0.000000;ui11->dPresetData[15] = 0.000000;
	ui11->cControlName = "PComb2 Dly";
	ui11->bOwnerControl = false;
	ui11->bMIDIControl = false;
	ui11->uMIDIControlCommand = 176;
	ui11->uMIDIControlName = 3;
	ui11->uMIDIControlChannel = 0;
	ui11->nGUIRow = nIndexer++;
	ui11->nGUIColumn = -1;
	ui11->bEnableParamSmoothing = false;
	ui11->fSmoothingTimeInMs = 100.00;
	ui11->uControlTheme[0] = 0; ui11->uControlTheme[1] = 0; ui11->uControlTheme[2] = 0; ui11->uControlTheme[3] = 0; ui11->uControlTheme[4] = 0; ui11->uControlTheme[5] = 0; ui11->uControlTheme[6] = 0; ui11->uControlTheme[7] = 0; ui11->uControlTheme[8] = 0; ui11->uControlTheme[9] = 0; ui11->uControlTheme[10] = 0; ui11->uControlTheme[11] = 0; ui11->uControlTheme[12] = 0; ui11->uControlTheme[13] = 0; ui11->uControlTheme[14] = 0; ui11->uControlTheme[15] = 0; ui11->uControlTheme[16] = 2; ui11->uControlTheme[17] = 0; ui11->uControlTheme[18] = 0; ui11->uControlTheme[19] = 0; ui11->uControlTheme[20] = 0; ui11->uControlTheme[21] = 0; ui11->uControlTheme[22] = 0; ui11->uControlTheme[23] = 0; ui11->uControlTheme[24] = 0; ui11->uControlTheme[25] = 0; ui11->uControlTheme[26] = 0; ui11->uControlTheme[27] = 0; ui11->uControlTheme[28] = 0; ui11->uControlTheme[29] = 0; ui11->uControlTheme[30] = 0; ui11->uControlTheme[31] = 0; 
	ui11->uFluxCapControl[0] = 0; ui11->uFluxCapControl[1] = 0; ui11->uFluxCapControl[2] = 0; ui11->uFluxCapControl[3] = 0; ui11->uFluxCapControl[4] = 0; ui11->uFluxCapControl[5] = 0; ui11->uFluxCapControl[6] = 0; ui11->uFluxCapControl[7] = 0; ui11->uFluxCapControl[8] = 0; ui11->uFluxCapControl[9] = 0; ui11->uFluxCapControl[10] = 0; ui11->uFluxCapControl[11] = 0; ui11->uFluxCapControl[12] = 0; ui11->uFluxCapControl[13] = 0; ui11->uFluxCapControl[14] = 0; ui11->uFluxCapControl[15] = 0; ui11->uFluxCapControl[16] = 0; ui11->uFluxCapControl[17] = 0; ui11->uFluxCapControl[18] = 0; ui11->uFluxCapControl[19] = 0; ui11->uFluxCapControl[20] = 0; ui11->uFluxCapControl[21] = 0; ui11->uFluxCapControl[22] = 0; ui11->uFluxCapControl[23] = 0; ui11->uFluxCapControl[24] = 0; ui11->uFluxCapControl[25] = 0; ui11->uFluxCapControl[26] = 0; ui11->uFluxCapControl[27] = 0; ui11->uFluxCapControl[28] = 0; ui11->uFluxCapControl[29] = 0; ui11->uFluxCapControl[30] = 0; ui11->uFluxCapControl[31] = 0; ui11->uFluxCapControl[32] = 0; ui11->uFluxCapControl[33] = 0; ui11->uFluxCapControl[34] = 0; ui11->uFluxCapControl[35] = 0; ui11->uFluxCapControl[36] = 0; ui11->uFluxCapControl[37] = 0; ui11->uFluxCapControl[38] = 0; ui11->uFluxCapControl[39] = 0; ui11->uFluxCapControl[40] = 0; ui11->uFluxCapControl[41] = 0; ui11->uFluxCapControl[42] = 0; ui11->uFluxCapControl[43] = 0; ui11->uFluxCapControl[44] = 0; ui11->uFluxCapControl[45] = 0; ui11->uFluxCapControl[46] = 0; ui11->uFluxCapControl[47] = 0; ui11->uFluxCapControl[48] = 0; ui11->uFluxCapControl[49] = 0; ui11->uFluxCapControl[50] = 0; ui11->uFluxCapControl[51] = 0; ui11->uFluxCapControl[52] = 0; ui11->uFluxCapControl[53] = 0; ui11->uFluxCapControl[54] = 0; ui11->uFluxCapControl[55] = 0; ui11->uFluxCapControl[56] = 0; ui11->uFluxCapControl[57] = 0; ui11->uFluxCapControl[58] = 0; ui11->uFluxCapControl[59] = 0; ui11->uFluxCapControl[60] = 0; ui11->uFluxCapControl[61] = 0; ui11->uFluxCapControl[62] = 0; ui11->uFluxCapControl[63] = 0; 
	ui11->fFluxCapData[0] = 0.000000; ui11->fFluxCapData[1] = 0.000000; ui11->fFluxCapData[2] = 0.000000; ui11->fFluxCapData[3] = 0.000000; ui11->fFluxCapData[4] = 0.000000; ui11->fFluxCapData[5] = 0.000000; ui11->fFluxCapData[6] = 0.000000; ui11->fFluxCapData[7] = 0.000000; ui11->fFluxCapData[8] = 0.000000; ui11->fFluxCapData[9] = 0.000000; ui11->fFluxCapData[10] = 0.000000; ui11->fFluxCapData[11] = 0.000000; ui11->fFluxCapData[12] = 0.000000; ui11->fFluxCapData[13] = 0.000000; ui11->fFluxCapData[14] = 0.000000; ui11->fFluxCapData[15] = 0.000000; ui11->fFluxCapData[16] = 0.000000; ui11->fFluxCapData[17] = 0.000000; ui11->fFluxCapData[18] = 0.000000; ui11->fFluxCapData[19] = 0.000000; ui11->fFluxCapData[20] = 0.000000; ui11->fFluxCapData[21] = 0.000000; ui11->fFluxCapData[22] = 0.000000; ui11->fFluxCapData[23] = 0.000000; ui11->fFluxCapData[24] = 0.000000; ui11->fFluxCapData[25] = 0.000000; ui11->fFluxCapData[26] = 0.000000; ui11->fFluxCapData[27] = 0.000000; ui11->fFluxCapData[28] = 0.000000; ui11->fFluxCapData[29] = 0.000000; ui11->fFluxCapData[30] = 0.000000; ui11->fFluxCapData[31] = 0.000000; ui11->fFluxCapData[32] = 0.000000; ui11->fFluxCapData[33] = 0.000000; ui11->fFluxCapData[34] = 0.000000; ui11->fFluxCapData[35] = 0.000000; ui11->fFluxCapData[36] = 0.000000; ui11->fFluxCapData[37] = 0.000000; ui11->fFluxCapData[38] = 0.000000; ui11->fFluxCapData[39] = 0.000000; ui11->fFluxCapData[40] = 0.000000; ui11->fFluxCapData[41] = 0.000000; ui11->fFluxCapData[42] = 0.000000; ui11->fFluxCapData[43] = 0.000000; ui11->fFluxCapData[44] = 0.000000; ui11->fFluxCapData[45] = 0.000000; ui11->fFluxCapData[46] = 0.000000; ui11->fFluxCapData[47] = 0.000000; ui11->fFluxCapData[48] = 0.000000; ui11->fFluxCapData[49] = 0.000000; ui11->fFluxCapData[50] = 0.000000; ui11->fFluxCapData[51] = 0.000000; ui11->fFluxCapData[52] = 0.000000; ui11->fFluxCapData[53] = 0.000000; ui11->fFluxCapData[54] = 0.000000; ui11->fFluxCapData[55] = 0.000000; ui11->fFluxCapData[56] = 0.000000; ui11->fFluxCapData[57] = 0.000000; ui11->fFluxCapData[58] = 0.000000; ui11->fFluxCapData[59] = 0.000000; ui11->fFluxCapData[60] = 0.000000; ui11->fFluxCapData[61] = 0.000000; ui11->fFluxCapData[62] = 0.000000; ui11->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui11);
	delete ui11;


	m_fPComb_3_Delay_mSec = 40.230000;
	CUICtrl* ui12 = new CUICtrl;
	ui12->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui12->uControlId = 14;
	ui12->bLogSlider = false;
	ui12->bExpSlider = false;
	ui12->fUserDisplayDataLoLimit = 0.000000;
	ui12->fUserDisplayDataHiLimit = 100.000000;
	ui12->uUserDataType = floatData;
	ui12->fInitUserIntValue = 0;
	ui12->fInitUserFloatValue = 40.230000;
	ui12->fInitUserDoubleValue = 0;
	ui12->fInitUserUINTValue = 0;
	ui12->m_pUserCookedIntData = NULL;
	ui12->m_pUserCookedFloatData = &m_fPComb_3_Delay_mSec;
	ui12->m_pUserCookedDoubleData = NULL;
	ui12->m_pUserCookedUINTData = NULL;
	ui12->cControlUnits = "mSec";
	ui12->cVariableName = "m_fPComb_3_Delay_mSec";
	ui12->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui12->dPresetData[0] = 40.230000;ui12->dPresetData[1] = 40.230000;ui12->dPresetData[2] = 40.230000;ui12->dPresetData[3] = 12.000001;ui12->dPresetData[4] = 0.000000;ui12->dPresetData[5] = 40.230000;ui12->dPresetData[6] = 0.000000;ui12->dPresetData[7] = 0.000000;ui12->dPresetData[8] = 0.000000;ui12->dPresetData[9] = 0.000000;ui12->dPresetData[10] = 0.000000;ui12->dPresetData[11] = 0.000000;ui12->dPresetData[12] = 0.000000;ui12->dPresetData[13] = 0.000000;ui12->dPresetData[14] = 0.000000;ui12->dPresetData[15] = 0.000000;
	ui12->cControlName = "PComb3 Dly";
	ui12->bOwnerControl = false;
	ui12->bMIDIControl = false;
	ui12->uMIDIControlCommand = 176;
	ui12->uMIDIControlName = 3;
	ui12->uMIDIControlChannel = 0;
	ui12->nGUIRow = nIndexer++;
	ui12->nGUIColumn = -1;
	ui12->bEnableParamSmoothing = false;
	ui12->fSmoothingTimeInMs = 100.00;
	ui12->uControlTheme[0] = 0; ui12->uControlTheme[1] = 0; ui12->uControlTheme[2] = 0; ui12->uControlTheme[3] = 0; ui12->uControlTheme[4] = 0; ui12->uControlTheme[5] = 0; ui12->uControlTheme[6] = 0; ui12->uControlTheme[7] = 0; ui12->uControlTheme[8] = 0; ui12->uControlTheme[9] = 0; ui12->uControlTheme[10] = 0; ui12->uControlTheme[11] = 0; ui12->uControlTheme[12] = 0; ui12->uControlTheme[13] = 0; ui12->uControlTheme[14] = 0; ui12->uControlTheme[15] = 0; ui12->uControlTheme[16] = 2; ui12->uControlTheme[17] = 0; ui12->uControlTheme[18] = 0; ui12->uControlTheme[19] = 0; ui12->uControlTheme[20] = 0; ui12->uControlTheme[21] = 0; ui12->uControlTheme[22] = 0; ui12->uControlTheme[23] = 0; ui12->uControlTheme[24] = 0; ui12->uControlTheme[25] = 0; ui12->uControlTheme[26] = 0; ui12->uControlTheme[27] = 0; ui12->uControlTheme[28] = 0; ui12->uControlTheme[29] = 0; ui12->uControlTheme[30] = 0; ui12->uControlTheme[31] = 0; 
	ui12->uFluxCapControl[0] = 0; ui12->uFluxCapControl[1] = 0; ui12->uFluxCapControl[2] = 0; ui12->uFluxCapControl[3] = 0; ui12->uFluxCapControl[4] = 0; ui12->uFluxCapControl[5] = 0; ui12->uFluxCapControl[6] = 0; ui12->uFluxCapControl[7] = 0; ui12->uFluxCapControl[8] = 0; ui12->uFluxCapControl[9] = 0; ui12->uFluxCapControl[10] = 0; ui12->uFluxCapControl[11] = 0; ui12->uFluxCapControl[12] = 0; ui12->uFluxCapControl[13] = 0; ui12->uFluxCapControl[14] = 0; ui12->uFluxCapControl[15] = 0; ui12->uFluxCapControl[16] = 0; ui12->uFluxCapControl[17] = 0; ui12->uFluxCapControl[18] = 0; ui12->uFluxCapControl[19] = 0; ui12->uFluxCapControl[20] = 0; ui12->uFluxCapControl[21] = 0; ui12->uFluxCapControl[22] = 0; ui12->uFluxCapControl[23] = 0; ui12->uFluxCapControl[24] = 0; ui12->uFluxCapControl[25] = 0; ui12->uFluxCapControl[26] = 0; ui12->uFluxCapControl[27] = 0; ui12->uFluxCapControl[28] = 0; ui12->uFluxCapControl[29] = 0; ui12->uFluxCapControl[30] = 0; ui12->uFluxCapControl[31] = 0; ui12->uFluxCapControl[32] = 0; ui12->uFluxCapControl[33] = 0; ui12->uFluxCapControl[34] = 0; ui12->uFluxCapControl[35] = 0; ui12->uFluxCapControl[36] = 0; ui12->uFluxCapControl[37] = 0; ui12->uFluxCapControl[38] = 0; ui12->uFluxCapControl[39] = 0; ui12->uFluxCapControl[40] = 0; ui12->uFluxCapControl[41] = 0; ui12->uFluxCapControl[42] = 0; ui12->uFluxCapControl[43] = 0; ui12->uFluxCapControl[44] = 0; ui12->uFluxCapControl[45] = 0; ui12->uFluxCapControl[46] = 0; ui12->uFluxCapControl[47] = 0; ui12->uFluxCapControl[48] = 0; ui12->uFluxCapControl[49] = 0; ui12->uFluxCapControl[50] = 0; ui12->uFluxCapControl[51] = 0; ui12->uFluxCapControl[52] = 0; ui12->uFluxCapControl[53] = 0; ui12->uFluxCapControl[54] = 0; ui12->uFluxCapControl[55] = 0; ui12->uFluxCapControl[56] = 0; ui12->uFluxCapControl[57] = 0; ui12->uFluxCapControl[58] = 0; ui12->uFluxCapControl[59] = 0; ui12->uFluxCapControl[60] = 0; ui12->uFluxCapControl[61] = 0; ui12->uFluxCapControl[62] = 0; ui12->uFluxCapControl[63] = 0; 
	ui12->fFluxCapData[0] = 0.000000; ui12->fFluxCapData[1] = 0.000000; ui12->fFluxCapData[2] = 0.000000; ui12->fFluxCapData[3] = 0.000000; ui12->fFluxCapData[4] = 0.000000; ui12->fFluxCapData[5] = 0.000000; ui12->fFluxCapData[6] = 0.000000; ui12->fFluxCapData[7] = 0.000000; ui12->fFluxCapData[8] = 0.000000; ui12->fFluxCapData[9] = 0.000000; ui12->fFluxCapData[10] = 0.000000; ui12->fFluxCapData[11] = 0.000000; ui12->fFluxCapData[12] = 0.000000; ui12->fFluxCapData[13] = 0.000000; ui12->fFluxCapData[14] = 0.000000; ui12->fFluxCapData[15] = 0.000000; ui12->fFluxCapData[16] = 0.000000; ui12->fFluxCapData[17] = 0.000000; ui12->fFluxCapData[18] = 0.000000; ui12->fFluxCapData[19] = 0.000000; ui12->fFluxCapData[20] = 0.000000; ui12->fFluxCapData[21] = 0.000000; ui12->fFluxCapData[22] = 0.000000; ui12->fFluxCapData[23] = 0.000000; ui12->fFluxCapData[24] = 0.000000; ui12->fFluxCapData[25] = 0.000000; ui12->fFluxCapData[26] = 0.000000; ui12->fFluxCapData[27] = 0.000000; ui12->fFluxCapData[28] = 0.000000; ui12->fFluxCapData[29] = 0.000000; ui12->fFluxCapData[30] = 0.000000; ui12->fFluxCapData[31] = 0.000000; ui12->fFluxCapData[32] = 0.000000; ui12->fFluxCapData[33] = 0.000000; ui12->fFluxCapData[34] = 0.000000; ui12->fFluxCapData[35] = 0.000000; ui12->fFluxCapData[36] = 0.000000; ui12->fFluxCapData[37] = 0.000000; ui12->fFluxCapData[38] = 0.000000; ui12->fFluxCapData[39] = 0.000000; ui12->fFluxCapData[40] = 0.000000; ui12->fFluxCapData[41] = 0.000000; ui12->fFluxCapData[42] = 0.000000; ui12->fFluxCapData[43] = 0.000000; ui12->fFluxCapData[44] = 0.000000; ui12->fFluxCapData[45] = 0.000000; ui12->fFluxCapData[46] = 0.000000; ui12->fFluxCapData[47] = 0.000000; ui12->fFluxCapData[48] = 0.000000; ui12->fFluxCapData[49] = 0.000000; ui12->fFluxCapData[50] = 0.000000; ui12->fFluxCapData[51] = 0.000000; ui12->fFluxCapData[52] = 0.000000; ui12->fFluxCapData[53] = 0.000000; ui12->fFluxCapData[54] = 0.000000; ui12->fFluxCapData[55] = 0.000000; ui12->fFluxCapData[56] = 0.000000; ui12->fFluxCapData[57] = 0.000000; ui12->fFluxCapData[58] = 0.000000; ui12->fFluxCapData[59] = 0.000000; ui12->fFluxCapData[60] = 0.000000; ui12->fFluxCapData[61] = 0.000000; ui12->fFluxCapData[62] = 0.000000; ui12->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui12);
	delete ui12;


	m_fPComb_4_Delay_mSec = 44.139999;
	CUICtrl* ui13 = new CUICtrl;
	ui13->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui13->uControlId = 15;
	ui13->bLogSlider = false;
	ui13->bExpSlider = false;
	ui13->fUserDisplayDataLoLimit = 0.000000;
	ui13->fUserDisplayDataHiLimit = 100.000000;
	ui13->uUserDataType = floatData;
	ui13->fInitUserIntValue = 0;
	ui13->fInitUserFloatValue = 44.139999;
	ui13->fInitUserDoubleValue = 0;
	ui13->fInitUserUINTValue = 0;
	ui13->m_pUserCookedIntData = NULL;
	ui13->m_pUserCookedFloatData = &m_fPComb_4_Delay_mSec;
	ui13->m_pUserCookedDoubleData = NULL;
	ui13->m_pUserCookedUINTData = NULL;
	ui13->cControlUnits = "mSec";
	ui13->cVariableName = "m_fPComb_4_Delay_mSec";
	ui13->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui13->dPresetData[0] = 44.139999;ui13->dPresetData[1] = 44.139999;ui13->dPresetData[2] = 44.139999;ui13->dPresetData[3] = 14.500001;ui13->dPresetData[4] = 0.000000;ui13->dPresetData[5] = 44.139999;ui13->dPresetData[6] = 0.000000;ui13->dPresetData[7] = 0.000000;ui13->dPresetData[8] = 0.000000;ui13->dPresetData[9] = 0.000000;ui13->dPresetData[10] = 0.000000;ui13->dPresetData[11] = 0.000000;ui13->dPresetData[12] = 0.000000;ui13->dPresetData[13] = 0.000000;ui13->dPresetData[14] = 0.000000;ui13->dPresetData[15] = 0.000000;
	ui13->cControlName = "PComb4 Dly";
	ui13->bOwnerControl = false;
	ui13->bMIDIControl = false;
	ui13->uMIDIControlCommand = 176;
	ui13->uMIDIControlName = 3;
	ui13->uMIDIControlChannel = 0;
	ui13->nGUIRow = nIndexer++;
	ui13->nGUIColumn = -1;
	ui13->bEnableParamSmoothing = false;
	ui13->fSmoothingTimeInMs = 100.00;
	ui13->uControlTheme[0] = 0; ui13->uControlTheme[1] = 0; ui13->uControlTheme[2] = 0; ui13->uControlTheme[3] = 0; ui13->uControlTheme[4] = 0; ui13->uControlTheme[5] = 0; ui13->uControlTheme[6] = 0; ui13->uControlTheme[7] = 0; ui13->uControlTheme[8] = 0; ui13->uControlTheme[9] = 0; ui13->uControlTheme[10] = 0; ui13->uControlTheme[11] = 0; ui13->uControlTheme[12] = 0; ui13->uControlTheme[13] = 0; ui13->uControlTheme[14] = 0; ui13->uControlTheme[15] = 0; ui13->uControlTheme[16] = 2; ui13->uControlTheme[17] = 0; ui13->uControlTheme[18] = 0; ui13->uControlTheme[19] = 0; ui13->uControlTheme[20] = 0; ui13->uControlTheme[21] = 0; ui13->uControlTheme[22] = 0; ui13->uControlTheme[23] = 0; ui13->uControlTheme[24] = 0; ui13->uControlTheme[25] = 0; ui13->uControlTheme[26] = 0; ui13->uControlTheme[27] = 0; ui13->uControlTheme[28] = 0; ui13->uControlTheme[29] = 0; ui13->uControlTheme[30] = 0; ui13->uControlTheme[31] = 0; 
	ui13->uFluxCapControl[0] = 0; ui13->uFluxCapControl[1] = 0; ui13->uFluxCapControl[2] = 0; ui13->uFluxCapControl[3] = 0; ui13->uFluxCapControl[4] = 0; ui13->uFluxCapControl[5] = 0; ui13->uFluxCapControl[6] = 0; ui13->uFluxCapControl[7] = 0; ui13->uFluxCapControl[8] = 0; ui13->uFluxCapControl[9] = 0; ui13->uFluxCapControl[10] = 0; ui13->uFluxCapControl[11] = 0; ui13->uFluxCapControl[12] = 0; ui13->uFluxCapControl[13] = 0; ui13->uFluxCapControl[14] = 0; ui13->uFluxCapControl[15] = 0; ui13->uFluxCapControl[16] = 0; ui13->uFluxCapControl[17] = 0; ui13->uFluxCapControl[18] = 0; ui13->uFluxCapControl[19] = 0; ui13->uFluxCapControl[20] = 0; ui13->uFluxCapControl[21] = 0; ui13->uFluxCapControl[22] = 0; ui13->uFluxCapControl[23] = 0; ui13->uFluxCapControl[24] = 0; ui13->uFluxCapControl[25] = 0; ui13->uFluxCapControl[26] = 0; ui13->uFluxCapControl[27] = 0; ui13->uFluxCapControl[28] = 0; ui13->uFluxCapControl[29] = 0; ui13->uFluxCapControl[30] = 0; ui13->uFluxCapControl[31] = 0; ui13->uFluxCapControl[32] = 0; ui13->uFluxCapControl[33] = 0; ui13->uFluxCapControl[34] = 0; ui13->uFluxCapControl[35] = 0; ui13->uFluxCapControl[36] = 0; ui13->uFluxCapControl[37] = 0; ui13->uFluxCapControl[38] = 0; ui13->uFluxCapControl[39] = 0; ui13->uFluxCapControl[40] = 0; ui13->uFluxCapControl[41] = 0; ui13->uFluxCapControl[42] = 0; ui13->uFluxCapControl[43] = 0; ui13->uFluxCapControl[44] = 0; ui13->uFluxCapControl[45] = 0; ui13->uFluxCapControl[46] = 0; ui13->uFluxCapControl[47] = 0; ui13->uFluxCapControl[48] = 0; ui13->uFluxCapControl[49] = 0; ui13->uFluxCapControl[50] = 0; ui13->uFluxCapControl[51] = 0; ui13->uFluxCapControl[52] = 0; ui13->uFluxCapControl[53] = 0; ui13->uFluxCapControl[54] = 0; ui13->uFluxCapControl[55] = 0; ui13->uFluxCapControl[56] = 0; ui13->uFluxCapControl[57] = 0; ui13->uFluxCapControl[58] = 0; ui13->uFluxCapControl[59] = 0; ui13->uFluxCapControl[60] = 0; ui13->uFluxCapControl[61] = 0; ui13->uFluxCapControl[62] = 0; ui13->uFluxCapControl[63] = 0; 
	ui13->fFluxCapData[0] = 0.000000; ui13->fFluxCapData[1] = 0.000000; ui13->fFluxCapData[2] = 0.000000; ui13->fFluxCapData[3] = 0.000000; ui13->fFluxCapData[4] = 0.000000; ui13->fFluxCapData[5] = 0.000000; ui13->fFluxCapData[6] = 0.000000; ui13->fFluxCapData[7] = 0.000000; ui13->fFluxCapData[8] = 0.000000; ui13->fFluxCapData[9] = 0.000000; ui13->fFluxCapData[10] = 0.000000; ui13->fFluxCapData[11] = 0.000000; ui13->fFluxCapData[12] = 0.000000; ui13->fFluxCapData[13] = 0.000000; ui13->fFluxCapData[14] = 0.000000; ui13->fFluxCapData[15] = 0.000000; ui13->fFluxCapData[16] = 0.000000; ui13->fFluxCapData[17] = 0.000000; ui13->fFluxCapData[18] = 0.000000; ui13->fFluxCapData[19] = 0.000000; ui13->fFluxCapData[20] = 0.000000; ui13->fFluxCapData[21] = 0.000000; ui13->fFluxCapData[22] = 0.000000; ui13->fFluxCapData[23] = 0.000000; ui13->fFluxCapData[24] = 0.000000; ui13->fFluxCapData[25] = 0.000000; ui13->fFluxCapData[26] = 0.000000; ui13->fFluxCapData[27] = 0.000000; ui13->fFluxCapData[28] = 0.000000; ui13->fFluxCapData[29] = 0.000000; ui13->fFluxCapData[30] = 0.000000; ui13->fFluxCapData[31] = 0.000000; ui13->fFluxCapData[32] = 0.000000; ui13->fFluxCapData[33] = 0.000000; ui13->fFluxCapData[34] = 0.000000; ui13->fFluxCapData[35] = 0.000000; ui13->fFluxCapData[36] = 0.000000; ui13->fFluxCapData[37] = 0.000000; ui13->fFluxCapData[38] = 0.000000; ui13->fFluxCapData[39] = 0.000000; ui13->fFluxCapData[40] = 0.000000; ui13->fFluxCapData[41] = 0.000000; ui13->fFluxCapData[42] = 0.000000; ui13->fFluxCapData[43] = 0.000000; ui13->fFluxCapData[44] = 0.000000; ui13->fFluxCapData[45] = 0.000000; ui13->fFluxCapData[46] = 0.000000; ui13->fFluxCapData[47] = 0.000000; ui13->fFluxCapData[48] = 0.000000; ui13->fFluxCapData[49] = 0.000000; ui13->fFluxCapData[50] = 0.000000; ui13->fFluxCapData[51] = 0.000000; ui13->fFluxCapData[52] = 0.000000; ui13->fFluxCapData[53] = 0.000000; ui13->fFluxCapData[54] = 0.000000; ui13->fFluxCapData[55] = 0.000000; ui13->fFluxCapData[56] = 0.000000; ui13->fFluxCapData[57] = 0.000000; ui13->fFluxCapData[58] = 0.000000; ui13->fFluxCapData[59] = 0.000000; ui13->fFluxCapData[60] = 0.000000; ui13->fFluxCapData[61] = 0.000000; ui13->fFluxCapData[62] = 0.000000; ui13->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui13);
	delete ui13;


	m_fAPF_3_Delay_mSec = 9.380000;
	CUICtrl* ui14 = new CUICtrl;
	ui14->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui14->uControlId = 18;
	ui14->bLogSlider = false;
	ui14->bExpSlider = false;
	ui14->fUserDisplayDataLoLimit = 0.000000;
	ui14->fUserDisplayDataHiLimit = 100.000000;
	ui14->uUserDataType = floatData;
	ui14->fInitUserIntValue = 0;
	ui14->fInitUserFloatValue = 9.380000;
	ui14->fInitUserDoubleValue = 0;
	ui14->fInitUserUINTValue = 0;
	ui14->m_pUserCookedIntData = NULL;
	ui14->m_pUserCookedFloatData = &m_fAPF_3_Delay_mSec;
	ui14->m_pUserCookedDoubleData = NULL;
	ui14->m_pUserCookedUINTData = NULL;
	ui14->cControlUnits = "mSec";
	ui14->cVariableName = "m_fAPF_3_Delay_mSec";
	ui14->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui14->dPresetData[0] = 9.380000;ui14->dPresetData[1] = 9.380000;ui14->dPresetData[2] = 9.380000;ui14->dPresetData[3] = 15.879999;ui14->dPresetData[4] = 0.000000;ui14->dPresetData[5] = 9.380000;ui14->dPresetData[6] = 0.000000;ui14->dPresetData[7] = 0.000000;ui14->dPresetData[8] = 0.000000;ui14->dPresetData[9] = 0.000000;ui14->dPresetData[10] = 0.000000;ui14->dPresetData[11] = 0.000000;ui14->dPresetData[12] = 0.000000;ui14->dPresetData[13] = 0.000000;ui14->dPresetData[14] = 0.000000;ui14->dPresetData[15] = 0.000000;
	ui14->cControlName = "APF3 Dly";
	ui14->bOwnerControl = false;
	ui14->bMIDIControl = false;
	ui14->uMIDIControlCommand = 176;
	ui14->uMIDIControlName = 3;
	ui14->uMIDIControlChannel = 0;
	ui14->nGUIRow = nIndexer++;
	ui14->nGUIColumn = -1;
	ui14->bEnableParamSmoothing = false;
	ui14->fSmoothingTimeInMs = 100.00;
	ui14->uControlTheme[0] = 0; ui14->uControlTheme[1] = 0; ui14->uControlTheme[2] = 0; ui14->uControlTheme[3] = 0; ui14->uControlTheme[4] = 0; ui14->uControlTheme[5] = 0; ui14->uControlTheme[6] = 0; ui14->uControlTheme[7] = 0; ui14->uControlTheme[8] = 0; ui14->uControlTheme[9] = 0; ui14->uControlTheme[10] = 0; ui14->uControlTheme[11] = 0; ui14->uControlTheme[12] = 0; ui14->uControlTheme[13] = 0; ui14->uControlTheme[14] = 0; ui14->uControlTheme[15] = 0; ui14->uControlTheme[16] = 2; ui14->uControlTheme[17] = 0; ui14->uControlTheme[18] = 0; ui14->uControlTheme[19] = 0; ui14->uControlTheme[20] = 0; ui14->uControlTheme[21] = 0; ui14->uControlTheme[22] = 0; ui14->uControlTheme[23] = 0; ui14->uControlTheme[24] = 0; ui14->uControlTheme[25] = 0; ui14->uControlTheme[26] = 0; ui14->uControlTheme[27] = 0; ui14->uControlTheme[28] = 0; ui14->uControlTheme[29] = 0; ui14->uControlTheme[30] = 0; ui14->uControlTheme[31] = 0; 
	ui14->uFluxCapControl[0] = 0; ui14->uFluxCapControl[1] = 0; ui14->uFluxCapControl[2] = 0; ui14->uFluxCapControl[3] = 0; ui14->uFluxCapControl[4] = 0; ui14->uFluxCapControl[5] = 0; ui14->uFluxCapControl[6] = 0; ui14->uFluxCapControl[7] = 0; ui14->uFluxCapControl[8] = 0; ui14->uFluxCapControl[9] = 0; ui14->uFluxCapControl[10] = 0; ui14->uFluxCapControl[11] = 0; ui14->uFluxCapControl[12] = 0; ui14->uFluxCapControl[13] = 0; ui14->uFluxCapControl[14] = 0; ui14->uFluxCapControl[15] = 0; ui14->uFluxCapControl[16] = 0; ui14->uFluxCapControl[17] = 0; ui14->uFluxCapControl[18] = 0; ui14->uFluxCapControl[19] = 0; ui14->uFluxCapControl[20] = 0; ui14->uFluxCapControl[21] = 0; ui14->uFluxCapControl[22] = 0; ui14->uFluxCapControl[23] = 0; ui14->uFluxCapControl[24] = 0; ui14->uFluxCapControl[25] = 0; ui14->uFluxCapControl[26] = 0; ui14->uFluxCapControl[27] = 0; ui14->uFluxCapControl[28] = 0; ui14->uFluxCapControl[29] = 0; ui14->uFluxCapControl[30] = 0; ui14->uFluxCapControl[31] = 0; ui14->uFluxCapControl[32] = 0; ui14->uFluxCapControl[33] = 0; ui14->uFluxCapControl[34] = 0; ui14->uFluxCapControl[35] = 0; ui14->uFluxCapControl[36] = 0; ui14->uFluxCapControl[37] = 0; ui14->uFluxCapControl[38] = 0; ui14->uFluxCapControl[39] = 0; ui14->uFluxCapControl[40] = 0; ui14->uFluxCapControl[41] = 0; ui14->uFluxCapControl[42] = 0; ui14->uFluxCapControl[43] = 0; ui14->uFluxCapControl[44] = 0; ui14->uFluxCapControl[45] = 0; ui14->uFluxCapControl[46] = 0; ui14->uFluxCapControl[47] = 0; ui14->uFluxCapControl[48] = 0; ui14->uFluxCapControl[49] = 0; ui14->uFluxCapControl[50] = 0; ui14->uFluxCapControl[51] = 0; ui14->uFluxCapControl[52] = 0; ui14->uFluxCapControl[53] = 0; ui14->uFluxCapControl[54] = 0; ui14->uFluxCapControl[55] = 0; ui14->uFluxCapControl[56] = 0; ui14->uFluxCapControl[57] = 0; ui14->uFluxCapControl[58] = 0; ui14->uFluxCapControl[59] = 0; ui14->uFluxCapControl[60] = 0; ui14->uFluxCapControl[61] = 0; ui14->uFluxCapControl[62] = 0; ui14->uFluxCapControl[63] = 0; 
	ui14->fFluxCapData[0] = 0.000000; ui14->fFluxCapData[1] = 0.000000; ui14->fFluxCapData[2] = 0.000000; ui14->fFluxCapData[3] = 0.000000; ui14->fFluxCapData[4] = 0.000000; ui14->fFluxCapData[5] = 0.000000; ui14->fFluxCapData[6] = 0.000000; ui14->fFluxCapData[7] = 0.000000; ui14->fFluxCapData[8] = 0.000000; ui14->fFluxCapData[9] = 0.000000; ui14->fFluxCapData[10] = 0.000000; ui14->fFluxCapData[11] = 0.000000; ui14->fFluxCapData[12] = 0.000000; ui14->fFluxCapData[13] = 0.000000; ui14->fFluxCapData[14] = 0.000000; ui14->fFluxCapData[15] = 0.000000; ui14->fFluxCapData[16] = 0.000000; ui14->fFluxCapData[17] = 0.000000; ui14->fFluxCapData[18] = 0.000000; ui14->fFluxCapData[19] = 0.000000; ui14->fFluxCapData[20] = 0.000000; ui14->fFluxCapData[21] = 0.000000; ui14->fFluxCapData[22] = 0.000000; ui14->fFluxCapData[23] = 0.000000; ui14->fFluxCapData[24] = 0.000000; ui14->fFluxCapData[25] = 0.000000; ui14->fFluxCapData[26] = 0.000000; ui14->fFluxCapData[27] = 0.000000; ui14->fFluxCapData[28] = 0.000000; ui14->fFluxCapData[29] = 0.000000; ui14->fFluxCapData[30] = 0.000000; ui14->fFluxCapData[31] = 0.000000; ui14->fFluxCapData[32] = 0.000000; ui14->fFluxCapData[33] = 0.000000; ui14->fFluxCapData[34] = 0.000000; ui14->fFluxCapData[35] = 0.000000; ui14->fFluxCapData[36] = 0.000000; ui14->fFluxCapData[37] = 0.000000; ui14->fFluxCapData[38] = 0.000000; ui14->fFluxCapData[39] = 0.000000; ui14->fFluxCapData[40] = 0.000000; ui14->fFluxCapData[41] = 0.000000; ui14->fFluxCapData[42] = 0.000000; ui14->fFluxCapData[43] = 0.000000; ui14->fFluxCapData[44] = 0.000000; ui14->fFluxCapData[45] = 0.000000; ui14->fFluxCapData[46] = 0.000000; ui14->fFluxCapData[47] = 0.000000; ui14->fFluxCapData[48] = 0.000000; ui14->fFluxCapData[49] = 0.000000; ui14->fFluxCapData[50] = 0.000000; ui14->fFluxCapData[51] = 0.000000; ui14->fFluxCapData[52] = 0.000000; ui14->fFluxCapData[53] = 0.000000; ui14->fFluxCapData[54] = 0.000000; ui14->fFluxCapData[55] = 0.000000; ui14->fFluxCapData[56] = 0.000000; ui14->fFluxCapData[57] = 0.000000; ui14->fFluxCapData[58] = 0.000000; ui14->fFluxCapData[59] = 0.000000; ui14->fFluxCapData[60] = 0.000000; ui14->fFluxCapData[61] = 0.000000; ui14->fFluxCapData[62] = 0.000000; ui14->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui14);
	delete ui14;


	m_fAPF_3_g = -0.600000;
	CUICtrl* ui15 = new CUICtrl;
	ui15->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui15->uControlId = 19;
	ui15->bLogSlider = false;
	ui15->bExpSlider = false;
	ui15->fUserDisplayDataLoLimit = -1.000000;
	ui15->fUserDisplayDataHiLimit = 1.000000;
	ui15->uUserDataType = floatData;
	ui15->fInitUserIntValue = 0;
	ui15->fInitUserFloatValue = -0.600000;
	ui15->fInitUserDoubleValue = 0;
	ui15->fInitUserUINTValue = 0;
	ui15->m_pUserCookedIntData = NULL;
	ui15->m_pUserCookedFloatData = &m_fAPF_3_g;
	ui15->m_pUserCookedDoubleData = NULL;
	ui15->m_pUserCookedUINTData = NULL;
	ui15->cControlUnits = "APF Bank 3";
	ui15->cVariableName = "m_fAPF_3_g";
	ui15->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui15->dPresetData[0] = -0.600000;ui15->dPresetData[1] = -0.500000;ui15->dPresetData[2] = -0.500000;ui15->dPresetData[3] = -0.500000;ui15->dPresetData[4] = 0.000000;ui15->dPresetData[5] = -0.500000;ui15->dPresetData[6] = 0.000000;ui15->dPresetData[7] = 0.000000;ui15->dPresetData[8] = 0.000000;ui15->dPresetData[9] = 0.000000;ui15->dPresetData[10] = 0.000000;ui15->dPresetData[11] = 0.000000;ui15->dPresetData[12] = 0.000000;ui15->dPresetData[13] = 0.000000;ui15->dPresetData[14] = 0.000000;ui15->dPresetData[15] = 0.000000;
	ui15->cControlName = "APF3 g";
	ui15->bOwnerControl = false;
	ui15->bMIDIControl = false;
	ui15->uMIDIControlCommand = 176;
	ui15->uMIDIControlName = 3;
	ui15->uMIDIControlChannel = 0;
	ui15->nGUIRow = nIndexer++;
	ui15->nGUIColumn = -1;
	ui15->bEnableParamSmoothing = false;
	ui15->fSmoothingTimeInMs = 100.00;
	ui15->uControlTheme[0] = 0; ui15->uControlTheme[1] = 0; ui15->uControlTheme[2] = 0; ui15->uControlTheme[3] = 0; ui15->uControlTheme[4] = 0; ui15->uControlTheme[5] = 0; ui15->uControlTheme[6] = 0; ui15->uControlTheme[7] = 0; ui15->uControlTheme[8] = 0; ui15->uControlTheme[9] = 0; ui15->uControlTheme[10] = 0; ui15->uControlTheme[11] = 0; ui15->uControlTheme[12] = 0; ui15->uControlTheme[13] = 0; ui15->uControlTheme[14] = 0; ui15->uControlTheme[15] = 0; ui15->uControlTheme[16] = 2; ui15->uControlTheme[17] = 0; ui15->uControlTheme[18] = 0; ui15->uControlTheme[19] = 0; ui15->uControlTheme[20] = 0; ui15->uControlTheme[21] = 0; ui15->uControlTheme[22] = 0; ui15->uControlTheme[23] = 0; ui15->uControlTheme[24] = 0; ui15->uControlTheme[25] = 0; ui15->uControlTheme[26] = 0; ui15->uControlTheme[27] = 0; ui15->uControlTheme[28] = 0; ui15->uControlTheme[29] = 0; ui15->uControlTheme[30] = 0; ui15->uControlTheme[31] = 0; 
	ui15->uFluxCapControl[0] = 0; ui15->uFluxCapControl[1] = 0; ui15->uFluxCapControl[2] = 0; ui15->uFluxCapControl[3] = 0; ui15->uFluxCapControl[4] = 0; ui15->uFluxCapControl[5] = 0; ui15->uFluxCapControl[6] = 0; ui15->uFluxCapControl[7] = 0; ui15->uFluxCapControl[8] = 0; ui15->uFluxCapControl[9] = 0; ui15->uFluxCapControl[10] = 0; ui15->uFluxCapControl[11] = 0; ui15->uFluxCapControl[12] = 0; ui15->uFluxCapControl[13] = 0; ui15->uFluxCapControl[14] = 0; ui15->uFluxCapControl[15] = 0; ui15->uFluxCapControl[16] = 0; ui15->uFluxCapControl[17] = 0; ui15->uFluxCapControl[18] = 0; ui15->uFluxCapControl[19] = 0; ui15->uFluxCapControl[20] = 0; ui15->uFluxCapControl[21] = 0; ui15->uFluxCapControl[22] = 0; ui15->uFluxCapControl[23] = 0; ui15->uFluxCapControl[24] = 0; ui15->uFluxCapControl[25] = 0; ui15->uFluxCapControl[26] = 0; ui15->uFluxCapControl[27] = 0; ui15->uFluxCapControl[28] = 0; ui15->uFluxCapControl[29] = 0; ui15->uFluxCapControl[30] = 0; ui15->uFluxCapControl[31] = 0; ui15->uFluxCapControl[32] = 0; ui15->uFluxCapControl[33] = 0; ui15->uFluxCapControl[34] = 0; ui15->uFluxCapControl[35] = 0; ui15->uFluxCapControl[36] = 0; ui15->uFluxCapControl[37] = 0; ui15->uFluxCapControl[38] = 0; ui15->uFluxCapControl[39] = 0; ui15->uFluxCapControl[40] = 0; ui15->uFluxCapControl[41] = 0; ui15->uFluxCapControl[42] = 0; ui15->uFluxCapControl[43] = 0; ui15->uFluxCapControl[44] = 0; ui15->uFluxCapControl[45] = 0; ui15->uFluxCapControl[46] = 0; ui15->uFluxCapControl[47] = 0; ui15->uFluxCapControl[48] = 0; ui15->uFluxCapControl[49] = 0; ui15->uFluxCapControl[50] = 0; ui15->uFluxCapControl[51] = 0; ui15->uFluxCapControl[52] = 0; ui15->uFluxCapControl[53] = 0; ui15->uFluxCapControl[54] = 0; ui15->uFluxCapControl[55] = 0; ui15->uFluxCapControl[56] = 0; ui15->uFluxCapControl[57] = 0; ui15->uFluxCapControl[58] = 0; ui15->uFluxCapControl[59] = 0; ui15->uFluxCapControl[60] = 0; ui15->uFluxCapControl[61] = 0; ui15->uFluxCapControl[62] = 0; ui15->uFluxCapControl[63] = 0; 
	ui15->fFluxCapData[0] = 0.000000; ui15->fFluxCapData[1] = 0.000000; ui15->fFluxCapData[2] = 0.000000; ui15->fFluxCapData[3] = 0.000000; ui15->fFluxCapData[4] = 0.000000; ui15->fFluxCapData[5] = 0.000000; ui15->fFluxCapData[6] = 0.000000; ui15->fFluxCapData[7] = 0.000000; ui15->fFluxCapData[8] = 0.000000; ui15->fFluxCapData[9] = 0.000000; ui15->fFluxCapData[10] = 0.000000; ui15->fFluxCapData[11] = 0.000000; ui15->fFluxCapData[12] = 0.000000; ui15->fFluxCapData[13] = 0.000000; ui15->fFluxCapData[14] = 0.000000; ui15->fFluxCapData[15] = 0.000000; ui15->fFluxCapData[16] = 0.000000; ui15->fFluxCapData[17] = 0.000000; ui15->fFluxCapData[18] = 0.000000; ui15->fFluxCapData[19] = 0.000000; ui15->fFluxCapData[20] = 0.000000; ui15->fFluxCapData[21] = 0.000000; ui15->fFluxCapData[22] = 0.000000; ui15->fFluxCapData[23] = 0.000000; ui15->fFluxCapData[24] = 0.000000; ui15->fFluxCapData[25] = 0.000000; ui15->fFluxCapData[26] = 0.000000; ui15->fFluxCapData[27] = 0.000000; ui15->fFluxCapData[28] = 0.000000; ui15->fFluxCapData[29] = 0.000000; ui15->fFluxCapData[30] = 0.000000; ui15->fFluxCapData[31] = 0.000000; ui15->fFluxCapData[32] = 0.000000; ui15->fFluxCapData[33] = 0.000000; ui15->fFluxCapData[34] = 0.000000; ui15->fFluxCapData[35] = 0.000000; ui15->fFluxCapData[36] = 0.000000; ui15->fFluxCapData[37] = 0.000000; ui15->fFluxCapData[38] = 0.000000; ui15->fFluxCapData[39] = 0.000000; ui15->fFluxCapData[40] = 0.000000; ui15->fFluxCapData[41] = 0.000000; ui15->fFluxCapData[42] = 0.000000; ui15->fFluxCapData[43] = 0.000000; ui15->fFluxCapData[44] = 0.000000; ui15->fFluxCapData[45] = 0.000000; ui15->fFluxCapData[46] = 0.000000; ui15->fFluxCapData[47] = 0.000000; ui15->fFluxCapData[48] = 0.000000; ui15->fFluxCapData[49] = 0.000000; ui15->fFluxCapData[50] = 0.000000; ui15->fFluxCapData[51] = 0.000000; ui15->fFluxCapData[52] = 0.000000; ui15->fFluxCapData[53] = 0.000000; ui15->fFluxCapData[54] = 0.000000; ui15->fFluxCapData[55] = 0.000000; ui15->fFluxCapData[56] = 0.000000; ui15->fFluxCapData[57] = 0.000000; ui15->fFluxCapData[58] = 0.000000; ui15->fFluxCapData[59] = 0.000000; ui15->fFluxCapData[60] = 0.000000; ui15->fFluxCapData[61] = 0.000000; ui15->fFluxCapData[62] = 0.000000; ui15->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui15);
	delete ui15;


	m_fPComb_5_Delay_mSec = 30.469999;
	CUICtrl* ui16 = new CUICtrl;
	ui16->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui16->uControlId = 22;
	ui16->bLogSlider = false;
	ui16->bExpSlider = false;
	ui16->fUserDisplayDataLoLimit = 0.000000;
	ui16->fUserDisplayDataHiLimit = 100.000000;
	ui16->uUserDataType = floatData;
	ui16->fInitUserIntValue = 0;
	ui16->fInitUserFloatValue = 30.469999;
	ui16->fInitUserDoubleValue = 0;
	ui16->fInitUserUINTValue = 0;
	ui16->m_pUserCookedIntData = NULL;
	ui16->m_pUserCookedFloatData = &m_fPComb_5_Delay_mSec;
	ui16->m_pUserCookedDoubleData = NULL;
	ui16->m_pUserCookedUINTData = NULL;
	ui16->cControlUnits = "mSec";
	ui16->cVariableName = "m_fPComb_5_Delay_mSec";
	ui16->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui16->dPresetData[0] = 30.469999;ui16->dPresetData[1] = 30.469999;ui16->dPresetData[2] = 30.469999;ui16->dPresetData[3] = 17.469997;ui16->dPresetData[4] = 0.000000;ui16->dPresetData[5] = 30.469999;ui16->dPresetData[6] = 0.000000;ui16->dPresetData[7] = 0.000000;ui16->dPresetData[8] = 0.000000;ui16->dPresetData[9] = 0.000000;ui16->dPresetData[10] = 0.000000;ui16->dPresetData[11] = 0.000000;ui16->dPresetData[12] = 0.000000;ui16->dPresetData[13] = 0.000000;ui16->dPresetData[14] = 0.000000;ui16->dPresetData[15] = 0.000000;
	ui16->cControlName = "PComb5 Dly";
	ui16->bOwnerControl = false;
	ui16->bMIDIControl = false;
	ui16->uMIDIControlCommand = 176;
	ui16->uMIDIControlName = 3;
	ui16->uMIDIControlChannel = 0;
	ui16->nGUIRow = nIndexer++;
	ui16->nGUIColumn = -1;
	ui16->bEnableParamSmoothing = false;
	ui16->fSmoothingTimeInMs = 100.00;
	ui16->uControlTheme[0] = 0; ui16->uControlTheme[1] = 0; ui16->uControlTheme[2] = 0; ui16->uControlTheme[3] = 0; ui16->uControlTheme[4] = 0; ui16->uControlTheme[5] = 0; ui16->uControlTheme[6] = 0; ui16->uControlTheme[7] = 0; ui16->uControlTheme[8] = 0; ui16->uControlTheme[9] = 0; ui16->uControlTheme[10] = 0; ui16->uControlTheme[11] = 0; ui16->uControlTheme[12] = 0; ui16->uControlTheme[13] = 0; ui16->uControlTheme[14] = 0; ui16->uControlTheme[15] = 0; ui16->uControlTheme[16] = 2; ui16->uControlTheme[17] = 0; ui16->uControlTheme[18] = 0; ui16->uControlTheme[19] = 0; ui16->uControlTheme[20] = 0; ui16->uControlTheme[21] = 0; ui16->uControlTheme[22] = 0; ui16->uControlTheme[23] = 0; ui16->uControlTheme[24] = 0; ui16->uControlTheme[25] = 0; ui16->uControlTheme[26] = 0; ui16->uControlTheme[27] = 0; ui16->uControlTheme[28] = 0; ui16->uControlTheme[29] = 0; ui16->uControlTheme[30] = 0; ui16->uControlTheme[31] = 0; 
	ui16->uFluxCapControl[0] = 0; ui16->uFluxCapControl[1] = 0; ui16->uFluxCapControl[2] = 0; ui16->uFluxCapControl[3] = 0; ui16->uFluxCapControl[4] = 0; ui16->uFluxCapControl[5] = 0; ui16->uFluxCapControl[6] = 0; ui16->uFluxCapControl[7] = 0; ui16->uFluxCapControl[8] = 0; ui16->uFluxCapControl[9] = 0; ui16->uFluxCapControl[10] = 0; ui16->uFluxCapControl[11] = 0; ui16->uFluxCapControl[12] = 0; ui16->uFluxCapControl[13] = 0; ui16->uFluxCapControl[14] = 0; ui16->uFluxCapControl[15] = 0; ui16->uFluxCapControl[16] = 0; ui16->uFluxCapControl[17] = 0; ui16->uFluxCapControl[18] = 0; ui16->uFluxCapControl[19] = 0; ui16->uFluxCapControl[20] = 0; ui16->uFluxCapControl[21] = 0; ui16->uFluxCapControl[22] = 0; ui16->uFluxCapControl[23] = 0; ui16->uFluxCapControl[24] = 0; ui16->uFluxCapControl[25] = 0; ui16->uFluxCapControl[26] = 0; ui16->uFluxCapControl[27] = 0; ui16->uFluxCapControl[28] = 0; ui16->uFluxCapControl[29] = 0; ui16->uFluxCapControl[30] = 0; ui16->uFluxCapControl[31] = 0; ui16->uFluxCapControl[32] = 0; ui16->uFluxCapControl[33] = 0; ui16->uFluxCapControl[34] = 0; ui16->uFluxCapControl[35] = 0; ui16->uFluxCapControl[36] = 0; ui16->uFluxCapControl[37] = 0; ui16->uFluxCapControl[38] = 0; ui16->uFluxCapControl[39] = 0; ui16->uFluxCapControl[40] = 0; ui16->uFluxCapControl[41] = 0; ui16->uFluxCapControl[42] = 0; ui16->uFluxCapControl[43] = 0; ui16->uFluxCapControl[44] = 0; ui16->uFluxCapControl[45] = 0; ui16->uFluxCapControl[46] = 0; ui16->uFluxCapControl[47] = 0; ui16->uFluxCapControl[48] = 0; ui16->uFluxCapControl[49] = 0; ui16->uFluxCapControl[50] = 0; ui16->uFluxCapControl[51] = 0; ui16->uFluxCapControl[52] = 0; ui16->uFluxCapControl[53] = 0; ui16->uFluxCapControl[54] = 0; ui16->uFluxCapControl[55] = 0; ui16->uFluxCapControl[56] = 0; ui16->uFluxCapControl[57] = 0; ui16->uFluxCapControl[58] = 0; ui16->uFluxCapControl[59] = 0; ui16->uFluxCapControl[60] = 0; ui16->uFluxCapControl[61] = 0; ui16->uFluxCapControl[62] = 0; ui16->uFluxCapControl[63] = 0; 
	ui16->fFluxCapData[0] = 0.000000; ui16->fFluxCapData[1] = 0.000000; ui16->fFluxCapData[2] = 0.000000; ui16->fFluxCapData[3] = 0.000000; ui16->fFluxCapData[4] = 0.000000; ui16->fFluxCapData[5] = 0.000000; ui16->fFluxCapData[6] = 0.000000; ui16->fFluxCapData[7] = 0.000000; ui16->fFluxCapData[8] = 0.000000; ui16->fFluxCapData[9] = 0.000000; ui16->fFluxCapData[10] = 0.000000; ui16->fFluxCapData[11] = 0.000000; ui16->fFluxCapData[12] = 0.000000; ui16->fFluxCapData[13] = 0.000000; ui16->fFluxCapData[14] = 0.000000; ui16->fFluxCapData[15] = 0.000000; ui16->fFluxCapData[16] = 0.000000; ui16->fFluxCapData[17] = 0.000000; ui16->fFluxCapData[18] = 0.000000; ui16->fFluxCapData[19] = 0.000000; ui16->fFluxCapData[20] = 0.000000; ui16->fFluxCapData[21] = 0.000000; ui16->fFluxCapData[22] = 0.000000; ui16->fFluxCapData[23] = 0.000000; ui16->fFluxCapData[24] = 0.000000; ui16->fFluxCapData[25] = 0.000000; ui16->fFluxCapData[26] = 0.000000; ui16->fFluxCapData[27] = 0.000000; ui16->fFluxCapData[28] = 0.000000; ui16->fFluxCapData[29] = 0.000000; ui16->fFluxCapData[30] = 0.000000; ui16->fFluxCapData[31] = 0.000000; ui16->fFluxCapData[32] = 0.000000; ui16->fFluxCapData[33] = 0.000000; ui16->fFluxCapData[34] = 0.000000; ui16->fFluxCapData[35] = 0.000000; ui16->fFluxCapData[36] = 0.000000; ui16->fFluxCapData[37] = 0.000000; ui16->fFluxCapData[38] = 0.000000; ui16->fFluxCapData[39] = 0.000000; ui16->fFluxCapData[40] = 0.000000; ui16->fFluxCapData[41] = 0.000000; ui16->fFluxCapData[42] = 0.000000; ui16->fFluxCapData[43] = 0.000000; ui16->fFluxCapData[44] = 0.000000; ui16->fFluxCapData[45] = 0.000000; ui16->fFluxCapData[46] = 0.000000; ui16->fFluxCapData[47] = 0.000000; ui16->fFluxCapData[48] = 0.000000; ui16->fFluxCapData[49] = 0.000000; ui16->fFluxCapData[50] = 0.000000; ui16->fFluxCapData[51] = 0.000000; ui16->fFluxCapData[52] = 0.000000; ui16->fFluxCapData[53] = 0.000000; ui16->fFluxCapData[54] = 0.000000; ui16->fFluxCapData[55] = 0.000000; ui16->fFluxCapData[56] = 0.000000; ui16->fFluxCapData[57] = 0.000000; ui16->fFluxCapData[58] = 0.000000; ui16->fFluxCapData[59] = 0.000000; ui16->fFluxCapData[60] = 0.000000; ui16->fFluxCapData[61] = 0.000000; ui16->fFluxCapData[62] = 0.000000; ui16->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui16);
	delete ui16;


	m_fPComb_6_Delay_mSec = 33.980000;
	CUICtrl* ui17 = new CUICtrl;
	ui17->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui17->uControlId = 23;
	ui17->bLogSlider = false;
	ui17->bExpSlider = false;
	ui17->fUserDisplayDataLoLimit = 0.000000;
	ui17->fUserDisplayDataHiLimit = 100.000000;
	ui17->uUserDataType = floatData;
	ui17->fInitUserIntValue = 0;
	ui17->fInitUserFloatValue = 33.980000;
	ui17->fInitUserDoubleValue = 0;
	ui17->fInitUserUINTValue = 0;
	ui17->m_pUserCookedIntData = NULL;
	ui17->m_pUserCookedFloatData = &m_fPComb_6_Delay_mSec;
	ui17->m_pUserCookedDoubleData = NULL;
	ui17->m_pUserCookedUINTData = NULL;
	ui17->cControlUnits = "mSec";
	ui17->cVariableName = "m_fPComb_6_Delay_mSec";
	ui17->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui17->dPresetData[0] = 33.980000;ui17->dPresetData[1] = 33.980000;ui17->dPresetData[2] = 33.980000;ui17->dPresetData[3] = 22.500000;ui17->dPresetData[4] = 0.000000;ui17->dPresetData[5] = 33.980000;ui17->dPresetData[6] = 0.000000;ui17->dPresetData[7] = 0.000000;ui17->dPresetData[8] = 0.000000;ui17->dPresetData[9] = 0.000000;ui17->dPresetData[10] = 0.000000;ui17->dPresetData[11] = 0.000000;ui17->dPresetData[12] = 0.000000;ui17->dPresetData[13] = 0.000000;ui17->dPresetData[14] = 0.000000;ui17->dPresetData[15] = 0.000000;
	ui17->cControlName = "PComb6 Dly";
	ui17->bOwnerControl = false;
	ui17->bMIDIControl = false;
	ui17->uMIDIControlCommand = 176;
	ui17->uMIDIControlName = 3;
	ui17->uMIDIControlChannel = 0;
	ui17->nGUIRow = nIndexer++;
	ui17->nGUIColumn = -1;
	ui17->bEnableParamSmoothing = false;
	ui17->fSmoothingTimeInMs = 100.00;
	ui17->uControlTheme[0] = 0; ui17->uControlTheme[1] = 0; ui17->uControlTheme[2] = 0; ui17->uControlTheme[3] = 0; ui17->uControlTheme[4] = 0; ui17->uControlTheme[5] = 0; ui17->uControlTheme[6] = 0; ui17->uControlTheme[7] = 0; ui17->uControlTheme[8] = 0; ui17->uControlTheme[9] = 0; ui17->uControlTheme[10] = 0; ui17->uControlTheme[11] = 0; ui17->uControlTheme[12] = 0; ui17->uControlTheme[13] = 0; ui17->uControlTheme[14] = 0; ui17->uControlTheme[15] = 0; ui17->uControlTheme[16] = 2; ui17->uControlTheme[17] = 0; ui17->uControlTheme[18] = 0; ui17->uControlTheme[19] = 0; ui17->uControlTheme[20] = 0; ui17->uControlTheme[21] = 0; ui17->uControlTheme[22] = 0; ui17->uControlTheme[23] = 0; ui17->uControlTheme[24] = 0; ui17->uControlTheme[25] = 0; ui17->uControlTheme[26] = 0; ui17->uControlTheme[27] = 0; ui17->uControlTheme[28] = 0; ui17->uControlTheme[29] = 0; ui17->uControlTheme[30] = 0; ui17->uControlTheme[31] = 0; 
	ui17->uFluxCapControl[0] = 0; ui17->uFluxCapControl[1] = 0; ui17->uFluxCapControl[2] = 0; ui17->uFluxCapControl[3] = 0; ui17->uFluxCapControl[4] = 0; ui17->uFluxCapControl[5] = 0; ui17->uFluxCapControl[6] = 0; ui17->uFluxCapControl[7] = 0; ui17->uFluxCapControl[8] = 0; ui17->uFluxCapControl[9] = 0; ui17->uFluxCapControl[10] = 0; ui17->uFluxCapControl[11] = 0; ui17->uFluxCapControl[12] = 0; ui17->uFluxCapControl[13] = 0; ui17->uFluxCapControl[14] = 0; ui17->uFluxCapControl[15] = 0; ui17->uFluxCapControl[16] = 0; ui17->uFluxCapControl[17] = 0; ui17->uFluxCapControl[18] = 0; ui17->uFluxCapControl[19] = 0; ui17->uFluxCapControl[20] = 0; ui17->uFluxCapControl[21] = 0; ui17->uFluxCapControl[22] = 0; ui17->uFluxCapControl[23] = 0; ui17->uFluxCapControl[24] = 0; ui17->uFluxCapControl[25] = 0; ui17->uFluxCapControl[26] = 0; ui17->uFluxCapControl[27] = 0; ui17->uFluxCapControl[28] = 0; ui17->uFluxCapControl[29] = 0; ui17->uFluxCapControl[30] = 0; ui17->uFluxCapControl[31] = 0; ui17->uFluxCapControl[32] = 0; ui17->uFluxCapControl[33] = 0; ui17->uFluxCapControl[34] = 0; ui17->uFluxCapControl[35] = 0; ui17->uFluxCapControl[36] = 0; ui17->uFluxCapControl[37] = 0; ui17->uFluxCapControl[38] = 0; ui17->uFluxCapControl[39] = 0; ui17->uFluxCapControl[40] = 0; ui17->uFluxCapControl[41] = 0; ui17->uFluxCapControl[42] = 0; ui17->uFluxCapControl[43] = 0; ui17->uFluxCapControl[44] = 0; ui17->uFluxCapControl[45] = 0; ui17->uFluxCapControl[46] = 0; ui17->uFluxCapControl[47] = 0; ui17->uFluxCapControl[48] = 0; ui17->uFluxCapControl[49] = 0; ui17->uFluxCapControl[50] = 0; ui17->uFluxCapControl[51] = 0; ui17->uFluxCapControl[52] = 0; ui17->uFluxCapControl[53] = 0; ui17->uFluxCapControl[54] = 0; ui17->uFluxCapControl[55] = 0; ui17->uFluxCapControl[56] = 0; ui17->uFluxCapControl[57] = 0; ui17->uFluxCapControl[58] = 0; ui17->uFluxCapControl[59] = 0; ui17->uFluxCapControl[60] = 0; ui17->uFluxCapControl[61] = 0; ui17->uFluxCapControl[62] = 0; ui17->uFluxCapControl[63] = 0; 
	ui17->fFluxCapData[0] = 0.000000; ui17->fFluxCapData[1] = 0.000000; ui17->fFluxCapData[2] = 0.000000; ui17->fFluxCapData[3] = 0.000000; ui17->fFluxCapData[4] = 0.000000; ui17->fFluxCapData[5] = 0.000000; ui17->fFluxCapData[6] = 0.000000; ui17->fFluxCapData[7] = 0.000000; ui17->fFluxCapData[8] = 0.000000; ui17->fFluxCapData[9] = 0.000000; ui17->fFluxCapData[10] = 0.000000; ui17->fFluxCapData[11] = 0.000000; ui17->fFluxCapData[12] = 0.000000; ui17->fFluxCapData[13] = 0.000000; ui17->fFluxCapData[14] = 0.000000; ui17->fFluxCapData[15] = 0.000000; ui17->fFluxCapData[16] = 0.000000; ui17->fFluxCapData[17] = 0.000000; ui17->fFluxCapData[18] = 0.000000; ui17->fFluxCapData[19] = 0.000000; ui17->fFluxCapData[20] = 0.000000; ui17->fFluxCapData[21] = 0.000000; ui17->fFluxCapData[22] = 0.000000; ui17->fFluxCapData[23] = 0.000000; ui17->fFluxCapData[24] = 0.000000; ui17->fFluxCapData[25] = 0.000000; ui17->fFluxCapData[26] = 0.000000; ui17->fFluxCapData[27] = 0.000000; ui17->fFluxCapData[28] = 0.000000; ui17->fFluxCapData[29] = 0.000000; ui17->fFluxCapData[30] = 0.000000; ui17->fFluxCapData[31] = 0.000000; ui17->fFluxCapData[32] = 0.000000; ui17->fFluxCapData[33] = 0.000000; ui17->fFluxCapData[34] = 0.000000; ui17->fFluxCapData[35] = 0.000000; ui17->fFluxCapData[36] = 0.000000; ui17->fFluxCapData[37] = 0.000000; ui17->fFluxCapData[38] = 0.000000; ui17->fFluxCapData[39] = 0.000000; ui17->fFluxCapData[40] = 0.000000; ui17->fFluxCapData[41] = 0.000000; ui17->fFluxCapData[42] = 0.000000; ui17->fFluxCapData[43] = 0.000000; ui17->fFluxCapData[44] = 0.000000; ui17->fFluxCapData[45] = 0.000000; ui17->fFluxCapData[46] = 0.000000; ui17->fFluxCapData[47] = 0.000000; ui17->fFluxCapData[48] = 0.000000; ui17->fFluxCapData[49] = 0.000000; ui17->fFluxCapData[50] = 0.000000; ui17->fFluxCapData[51] = 0.000000; ui17->fFluxCapData[52] = 0.000000; ui17->fFluxCapData[53] = 0.000000; ui17->fFluxCapData[54] = 0.000000; ui17->fFluxCapData[55] = 0.000000; ui17->fFluxCapData[56] = 0.000000; ui17->fFluxCapData[57] = 0.000000; ui17->fFluxCapData[58] = 0.000000; ui17->fFluxCapData[59] = 0.000000; ui17->fFluxCapData[60] = 0.000000; ui17->fFluxCapData[61] = 0.000000; ui17->fFluxCapData[62] = 0.000000; ui17->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui17);
	delete ui17;


	m_fPComb_7_Delay_mSec = 41.410000;
	CUICtrl* ui18 = new CUICtrl;
	ui18->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui18->uControlId = 24;
	ui18->bLogSlider = false;
	ui18->bExpSlider = false;
	ui18->fUserDisplayDataLoLimit = 0.000000;
	ui18->fUserDisplayDataHiLimit = 100.000000;
	ui18->uUserDataType = floatData;
	ui18->fInitUserIntValue = 0;
	ui18->fInitUserFloatValue = 41.410000;
	ui18->fInitUserDoubleValue = 0;
	ui18->fInitUserUINTValue = 0;
	ui18->m_pUserCookedIntData = NULL;
	ui18->m_pUserCookedFloatData = &m_fPComb_7_Delay_mSec;
	ui18->m_pUserCookedDoubleData = NULL;
	ui18->m_pUserCookedUINTData = NULL;
	ui18->cControlUnits = "mSec";
	ui18->cVariableName = "m_fPComb_7_Delay_mSec";
	ui18->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui18->dPresetData[0] = 41.410000;ui18->dPresetData[1] = 41.410000;ui18->dPresetData[2] = 41.410000;ui18->dPresetData[3] = 27.500000;ui18->dPresetData[4] = 0.000000;ui18->dPresetData[5] = 41.410000;ui18->dPresetData[6] = 0.000000;ui18->dPresetData[7] = 0.000000;ui18->dPresetData[8] = 0.000000;ui18->dPresetData[9] = 0.000000;ui18->dPresetData[10] = 0.000000;ui18->dPresetData[11] = 0.000000;ui18->dPresetData[12] = 0.000000;ui18->dPresetData[13] = 0.000000;ui18->dPresetData[14] = 0.000000;ui18->dPresetData[15] = 0.000000;
	ui18->cControlName = "PComb7 Dly";
	ui18->bOwnerControl = false;
	ui18->bMIDIControl = false;
	ui18->uMIDIControlCommand = 176;
	ui18->uMIDIControlName = 3;
	ui18->uMIDIControlChannel = 0;
	ui18->nGUIRow = nIndexer++;
	ui18->nGUIColumn = -1;
	ui18->bEnableParamSmoothing = false;
	ui18->fSmoothingTimeInMs = 100.00;
	ui18->uControlTheme[0] = 0; ui18->uControlTheme[1] = 0; ui18->uControlTheme[2] = 0; ui18->uControlTheme[3] = 0; ui18->uControlTheme[4] = 0; ui18->uControlTheme[5] = 0; ui18->uControlTheme[6] = 0; ui18->uControlTheme[7] = 0; ui18->uControlTheme[8] = 0; ui18->uControlTheme[9] = 0; ui18->uControlTheme[10] = 0; ui18->uControlTheme[11] = 0; ui18->uControlTheme[12] = 0; ui18->uControlTheme[13] = 0; ui18->uControlTheme[14] = 0; ui18->uControlTheme[15] = 0; ui18->uControlTheme[16] = 2; ui18->uControlTheme[17] = 0; ui18->uControlTheme[18] = 0; ui18->uControlTheme[19] = 0; ui18->uControlTheme[20] = 0; ui18->uControlTheme[21] = 0; ui18->uControlTheme[22] = 0; ui18->uControlTheme[23] = 0; ui18->uControlTheme[24] = 0; ui18->uControlTheme[25] = 0; ui18->uControlTheme[26] = 0; ui18->uControlTheme[27] = 0; ui18->uControlTheme[28] = 0; ui18->uControlTheme[29] = 0; ui18->uControlTheme[30] = 0; ui18->uControlTheme[31] = 0; 
	ui18->uFluxCapControl[0] = 0; ui18->uFluxCapControl[1] = 0; ui18->uFluxCapControl[2] = 0; ui18->uFluxCapControl[3] = 0; ui18->uFluxCapControl[4] = 0; ui18->uFluxCapControl[5] = 0; ui18->uFluxCapControl[6] = 0; ui18->uFluxCapControl[7] = 0; ui18->uFluxCapControl[8] = 0; ui18->uFluxCapControl[9] = 0; ui18->uFluxCapControl[10] = 0; ui18->uFluxCapControl[11] = 0; ui18->uFluxCapControl[12] = 0; ui18->uFluxCapControl[13] = 0; ui18->uFluxCapControl[14] = 0; ui18->uFluxCapControl[15] = 0; ui18->uFluxCapControl[16] = 0; ui18->uFluxCapControl[17] = 0; ui18->uFluxCapControl[18] = 0; ui18->uFluxCapControl[19] = 0; ui18->uFluxCapControl[20] = 0; ui18->uFluxCapControl[21] = 0; ui18->uFluxCapControl[22] = 0; ui18->uFluxCapControl[23] = 0; ui18->uFluxCapControl[24] = 0; ui18->uFluxCapControl[25] = 0; ui18->uFluxCapControl[26] = 0; ui18->uFluxCapControl[27] = 0; ui18->uFluxCapControl[28] = 0; ui18->uFluxCapControl[29] = 0; ui18->uFluxCapControl[30] = 0; ui18->uFluxCapControl[31] = 0; ui18->uFluxCapControl[32] = 0; ui18->uFluxCapControl[33] = 0; ui18->uFluxCapControl[34] = 0; ui18->uFluxCapControl[35] = 0; ui18->uFluxCapControl[36] = 0; ui18->uFluxCapControl[37] = 0; ui18->uFluxCapControl[38] = 0; ui18->uFluxCapControl[39] = 0; ui18->uFluxCapControl[40] = 0; ui18->uFluxCapControl[41] = 0; ui18->uFluxCapControl[42] = 0; ui18->uFluxCapControl[43] = 0; ui18->uFluxCapControl[44] = 0; ui18->uFluxCapControl[45] = 0; ui18->uFluxCapControl[46] = 0; ui18->uFluxCapControl[47] = 0; ui18->uFluxCapControl[48] = 0; ui18->uFluxCapControl[49] = 0; ui18->uFluxCapControl[50] = 0; ui18->uFluxCapControl[51] = 0; ui18->uFluxCapControl[52] = 0; ui18->uFluxCapControl[53] = 0; ui18->uFluxCapControl[54] = 0; ui18->uFluxCapControl[55] = 0; ui18->uFluxCapControl[56] = 0; ui18->uFluxCapControl[57] = 0; ui18->uFluxCapControl[58] = 0; ui18->uFluxCapControl[59] = 0; ui18->uFluxCapControl[60] = 0; ui18->uFluxCapControl[61] = 0; ui18->uFluxCapControl[62] = 0; ui18->uFluxCapControl[63] = 0; 
	ui18->fFluxCapData[0] = 0.000000; ui18->fFluxCapData[1] = 0.000000; ui18->fFluxCapData[2] = 0.000000; ui18->fFluxCapData[3] = 0.000000; ui18->fFluxCapData[4] = 0.000000; ui18->fFluxCapData[5] = 0.000000; ui18->fFluxCapData[6] = 0.000000; ui18->fFluxCapData[7] = 0.000000; ui18->fFluxCapData[8] = 0.000000; ui18->fFluxCapData[9] = 0.000000; ui18->fFluxCapData[10] = 0.000000; ui18->fFluxCapData[11] = 0.000000; ui18->fFluxCapData[12] = 0.000000; ui18->fFluxCapData[13] = 0.000000; ui18->fFluxCapData[14] = 0.000000; ui18->fFluxCapData[15] = 0.000000; ui18->fFluxCapData[16] = 0.000000; ui18->fFluxCapData[17] = 0.000000; ui18->fFluxCapData[18] = 0.000000; ui18->fFluxCapData[19] = 0.000000; ui18->fFluxCapData[20] = 0.000000; ui18->fFluxCapData[21] = 0.000000; ui18->fFluxCapData[22] = 0.000000; ui18->fFluxCapData[23] = 0.000000; ui18->fFluxCapData[24] = 0.000000; ui18->fFluxCapData[25] = 0.000000; ui18->fFluxCapData[26] = 0.000000; ui18->fFluxCapData[27] = 0.000000; ui18->fFluxCapData[28] = 0.000000; ui18->fFluxCapData[29] = 0.000000; ui18->fFluxCapData[30] = 0.000000; ui18->fFluxCapData[31] = 0.000000; ui18->fFluxCapData[32] = 0.000000; ui18->fFluxCapData[33] = 0.000000; ui18->fFluxCapData[34] = 0.000000; ui18->fFluxCapData[35] = 0.000000; ui18->fFluxCapData[36] = 0.000000; ui18->fFluxCapData[37] = 0.000000; ui18->fFluxCapData[38] = 0.000000; ui18->fFluxCapData[39] = 0.000000; ui18->fFluxCapData[40] = 0.000000; ui18->fFluxCapData[41] = 0.000000; ui18->fFluxCapData[42] = 0.000000; ui18->fFluxCapData[43] = 0.000000; ui18->fFluxCapData[44] = 0.000000; ui18->fFluxCapData[45] = 0.000000; ui18->fFluxCapData[46] = 0.000000; ui18->fFluxCapData[47] = 0.000000; ui18->fFluxCapData[48] = 0.000000; ui18->fFluxCapData[49] = 0.000000; ui18->fFluxCapData[50] = 0.000000; ui18->fFluxCapData[51] = 0.000000; ui18->fFluxCapData[52] = 0.000000; ui18->fFluxCapData[53] = 0.000000; ui18->fFluxCapData[54] = 0.000000; ui18->fFluxCapData[55] = 0.000000; ui18->fFluxCapData[56] = 0.000000; ui18->fFluxCapData[57] = 0.000000; ui18->fFluxCapData[58] = 0.000000; ui18->fFluxCapData[59] = 0.000000; ui18->fFluxCapData[60] = 0.000000; ui18->fFluxCapData[61] = 0.000000; ui18->fFluxCapData[62] = 0.000000; ui18->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui18);
	delete ui18;


	m_fPComb_8_Delay_mSec = 42.580002;
	CUICtrl* ui19 = new CUICtrl;
	ui19->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui19->uControlId = 25;
	ui19->bLogSlider = false;
	ui19->bExpSlider = false;
	ui19->fUserDisplayDataLoLimit = 0.000000;
	ui19->fUserDisplayDataHiLimit = 100.000000;
	ui19->uUserDataType = floatData;
	ui19->fInitUserIntValue = 0;
	ui19->fInitUserFloatValue = 42.580002;
	ui19->fInitUserDoubleValue = 0;
	ui19->fInitUserUINTValue = 0;
	ui19->m_pUserCookedIntData = NULL;
	ui19->m_pUserCookedFloatData = &m_fPComb_8_Delay_mSec;
	ui19->m_pUserCookedDoubleData = NULL;
	ui19->m_pUserCookedUINTData = NULL;
	ui19->cControlUnits = "mSec";
	ui19->cVariableName = "m_fPComb_8_Delay_mSec";
	ui19->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui19->dPresetData[0] = 42.580002;ui19->dPresetData[1] = 42.580002;ui19->dPresetData[2] = 42.580002;ui19->dPresetData[3] = 31.500000;ui19->dPresetData[4] = 0.000000;ui19->dPresetData[5] = 42.580002;ui19->dPresetData[6] = 0.000000;ui19->dPresetData[7] = 0.000000;ui19->dPresetData[8] = 0.000000;ui19->dPresetData[9] = 0.000000;ui19->dPresetData[10] = 0.000000;ui19->dPresetData[11] = 0.000000;ui19->dPresetData[12] = 0.000000;ui19->dPresetData[13] = 0.000000;ui19->dPresetData[14] = 0.000000;ui19->dPresetData[15] = 0.000000;
	ui19->cControlName = "PComb8 Dly";
	ui19->bOwnerControl = false;
	ui19->bMIDIControl = false;
	ui19->uMIDIControlCommand = 176;
	ui19->uMIDIControlName = 3;
	ui19->uMIDIControlChannel = 0;
	ui19->nGUIRow = nIndexer++;
	ui19->nGUIColumn = -1;
	ui19->bEnableParamSmoothing = false;
	ui19->fSmoothingTimeInMs = 100.00;
	ui19->uControlTheme[0] = 0; ui19->uControlTheme[1] = 0; ui19->uControlTheme[2] = 0; ui19->uControlTheme[3] = 0; ui19->uControlTheme[4] = 0; ui19->uControlTheme[5] = 0; ui19->uControlTheme[6] = 0; ui19->uControlTheme[7] = 0; ui19->uControlTheme[8] = 0; ui19->uControlTheme[9] = 0; ui19->uControlTheme[10] = 0; ui19->uControlTheme[11] = 0; ui19->uControlTheme[12] = 0; ui19->uControlTheme[13] = 0; ui19->uControlTheme[14] = 0; ui19->uControlTheme[15] = 0; ui19->uControlTheme[16] = 2; ui19->uControlTheme[17] = 0; ui19->uControlTheme[18] = 0; ui19->uControlTheme[19] = 0; ui19->uControlTheme[20] = 0; ui19->uControlTheme[21] = 0; ui19->uControlTheme[22] = 0; ui19->uControlTheme[23] = 0; ui19->uControlTheme[24] = 0; ui19->uControlTheme[25] = 0; ui19->uControlTheme[26] = 0; ui19->uControlTheme[27] = 0; ui19->uControlTheme[28] = 0; ui19->uControlTheme[29] = 0; ui19->uControlTheme[30] = 0; ui19->uControlTheme[31] = 0; 
	ui19->uFluxCapControl[0] = 0; ui19->uFluxCapControl[1] = 0; ui19->uFluxCapControl[2] = 0; ui19->uFluxCapControl[3] = 0; ui19->uFluxCapControl[4] = 0; ui19->uFluxCapControl[5] = 0; ui19->uFluxCapControl[6] = 0; ui19->uFluxCapControl[7] = 0; ui19->uFluxCapControl[8] = 0; ui19->uFluxCapControl[9] = 0; ui19->uFluxCapControl[10] = 0; ui19->uFluxCapControl[11] = 0; ui19->uFluxCapControl[12] = 0; ui19->uFluxCapControl[13] = 0; ui19->uFluxCapControl[14] = 0; ui19->uFluxCapControl[15] = 0; ui19->uFluxCapControl[16] = 0; ui19->uFluxCapControl[17] = 0; ui19->uFluxCapControl[18] = 0; ui19->uFluxCapControl[19] = 0; ui19->uFluxCapControl[20] = 0; ui19->uFluxCapControl[21] = 0; ui19->uFluxCapControl[22] = 0; ui19->uFluxCapControl[23] = 0; ui19->uFluxCapControl[24] = 0; ui19->uFluxCapControl[25] = 0; ui19->uFluxCapControl[26] = 0; ui19->uFluxCapControl[27] = 0; ui19->uFluxCapControl[28] = 0; ui19->uFluxCapControl[29] = 0; ui19->uFluxCapControl[30] = 0; ui19->uFluxCapControl[31] = 0; ui19->uFluxCapControl[32] = 0; ui19->uFluxCapControl[33] = 0; ui19->uFluxCapControl[34] = 0; ui19->uFluxCapControl[35] = 0; ui19->uFluxCapControl[36] = 0; ui19->uFluxCapControl[37] = 0; ui19->uFluxCapControl[38] = 0; ui19->uFluxCapControl[39] = 0; ui19->uFluxCapControl[40] = 0; ui19->uFluxCapControl[41] = 0; ui19->uFluxCapControl[42] = 0; ui19->uFluxCapControl[43] = 0; ui19->uFluxCapControl[44] = 0; ui19->uFluxCapControl[45] = 0; ui19->uFluxCapControl[46] = 0; ui19->uFluxCapControl[47] = 0; ui19->uFluxCapControl[48] = 0; ui19->uFluxCapControl[49] = 0; ui19->uFluxCapControl[50] = 0; ui19->uFluxCapControl[51] = 0; ui19->uFluxCapControl[52] = 0; ui19->uFluxCapControl[53] = 0; ui19->uFluxCapControl[54] = 0; ui19->uFluxCapControl[55] = 0; ui19->uFluxCapControl[56] = 0; ui19->uFluxCapControl[57] = 0; ui19->uFluxCapControl[58] = 0; ui19->uFluxCapControl[59] = 0; ui19->uFluxCapControl[60] = 0; ui19->uFluxCapControl[61] = 0; ui19->uFluxCapControl[62] = 0; ui19->uFluxCapControl[63] = 0; 
	ui19->fFluxCapData[0] = 0.000000; ui19->fFluxCapData[1] = 0.000000; ui19->fFluxCapData[2] = 0.000000; ui19->fFluxCapData[3] = 0.000000; ui19->fFluxCapData[4] = 0.000000; ui19->fFluxCapData[5] = 0.000000; ui19->fFluxCapData[6] = 0.000000; ui19->fFluxCapData[7] = 0.000000; ui19->fFluxCapData[8] = 0.000000; ui19->fFluxCapData[9] = 0.000000; ui19->fFluxCapData[10] = 0.000000; ui19->fFluxCapData[11] = 0.000000; ui19->fFluxCapData[12] = 0.000000; ui19->fFluxCapData[13] = 0.000000; ui19->fFluxCapData[14] = 0.000000; ui19->fFluxCapData[15] = 0.000000; ui19->fFluxCapData[16] = 0.000000; ui19->fFluxCapData[17] = 0.000000; ui19->fFluxCapData[18] = 0.000000; ui19->fFluxCapData[19] = 0.000000; ui19->fFluxCapData[20] = 0.000000; ui19->fFluxCapData[21] = 0.000000; ui19->fFluxCapData[22] = 0.000000; ui19->fFluxCapData[23] = 0.000000; ui19->fFluxCapData[24] = 0.000000; ui19->fFluxCapData[25] = 0.000000; ui19->fFluxCapData[26] = 0.000000; ui19->fFluxCapData[27] = 0.000000; ui19->fFluxCapData[28] = 0.000000; ui19->fFluxCapData[29] = 0.000000; ui19->fFluxCapData[30] = 0.000000; ui19->fFluxCapData[31] = 0.000000; ui19->fFluxCapData[32] = 0.000000; ui19->fFluxCapData[33] = 0.000000; ui19->fFluxCapData[34] = 0.000000; ui19->fFluxCapData[35] = 0.000000; ui19->fFluxCapData[36] = 0.000000; ui19->fFluxCapData[37] = 0.000000; ui19->fFluxCapData[38] = 0.000000; ui19->fFluxCapData[39] = 0.000000; ui19->fFluxCapData[40] = 0.000000; ui19->fFluxCapData[41] = 0.000000; ui19->fFluxCapData[42] = 0.000000; ui19->fFluxCapData[43] = 0.000000; ui19->fFluxCapData[44] = 0.000000; ui19->fFluxCapData[45] = 0.000000; ui19->fFluxCapData[46] = 0.000000; ui19->fFluxCapData[47] = 0.000000; ui19->fFluxCapData[48] = 0.000000; ui19->fFluxCapData[49] = 0.000000; ui19->fFluxCapData[50] = 0.000000; ui19->fFluxCapData[51] = 0.000000; ui19->fFluxCapData[52] = 0.000000; ui19->fFluxCapData[53] = 0.000000; ui19->fFluxCapData[54] = 0.000000; ui19->fFluxCapData[55] = 0.000000; ui19->fFluxCapData[56] = 0.000000; ui19->fFluxCapData[57] = 0.000000; ui19->fFluxCapData[58] = 0.000000; ui19->fFluxCapData[59] = 0.000000; ui19->fFluxCapData[60] = 0.000000; ui19->fFluxCapData[61] = 0.000000; ui19->fFluxCapData[62] = 0.000000; ui19->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui19);
	delete ui19;


	m_fAPF_4_Delay_mSec = 11.000000;
	CUICtrl* ui20 = new CUICtrl;
	ui20->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui20->uControlId = 28;
	ui20->bLogSlider = false;
	ui20->bExpSlider = false;
	ui20->fUserDisplayDataLoLimit = 0.000000;
	ui20->fUserDisplayDataHiLimit = 100.000000;
	ui20->uUserDataType = floatData;
	ui20->fInitUserIntValue = 0;
	ui20->fInitUserFloatValue = 11.000000;
	ui20->fInitUserDoubleValue = 0;
	ui20->fInitUserUINTValue = 0;
	ui20->m_pUserCookedIntData = NULL;
	ui20->m_pUserCookedFloatData = &m_fAPF_4_Delay_mSec;
	ui20->m_pUserCookedDoubleData = NULL;
	ui20->m_pUserCookedUINTData = NULL;
	ui20->cControlUnits = "mSec";
	ui20->cVariableName = "m_fAPF_4_Delay_mSec";
	ui20->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui20->dPresetData[0] = 11.000000;ui20->dPresetData[1] = 32.000000;ui20->dPresetData[2] = 32.000000;ui20->dPresetData[3] = 20.000000;ui20->dPresetData[4] = 0.000000;ui20->dPresetData[5] = 32.000000;ui20->dPresetData[6] = 0.000000;ui20->dPresetData[7] = 0.000000;ui20->dPresetData[8] = 0.000000;ui20->dPresetData[9] = 0.000000;ui20->dPresetData[10] = 0.000000;ui20->dPresetData[11] = 0.000000;ui20->dPresetData[12] = 0.000000;ui20->dPresetData[13] = 0.000000;ui20->dPresetData[14] = 0.000000;ui20->dPresetData[15] = 0.000000;
	ui20->cControlName = "APF4 Dly";
	ui20->bOwnerControl = false;
	ui20->bMIDIControl = false;
	ui20->uMIDIControlCommand = 176;
	ui20->uMIDIControlName = 3;
	ui20->uMIDIControlChannel = 0;
	ui20->nGUIRow = nIndexer++;
	ui20->nGUIColumn = -1;
	ui20->bEnableParamSmoothing = false;
	ui20->fSmoothingTimeInMs = 100.00;
	ui20->uControlTheme[0] = 0; ui20->uControlTheme[1] = 0; ui20->uControlTheme[2] = 0; ui20->uControlTheme[3] = 0; ui20->uControlTheme[4] = 0; ui20->uControlTheme[5] = 0; ui20->uControlTheme[6] = 0; ui20->uControlTheme[7] = 0; ui20->uControlTheme[8] = 0; ui20->uControlTheme[9] = 0; ui20->uControlTheme[10] = 0; ui20->uControlTheme[11] = 0; ui20->uControlTheme[12] = 0; ui20->uControlTheme[13] = 0; ui20->uControlTheme[14] = 0; ui20->uControlTheme[15] = 0; ui20->uControlTheme[16] = 2; ui20->uControlTheme[17] = 0; ui20->uControlTheme[18] = 0; ui20->uControlTheme[19] = 0; ui20->uControlTheme[20] = 0; ui20->uControlTheme[21] = 0; ui20->uControlTheme[22] = 0; ui20->uControlTheme[23] = 0; ui20->uControlTheme[24] = 0; ui20->uControlTheme[25] = 0; ui20->uControlTheme[26] = 0; ui20->uControlTheme[27] = 0; ui20->uControlTheme[28] = 0; ui20->uControlTheme[29] = 0; ui20->uControlTheme[30] = 0; ui20->uControlTheme[31] = 0; 
	ui20->uFluxCapControl[0] = 0; ui20->uFluxCapControl[1] = 0; ui20->uFluxCapControl[2] = 0; ui20->uFluxCapControl[3] = 0; ui20->uFluxCapControl[4] = 0; ui20->uFluxCapControl[5] = 0; ui20->uFluxCapControl[6] = 0; ui20->uFluxCapControl[7] = 0; ui20->uFluxCapControl[8] = 0; ui20->uFluxCapControl[9] = 0; ui20->uFluxCapControl[10] = 0; ui20->uFluxCapControl[11] = 0; ui20->uFluxCapControl[12] = 0; ui20->uFluxCapControl[13] = 0; ui20->uFluxCapControl[14] = 0; ui20->uFluxCapControl[15] = 0; ui20->uFluxCapControl[16] = 0; ui20->uFluxCapControl[17] = 0; ui20->uFluxCapControl[18] = 0; ui20->uFluxCapControl[19] = 0; ui20->uFluxCapControl[20] = 0; ui20->uFluxCapControl[21] = 0; ui20->uFluxCapControl[22] = 0; ui20->uFluxCapControl[23] = 0; ui20->uFluxCapControl[24] = 0; ui20->uFluxCapControl[25] = 0; ui20->uFluxCapControl[26] = 0; ui20->uFluxCapControl[27] = 0; ui20->uFluxCapControl[28] = 0; ui20->uFluxCapControl[29] = 0; ui20->uFluxCapControl[30] = 0; ui20->uFluxCapControl[31] = 0; ui20->uFluxCapControl[32] = 0; ui20->uFluxCapControl[33] = 0; ui20->uFluxCapControl[34] = 0; ui20->uFluxCapControl[35] = 0; ui20->uFluxCapControl[36] = 0; ui20->uFluxCapControl[37] = 0; ui20->uFluxCapControl[38] = 0; ui20->uFluxCapControl[39] = 0; ui20->uFluxCapControl[40] = 0; ui20->uFluxCapControl[41] = 0; ui20->uFluxCapControl[42] = 0; ui20->uFluxCapControl[43] = 0; ui20->uFluxCapControl[44] = 0; ui20->uFluxCapControl[45] = 0; ui20->uFluxCapControl[46] = 0; ui20->uFluxCapControl[47] = 0; ui20->uFluxCapControl[48] = 0; ui20->uFluxCapControl[49] = 0; ui20->uFluxCapControl[50] = 0; ui20->uFluxCapControl[51] = 0; ui20->uFluxCapControl[52] = 0; ui20->uFluxCapControl[53] = 0; ui20->uFluxCapControl[54] = 0; ui20->uFluxCapControl[55] = 0; ui20->uFluxCapControl[56] = 0; ui20->uFluxCapControl[57] = 0; ui20->uFluxCapControl[58] = 0; ui20->uFluxCapControl[59] = 0; ui20->uFluxCapControl[60] = 0; ui20->uFluxCapControl[61] = 0; ui20->uFluxCapControl[62] = 0; ui20->uFluxCapControl[63] = 0; 
	ui20->fFluxCapData[0] = 0.000000; ui20->fFluxCapData[1] = 0.000000; ui20->fFluxCapData[2] = 0.000000; ui20->fFluxCapData[3] = 0.000000; ui20->fFluxCapData[4] = 0.000000; ui20->fFluxCapData[5] = 0.000000; ui20->fFluxCapData[6] = 0.000000; ui20->fFluxCapData[7] = 0.000000; ui20->fFluxCapData[8] = 0.000000; ui20->fFluxCapData[9] = 0.000000; ui20->fFluxCapData[10] = 0.000000; ui20->fFluxCapData[11] = 0.000000; ui20->fFluxCapData[12] = 0.000000; ui20->fFluxCapData[13] = 0.000000; ui20->fFluxCapData[14] = 0.000000; ui20->fFluxCapData[15] = 0.000000; ui20->fFluxCapData[16] = 0.000000; ui20->fFluxCapData[17] = 0.000000; ui20->fFluxCapData[18] = 0.000000; ui20->fFluxCapData[19] = 0.000000; ui20->fFluxCapData[20] = 0.000000; ui20->fFluxCapData[21] = 0.000000; ui20->fFluxCapData[22] = 0.000000; ui20->fFluxCapData[23] = 0.000000; ui20->fFluxCapData[24] = 0.000000; ui20->fFluxCapData[25] = 0.000000; ui20->fFluxCapData[26] = 0.000000; ui20->fFluxCapData[27] = 0.000000; ui20->fFluxCapData[28] = 0.000000; ui20->fFluxCapData[29] = 0.000000; ui20->fFluxCapData[30] = 0.000000; ui20->fFluxCapData[31] = 0.000000; ui20->fFluxCapData[32] = 0.000000; ui20->fFluxCapData[33] = 0.000000; ui20->fFluxCapData[34] = 0.000000; ui20->fFluxCapData[35] = 0.000000; ui20->fFluxCapData[36] = 0.000000; ui20->fFluxCapData[37] = 0.000000; ui20->fFluxCapData[38] = 0.000000; ui20->fFluxCapData[39] = 0.000000; ui20->fFluxCapData[40] = 0.000000; ui20->fFluxCapData[41] = 0.000000; ui20->fFluxCapData[42] = 0.000000; ui20->fFluxCapData[43] = 0.000000; ui20->fFluxCapData[44] = 0.000000; ui20->fFluxCapData[45] = 0.000000; ui20->fFluxCapData[46] = 0.000000; ui20->fFluxCapData[47] = 0.000000; ui20->fFluxCapData[48] = 0.000000; ui20->fFluxCapData[49] = 0.000000; ui20->fFluxCapData[50] = 0.000000; ui20->fFluxCapData[51] = 0.000000; ui20->fFluxCapData[52] = 0.000000; ui20->fFluxCapData[53] = 0.000000; ui20->fFluxCapData[54] = 0.000000; ui20->fFluxCapData[55] = 0.000000; ui20->fFluxCapData[56] = 0.000000; ui20->fFluxCapData[57] = 0.000000; ui20->fFluxCapData[58] = 0.000000; ui20->fFluxCapData[59] = 0.000000; ui20->fFluxCapData[60] = 0.000000; ui20->fFluxCapData[61] = 0.000000; ui20->fFluxCapData[62] = 0.000000; ui20->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui20);
	delete ui20;


	m_fAPF_4_g = 0.600000;
	CUICtrl* ui21 = new CUICtrl;
	ui21->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui21->uControlId = 29;
	ui21->bLogSlider = false;
	ui21->bExpSlider = false;
	ui21->fUserDisplayDataLoLimit = -1.000000;
	ui21->fUserDisplayDataHiLimit = 1.000000;
	ui21->uUserDataType = floatData;
	ui21->fInitUserIntValue = 0;
	ui21->fInitUserFloatValue = 0.600000;
	ui21->fInitUserDoubleValue = 0;
	ui21->fInitUserUINTValue = 0;
	ui21->m_pUserCookedIntData = NULL;
	ui21->m_pUserCookedFloatData = &m_fAPF_4_g;
	ui21->m_pUserCookedDoubleData = NULL;
	ui21->m_pUserCookedUINTData = NULL;
	ui21->cControlUnits = "APF Bank 4";
	ui21->cVariableName = "m_fAPF_4_g";
	ui21->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui21->dPresetData[0] = 0.600000;ui21->dPresetData[1] = 0.500000;ui21->dPresetData[2] = 0.500000;ui21->dPresetData[3] = 0.500000;ui21->dPresetData[4] = 0.000000;ui21->dPresetData[5] = 0.500000;ui21->dPresetData[6] = 0.000000;ui21->dPresetData[7] = 0.000000;ui21->dPresetData[8] = 0.000000;ui21->dPresetData[9] = 0.000000;ui21->dPresetData[10] = 0.000000;ui21->dPresetData[11] = 0.000000;ui21->dPresetData[12] = 0.000000;ui21->dPresetData[13] = 0.000000;ui21->dPresetData[14] = 0.000000;ui21->dPresetData[15] = 0.000000;
	ui21->cControlName = "APF4 g";
	ui21->bOwnerControl = false;
	ui21->bMIDIControl = false;
	ui21->uMIDIControlCommand = 176;
	ui21->uMIDIControlName = 3;
	ui21->uMIDIControlChannel = 0;
	ui21->nGUIRow = nIndexer++;
	ui21->nGUIColumn = -1;
	ui21->bEnableParamSmoothing = false;
	ui21->fSmoothingTimeInMs = 100.00;
	ui21->uControlTheme[0] = 0; ui21->uControlTheme[1] = 0; ui21->uControlTheme[2] = 0; ui21->uControlTheme[3] = 0; ui21->uControlTheme[4] = 0; ui21->uControlTheme[5] = 0; ui21->uControlTheme[6] = 0; ui21->uControlTheme[7] = 0; ui21->uControlTheme[8] = 0; ui21->uControlTheme[9] = 0; ui21->uControlTheme[10] = 0; ui21->uControlTheme[11] = 0; ui21->uControlTheme[12] = 0; ui21->uControlTheme[13] = 0; ui21->uControlTheme[14] = 0; ui21->uControlTheme[15] = 0; ui21->uControlTheme[16] = 2; ui21->uControlTheme[17] = 0; ui21->uControlTheme[18] = 0; ui21->uControlTheme[19] = 0; ui21->uControlTheme[20] = 0; ui21->uControlTheme[21] = 0; ui21->uControlTheme[22] = 0; ui21->uControlTheme[23] = 0; ui21->uControlTheme[24] = 0; ui21->uControlTheme[25] = 0; ui21->uControlTheme[26] = 0; ui21->uControlTheme[27] = 0; ui21->uControlTheme[28] = 0; ui21->uControlTheme[29] = 0; ui21->uControlTheme[30] = 0; ui21->uControlTheme[31] = 0; 
	ui21->uFluxCapControl[0] = 0; ui21->uFluxCapControl[1] = 0; ui21->uFluxCapControl[2] = 0; ui21->uFluxCapControl[3] = 0; ui21->uFluxCapControl[4] = 0; ui21->uFluxCapControl[5] = 0; ui21->uFluxCapControl[6] = 0; ui21->uFluxCapControl[7] = 0; ui21->uFluxCapControl[8] = 0; ui21->uFluxCapControl[9] = 0; ui21->uFluxCapControl[10] = 0; ui21->uFluxCapControl[11] = 0; ui21->uFluxCapControl[12] = 0; ui21->uFluxCapControl[13] = 0; ui21->uFluxCapControl[14] = 0; ui21->uFluxCapControl[15] = 0; ui21->uFluxCapControl[16] = 0; ui21->uFluxCapControl[17] = 0; ui21->uFluxCapControl[18] = 0; ui21->uFluxCapControl[19] = 0; ui21->uFluxCapControl[20] = 0; ui21->uFluxCapControl[21] = 0; ui21->uFluxCapControl[22] = 0; ui21->uFluxCapControl[23] = 0; ui21->uFluxCapControl[24] = 0; ui21->uFluxCapControl[25] = 0; ui21->uFluxCapControl[26] = 0; ui21->uFluxCapControl[27] = 0; ui21->uFluxCapControl[28] = 0; ui21->uFluxCapControl[29] = 0; ui21->uFluxCapControl[30] = 0; ui21->uFluxCapControl[31] = 0; ui21->uFluxCapControl[32] = 0; ui21->uFluxCapControl[33] = 0; ui21->uFluxCapControl[34] = 0; ui21->uFluxCapControl[35] = 0; ui21->uFluxCapControl[36] = 0; ui21->uFluxCapControl[37] = 0; ui21->uFluxCapControl[38] = 0; ui21->uFluxCapControl[39] = 0; ui21->uFluxCapControl[40] = 0; ui21->uFluxCapControl[41] = 0; ui21->uFluxCapControl[42] = 0; ui21->uFluxCapControl[43] = 0; ui21->uFluxCapControl[44] = 0; ui21->uFluxCapControl[45] = 0; ui21->uFluxCapControl[46] = 0; ui21->uFluxCapControl[47] = 0; ui21->uFluxCapControl[48] = 0; ui21->uFluxCapControl[49] = 0; ui21->uFluxCapControl[50] = 0; ui21->uFluxCapControl[51] = 0; ui21->uFluxCapControl[52] = 0; ui21->uFluxCapControl[53] = 0; ui21->uFluxCapControl[54] = 0; ui21->uFluxCapControl[55] = 0; ui21->uFluxCapControl[56] = 0; ui21->uFluxCapControl[57] = 0; ui21->uFluxCapControl[58] = 0; ui21->uFluxCapControl[59] = 0; ui21->uFluxCapControl[60] = 0; ui21->uFluxCapControl[61] = 0; ui21->uFluxCapControl[62] = 0; ui21->uFluxCapControl[63] = 0; 
	ui21->fFluxCapData[0] = 0.000000; ui21->fFluxCapData[1] = 0.000000; ui21->fFluxCapData[2] = 0.000000; ui21->fFluxCapData[3] = 0.000000; ui21->fFluxCapData[4] = 0.000000; ui21->fFluxCapData[5] = 0.000000; ui21->fFluxCapData[6] = 0.000000; ui21->fFluxCapData[7] = 0.000000; ui21->fFluxCapData[8] = 0.000000; ui21->fFluxCapData[9] = 0.000000; ui21->fFluxCapData[10] = 0.000000; ui21->fFluxCapData[11] = 0.000000; ui21->fFluxCapData[12] = 0.000000; ui21->fFluxCapData[13] = 0.000000; ui21->fFluxCapData[14] = 0.000000; ui21->fFluxCapData[15] = 0.000000; ui21->fFluxCapData[16] = 0.000000; ui21->fFluxCapData[17] = 0.000000; ui21->fFluxCapData[18] = 0.000000; ui21->fFluxCapData[19] = 0.000000; ui21->fFluxCapData[20] = 0.000000; ui21->fFluxCapData[21] = 0.000000; ui21->fFluxCapData[22] = 0.000000; ui21->fFluxCapData[23] = 0.000000; ui21->fFluxCapData[24] = 0.000000; ui21->fFluxCapData[25] = 0.000000; ui21->fFluxCapData[26] = 0.000000; ui21->fFluxCapData[27] = 0.000000; ui21->fFluxCapData[28] = 0.000000; ui21->fFluxCapData[29] = 0.000000; ui21->fFluxCapData[30] = 0.000000; ui21->fFluxCapData[31] = 0.000000; ui21->fFluxCapData[32] = 0.000000; ui21->fFluxCapData[33] = 0.000000; ui21->fFluxCapData[34] = 0.000000; ui21->fFluxCapData[35] = 0.000000; ui21->fFluxCapData[36] = 0.000000; ui21->fFluxCapData[37] = 0.000000; ui21->fFluxCapData[38] = 0.000000; ui21->fFluxCapData[39] = 0.000000; ui21->fFluxCapData[40] = 0.000000; ui21->fFluxCapData[41] = 0.000000; ui21->fFluxCapData[42] = 0.000000; ui21->fFluxCapData[43] = 0.000000; ui21->fFluxCapData[44] = 0.000000; ui21->fFluxCapData[45] = 0.000000; ui21->fFluxCapData[46] = 0.000000; ui21->fFluxCapData[47] = 0.000000; ui21->fFluxCapData[48] = 0.000000; ui21->fFluxCapData[49] = 0.000000; ui21->fFluxCapData[50] = 0.000000; ui21->fFluxCapData[51] = 0.000000; ui21->fFluxCapData[52] = 0.000000; ui21->fFluxCapData[53] = 0.000000; ui21->fFluxCapData[54] = 0.000000; ui21->fFluxCapData[55] = 0.000000; ui21->fFluxCapData[56] = 0.000000; ui21->fFluxCapData[57] = 0.000000; ui21->fFluxCapData[58] = 0.000000; ui21->fFluxCapData[59] = 0.000000; ui21->fFluxCapData[60] = 0.000000; ui21->fFluxCapData[61] = 0.000000; ui21->fFluxCapData[62] = 0.000000; ui21->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui21);
	delete ui21;


	m_fModFrequency_Hz = 0.180000;
	CUICtrl* ui22 = new CUICtrl;
	ui22->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui22->uControlId = 30;
	ui22->bLogSlider = false;
	ui22->bExpSlider = false;
	ui22->fUserDisplayDataLoLimit = 0.020000;
	ui22->fUserDisplayDataHiLimit = 5.000000;
	ui22->uUserDataType = floatData;
	ui22->fInitUserIntValue = 0;
	ui22->fInitUserFloatValue = 0.180000;
	ui22->fInitUserDoubleValue = 0;
	ui22->fInitUserUINTValue = 0;
	ui22->m_pUserCookedIntData = NULL;
	ui22->m_pUserCookedFloatData = &m_fModFrequency_Hz;
	ui22->m_pUserCookedDoubleData = NULL;
	ui22->m_pUserCookedUINTData = NULL;
	ui22->cControlUnits = "Hz";
	ui22->cVariableName = "m_fModFrequency_Hz";
	ui22->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui22->dPresetData[0] = 0.180000;ui22->dPresetData[1] = 0.000000;ui22->dPresetData[2] = 0.000000;ui22->dPresetData[3] = 0.000000;ui22->dPresetData[4] = 0.000000;ui22->dPresetData[5] = 0.000000;ui22->dPresetData[6] = 0.000000;ui22->dPresetData[7] = 0.000000;ui22->dPresetData[8] = 0.000000;ui22->dPresetData[9] = 0.000000;ui22->dPresetData[10] = 0.000000;ui22->dPresetData[11] = 0.000000;ui22->dPresetData[12] = 0.000000;ui22->dPresetData[13] = 0.000000;ui22->dPresetData[14] = 0.000000;ui22->dPresetData[15] = 0.000000;
	ui22->cControlName = "Rate";
	ui22->bOwnerControl = false;
	ui22->bMIDIControl = false;
	ui22->uMIDIControlCommand = 176;
	ui22->uMIDIControlName = 3;
	ui22->uMIDIControlChannel = 0;
	ui22->nGUIRow = nIndexer++;
	ui22->nGUIColumn = -1;
	ui22->bEnableParamSmoothing = false;
	ui22->fSmoothingTimeInMs = 100.00;
	ui22->uControlTheme[0] = 0; ui22->uControlTheme[1] = 0; ui22->uControlTheme[2] = 0; ui22->uControlTheme[3] = 0; ui22->uControlTheme[4] = 0; ui22->uControlTheme[5] = 0; ui22->uControlTheme[6] = 0; ui22->uControlTheme[7] = 0; ui22->uControlTheme[8] = 25; ui22->uControlTheme[9] = 0; ui22->uControlTheme[10] = 0; ui22->uControlTheme[11] = 0; ui22->uControlTheme[12] = 0; ui22->uControlTheme[13] = 0; ui22->uControlTheme[14] = 0; ui22->uControlTheme[15] = 0; ui22->uControlTheme[16] = 2; ui22->uControlTheme[17] = 0; ui22->uControlTheme[18] = 0; ui22->uControlTheme[19] = 0; ui22->uControlTheme[20] = 0; ui22->uControlTheme[21] = 0; ui22->uControlTheme[22] = 0; ui22->uControlTheme[23] = 0; ui22->uControlTheme[24] = 0; ui22->uControlTheme[25] = 0; ui22->uControlTheme[26] = 0; ui22->uControlTheme[27] = 0; ui22->uControlTheme[28] = 0; ui22->uControlTheme[29] = 0; ui22->uControlTheme[30] = 0; ui22->uControlTheme[31] = 0; 
	ui22->uFluxCapControl[0] = 0; ui22->uFluxCapControl[1] = 0; ui22->uFluxCapControl[2] = 0; ui22->uFluxCapControl[3] = 0; ui22->uFluxCapControl[4] = 0; ui22->uFluxCapControl[5] = 0; ui22->uFluxCapControl[6] = 0; ui22->uFluxCapControl[7] = 0; ui22->uFluxCapControl[8] = 0; ui22->uFluxCapControl[9] = 0; ui22->uFluxCapControl[10] = 0; ui22->uFluxCapControl[11] = 0; ui22->uFluxCapControl[12] = 0; ui22->uFluxCapControl[13] = 0; ui22->uFluxCapControl[14] = 0; ui22->uFluxCapControl[15] = 0; ui22->uFluxCapControl[16] = 0; ui22->uFluxCapControl[17] = 0; ui22->uFluxCapControl[18] = 0; ui22->uFluxCapControl[19] = 0; ui22->uFluxCapControl[20] = 0; ui22->uFluxCapControl[21] = 0; ui22->uFluxCapControl[22] = 0; ui22->uFluxCapControl[23] = 0; ui22->uFluxCapControl[24] = 0; ui22->uFluxCapControl[25] = 0; ui22->uFluxCapControl[26] = 0; ui22->uFluxCapControl[27] = 0; ui22->uFluxCapControl[28] = 0; ui22->uFluxCapControl[29] = 0; ui22->uFluxCapControl[30] = 0; ui22->uFluxCapControl[31] = 0; ui22->uFluxCapControl[32] = 0; ui22->uFluxCapControl[33] = 0; ui22->uFluxCapControl[34] = 0; ui22->uFluxCapControl[35] = 0; ui22->uFluxCapControl[36] = 0; ui22->uFluxCapControl[37] = 0; ui22->uFluxCapControl[38] = 0; ui22->uFluxCapControl[39] = 0; ui22->uFluxCapControl[40] = 0; ui22->uFluxCapControl[41] = 0; ui22->uFluxCapControl[42] = 0; ui22->uFluxCapControl[43] = 0; ui22->uFluxCapControl[44] = 0; ui22->uFluxCapControl[45] = 0; ui22->uFluxCapControl[46] = 0; ui22->uFluxCapControl[47] = 0; ui22->uFluxCapControl[48] = 0; ui22->uFluxCapControl[49] = 0; ui22->uFluxCapControl[50] = 0; ui22->uFluxCapControl[51] = 0; ui22->uFluxCapControl[52] = 0; ui22->uFluxCapControl[53] = 0; ui22->uFluxCapControl[54] = 0; ui22->uFluxCapControl[55] = 0; ui22->uFluxCapControl[56] = 0; ui22->uFluxCapControl[57] = 0; ui22->uFluxCapControl[58] = 0; ui22->uFluxCapControl[59] = 0; ui22->uFluxCapControl[60] = 0; ui22->uFluxCapControl[61] = 0; ui22->uFluxCapControl[62] = 0; ui22->uFluxCapControl[63] = 0; 
	ui22->fFluxCapData[0] = 0.000000; ui22->fFluxCapData[1] = 0.000000; ui22->fFluxCapData[2] = 0.000000; ui22->fFluxCapData[3] = 0.000000; ui22->fFluxCapData[4] = 0.000000; ui22->fFluxCapData[5] = 0.000000; ui22->fFluxCapData[6] = 0.000000; ui22->fFluxCapData[7] = 0.000000; ui22->fFluxCapData[8] = 0.000000; ui22->fFluxCapData[9] = 0.000000; ui22->fFluxCapData[10] = 0.000000; ui22->fFluxCapData[11] = 0.000000; ui22->fFluxCapData[12] = 0.000000; ui22->fFluxCapData[13] = 0.000000; ui22->fFluxCapData[14] = 0.000000; ui22->fFluxCapData[15] = 0.000000; ui22->fFluxCapData[16] = 0.000000; ui22->fFluxCapData[17] = 0.000000; ui22->fFluxCapData[18] = 0.000000; ui22->fFluxCapData[19] = 0.000000; ui22->fFluxCapData[20] = 0.000000; ui22->fFluxCapData[21] = 0.000000; ui22->fFluxCapData[22] = 0.000000; ui22->fFluxCapData[23] = 0.000000; ui22->fFluxCapData[24] = 0.000000; ui22->fFluxCapData[25] = 0.000000; ui22->fFluxCapData[26] = 0.000000; ui22->fFluxCapData[27] = 0.000000; ui22->fFluxCapData[28] = 0.000000; ui22->fFluxCapData[29] = 0.000000; ui22->fFluxCapData[30] = 0.000000; ui22->fFluxCapData[31] = 0.000000; ui22->fFluxCapData[32] = 0.000000; ui22->fFluxCapData[33] = 0.000000; ui22->fFluxCapData[34] = 0.000000; ui22->fFluxCapData[35] = 0.000000; ui22->fFluxCapData[36] = 0.000000; ui22->fFluxCapData[37] = 0.000000; ui22->fFluxCapData[38] = 0.000000; ui22->fFluxCapData[39] = 0.000000; ui22->fFluxCapData[40] = 0.000000; ui22->fFluxCapData[41] = 0.000000; ui22->fFluxCapData[42] = 0.000000; ui22->fFluxCapData[43] = 0.000000; ui22->fFluxCapData[44] = 0.000000; ui22->fFluxCapData[45] = 0.000000; ui22->fFluxCapData[46] = 0.000000; ui22->fFluxCapData[47] = 0.000000; ui22->fFluxCapData[48] = 0.000000; ui22->fFluxCapData[49] = 0.000000; ui22->fFluxCapData[50] = 0.000000; ui22->fFluxCapData[51] = 0.000000; ui22->fFluxCapData[52] = 0.000000; ui22->fFluxCapData[53] = 0.000000; ui22->fFluxCapData[54] = 0.000000; ui22->fFluxCapData[55] = 0.000000; ui22->fFluxCapData[56] = 0.000000; ui22->fFluxCapData[57] = 0.000000; ui22->fFluxCapData[58] = 0.000000; ui22->fFluxCapData[59] = 0.000000; ui22->fFluxCapData[60] = 0.000000; ui22->fFluxCapData[61] = 0.000000; ui22->fFluxCapData[62] = 0.000000; ui22->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui22);
	delete ui22;


	m_fModDepth_pct = 50.000000;
	CUICtrl* ui23 = new CUICtrl;
	ui23->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui23->uControlId = 31;
	ui23->bLogSlider = false;
	ui23->bExpSlider = false;
	ui23->fUserDisplayDataLoLimit = 0.000000;
	ui23->fUserDisplayDataHiLimit = 100.000000;
	ui23->uUserDataType = floatData;
	ui23->fInitUserIntValue = 0;
	ui23->fInitUserFloatValue = 50.000000;
	ui23->fInitUserDoubleValue = 0;
	ui23->fInitUserUINTValue = 0;
	ui23->m_pUserCookedIntData = NULL;
	ui23->m_pUserCookedFloatData = &m_fModDepth_pct;
	ui23->m_pUserCookedDoubleData = NULL;
	ui23->m_pUserCookedUINTData = NULL;
	ui23->cControlUnits = "%";
	ui23->cVariableName = "m_fModDepth_pct";
	ui23->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui23->dPresetData[0] = 50.000000;ui23->dPresetData[1] = 0.000000;ui23->dPresetData[2] = 0.000000;ui23->dPresetData[3] = 0.000000;ui23->dPresetData[4] = 0.000000;ui23->dPresetData[5] = 0.000000;ui23->dPresetData[6] = 0.000000;ui23->dPresetData[7] = 0.000000;ui23->dPresetData[8] = 0.000000;ui23->dPresetData[9] = 0.000000;ui23->dPresetData[10] = 0.000000;ui23->dPresetData[11] = 0.000000;ui23->dPresetData[12] = 0.000000;ui23->dPresetData[13] = 0.000000;ui23->dPresetData[14] = 0.000000;ui23->dPresetData[15] = 0.000000;
	ui23->cControlName = "Depth";
	ui23->bOwnerControl = false;
	ui23->bMIDIControl = false;
	ui23->uMIDIControlCommand = 176;
	ui23->uMIDIControlName = 3;
	ui23->uMIDIControlChannel = 0;
	ui23->nGUIRow = nIndexer++;
	ui23->nGUIColumn = -1;
	ui23->bEnableParamSmoothing = true;
	ui23->fSmoothingTimeInMs = 100.00;
	ui23->uControlTheme[0] = 0; ui23->uControlTheme[1] = 0; ui23->uControlTheme[2] = 0; ui23->uControlTheme[3] = 0; ui23->uControlTheme[4] = 0; ui23->uControlTheme[5] = 0; ui23->uControlTheme[6] = 0; ui23->uControlTheme[7] = 0; ui23->uControlTheme[8] = 24; ui23->uControlTheme[9] = 0; ui23->uControlTheme[10] = 0; ui23->uControlTheme[11] = 0; ui23->uControlTheme[12] = 0; ui23->uControlTheme[13] = 0; ui23->uControlTheme[14] = 0; ui23->uControlTheme[15] = 0; ui23->uControlTheme[16] = 2; ui23->uControlTheme[17] = 0; ui23->uControlTheme[18] = 0; ui23->uControlTheme[19] = 0; ui23->uControlTheme[20] = 0; ui23->uControlTheme[21] = 0; ui23->uControlTheme[22] = 0; ui23->uControlTheme[23] = 0; ui23->uControlTheme[24] = 0; ui23->uControlTheme[25] = 0; ui23->uControlTheme[26] = 0; ui23->uControlTheme[27] = 0; ui23->uControlTheme[28] = 0; ui23->uControlTheme[29] = 0; ui23->uControlTheme[30] = 0; ui23->uControlTheme[31] = 0; 
	ui23->uFluxCapControl[0] = 0; ui23->uFluxCapControl[1] = 0; ui23->uFluxCapControl[2] = 0; ui23->uFluxCapControl[3] = 0; ui23->uFluxCapControl[4] = 0; ui23->uFluxCapControl[5] = 0; ui23->uFluxCapControl[6] = 0; ui23->uFluxCapControl[7] = 0; ui23->uFluxCapControl[8] = 0; ui23->uFluxCapControl[9] = 0; ui23->uFluxCapControl[10] = 0; ui23->uFluxCapControl[11] = 0; ui23->uFluxCapControl[12] = 0; ui23->uFluxCapControl[13] = 0; ui23->uFluxCapControl[14] = 0; ui23->uFluxCapControl[15] = 0; ui23->uFluxCapControl[16] = 0; ui23->uFluxCapControl[17] = 0; ui23->uFluxCapControl[18] = 0; ui23->uFluxCapControl[19] = 0; ui23->uFluxCapControl[20] = 0; ui23->uFluxCapControl[21] = 0; ui23->uFluxCapControl[22] = 0; ui23->uFluxCapControl[23] = 0; ui23->uFluxCapControl[24] = 0; ui23->uFluxCapControl[25] = 0; ui23->uFluxCapControl[26] = 0; ui23->uFluxCapControl[27] = 0; ui23->uFluxCapControl[28] = 0; ui23->uFluxCapControl[29] = 0; ui23->uFluxCapControl[30] = 0; ui23->uFluxCapControl[31] = 0; ui23->uFluxCapControl[32] = 0; ui23->uFluxCapControl[33] = 0; ui23->uFluxCapControl[34] = 0; ui23->uFluxCapControl[35] = 0; ui23->uFluxCapControl[36] = 0; ui23->uFluxCapControl[37] = 0; ui23->uFluxCapControl[38] = 0; ui23->uFluxCapControl[39] = 0; ui23->uFluxCapControl[40] = 0; ui23->uFluxCapControl[41] = 0; ui23->uFluxCapControl[42] = 0; ui23->uFluxCapControl[43] = 0; ui23->uFluxCapControl[44] = 0; ui23->uFluxCapControl[45] = 0; ui23->uFluxCapControl[46] = 0; ui23->uFluxCapControl[47] = 0; ui23->uFluxCapControl[48] = 0; ui23->uFluxCapControl[49] = 0; ui23->uFluxCapControl[50] = 0; ui23->uFluxCapControl[51] = 0; ui23->uFluxCapControl[52] = 0; ui23->uFluxCapControl[53] = 0; ui23->uFluxCapControl[54] = 0; ui23->uFluxCapControl[55] = 0; ui23->uFluxCapControl[56] = 0; ui23->uFluxCapControl[57] = 0; ui23->uFluxCapControl[58] = 0; ui23->uFluxCapControl[59] = 0; ui23->uFluxCapControl[60] = 0; ui23->uFluxCapControl[61] = 0; ui23->uFluxCapControl[62] = 0; ui23->uFluxCapControl[63] = 0; 
	ui23->fFluxCapData[0] = 0.000000; ui23->fFluxCapData[1] = 0.000000; ui23->fFluxCapData[2] = 0.000000; ui23->fFluxCapData[3] = 0.000000; ui23->fFluxCapData[4] = 0.000000; ui23->fFluxCapData[5] = 0.000000; ui23->fFluxCapData[6] = 0.000000; ui23->fFluxCapData[7] = 0.000000; ui23->fFluxCapData[8] = 0.000000; ui23->fFluxCapData[9] = 0.000000; ui23->fFluxCapData[10] = 0.000000; ui23->fFluxCapData[11] = 0.000000; ui23->fFluxCapData[12] = 0.000000; ui23->fFluxCapData[13] = 0.000000; ui23->fFluxCapData[14] = 0.000000; ui23->fFluxCapData[15] = 0.000000; ui23->fFluxCapData[16] = 0.000000; ui23->fFluxCapData[17] = 0.000000; ui23->fFluxCapData[18] = 0.000000; ui23->fFluxCapData[19] = 0.000000; ui23->fFluxCapData[20] = 0.000000; ui23->fFluxCapData[21] = 0.000000; ui23->fFluxCapData[22] = 0.000000; ui23->fFluxCapData[23] = 0.000000; ui23->fFluxCapData[24] = 0.000000; ui23->fFluxCapData[25] = 0.000000; ui23->fFluxCapData[26] = 0.000000; ui23->fFluxCapData[27] = 0.000000; ui23->fFluxCapData[28] = 0.000000; ui23->fFluxCapData[29] = 0.000000; ui23->fFluxCapData[30] = 0.000000; ui23->fFluxCapData[31] = 0.000000; ui23->fFluxCapData[32] = 0.000000; ui23->fFluxCapData[33] = 0.000000; ui23->fFluxCapData[34] = 0.000000; ui23->fFluxCapData[35] = 0.000000; ui23->fFluxCapData[36] = 0.000000; ui23->fFluxCapData[37] = 0.000000; ui23->fFluxCapData[38] = 0.000000; ui23->fFluxCapData[39] = 0.000000; ui23->fFluxCapData[40] = 0.000000; ui23->fFluxCapData[41] = 0.000000; ui23->fFluxCapData[42] = 0.000000; ui23->fFluxCapData[43] = 0.000000; ui23->fFluxCapData[44] = 0.000000; ui23->fFluxCapData[45] = 0.000000; ui23->fFluxCapData[46] = 0.000000; ui23->fFluxCapData[47] = 0.000000; ui23->fFluxCapData[48] = 0.000000; ui23->fFluxCapData[49] = 0.000000; ui23->fFluxCapData[50] = 0.000000; ui23->fFluxCapData[51] = 0.000000; ui23->fFluxCapData[52] = 0.000000; ui23->fFluxCapData[53] = 0.000000; ui23->fFluxCapData[54] = 0.000000; ui23->fFluxCapData[55] = 0.000000; ui23->fFluxCapData[56] = 0.000000; ui23->fFluxCapData[57] = 0.000000; ui23->fFluxCapData[58] = 0.000000; ui23->fFluxCapData[59] = 0.000000; ui23->fFluxCapData[60] = 0.000000; ui23->fFluxCapData[61] = 0.000000; ui23->fFluxCapData[62] = 0.000000; ui23->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui23);
	delete ui23;


	m_fFeedback_pct = 50.000000;
	CUICtrl* ui24 = new CUICtrl;
	ui24->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui24->uControlId = 32;
	ui24->bLogSlider = false;
	ui24->bExpSlider = false;
	ui24->fUserDisplayDataLoLimit = 0.000000;
	ui24->fUserDisplayDataHiLimit = 100.000000;
	ui24->uUserDataType = floatData;
	ui24->fInitUserIntValue = 0;
	ui24->fInitUserFloatValue = 50.000000;
	ui24->fInitUserDoubleValue = 0;
	ui24->fInitUserUINTValue = 0;
	ui24->m_pUserCookedIntData = NULL;
	ui24->m_pUserCookedFloatData = &m_fFeedback_pct;
	ui24->m_pUserCookedDoubleData = NULL;
	ui24->m_pUserCookedUINTData = NULL;
	ui24->cControlUnits = "%";
	ui24->cVariableName = "m_fFeedback_pct";
	ui24->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui24->dPresetData[0] = 50.000000;ui24->dPresetData[1] = 0.000000;ui24->dPresetData[2] = 0.000000;ui24->dPresetData[3] = 0.000000;ui24->dPresetData[4] = 0.000000;ui24->dPresetData[5] = 0.000000;ui24->dPresetData[6] = 0.000000;ui24->dPresetData[7] = 0.000000;ui24->dPresetData[8] = 0.000000;ui24->dPresetData[9] = 0.000000;ui24->dPresetData[10] = 0.000000;ui24->dPresetData[11] = 0.000000;ui24->dPresetData[12] = 0.000000;ui24->dPresetData[13] = 0.000000;ui24->dPresetData[14] = 0.000000;ui24->dPresetData[15] = 0.000000;
	ui24->cControlName = "Resonance";
	ui24->bOwnerControl = false;
	ui24->bMIDIControl = false;
	ui24->uMIDIControlCommand = 176;
	ui24->uMIDIControlName = 3;
	ui24->uMIDIControlChannel = 0;
	ui24->nGUIRow = nIndexer++;
	ui24->nGUIColumn = -1;
	ui24->bEnableParamSmoothing = false;
	ui24->fSmoothingTimeInMs = 100.00;
	ui24->uControlTheme[0] = 0; ui24->uControlTheme[1] = 0; ui24->uControlTheme[2] = 0; ui24->uControlTheme[3] = 0; ui24->uControlTheme[4] = 0; ui24->uControlTheme[5] = 0; ui24->uControlTheme[6] = 0; ui24->uControlTheme[7] = 0; ui24->uControlTheme[8] = 0; ui24->uControlTheme[9] = 0; ui24->uControlTheme[10] = 0; ui24->uControlTheme[11] = 0; ui24->uControlTheme[12] = 0; ui24->uControlTheme[13] = 0; ui24->uControlTheme[14] = 0; ui24->uControlTheme[15] = 0; ui24->uControlTheme[16] = 2; ui24->uControlTheme[17] = 0; ui24->uControlTheme[18] = 0; ui24->uControlTheme[19] = 0; ui24->uControlTheme[20] = 0; ui24->uControlTheme[21] = 0; ui24->uControlTheme[22] = 0; ui24->uControlTheme[23] = 0; ui24->uControlTheme[24] = 0; ui24->uControlTheme[25] = 0; ui24->uControlTheme[26] = 0; ui24->uControlTheme[27] = 0; ui24->uControlTheme[28] = 0; ui24->uControlTheme[29] = 0; ui24->uControlTheme[30] = 0; ui24->uControlTheme[31] = 0; 
	ui24->uFluxCapControl[0] = 0; ui24->uFluxCapControl[1] = 0; ui24->uFluxCapControl[2] = 0; ui24->uFluxCapControl[3] = 0; ui24->uFluxCapControl[4] = 0; ui24->uFluxCapControl[5] = 0; ui24->uFluxCapControl[6] = 0; ui24->uFluxCapControl[7] = 0; ui24->uFluxCapControl[8] = 0; ui24->uFluxCapControl[9] = 0; ui24->uFluxCapControl[10] = 0; ui24->uFluxCapControl[11] = 0; ui24->uFluxCapControl[12] = 0; ui24->uFluxCapControl[13] = 0; ui24->uFluxCapControl[14] = 0; ui24->uFluxCapControl[15] = 0; ui24->uFluxCapControl[16] = 0; ui24->uFluxCapControl[17] = 0; ui24->uFluxCapControl[18] = 0; ui24->uFluxCapControl[19] = 0; ui24->uFluxCapControl[20] = 0; ui24->uFluxCapControl[21] = 0; ui24->uFluxCapControl[22] = 0; ui24->uFluxCapControl[23] = 0; ui24->uFluxCapControl[24] = 0; ui24->uFluxCapControl[25] = 0; ui24->uFluxCapControl[26] = 0; ui24->uFluxCapControl[27] = 0; ui24->uFluxCapControl[28] = 0; ui24->uFluxCapControl[29] = 0; ui24->uFluxCapControl[30] = 0; ui24->uFluxCapControl[31] = 0; ui24->uFluxCapControl[32] = 0; ui24->uFluxCapControl[33] = 0; ui24->uFluxCapControl[34] = 0; ui24->uFluxCapControl[35] = 0; ui24->uFluxCapControl[36] = 0; ui24->uFluxCapControl[37] = 0; ui24->uFluxCapControl[38] = 0; ui24->uFluxCapControl[39] = 0; ui24->uFluxCapControl[40] = 0; ui24->uFluxCapControl[41] = 0; ui24->uFluxCapControl[42] = 0; ui24->uFluxCapControl[43] = 0; ui24->uFluxCapControl[44] = 0; ui24->uFluxCapControl[45] = 0; ui24->uFluxCapControl[46] = 0; ui24->uFluxCapControl[47] = 0; ui24->uFluxCapControl[48] = 0; ui24->uFluxCapControl[49] = 0; ui24->uFluxCapControl[50] = 0; ui24->uFluxCapControl[51] = 0; ui24->uFluxCapControl[52] = 0; ui24->uFluxCapControl[53] = 0; ui24->uFluxCapControl[54] = 0; ui24->uFluxCapControl[55] = 0; ui24->uFluxCapControl[56] = 0; ui24->uFluxCapControl[57] = 0; ui24->uFluxCapControl[58] = 0; ui24->uFluxCapControl[59] = 0; ui24->uFluxCapControl[60] = 0; ui24->uFluxCapControl[61] = 0; ui24->uFluxCapControl[62] = 0; ui24->uFluxCapControl[63] = 0; 
	ui24->fFluxCapData[0] = 0.000000; ui24->fFluxCapData[1] = 0.000000; ui24->fFluxCapData[2] = 0.000000; ui24->fFluxCapData[3] = 0.000000; ui24->fFluxCapData[4] = 0.000000; ui24->fFluxCapData[5] = 0.000000; ui24->fFluxCapData[6] = 0.000000; ui24->fFluxCapData[7] = 0.000000; ui24->fFluxCapData[8] = 0.000000; ui24->fFluxCapData[9] = 0.000000; ui24->fFluxCapData[10] = 0.000000; ui24->fFluxCapData[11] = 0.000000; ui24->fFluxCapData[12] = 0.000000; ui24->fFluxCapData[13] = 0.000000; ui24->fFluxCapData[14] = 0.000000; ui24->fFluxCapData[15] = 0.000000; ui24->fFluxCapData[16] = 0.000000; ui24->fFluxCapData[17] = 0.000000; ui24->fFluxCapData[18] = 0.000000; ui24->fFluxCapData[19] = 0.000000; ui24->fFluxCapData[20] = 0.000000; ui24->fFluxCapData[21] = 0.000000; ui24->fFluxCapData[22] = 0.000000; ui24->fFluxCapData[23] = 0.000000; ui24->fFluxCapData[24] = 0.000000; ui24->fFluxCapData[25] = 0.000000; ui24->fFluxCapData[26] = 0.000000; ui24->fFluxCapData[27] = 0.000000; ui24->fFluxCapData[28] = 0.000000; ui24->fFluxCapData[29] = 0.000000; ui24->fFluxCapData[30] = 0.000000; ui24->fFluxCapData[31] = 0.000000; ui24->fFluxCapData[32] = 0.000000; ui24->fFluxCapData[33] = 0.000000; ui24->fFluxCapData[34] = 0.000000; ui24->fFluxCapData[35] = 0.000000; ui24->fFluxCapData[36] = 0.000000; ui24->fFluxCapData[37] = 0.000000; ui24->fFluxCapData[38] = 0.000000; ui24->fFluxCapData[39] = 0.000000; ui24->fFluxCapData[40] = 0.000000; ui24->fFluxCapData[41] = 0.000000; ui24->fFluxCapData[42] = 0.000000; ui24->fFluxCapData[43] = 0.000000; ui24->fFluxCapData[44] = 0.000000; ui24->fFluxCapData[45] = 0.000000; ui24->fFluxCapData[46] = 0.000000; ui24->fFluxCapData[47] = 0.000000; ui24->fFluxCapData[48] = 0.000000; ui24->fFluxCapData[49] = 0.000000; ui24->fFluxCapData[50] = 0.000000; ui24->fFluxCapData[51] = 0.000000; ui24->fFluxCapData[52] = 0.000000; ui24->fFluxCapData[53] = 0.000000; ui24->fFluxCapData[54] = 0.000000; ui24->fFluxCapData[55] = 0.000000; ui24->fFluxCapData[56] = 0.000000; ui24->fFluxCapData[57] = 0.000000; ui24->fFluxCapData[58] = 0.000000; ui24->fFluxCapData[59] = 0.000000; ui24->fFluxCapData[60] = 0.000000; ui24->fFluxCapData[61] = 0.000000; ui24->fFluxCapData[62] = 0.000000; ui24->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui24);
	delete ui24;


	m_fPreDelay = 0.000000;
	CUICtrl* ui25 = new CUICtrl;
	ui25->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui25->uControlId = 33;
	ui25->bLogSlider = false;
	ui25->bExpSlider = false;
	ui25->fUserDisplayDataLoLimit = 0.000000;
	ui25->fUserDisplayDataHiLimit = 200.000000;
	ui25->uUserDataType = floatData;
	ui25->fInitUserIntValue = 0;
	ui25->fInitUserFloatValue = 0.000000;
	ui25->fInitUserDoubleValue = 0;
	ui25->fInitUserUINTValue = 0;
	ui25->m_pUserCookedIntData = NULL;
	ui25->m_pUserCookedFloatData = &m_fPreDelay;
	ui25->m_pUserCookedDoubleData = NULL;
	ui25->m_pUserCookedUINTData = NULL;
	ui25->cControlUnits = "mSec";
	ui25->cVariableName = "m_fPreDelay";
	ui25->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui25->dPresetData[0] = 0.000000;ui25->dPresetData[1] = 0.000000;ui25->dPresetData[2] = 0.000000;ui25->dPresetData[3] = 0.000000;ui25->dPresetData[4] = 0.000000;ui25->dPresetData[5] = 0.000000;ui25->dPresetData[6] = 0.000000;ui25->dPresetData[7] = 0.000000;ui25->dPresetData[8] = 0.000000;ui25->dPresetData[9] = 0.000000;ui25->dPresetData[10] = 0.000000;ui25->dPresetData[11] = 0.000000;ui25->dPresetData[12] = 0.000000;ui25->dPresetData[13] = 0.000000;ui25->dPresetData[14] = 0.000000;ui25->dPresetData[15] = 0.000000;
	ui25->cControlName = "Pre Delay";
	ui25->bOwnerControl = false;
	ui25->bMIDIControl = false;
	ui25->uMIDIControlCommand = 176;
	ui25->uMIDIControlName = 3;
	ui25->uMIDIControlChannel = 0;
	ui25->nGUIRow = nIndexer++;
	ui25->nGUIColumn = -1;
	ui25->bEnableParamSmoothing = false;
	ui25->fSmoothingTimeInMs = 100.00;
	ui25->uControlTheme[0] = 0; ui25->uControlTheme[1] = 0; ui25->uControlTheme[2] = 0; ui25->uControlTheme[3] = 0; ui25->uControlTheme[4] = 0; ui25->uControlTheme[5] = 0; ui25->uControlTheme[6] = 0; ui25->uControlTheme[7] = 0; ui25->uControlTheme[8] = 0; ui25->uControlTheme[9] = 0; ui25->uControlTheme[10] = 0; ui25->uControlTheme[11] = 0; ui25->uControlTheme[12] = 0; ui25->uControlTheme[13] = 0; ui25->uControlTheme[14] = 0; ui25->uControlTheme[15] = 0; ui25->uControlTheme[16] = 2; ui25->uControlTheme[17] = 0; ui25->uControlTheme[18] = 0; ui25->uControlTheme[19] = 0; ui25->uControlTheme[20] = 0; ui25->uControlTheme[21] = 0; ui25->uControlTheme[22] = 0; ui25->uControlTheme[23] = 0; ui25->uControlTheme[24] = 0; ui25->uControlTheme[25] = 0; ui25->uControlTheme[26] = 0; ui25->uControlTheme[27] = 0; ui25->uControlTheme[28] = 0; ui25->uControlTheme[29] = 0; ui25->uControlTheme[30] = 0; ui25->uControlTheme[31] = 0; 
	ui25->uFluxCapControl[0] = 0; ui25->uFluxCapControl[1] = 0; ui25->uFluxCapControl[2] = 0; ui25->uFluxCapControl[3] = 0; ui25->uFluxCapControl[4] = 0; ui25->uFluxCapControl[5] = 0; ui25->uFluxCapControl[6] = 0; ui25->uFluxCapControl[7] = 0; ui25->uFluxCapControl[8] = 0; ui25->uFluxCapControl[9] = 0; ui25->uFluxCapControl[10] = 0; ui25->uFluxCapControl[11] = 0; ui25->uFluxCapControl[12] = 0; ui25->uFluxCapControl[13] = 0; ui25->uFluxCapControl[14] = 0; ui25->uFluxCapControl[15] = 0; ui25->uFluxCapControl[16] = 0; ui25->uFluxCapControl[17] = 0; ui25->uFluxCapControl[18] = 0; ui25->uFluxCapControl[19] = 0; ui25->uFluxCapControl[20] = 0; ui25->uFluxCapControl[21] = 0; ui25->uFluxCapControl[22] = 0; ui25->uFluxCapControl[23] = 0; ui25->uFluxCapControl[24] = 0; ui25->uFluxCapControl[25] = 0; ui25->uFluxCapControl[26] = 0; ui25->uFluxCapControl[27] = 0; ui25->uFluxCapControl[28] = 0; ui25->uFluxCapControl[29] = 0; ui25->uFluxCapControl[30] = 0; ui25->uFluxCapControl[31] = 0; ui25->uFluxCapControl[32] = 0; ui25->uFluxCapControl[33] = 0; ui25->uFluxCapControl[34] = 0; ui25->uFluxCapControl[35] = 0; ui25->uFluxCapControl[36] = 0; ui25->uFluxCapControl[37] = 0; ui25->uFluxCapControl[38] = 0; ui25->uFluxCapControl[39] = 0; ui25->uFluxCapControl[40] = 0; ui25->uFluxCapControl[41] = 0; ui25->uFluxCapControl[42] = 0; ui25->uFluxCapControl[43] = 0; ui25->uFluxCapControl[44] = 0; ui25->uFluxCapControl[45] = 0; ui25->uFluxCapControl[46] = 0; ui25->uFluxCapControl[47] = 0; ui25->uFluxCapControl[48] = 0; ui25->uFluxCapControl[49] = 0; ui25->uFluxCapControl[50] = 0; ui25->uFluxCapControl[51] = 0; ui25->uFluxCapControl[52] = 0; ui25->uFluxCapControl[53] = 0; ui25->uFluxCapControl[54] = 0; ui25->uFluxCapControl[55] = 0; ui25->uFluxCapControl[56] = 0; ui25->uFluxCapControl[57] = 0; ui25->uFluxCapControl[58] = 0; ui25->uFluxCapControl[59] = 0; ui25->uFluxCapControl[60] = 0; ui25->uFluxCapControl[61] = 0; ui25->uFluxCapControl[62] = 0; ui25->uFluxCapControl[63] = 0; 
	ui25->fFluxCapData[0] = 0.000000; ui25->fFluxCapData[1] = 0.000000; ui25->fFluxCapData[2] = 0.000000; ui25->fFluxCapData[3] = 0.000000; ui25->fFluxCapData[4] = 0.000000; ui25->fFluxCapData[5] = 0.000000; ui25->fFluxCapData[6] = 0.000000; ui25->fFluxCapData[7] = 0.000000; ui25->fFluxCapData[8] = 0.000000; ui25->fFluxCapData[9] = 0.000000; ui25->fFluxCapData[10] = 0.000000; ui25->fFluxCapData[11] = 0.000000; ui25->fFluxCapData[12] = 0.000000; ui25->fFluxCapData[13] = 0.000000; ui25->fFluxCapData[14] = 0.000000; ui25->fFluxCapData[15] = 0.000000; ui25->fFluxCapData[16] = 0.000000; ui25->fFluxCapData[17] = 0.000000; ui25->fFluxCapData[18] = 0.000000; ui25->fFluxCapData[19] = 0.000000; ui25->fFluxCapData[20] = 0.000000; ui25->fFluxCapData[21] = 0.000000; ui25->fFluxCapData[22] = 0.000000; ui25->fFluxCapData[23] = 0.000000; ui25->fFluxCapData[24] = 0.000000; ui25->fFluxCapData[25] = 0.000000; ui25->fFluxCapData[26] = 0.000000; ui25->fFluxCapData[27] = 0.000000; ui25->fFluxCapData[28] = 0.000000; ui25->fFluxCapData[29] = 0.000000; ui25->fFluxCapData[30] = 0.000000; ui25->fFluxCapData[31] = 0.000000; ui25->fFluxCapData[32] = 0.000000; ui25->fFluxCapData[33] = 0.000000; ui25->fFluxCapData[34] = 0.000000; ui25->fFluxCapData[35] = 0.000000; ui25->fFluxCapData[36] = 0.000000; ui25->fFluxCapData[37] = 0.000000; ui25->fFluxCapData[38] = 0.000000; ui25->fFluxCapData[39] = 0.000000; ui25->fFluxCapData[40] = 0.000000; ui25->fFluxCapData[41] = 0.000000; ui25->fFluxCapData[42] = 0.000000; ui25->fFluxCapData[43] = 0.000000; ui25->fFluxCapData[44] = 0.000000; ui25->fFluxCapData[45] = 0.000000; ui25->fFluxCapData[46] = 0.000000; ui25->fFluxCapData[47] = 0.000000; ui25->fFluxCapData[48] = 0.000000; ui25->fFluxCapData[49] = 0.000000; ui25->fFluxCapData[50] = 0.000000; ui25->fFluxCapData[51] = 0.000000; ui25->fFluxCapData[52] = 0.000000; ui25->fFluxCapData[53] = 0.000000; ui25->fFluxCapData[54] = 0.000000; ui25->fFluxCapData[55] = 0.000000; ui25->fFluxCapData[56] = 0.000000; ui25->fFluxCapData[57] = 0.000000; ui25->fFluxCapData[58] = 0.000000; ui25->fFluxCapData[59] = 0.000000; ui25->fFluxCapData[60] = 0.000000; ui25->fFluxCapData[61] = 0.000000; ui25->fFluxCapData[62] = 0.000000; ui25->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui25);
	delete ui25;


	m_fChorusOffset = 0.000000;
	CUICtrl* ui26 = new CUICtrl;
	ui26->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui26->uControlId = 34;
	ui26->bLogSlider = false;
	ui26->bExpSlider = false;
	ui26->fUserDisplayDataLoLimit = 0.000000;
	ui26->fUserDisplayDataHiLimit = 30.000000;
	ui26->uUserDataType = floatData;
	ui26->fInitUserIntValue = 0;
	ui26->fInitUserFloatValue = 0.000000;
	ui26->fInitUserDoubleValue = 0;
	ui26->fInitUserUINTValue = 0;
	ui26->m_pUserCookedIntData = NULL;
	ui26->m_pUserCookedFloatData = &m_fChorusOffset;
	ui26->m_pUserCookedDoubleData = NULL;
	ui26->m_pUserCookedUINTData = NULL;
	ui26->cControlUnits = "mSec";
	ui26->cVariableName = "m_fChorusOffset";
	ui26->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui26->dPresetData[0] = 0.000000;ui26->dPresetData[1] = 0.000000;ui26->dPresetData[2] = 0.000000;ui26->dPresetData[3] = 0.000000;ui26->dPresetData[4] = 0.000000;ui26->dPresetData[5] = 0.000000;ui26->dPresetData[6] = 0.000000;ui26->dPresetData[7] = 0.000000;ui26->dPresetData[8] = 0.000000;ui26->dPresetData[9] = 0.000000;ui26->dPresetData[10] = 0.000000;ui26->dPresetData[11] = 0.000000;ui26->dPresetData[12] = 0.000000;ui26->dPresetData[13] = 0.000000;ui26->dPresetData[14] = 0.000000;ui26->dPresetData[15] = 0.000000;
	ui26->cControlName = "Chorus Offset";
	ui26->bOwnerControl = false;
	ui26->bMIDIControl = false;
	ui26->uMIDIControlCommand = 176;
	ui26->uMIDIControlName = 3;
	ui26->uMIDIControlChannel = 0;
	ui26->nGUIRow = nIndexer++;
	ui26->nGUIColumn = -1;
	ui26->bEnableParamSmoothing = false;
	ui26->fSmoothingTimeInMs = 100.00;
	ui26->uControlTheme[0] = 0; ui26->uControlTheme[1] = 0; ui26->uControlTheme[2] = 0; ui26->uControlTheme[3] = 0; ui26->uControlTheme[4] = 0; ui26->uControlTheme[5] = 0; ui26->uControlTheme[6] = 0; ui26->uControlTheme[7] = 0; ui26->uControlTheme[8] = 0; ui26->uControlTheme[9] = 0; ui26->uControlTheme[10] = 0; ui26->uControlTheme[11] = 0; ui26->uControlTheme[12] = 0; ui26->uControlTheme[13] = 0; ui26->uControlTheme[14] = 0; ui26->uControlTheme[15] = 0; ui26->uControlTheme[16] = 2; ui26->uControlTheme[17] = 0; ui26->uControlTheme[18] = 0; ui26->uControlTheme[19] = 0; ui26->uControlTheme[20] = 0; ui26->uControlTheme[21] = 0; ui26->uControlTheme[22] = 0; ui26->uControlTheme[23] = 0; ui26->uControlTheme[24] = 0; ui26->uControlTheme[25] = 0; ui26->uControlTheme[26] = 0; ui26->uControlTheme[27] = 0; ui26->uControlTheme[28] = 0; ui26->uControlTheme[29] = 0; ui26->uControlTheme[30] = 0; ui26->uControlTheme[31] = 0; 
	ui26->uFluxCapControl[0] = 0; ui26->uFluxCapControl[1] = 0; ui26->uFluxCapControl[2] = 0; ui26->uFluxCapControl[3] = 0; ui26->uFluxCapControl[4] = 0; ui26->uFluxCapControl[5] = 0; ui26->uFluxCapControl[6] = 0; ui26->uFluxCapControl[7] = 0; ui26->uFluxCapControl[8] = 0; ui26->uFluxCapControl[9] = 0; ui26->uFluxCapControl[10] = 0; ui26->uFluxCapControl[11] = 0; ui26->uFluxCapControl[12] = 0; ui26->uFluxCapControl[13] = 0; ui26->uFluxCapControl[14] = 0; ui26->uFluxCapControl[15] = 0; ui26->uFluxCapControl[16] = 0; ui26->uFluxCapControl[17] = 0; ui26->uFluxCapControl[18] = 0; ui26->uFluxCapControl[19] = 0; ui26->uFluxCapControl[20] = 0; ui26->uFluxCapControl[21] = 0; ui26->uFluxCapControl[22] = 0; ui26->uFluxCapControl[23] = 0; ui26->uFluxCapControl[24] = 0; ui26->uFluxCapControl[25] = 0; ui26->uFluxCapControl[26] = 0; ui26->uFluxCapControl[27] = 0; ui26->uFluxCapControl[28] = 0; ui26->uFluxCapControl[29] = 0; ui26->uFluxCapControl[30] = 0; ui26->uFluxCapControl[31] = 0; ui26->uFluxCapControl[32] = 0; ui26->uFluxCapControl[33] = 0; ui26->uFluxCapControl[34] = 0; ui26->uFluxCapControl[35] = 0; ui26->uFluxCapControl[36] = 0; ui26->uFluxCapControl[37] = 0; ui26->uFluxCapControl[38] = 0; ui26->uFluxCapControl[39] = 0; ui26->uFluxCapControl[40] = 0; ui26->uFluxCapControl[41] = 0; ui26->uFluxCapControl[42] = 0; ui26->uFluxCapControl[43] = 0; ui26->uFluxCapControl[44] = 0; ui26->uFluxCapControl[45] = 0; ui26->uFluxCapControl[46] = 0; ui26->uFluxCapControl[47] = 0; ui26->uFluxCapControl[48] = 0; ui26->uFluxCapControl[49] = 0; ui26->uFluxCapControl[50] = 0; ui26->uFluxCapControl[51] = 0; ui26->uFluxCapControl[52] = 0; ui26->uFluxCapControl[53] = 0; ui26->uFluxCapControl[54] = 0; ui26->uFluxCapControl[55] = 0; ui26->uFluxCapControl[56] = 0; ui26->uFluxCapControl[57] = 0; ui26->uFluxCapControl[58] = 0; ui26->uFluxCapControl[59] = 0; ui26->uFluxCapControl[60] = 0; ui26->uFluxCapControl[61] = 0; ui26->uFluxCapControl[62] = 0; ui26->uFluxCapControl[63] = 0; 
	ui26->fFluxCapData[0] = 0.000000; ui26->fFluxCapData[1] = 0.000000; ui26->fFluxCapData[2] = 0.000000; ui26->fFluxCapData[3] = 0.000000; ui26->fFluxCapData[4] = 0.000000; ui26->fFluxCapData[5] = 0.000000; ui26->fFluxCapData[6] = 0.000000; ui26->fFluxCapData[7] = 0.000000; ui26->fFluxCapData[8] = 0.000000; ui26->fFluxCapData[9] = 0.000000; ui26->fFluxCapData[10] = 0.000000; ui26->fFluxCapData[11] = 0.000000; ui26->fFluxCapData[12] = 0.000000; ui26->fFluxCapData[13] = 0.000000; ui26->fFluxCapData[14] = 0.000000; ui26->fFluxCapData[15] = 0.000000; ui26->fFluxCapData[16] = 0.000000; ui26->fFluxCapData[17] = 0.000000; ui26->fFluxCapData[18] = 0.000000; ui26->fFluxCapData[19] = 0.000000; ui26->fFluxCapData[20] = 0.000000; ui26->fFluxCapData[21] = 0.000000; ui26->fFluxCapData[22] = 0.000000; ui26->fFluxCapData[23] = 0.000000; ui26->fFluxCapData[24] = 0.000000; ui26->fFluxCapData[25] = 0.000000; ui26->fFluxCapData[26] = 0.000000; ui26->fFluxCapData[27] = 0.000000; ui26->fFluxCapData[28] = 0.000000; ui26->fFluxCapData[29] = 0.000000; ui26->fFluxCapData[30] = 0.000000; ui26->fFluxCapData[31] = 0.000000; ui26->fFluxCapData[32] = 0.000000; ui26->fFluxCapData[33] = 0.000000; ui26->fFluxCapData[34] = 0.000000; ui26->fFluxCapData[35] = 0.000000; ui26->fFluxCapData[36] = 0.000000; ui26->fFluxCapData[37] = 0.000000; ui26->fFluxCapData[38] = 0.000000; ui26->fFluxCapData[39] = 0.000000; ui26->fFluxCapData[40] = 0.000000; ui26->fFluxCapData[41] = 0.000000; ui26->fFluxCapData[42] = 0.000000; ui26->fFluxCapData[43] = 0.000000; ui26->fFluxCapData[44] = 0.000000; ui26->fFluxCapData[45] = 0.000000; ui26->fFluxCapData[46] = 0.000000; ui26->fFluxCapData[47] = 0.000000; ui26->fFluxCapData[48] = 0.000000; ui26->fFluxCapData[49] = 0.000000; ui26->fFluxCapData[50] = 0.000000; ui26->fFluxCapData[51] = 0.000000; ui26->fFluxCapData[52] = 0.000000; ui26->fFluxCapData[53] = 0.000000; ui26->fFluxCapData[54] = 0.000000; ui26->fFluxCapData[55] = 0.000000; ui26->fFluxCapData[56] = 0.000000; ui26->fFluxCapData[57] = 0.000000; ui26->fFluxCapData[58] = 0.000000; ui26->fFluxCapData[59] = 0.000000; ui26->fFluxCapData[60] = 0.000000; ui26->fFluxCapData[61] = 0.000000; ui26->fFluxCapData[62] = 0.000000; ui26->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui26);
	delete ui26;


	m_fAPF_5_Delay_mSec = 11.000000;
	CUICtrl* ui27 = new CUICtrl;
	ui27->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui27->uControlId = 38;
	ui27->bLogSlider = false;
	ui27->bExpSlider = false;
	ui27->fUserDisplayDataLoLimit = 0.000000;
	ui27->fUserDisplayDataHiLimit = 100.000000;
	ui27->uUserDataType = floatData;
	ui27->fInitUserIntValue = 0;
	ui27->fInitUserFloatValue = 11.000000;
	ui27->fInitUserDoubleValue = 0;
	ui27->fInitUserUINTValue = 0;
	ui27->m_pUserCookedIntData = NULL;
	ui27->m_pUserCookedFloatData = &m_fAPF_5_Delay_mSec;
	ui27->m_pUserCookedDoubleData = NULL;
	ui27->m_pUserCookedUINTData = NULL;
	ui27->cControlUnits = "mSec";
	ui27->cVariableName = "m_fAPF_5_Delay_mSec";
	ui27->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui27->dPresetData[0] = 11.000000;ui27->dPresetData[1] = 51.000000;ui27->dPresetData[2] = 51.000000;ui27->dPresetData[3] = 24.999998;ui27->dPresetData[4] = 0.000000;ui27->dPresetData[5] = 51.000000;ui27->dPresetData[6] = 0.000000;ui27->dPresetData[7] = 0.000000;ui27->dPresetData[8] = 0.000000;ui27->dPresetData[9] = 0.000000;ui27->dPresetData[10] = 0.000000;ui27->dPresetData[11] = 0.000000;ui27->dPresetData[12] = 0.000000;ui27->dPresetData[13] = 0.000000;ui27->dPresetData[14] = 0.000000;ui27->dPresetData[15] = 0.000000;
	ui27->cControlName = "APF5 Dly";
	ui27->bOwnerControl = false;
	ui27->bMIDIControl = false;
	ui27->uMIDIControlCommand = 176;
	ui27->uMIDIControlName = 3;
	ui27->uMIDIControlChannel = 0;
	ui27->nGUIRow = nIndexer++;
	ui27->nGUIColumn = -1;
	ui27->bEnableParamSmoothing = false;
	ui27->fSmoothingTimeInMs = 100.00;
	ui27->uControlTheme[0] = 0; ui27->uControlTheme[1] = 0; ui27->uControlTheme[2] = 0; ui27->uControlTheme[3] = 0; ui27->uControlTheme[4] = 0; ui27->uControlTheme[5] = 0; ui27->uControlTheme[6] = 0; ui27->uControlTheme[7] = 0; ui27->uControlTheme[8] = 0; ui27->uControlTheme[9] = 0; ui27->uControlTheme[10] = 0; ui27->uControlTheme[11] = 0; ui27->uControlTheme[12] = 0; ui27->uControlTheme[13] = 0; ui27->uControlTheme[14] = 0; ui27->uControlTheme[15] = 0; ui27->uControlTheme[16] = 2; ui27->uControlTheme[17] = 0; ui27->uControlTheme[18] = 0; ui27->uControlTheme[19] = 0; ui27->uControlTheme[20] = 0; ui27->uControlTheme[21] = 0; ui27->uControlTheme[22] = 0; ui27->uControlTheme[23] = 0; ui27->uControlTheme[24] = 0; ui27->uControlTheme[25] = 0; ui27->uControlTheme[26] = 0; ui27->uControlTheme[27] = 0; ui27->uControlTheme[28] = 0; ui27->uControlTheme[29] = 0; ui27->uControlTheme[30] = 0; ui27->uControlTheme[31] = 0; 
	ui27->uFluxCapControl[0] = 0; ui27->uFluxCapControl[1] = 0; ui27->uFluxCapControl[2] = 0; ui27->uFluxCapControl[3] = 0; ui27->uFluxCapControl[4] = 0; ui27->uFluxCapControl[5] = 0; ui27->uFluxCapControl[6] = 0; ui27->uFluxCapControl[7] = 0; ui27->uFluxCapControl[8] = 0; ui27->uFluxCapControl[9] = 0; ui27->uFluxCapControl[10] = 0; ui27->uFluxCapControl[11] = 0; ui27->uFluxCapControl[12] = 0; ui27->uFluxCapControl[13] = 0; ui27->uFluxCapControl[14] = 0; ui27->uFluxCapControl[15] = 0; ui27->uFluxCapControl[16] = 0; ui27->uFluxCapControl[17] = 0; ui27->uFluxCapControl[18] = 0; ui27->uFluxCapControl[19] = 0; ui27->uFluxCapControl[20] = 0; ui27->uFluxCapControl[21] = 0; ui27->uFluxCapControl[22] = 0; ui27->uFluxCapControl[23] = 0; ui27->uFluxCapControl[24] = 0; ui27->uFluxCapControl[25] = 0; ui27->uFluxCapControl[26] = 0; ui27->uFluxCapControl[27] = 0; ui27->uFluxCapControl[28] = 0; ui27->uFluxCapControl[29] = 0; ui27->uFluxCapControl[30] = 0; ui27->uFluxCapControl[31] = 0; ui27->uFluxCapControl[32] = 0; ui27->uFluxCapControl[33] = 0; ui27->uFluxCapControl[34] = 0; ui27->uFluxCapControl[35] = 0; ui27->uFluxCapControl[36] = 0; ui27->uFluxCapControl[37] = 0; ui27->uFluxCapControl[38] = 0; ui27->uFluxCapControl[39] = 0; ui27->uFluxCapControl[40] = 0; ui27->uFluxCapControl[41] = 0; ui27->uFluxCapControl[42] = 0; ui27->uFluxCapControl[43] = 0; ui27->uFluxCapControl[44] = 0; ui27->uFluxCapControl[45] = 0; ui27->uFluxCapControl[46] = 0; ui27->uFluxCapControl[47] = 0; ui27->uFluxCapControl[48] = 0; ui27->uFluxCapControl[49] = 0; ui27->uFluxCapControl[50] = 0; ui27->uFluxCapControl[51] = 0; ui27->uFluxCapControl[52] = 0; ui27->uFluxCapControl[53] = 0; ui27->uFluxCapControl[54] = 0; ui27->uFluxCapControl[55] = 0; ui27->uFluxCapControl[56] = 0; ui27->uFluxCapControl[57] = 0; ui27->uFluxCapControl[58] = 0; ui27->uFluxCapControl[59] = 0; ui27->uFluxCapControl[60] = 0; ui27->uFluxCapControl[61] = 0; ui27->uFluxCapControl[62] = 0; ui27->uFluxCapControl[63] = 0; 
	ui27->fFluxCapData[0] = 0.000000; ui27->fFluxCapData[1] = 0.000000; ui27->fFluxCapData[2] = 0.000000; ui27->fFluxCapData[3] = 0.000000; ui27->fFluxCapData[4] = 0.000000; ui27->fFluxCapData[5] = 0.000000; ui27->fFluxCapData[6] = 0.000000; ui27->fFluxCapData[7] = 0.000000; ui27->fFluxCapData[8] = 0.000000; ui27->fFluxCapData[9] = 0.000000; ui27->fFluxCapData[10] = 0.000000; ui27->fFluxCapData[11] = 0.000000; ui27->fFluxCapData[12] = 0.000000; ui27->fFluxCapData[13] = 0.000000; ui27->fFluxCapData[14] = 0.000000; ui27->fFluxCapData[15] = 0.000000; ui27->fFluxCapData[16] = 0.000000; ui27->fFluxCapData[17] = 0.000000; ui27->fFluxCapData[18] = 0.000000; ui27->fFluxCapData[19] = 0.000000; ui27->fFluxCapData[20] = 0.000000; ui27->fFluxCapData[21] = 0.000000; ui27->fFluxCapData[22] = 0.000000; ui27->fFluxCapData[23] = 0.000000; ui27->fFluxCapData[24] = 0.000000; ui27->fFluxCapData[25] = 0.000000; ui27->fFluxCapData[26] = 0.000000; ui27->fFluxCapData[27] = 0.000000; ui27->fFluxCapData[28] = 0.000000; ui27->fFluxCapData[29] = 0.000000; ui27->fFluxCapData[30] = 0.000000; ui27->fFluxCapData[31] = 0.000000; ui27->fFluxCapData[32] = 0.000000; ui27->fFluxCapData[33] = 0.000000; ui27->fFluxCapData[34] = 0.000000; ui27->fFluxCapData[35] = 0.000000; ui27->fFluxCapData[36] = 0.000000; ui27->fFluxCapData[37] = 0.000000; ui27->fFluxCapData[38] = 0.000000; ui27->fFluxCapData[39] = 0.000000; ui27->fFluxCapData[40] = 0.000000; ui27->fFluxCapData[41] = 0.000000; ui27->fFluxCapData[42] = 0.000000; ui27->fFluxCapData[43] = 0.000000; ui27->fFluxCapData[44] = 0.000000; ui27->fFluxCapData[45] = 0.000000; ui27->fFluxCapData[46] = 0.000000; ui27->fFluxCapData[47] = 0.000000; ui27->fFluxCapData[48] = 0.000000; ui27->fFluxCapData[49] = 0.000000; ui27->fFluxCapData[50] = 0.000000; ui27->fFluxCapData[51] = 0.000000; ui27->fFluxCapData[52] = 0.000000; ui27->fFluxCapData[53] = 0.000000; ui27->fFluxCapData[54] = 0.000000; ui27->fFluxCapData[55] = 0.000000; ui27->fFluxCapData[56] = 0.000000; ui27->fFluxCapData[57] = 0.000000; ui27->fFluxCapData[58] = 0.000000; ui27->fFluxCapData[59] = 0.000000; ui27->fFluxCapData[60] = 0.000000; ui27->fFluxCapData[61] = 0.000000; ui27->fFluxCapData[62] = 0.000000; ui27->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui27);
	delete ui27;


	m_fAPF_5_g = 0.600000;
	CUICtrl* ui28 = new CUICtrl;
	ui28->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui28->uControlId = 39;
	ui28->bLogSlider = false;
	ui28->bExpSlider = false;
	ui28->fUserDisplayDataLoLimit = -1.000000;
	ui28->fUserDisplayDataHiLimit = 1.000000;
	ui28->uUserDataType = floatData;
	ui28->fInitUserIntValue = 0;
	ui28->fInitUserFloatValue = 0.600000;
	ui28->fInitUserDoubleValue = 0;
	ui28->fInitUserUINTValue = 0;
	ui28->m_pUserCookedIntData = NULL;
	ui28->m_pUserCookedFloatData = &m_fAPF_5_g;
	ui28->m_pUserCookedDoubleData = NULL;
	ui28->m_pUserCookedUINTData = NULL;
	ui28->cControlUnits = "APF Bank 5";
	ui28->cVariableName = "m_fAPF_5_g";
	ui28->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui28->dPresetData[0] = 0.600000;ui28->dPresetData[1] = -0.500000;ui28->dPresetData[2] = -0.500000;ui28->dPresetData[3] = -0.500000;ui28->dPresetData[4] = 0.000000;ui28->dPresetData[5] = -0.500000;ui28->dPresetData[6] = 0.000000;ui28->dPresetData[7] = 0.000000;ui28->dPresetData[8] = 0.000000;ui28->dPresetData[9] = 0.000000;ui28->dPresetData[10] = 0.000000;ui28->dPresetData[11] = 0.000000;ui28->dPresetData[12] = 0.000000;ui28->dPresetData[13] = 0.000000;ui28->dPresetData[14] = 0.000000;ui28->dPresetData[15] = 0.000000;
	ui28->cControlName = "APF5 g";
	ui28->bOwnerControl = false;
	ui28->bMIDIControl = false;
	ui28->uMIDIControlCommand = 176;
	ui28->uMIDIControlName = 3;
	ui28->uMIDIControlChannel = 0;
	ui28->nGUIRow = nIndexer++;
	ui28->nGUIColumn = -1;
	ui28->bEnableParamSmoothing = false;
	ui28->fSmoothingTimeInMs = 100.00;
	ui28->uControlTheme[0] = 0; ui28->uControlTheme[1] = 0; ui28->uControlTheme[2] = 0; ui28->uControlTheme[3] = 0; ui28->uControlTheme[4] = 0; ui28->uControlTheme[5] = 0; ui28->uControlTheme[6] = 0; ui28->uControlTheme[7] = 0; ui28->uControlTheme[8] = 0; ui28->uControlTheme[9] = 0; ui28->uControlTheme[10] = 0; ui28->uControlTheme[11] = 0; ui28->uControlTheme[12] = 0; ui28->uControlTheme[13] = 0; ui28->uControlTheme[14] = 0; ui28->uControlTheme[15] = 0; ui28->uControlTheme[16] = 2; ui28->uControlTheme[17] = 0; ui28->uControlTheme[18] = 0; ui28->uControlTheme[19] = 0; ui28->uControlTheme[20] = 0; ui28->uControlTheme[21] = 0; ui28->uControlTheme[22] = 0; ui28->uControlTheme[23] = 0; ui28->uControlTheme[24] = 0; ui28->uControlTheme[25] = 0; ui28->uControlTheme[26] = 0; ui28->uControlTheme[27] = 0; ui28->uControlTheme[28] = 0; ui28->uControlTheme[29] = 0; ui28->uControlTheme[30] = 0; ui28->uControlTheme[31] = 0; 
	ui28->uFluxCapControl[0] = 0; ui28->uFluxCapControl[1] = 0; ui28->uFluxCapControl[2] = 0; ui28->uFluxCapControl[3] = 0; ui28->uFluxCapControl[4] = 0; ui28->uFluxCapControl[5] = 0; ui28->uFluxCapControl[6] = 0; ui28->uFluxCapControl[7] = 0; ui28->uFluxCapControl[8] = 0; ui28->uFluxCapControl[9] = 0; ui28->uFluxCapControl[10] = 0; ui28->uFluxCapControl[11] = 0; ui28->uFluxCapControl[12] = 0; ui28->uFluxCapControl[13] = 0; ui28->uFluxCapControl[14] = 0; ui28->uFluxCapControl[15] = 0; ui28->uFluxCapControl[16] = 0; ui28->uFluxCapControl[17] = 0; ui28->uFluxCapControl[18] = 0; ui28->uFluxCapControl[19] = 0; ui28->uFluxCapControl[20] = 0; ui28->uFluxCapControl[21] = 0; ui28->uFluxCapControl[22] = 0; ui28->uFluxCapControl[23] = 0; ui28->uFluxCapControl[24] = 0; ui28->uFluxCapControl[25] = 0; ui28->uFluxCapControl[26] = 0; ui28->uFluxCapControl[27] = 0; ui28->uFluxCapControl[28] = 0; ui28->uFluxCapControl[29] = 0; ui28->uFluxCapControl[30] = 0; ui28->uFluxCapControl[31] = 0; ui28->uFluxCapControl[32] = 0; ui28->uFluxCapControl[33] = 0; ui28->uFluxCapControl[34] = 0; ui28->uFluxCapControl[35] = 0; ui28->uFluxCapControl[36] = 0; ui28->uFluxCapControl[37] = 0; ui28->uFluxCapControl[38] = 0; ui28->uFluxCapControl[39] = 0; ui28->uFluxCapControl[40] = 0; ui28->uFluxCapControl[41] = 0; ui28->uFluxCapControl[42] = 0; ui28->uFluxCapControl[43] = 0; ui28->uFluxCapControl[44] = 0; ui28->uFluxCapControl[45] = 0; ui28->uFluxCapControl[46] = 0; ui28->uFluxCapControl[47] = 0; ui28->uFluxCapControl[48] = 0; ui28->uFluxCapControl[49] = 0; ui28->uFluxCapControl[50] = 0; ui28->uFluxCapControl[51] = 0; ui28->uFluxCapControl[52] = 0; ui28->uFluxCapControl[53] = 0; ui28->uFluxCapControl[54] = 0; ui28->uFluxCapControl[55] = 0; ui28->uFluxCapControl[56] = 0; ui28->uFluxCapControl[57] = 0; ui28->uFluxCapControl[58] = 0; ui28->uFluxCapControl[59] = 0; ui28->uFluxCapControl[60] = 0; ui28->uFluxCapControl[61] = 0; ui28->uFluxCapControl[62] = 0; ui28->uFluxCapControl[63] = 0; 
	ui28->fFluxCapData[0] = 0.000000; ui28->fFluxCapData[1] = 0.000000; ui28->fFluxCapData[2] = 0.000000; ui28->fFluxCapData[3] = 0.000000; ui28->fFluxCapData[4] = 0.000000; ui28->fFluxCapData[5] = 0.000000; ui28->fFluxCapData[6] = 0.000000; ui28->fFluxCapData[7] = 0.000000; ui28->fFluxCapData[8] = 0.000000; ui28->fFluxCapData[9] = 0.000000; ui28->fFluxCapData[10] = 0.000000; ui28->fFluxCapData[11] = 0.000000; ui28->fFluxCapData[12] = 0.000000; ui28->fFluxCapData[13] = 0.000000; ui28->fFluxCapData[14] = 0.000000; ui28->fFluxCapData[15] = 0.000000; ui28->fFluxCapData[16] = 0.000000; ui28->fFluxCapData[17] = 0.000000; ui28->fFluxCapData[18] = 0.000000; ui28->fFluxCapData[19] = 0.000000; ui28->fFluxCapData[20] = 0.000000; ui28->fFluxCapData[21] = 0.000000; ui28->fFluxCapData[22] = 0.000000; ui28->fFluxCapData[23] = 0.000000; ui28->fFluxCapData[24] = 0.000000; ui28->fFluxCapData[25] = 0.000000; ui28->fFluxCapData[26] = 0.000000; ui28->fFluxCapData[27] = 0.000000; ui28->fFluxCapData[28] = 0.000000; ui28->fFluxCapData[29] = 0.000000; ui28->fFluxCapData[30] = 0.000000; ui28->fFluxCapData[31] = 0.000000; ui28->fFluxCapData[32] = 0.000000; ui28->fFluxCapData[33] = 0.000000; ui28->fFluxCapData[34] = 0.000000; ui28->fFluxCapData[35] = 0.000000; ui28->fFluxCapData[36] = 0.000000; ui28->fFluxCapData[37] = 0.000000; ui28->fFluxCapData[38] = 0.000000; ui28->fFluxCapData[39] = 0.000000; ui28->fFluxCapData[40] = 0.000000; ui28->fFluxCapData[41] = 0.000000; ui28->fFluxCapData[42] = 0.000000; ui28->fFluxCapData[43] = 0.000000; ui28->fFluxCapData[44] = 0.000000; ui28->fFluxCapData[45] = 0.000000; ui28->fFluxCapData[46] = 0.000000; ui28->fFluxCapData[47] = 0.000000; ui28->fFluxCapData[48] = 0.000000; ui28->fFluxCapData[49] = 0.000000; ui28->fFluxCapData[50] = 0.000000; ui28->fFluxCapData[51] = 0.000000; ui28->fFluxCapData[52] = 0.000000; ui28->fFluxCapData[53] = 0.000000; ui28->fFluxCapData[54] = 0.000000; ui28->fFluxCapData[55] = 0.000000; ui28->fFluxCapData[56] = 0.000000; ui28->fFluxCapData[57] = 0.000000; ui28->fFluxCapData[58] = 0.000000; ui28->fFluxCapData[59] = 0.000000; ui28->fFluxCapData[60] = 0.000000; ui28->fFluxCapData[61] = 0.000000; ui28->fFluxCapData[62] = 0.000000; ui28->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui28);
	delete ui28;


	m_fMTDelay = 0.000000;
	CUICtrl* ui29 = new CUICtrl;
	ui29->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui29->uControlId = 102;
	ui29->bLogSlider = false;
	ui29->bExpSlider = false;
	ui29->fUserDisplayDataLoLimit = 0.000000;
	ui29->fUserDisplayDataHiLimit = 2000.000000;
	ui29->uUserDataType = floatData;
	ui29->fInitUserIntValue = 0;
	ui29->fInitUserFloatValue = 0.000000;
	ui29->fInitUserDoubleValue = 0;
	ui29->fInitUserUINTValue = 0;
	ui29->m_pUserCookedIntData = NULL;
	ui29->m_pUserCookedFloatData = &m_fMTDelay;
	ui29->m_pUserCookedDoubleData = NULL;
	ui29->m_pUserCookedUINTData = NULL;
	ui29->cControlUnits = "m_Sec";
	ui29->cVariableName = "m_fMTDelay";
	ui29->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui29->dPresetData[0] = 0.000000;ui29->dPresetData[1] = 909.999939;ui29->dPresetData[2] = 0.000000;ui29->dPresetData[3] = 0.000000;ui29->dPresetData[4] = 0.000000;ui29->dPresetData[5] = 0.000000;ui29->dPresetData[6] = 0.000000;ui29->dPresetData[7] = 0.000000;ui29->dPresetData[8] = 0.000000;ui29->dPresetData[9] = 0.000000;ui29->dPresetData[10] = 0.000000;ui29->dPresetData[11] = 0.000000;ui29->dPresetData[12] = 0.000000;ui29->dPresetData[13] = 0.000000;ui29->dPresetData[14] = 0.000000;ui29->dPresetData[15] = 0.000000;
	ui29->cControlName = "Multi-Tap Delay";
	ui29->bOwnerControl = false;
	ui29->bMIDIControl = false;
	ui29->uMIDIControlCommand = 176;
	ui29->uMIDIControlName = 3;
	ui29->uMIDIControlChannel = 0;
	ui29->nGUIRow = nIndexer++;
	ui29->nGUIColumn = -1;
	ui29->bEnableParamSmoothing = false;
	ui29->fSmoothingTimeInMs = 100.00;
	ui29->uControlTheme[0] = 0; ui29->uControlTheme[1] = 0; ui29->uControlTheme[2] = 0; ui29->uControlTheme[3] = 0; ui29->uControlTheme[4] = 0; ui29->uControlTheme[5] = 0; ui29->uControlTheme[6] = 0; ui29->uControlTheme[7] = 0; ui29->uControlTheme[8] = 0; ui29->uControlTheme[9] = 0; ui29->uControlTheme[10] = 0; ui29->uControlTheme[11] = 0; ui29->uControlTheme[12] = 0; ui29->uControlTheme[13] = 0; ui29->uControlTheme[14] = 0; ui29->uControlTheme[15] = 0; ui29->uControlTheme[16] = 2; ui29->uControlTheme[17] = 0; ui29->uControlTheme[18] = 0; ui29->uControlTheme[19] = 0; ui29->uControlTheme[20] = 0; ui29->uControlTheme[21] = 0; ui29->uControlTheme[22] = 0; ui29->uControlTheme[23] = 0; ui29->uControlTheme[24] = 0; ui29->uControlTheme[25] = 0; ui29->uControlTheme[26] = 0; ui29->uControlTheme[27] = 0; ui29->uControlTheme[28] = 0; ui29->uControlTheme[29] = 0; ui29->uControlTheme[30] = 0; ui29->uControlTheme[31] = 0; 
	ui29->uFluxCapControl[0] = 0; ui29->uFluxCapControl[1] = 0; ui29->uFluxCapControl[2] = 0; ui29->uFluxCapControl[3] = 0; ui29->uFluxCapControl[4] = 0; ui29->uFluxCapControl[5] = 0; ui29->uFluxCapControl[6] = 0; ui29->uFluxCapControl[7] = 0; ui29->uFluxCapControl[8] = 0; ui29->uFluxCapControl[9] = 0; ui29->uFluxCapControl[10] = 0; ui29->uFluxCapControl[11] = 0; ui29->uFluxCapControl[12] = 0; ui29->uFluxCapControl[13] = 0; ui29->uFluxCapControl[14] = 0; ui29->uFluxCapControl[15] = 0; ui29->uFluxCapControl[16] = 0; ui29->uFluxCapControl[17] = 0; ui29->uFluxCapControl[18] = 0; ui29->uFluxCapControl[19] = 0; ui29->uFluxCapControl[20] = 0; ui29->uFluxCapControl[21] = 0; ui29->uFluxCapControl[22] = 0; ui29->uFluxCapControl[23] = 0; ui29->uFluxCapControl[24] = 0; ui29->uFluxCapControl[25] = 0; ui29->uFluxCapControl[26] = 0; ui29->uFluxCapControl[27] = 0; ui29->uFluxCapControl[28] = 0; ui29->uFluxCapControl[29] = 0; ui29->uFluxCapControl[30] = 0; ui29->uFluxCapControl[31] = 0; ui29->uFluxCapControl[32] = 0; ui29->uFluxCapControl[33] = 0; ui29->uFluxCapControl[34] = 0; ui29->uFluxCapControl[35] = 0; ui29->uFluxCapControl[36] = 0; ui29->uFluxCapControl[37] = 0; ui29->uFluxCapControl[38] = 0; ui29->uFluxCapControl[39] = 0; ui29->uFluxCapControl[40] = 0; ui29->uFluxCapControl[41] = 0; ui29->uFluxCapControl[42] = 0; ui29->uFluxCapControl[43] = 0; ui29->uFluxCapControl[44] = 0; ui29->uFluxCapControl[45] = 0; ui29->uFluxCapControl[46] = 0; ui29->uFluxCapControl[47] = 0; ui29->uFluxCapControl[48] = 0; ui29->uFluxCapControl[49] = 0; ui29->uFluxCapControl[50] = 0; ui29->uFluxCapControl[51] = 0; ui29->uFluxCapControl[52] = 0; ui29->uFluxCapControl[53] = 0; ui29->uFluxCapControl[54] = 0; ui29->uFluxCapControl[55] = 0; ui29->uFluxCapControl[56] = 0; ui29->uFluxCapControl[57] = 0; ui29->uFluxCapControl[58] = 0; ui29->uFluxCapControl[59] = 0; ui29->uFluxCapControl[60] = 0; ui29->uFluxCapControl[61] = 0; ui29->uFluxCapControl[62] = 0; ui29->uFluxCapControl[63] = 0; 
	ui29->fFluxCapData[0] = 0.000000; ui29->fFluxCapData[1] = 0.000000; ui29->fFluxCapData[2] = 0.000000; ui29->fFluxCapData[3] = 0.000000; ui29->fFluxCapData[4] = 0.000000; ui29->fFluxCapData[5] = 0.000000; ui29->fFluxCapData[6] = 0.000000; ui29->fFluxCapData[7] = 0.000000; ui29->fFluxCapData[8] = 0.000000; ui29->fFluxCapData[9] = 0.000000; ui29->fFluxCapData[10] = 0.000000; ui29->fFluxCapData[11] = 0.000000; ui29->fFluxCapData[12] = 0.000000; ui29->fFluxCapData[13] = 0.000000; ui29->fFluxCapData[14] = 0.000000; ui29->fFluxCapData[15] = 0.000000; ui29->fFluxCapData[16] = 0.000000; ui29->fFluxCapData[17] = 0.000000; ui29->fFluxCapData[18] = 0.000000; ui29->fFluxCapData[19] = 0.000000; ui29->fFluxCapData[20] = 0.000000; ui29->fFluxCapData[21] = 0.000000; ui29->fFluxCapData[22] = 0.000000; ui29->fFluxCapData[23] = 0.000000; ui29->fFluxCapData[24] = 0.000000; ui29->fFluxCapData[25] = 0.000000; ui29->fFluxCapData[26] = 0.000000; ui29->fFluxCapData[27] = 0.000000; ui29->fFluxCapData[28] = 0.000000; ui29->fFluxCapData[29] = 0.000000; ui29->fFluxCapData[30] = 0.000000; ui29->fFluxCapData[31] = 0.000000; ui29->fFluxCapData[32] = 0.000000; ui29->fFluxCapData[33] = 0.000000; ui29->fFluxCapData[34] = 0.000000; ui29->fFluxCapData[35] = 0.000000; ui29->fFluxCapData[36] = 0.000000; ui29->fFluxCapData[37] = 0.000000; ui29->fFluxCapData[38] = 0.000000; ui29->fFluxCapData[39] = 0.000000; ui29->fFluxCapData[40] = 0.000000; ui29->fFluxCapData[41] = 0.000000; ui29->fFluxCapData[42] = 0.000000; ui29->fFluxCapData[43] = 0.000000; ui29->fFluxCapData[44] = 0.000000; ui29->fFluxCapData[45] = 0.000000; ui29->fFluxCapData[46] = 0.000000; ui29->fFluxCapData[47] = 0.000000; ui29->fFluxCapData[48] = 0.000000; ui29->fFluxCapData[49] = 0.000000; ui29->fFluxCapData[50] = 0.000000; ui29->fFluxCapData[51] = 0.000000; ui29->fFluxCapData[52] = 0.000000; ui29->fFluxCapData[53] = 0.000000; ui29->fFluxCapData[54] = 0.000000; ui29->fFluxCapData[55] = 0.000000; ui29->fFluxCapData[56] = 0.000000; ui29->fFluxCapData[57] = 0.000000; ui29->fFluxCapData[58] = 0.000000; ui29->fFluxCapData[59] = 0.000000; ui29->fFluxCapData[60] = 0.000000; ui29->fFluxCapData[61] = 0.000000; ui29->fFluxCapData[62] = 0.000000; ui29->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui29);
	delete ui29;


	m_fAPF_6_Delay_mSec = 11.000000;
	CUICtrl* ui30 = new CUICtrl;
	ui30->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui30->uControlId = 108;
	ui30->bLogSlider = false;
	ui30->bExpSlider = false;
	ui30->fUserDisplayDataLoLimit = 0.000000;
	ui30->fUserDisplayDataHiLimit = 100.000000;
	ui30->uUserDataType = floatData;
	ui30->fInitUserIntValue = 0;
	ui30->fInitUserFloatValue = 11.000000;
	ui30->fInitUserDoubleValue = 0;
	ui30->fInitUserUINTValue = 0;
	ui30->m_pUserCookedIntData = NULL;
	ui30->m_pUserCookedFloatData = &m_fAPF_6_Delay_mSec;
	ui30->m_pUserCookedDoubleData = NULL;
	ui30->m_pUserCookedUINTData = NULL;
	ui30->cControlUnits = "mSec";
	ui30->cVariableName = "m_fAPF_6_Delay_mSec";
	ui30->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui30->dPresetData[0] = 11.000000;ui30->dPresetData[1] = 59.499996;ui30->dPresetData[2] = 59.499996;ui30->dPresetData[3] = 30.499994;ui30->dPresetData[4] = 0.000000;ui30->dPresetData[5] = 59.499996;ui30->dPresetData[6] = 0.000000;ui30->dPresetData[7] = 0.000000;ui30->dPresetData[8] = 0.000000;ui30->dPresetData[9] = 0.000000;ui30->dPresetData[10] = 0.000000;ui30->dPresetData[11] = 0.000000;ui30->dPresetData[12] = 0.000000;ui30->dPresetData[13] = 0.000000;ui30->dPresetData[14] = 0.000000;ui30->dPresetData[15] = 0.000000;
	ui30->cControlName = "APF6 Dly";
	ui30->bOwnerControl = false;
	ui30->bMIDIControl = false;
	ui30->uMIDIControlCommand = 176;
	ui30->uMIDIControlName = 3;
	ui30->uMIDIControlChannel = 0;
	ui30->nGUIRow = nIndexer++;
	ui30->nGUIColumn = -1;
	ui30->bEnableParamSmoothing = false;
	ui30->fSmoothingTimeInMs = 100.00;
	ui30->uControlTheme[0] = 0; ui30->uControlTheme[1] = 0; ui30->uControlTheme[2] = 0; ui30->uControlTheme[3] = 0; ui30->uControlTheme[4] = 0; ui30->uControlTheme[5] = 0; ui30->uControlTheme[6] = 0; ui30->uControlTheme[7] = 0; ui30->uControlTheme[8] = 0; ui30->uControlTheme[9] = 0; ui30->uControlTheme[10] = 0; ui30->uControlTheme[11] = 0; ui30->uControlTheme[12] = 0; ui30->uControlTheme[13] = 0; ui30->uControlTheme[14] = 0; ui30->uControlTheme[15] = 0; ui30->uControlTheme[16] = 2; ui30->uControlTheme[17] = 0; ui30->uControlTheme[18] = 0; ui30->uControlTheme[19] = 0; ui30->uControlTheme[20] = 0; ui30->uControlTheme[21] = 0; ui30->uControlTheme[22] = 0; ui30->uControlTheme[23] = 0; ui30->uControlTheme[24] = 0; ui30->uControlTheme[25] = 0; ui30->uControlTheme[26] = 0; ui30->uControlTheme[27] = 0; ui30->uControlTheme[28] = 0; ui30->uControlTheme[29] = 0; ui30->uControlTheme[30] = 0; ui30->uControlTheme[31] = 0; 
	ui30->uFluxCapControl[0] = 0; ui30->uFluxCapControl[1] = 0; ui30->uFluxCapControl[2] = 0; ui30->uFluxCapControl[3] = 0; ui30->uFluxCapControl[4] = 0; ui30->uFluxCapControl[5] = 0; ui30->uFluxCapControl[6] = 0; ui30->uFluxCapControl[7] = 0; ui30->uFluxCapControl[8] = 0; ui30->uFluxCapControl[9] = 0; ui30->uFluxCapControl[10] = 0; ui30->uFluxCapControl[11] = 0; ui30->uFluxCapControl[12] = 0; ui30->uFluxCapControl[13] = 0; ui30->uFluxCapControl[14] = 0; ui30->uFluxCapControl[15] = 0; ui30->uFluxCapControl[16] = 0; ui30->uFluxCapControl[17] = 0; ui30->uFluxCapControl[18] = 0; ui30->uFluxCapControl[19] = 0; ui30->uFluxCapControl[20] = 0; ui30->uFluxCapControl[21] = 0; ui30->uFluxCapControl[22] = 0; ui30->uFluxCapControl[23] = 0; ui30->uFluxCapControl[24] = 0; ui30->uFluxCapControl[25] = 0; ui30->uFluxCapControl[26] = 0; ui30->uFluxCapControl[27] = 0; ui30->uFluxCapControl[28] = 0; ui30->uFluxCapControl[29] = 0; ui30->uFluxCapControl[30] = 0; ui30->uFluxCapControl[31] = 0; ui30->uFluxCapControl[32] = 0; ui30->uFluxCapControl[33] = 0; ui30->uFluxCapControl[34] = 0; ui30->uFluxCapControl[35] = 0; ui30->uFluxCapControl[36] = 0; ui30->uFluxCapControl[37] = 0; ui30->uFluxCapControl[38] = 0; ui30->uFluxCapControl[39] = 0; ui30->uFluxCapControl[40] = 0; ui30->uFluxCapControl[41] = 0; ui30->uFluxCapControl[42] = 0; ui30->uFluxCapControl[43] = 0; ui30->uFluxCapControl[44] = 0; ui30->uFluxCapControl[45] = 0; ui30->uFluxCapControl[46] = 0; ui30->uFluxCapControl[47] = 0; ui30->uFluxCapControl[48] = 0; ui30->uFluxCapControl[49] = 0; ui30->uFluxCapControl[50] = 0; ui30->uFluxCapControl[51] = 0; ui30->uFluxCapControl[52] = 0; ui30->uFluxCapControl[53] = 0; ui30->uFluxCapControl[54] = 0; ui30->uFluxCapControl[55] = 0; ui30->uFluxCapControl[56] = 0; ui30->uFluxCapControl[57] = 0; ui30->uFluxCapControl[58] = 0; ui30->uFluxCapControl[59] = 0; ui30->uFluxCapControl[60] = 0; ui30->uFluxCapControl[61] = 0; ui30->uFluxCapControl[62] = 0; ui30->uFluxCapControl[63] = 0; 
	ui30->fFluxCapData[0] = 0.000000; ui30->fFluxCapData[1] = 0.000000; ui30->fFluxCapData[2] = 0.000000; ui30->fFluxCapData[3] = 0.000000; ui30->fFluxCapData[4] = 0.000000; ui30->fFluxCapData[5] = 0.000000; ui30->fFluxCapData[6] = 0.000000; ui30->fFluxCapData[7] = 0.000000; ui30->fFluxCapData[8] = 0.000000; ui30->fFluxCapData[9] = 0.000000; ui30->fFluxCapData[10] = 0.000000; ui30->fFluxCapData[11] = 0.000000; ui30->fFluxCapData[12] = 0.000000; ui30->fFluxCapData[13] = 0.000000; ui30->fFluxCapData[14] = 0.000000; ui30->fFluxCapData[15] = 0.000000; ui30->fFluxCapData[16] = 0.000000; ui30->fFluxCapData[17] = 0.000000; ui30->fFluxCapData[18] = 0.000000; ui30->fFluxCapData[19] = 0.000000; ui30->fFluxCapData[20] = 0.000000; ui30->fFluxCapData[21] = 0.000000; ui30->fFluxCapData[22] = 0.000000; ui30->fFluxCapData[23] = 0.000000; ui30->fFluxCapData[24] = 0.000000; ui30->fFluxCapData[25] = 0.000000; ui30->fFluxCapData[26] = 0.000000; ui30->fFluxCapData[27] = 0.000000; ui30->fFluxCapData[28] = 0.000000; ui30->fFluxCapData[29] = 0.000000; ui30->fFluxCapData[30] = 0.000000; ui30->fFluxCapData[31] = 0.000000; ui30->fFluxCapData[32] = 0.000000; ui30->fFluxCapData[33] = 0.000000; ui30->fFluxCapData[34] = 0.000000; ui30->fFluxCapData[35] = 0.000000; ui30->fFluxCapData[36] = 0.000000; ui30->fFluxCapData[37] = 0.000000; ui30->fFluxCapData[38] = 0.000000; ui30->fFluxCapData[39] = 0.000000; ui30->fFluxCapData[40] = 0.000000; ui30->fFluxCapData[41] = 0.000000; ui30->fFluxCapData[42] = 0.000000; ui30->fFluxCapData[43] = 0.000000; ui30->fFluxCapData[44] = 0.000000; ui30->fFluxCapData[45] = 0.000000; ui30->fFluxCapData[46] = 0.000000; ui30->fFluxCapData[47] = 0.000000; ui30->fFluxCapData[48] = 0.000000; ui30->fFluxCapData[49] = 0.000000; ui30->fFluxCapData[50] = 0.000000; ui30->fFluxCapData[51] = 0.000000; ui30->fFluxCapData[52] = 0.000000; ui30->fFluxCapData[53] = 0.000000; ui30->fFluxCapData[54] = 0.000000; ui30->fFluxCapData[55] = 0.000000; ui30->fFluxCapData[56] = 0.000000; ui30->fFluxCapData[57] = 0.000000; ui30->fFluxCapData[58] = 0.000000; ui30->fFluxCapData[59] = 0.000000; ui30->fFluxCapData[60] = 0.000000; ui30->fFluxCapData[61] = 0.000000; ui30->fFluxCapData[62] = 0.000000; ui30->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui30);
	delete ui30;


	m_fAPF_6_g = 0.600000;
	CUICtrl* ui31 = new CUICtrl;
	ui31->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui31->uControlId = 109;
	ui31->bLogSlider = false;
	ui31->bExpSlider = false;
	ui31->fUserDisplayDataLoLimit = -1.000000;
	ui31->fUserDisplayDataHiLimit = 1.000000;
	ui31->uUserDataType = floatData;
	ui31->fInitUserIntValue = 0;
	ui31->fInitUserFloatValue = 0.600000;
	ui31->fInitUserDoubleValue = 0;
	ui31->fInitUserUINTValue = 0;
	ui31->m_pUserCookedIntData = NULL;
	ui31->m_pUserCookedFloatData = &m_fAPF_6_g;
	ui31->m_pUserCookedDoubleData = NULL;
	ui31->m_pUserCookedUINTData = NULL;
	ui31->cControlUnits = "APF Bank 6";
	ui31->cVariableName = "m_fAPF_6_g";
	ui31->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui31->dPresetData[0] = 0.600000;ui31->dPresetData[1] = 0.500000;ui31->dPresetData[2] = 0.500000;ui31->dPresetData[3] = 0.500000;ui31->dPresetData[4] = 0.000000;ui31->dPresetData[5] = 0.500000;ui31->dPresetData[6] = 0.000000;ui31->dPresetData[7] = 0.000000;ui31->dPresetData[8] = 0.000000;ui31->dPresetData[9] = 0.000000;ui31->dPresetData[10] = 0.000000;ui31->dPresetData[11] = 0.000000;ui31->dPresetData[12] = 0.000000;ui31->dPresetData[13] = 0.000000;ui31->dPresetData[14] = 0.000000;ui31->dPresetData[15] = 0.000000;
	ui31->cControlName = "APF6 g";
	ui31->bOwnerControl = false;
	ui31->bMIDIControl = false;
	ui31->uMIDIControlCommand = 176;
	ui31->uMIDIControlName = 3;
	ui31->uMIDIControlChannel = 0;
	ui31->nGUIRow = nIndexer++;
	ui31->nGUIColumn = -1;
	ui31->bEnableParamSmoothing = false;
	ui31->fSmoothingTimeInMs = 100.00;
	ui31->uControlTheme[0] = 0; ui31->uControlTheme[1] = 0; ui31->uControlTheme[2] = 0; ui31->uControlTheme[3] = 0; ui31->uControlTheme[4] = 0; ui31->uControlTheme[5] = 0; ui31->uControlTheme[6] = 0; ui31->uControlTheme[7] = 0; ui31->uControlTheme[8] = 0; ui31->uControlTheme[9] = 0; ui31->uControlTheme[10] = 0; ui31->uControlTheme[11] = 0; ui31->uControlTheme[12] = 0; ui31->uControlTheme[13] = 0; ui31->uControlTheme[14] = 0; ui31->uControlTheme[15] = 0; ui31->uControlTheme[16] = 2; ui31->uControlTheme[17] = 0; ui31->uControlTheme[18] = 0; ui31->uControlTheme[19] = 0; ui31->uControlTheme[20] = 0; ui31->uControlTheme[21] = 0; ui31->uControlTheme[22] = 0; ui31->uControlTheme[23] = 0; ui31->uControlTheme[24] = 0; ui31->uControlTheme[25] = 0; ui31->uControlTheme[26] = 0; ui31->uControlTheme[27] = 0; ui31->uControlTheme[28] = 0; ui31->uControlTheme[29] = 0; ui31->uControlTheme[30] = 0; ui31->uControlTheme[31] = 0; 
	ui31->uFluxCapControl[0] = 0; ui31->uFluxCapControl[1] = 0; ui31->uFluxCapControl[2] = 0; ui31->uFluxCapControl[3] = 0; ui31->uFluxCapControl[4] = 0; ui31->uFluxCapControl[5] = 0; ui31->uFluxCapControl[6] = 0; ui31->uFluxCapControl[7] = 0; ui31->uFluxCapControl[8] = 0; ui31->uFluxCapControl[9] = 0; ui31->uFluxCapControl[10] = 0; ui31->uFluxCapControl[11] = 0; ui31->uFluxCapControl[12] = 0; ui31->uFluxCapControl[13] = 0; ui31->uFluxCapControl[14] = 0; ui31->uFluxCapControl[15] = 0; ui31->uFluxCapControl[16] = 0; ui31->uFluxCapControl[17] = 0; ui31->uFluxCapControl[18] = 0; ui31->uFluxCapControl[19] = 0; ui31->uFluxCapControl[20] = 0; ui31->uFluxCapControl[21] = 0; ui31->uFluxCapControl[22] = 0; ui31->uFluxCapControl[23] = 0; ui31->uFluxCapControl[24] = 0; ui31->uFluxCapControl[25] = 0; ui31->uFluxCapControl[26] = 0; ui31->uFluxCapControl[27] = 0; ui31->uFluxCapControl[28] = 0; ui31->uFluxCapControl[29] = 0; ui31->uFluxCapControl[30] = 0; ui31->uFluxCapControl[31] = 0; ui31->uFluxCapControl[32] = 0; ui31->uFluxCapControl[33] = 0; ui31->uFluxCapControl[34] = 0; ui31->uFluxCapControl[35] = 0; ui31->uFluxCapControl[36] = 0; ui31->uFluxCapControl[37] = 0; ui31->uFluxCapControl[38] = 0; ui31->uFluxCapControl[39] = 0; ui31->uFluxCapControl[40] = 0; ui31->uFluxCapControl[41] = 0; ui31->uFluxCapControl[42] = 0; ui31->uFluxCapControl[43] = 0; ui31->uFluxCapControl[44] = 0; ui31->uFluxCapControl[45] = 0; ui31->uFluxCapControl[46] = 0; ui31->uFluxCapControl[47] = 0; ui31->uFluxCapControl[48] = 0; ui31->uFluxCapControl[49] = 0; ui31->uFluxCapControl[50] = 0; ui31->uFluxCapControl[51] = 0; ui31->uFluxCapControl[52] = 0; ui31->uFluxCapControl[53] = 0; ui31->uFluxCapControl[54] = 0; ui31->uFluxCapControl[55] = 0; ui31->uFluxCapControl[56] = 0; ui31->uFluxCapControl[57] = 0; ui31->uFluxCapControl[58] = 0; ui31->uFluxCapControl[59] = 0; ui31->uFluxCapControl[60] = 0; ui31->uFluxCapControl[61] = 0; ui31->uFluxCapControl[62] = 0; ui31->uFluxCapControl[63] = 0; 
	ui31->fFluxCapData[0] = 0.000000; ui31->fFluxCapData[1] = 0.000000; ui31->fFluxCapData[2] = 0.000000; ui31->fFluxCapData[3] = 0.000000; ui31->fFluxCapData[4] = 0.000000; ui31->fFluxCapData[5] = 0.000000; ui31->fFluxCapData[6] = 0.000000; ui31->fFluxCapData[7] = 0.000000; ui31->fFluxCapData[8] = 0.000000; ui31->fFluxCapData[9] = 0.000000; ui31->fFluxCapData[10] = 0.000000; ui31->fFluxCapData[11] = 0.000000; ui31->fFluxCapData[12] = 0.000000; ui31->fFluxCapData[13] = 0.000000; ui31->fFluxCapData[14] = 0.000000; ui31->fFluxCapData[15] = 0.000000; ui31->fFluxCapData[16] = 0.000000; ui31->fFluxCapData[17] = 0.000000; ui31->fFluxCapData[18] = 0.000000; ui31->fFluxCapData[19] = 0.000000; ui31->fFluxCapData[20] = 0.000000; ui31->fFluxCapData[21] = 0.000000; ui31->fFluxCapData[22] = 0.000000; ui31->fFluxCapData[23] = 0.000000; ui31->fFluxCapData[24] = 0.000000; ui31->fFluxCapData[25] = 0.000000; ui31->fFluxCapData[26] = 0.000000; ui31->fFluxCapData[27] = 0.000000; ui31->fFluxCapData[28] = 0.000000; ui31->fFluxCapData[29] = 0.000000; ui31->fFluxCapData[30] = 0.000000; ui31->fFluxCapData[31] = 0.000000; ui31->fFluxCapData[32] = 0.000000; ui31->fFluxCapData[33] = 0.000000; ui31->fFluxCapData[34] = 0.000000; ui31->fFluxCapData[35] = 0.000000; ui31->fFluxCapData[36] = 0.000000; ui31->fFluxCapData[37] = 0.000000; ui31->fFluxCapData[38] = 0.000000; ui31->fFluxCapData[39] = 0.000000; ui31->fFluxCapData[40] = 0.000000; ui31->fFluxCapData[41] = 0.000000; ui31->fFluxCapData[42] = 0.000000; ui31->fFluxCapData[43] = 0.000000; ui31->fFluxCapData[44] = 0.000000; ui31->fFluxCapData[45] = 0.000000; ui31->fFluxCapData[46] = 0.000000; ui31->fFluxCapData[47] = 0.000000; ui31->fFluxCapData[48] = 0.000000; ui31->fFluxCapData[49] = 0.000000; ui31->fFluxCapData[50] = 0.000000; ui31->fFluxCapData[51] = 0.000000; ui31->fFluxCapData[52] = 0.000000; ui31->fFluxCapData[53] = 0.000000; ui31->fFluxCapData[54] = 0.000000; ui31->fFluxCapData[55] = 0.000000; ui31->fFluxCapData[56] = 0.000000; ui31->fFluxCapData[57] = 0.000000; ui31->fFluxCapData[58] = 0.000000; ui31->fFluxCapData[59] = 0.000000; ui31->fFluxCapData[60] = 0.000000; ui31->fFluxCapData[61] = 0.000000; ui31->fFluxCapData[62] = 0.000000; ui31->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui31);
	delete ui31;


	m_fAPF_7_Delay_mSec = 11.000000;
	CUICtrl* ui32 = new CUICtrl;
	ui32->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui32->uControlId = 118;
	ui32->bLogSlider = false;
	ui32->bExpSlider = false;
	ui32->fUserDisplayDataLoLimit = 0.000000;
	ui32->fUserDisplayDataHiLimit = 100.000000;
	ui32->uUserDataType = floatData;
	ui32->fInitUserIntValue = 0;
	ui32->fInitUserFloatValue = 11.000000;
	ui32->fInitUserDoubleValue = 0;
	ui32->fInitUserUINTValue = 0;
	ui32->m_pUserCookedIntData = NULL;
	ui32->m_pUserCookedFloatData = &m_fAPF_7_Delay_mSec;
	ui32->m_pUserCookedDoubleData = NULL;
	ui32->m_pUserCookedUINTData = NULL;
	ui32->cControlUnits = "mSec";
	ui32->cVariableName = "m_fAPF_7_Delay_mSec";
	ui32->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui32->dPresetData[0] = 11.000000;ui32->dPresetData[1] = 67.500000;ui32->dPresetData[2] = 67.500000;ui32->dPresetData[3] = 35.000004;ui32->dPresetData[4] = 0.000000;ui32->dPresetData[5] = 67.500000;ui32->dPresetData[6] = 0.000000;ui32->dPresetData[7] = 0.000000;ui32->dPresetData[8] = 0.000000;ui32->dPresetData[9] = 0.000000;ui32->dPresetData[10] = 0.000000;ui32->dPresetData[11] = 0.000000;ui32->dPresetData[12] = 0.000000;ui32->dPresetData[13] = 0.000000;ui32->dPresetData[14] = 0.000000;ui32->dPresetData[15] = 0.000000;
	ui32->cControlName = "APF7 Dly";
	ui32->bOwnerControl = false;
	ui32->bMIDIControl = false;
	ui32->uMIDIControlCommand = 176;
	ui32->uMIDIControlName = 3;
	ui32->uMIDIControlChannel = 0;
	ui32->nGUIRow = nIndexer++;
	ui32->nGUIColumn = -1;
	ui32->bEnableParamSmoothing = false;
	ui32->fSmoothingTimeInMs = 100.00;
	ui32->uControlTheme[0] = 0; ui32->uControlTheme[1] = 0; ui32->uControlTheme[2] = 0; ui32->uControlTheme[3] = 0; ui32->uControlTheme[4] = 0; ui32->uControlTheme[5] = 0; ui32->uControlTheme[6] = 0; ui32->uControlTheme[7] = 0; ui32->uControlTheme[8] = 0; ui32->uControlTheme[9] = 0; ui32->uControlTheme[10] = 0; ui32->uControlTheme[11] = 0; ui32->uControlTheme[12] = 0; ui32->uControlTheme[13] = 0; ui32->uControlTheme[14] = 0; ui32->uControlTheme[15] = 0; ui32->uControlTheme[16] = 2; ui32->uControlTheme[17] = 0; ui32->uControlTheme[18] = 0; ui32->uControlTheme[19] = 0; ui32->uControlTheme[20] = 0; ui32->uControlTheme[21] = 0; ui32->uControlTheme[22] = 0; ui32->uControlTheme[23] = 0; ui32->uControlTheme[24] = 0; ui32->uControlTheme[25] = 0; ui32->uControlTheme[26] = 0; ui32->uControlTheme[27] = 0; ui32->uControlTheme[28] = 0; ui32->uControlTheme[29] = 0; ui32->uControlTheme[30] = 0; ui32->uControlTheme[31] = 0; 
	ui32->uFluxCapControl[0] = 0; ui32->uFluxCapControl[1] = 0; ui32->uFluxCapControl[2] = 0; ui32->uFluxCapControl[3] = 0; ui32->uFluxCapControl[4] = 0; ui32->uFluxCapControl[5] = 0; ui32->uFluxCapControl[6] = 0; ui32->uFluxCapControl[7] = 0; ui32->uFluxCapControl[8] = 0; ui32->uFluxCapControl[9] = 0; ui32->uFluxCapControl[10] = 0; ui32->uFluxCapControl[11] = 0; ui32->uFluxCapControl[12] = 0; ui32->uFluxCapControl[13] = 0; ui32->uFluxCapControl[14] = 0; ui32->uFluxCapControl[15] = 0; ui32->uFluxCapControl[16] = 0; ui32->uFluxCapControl[17] = 0; ui32->uFluxCapControl[18] = 0; ui32->uFluxCapControl[19] = 0; ui32->uFluxCapControl[20] = 0; ui32->uFluxCapControl[21] = 0; ui32->uFluxCapControl[22] = 0; ui32->uFluxCapControl[23] = 0; ui32->uFluxCapControl[24] = 0; ui32->uFluxCapControl[25] = 0; ui32->uFluxCapControl[26] = 0; ui32->uFluxCapControl[27] = 0; ui32->uFluxCapControl[28] = 0; ui32->uFluxCapControl[29] = 0; ui32->uFluxCapControl[30] = 0; ui32->uFluxCapControl[31] = 0; ui32->uFluxCapControl[32] = 0; ui32->uFluxCapControl[33] = 0; ui32->uFluxCapControl[34] = 0; ui32->uFluxCapControl[35] = 0; ui32->uFluxCapControl[36] = 0; ui32->uFluxCapControl[37] = 0; ui32->uFluxCapControl[38] = 0; ui32->uFluxCapControl[39] = 0; ui32->uFluxCapControl[40] = 0; ui32->uFluxCapControl[41] = 0; ui32->uFluxCapControl[42] = 0; ui32->uFluxCapControl[43] = 0; ui32->uFluxCapControl[44] = 0; ui32->uFluxCapControl[45] = 0; ui32->uFluxCapControl[46] = 0; ui32->uFluxCapControl[47] = 0; ui32->uFluxCapControl[48] = 0; ui32->uFluxCapControl[49] = 0; ui32->uFluxCapControl[50] = 0; ui32->uFluxCapControl[51] = 0; ui32->uFluxCapControl[52] = 0; ui32->uFluxCapControl[53] = 0; ui32->uFluxCapControl[54] = 0; ui32->uFluxCapControl[55] = 0; ui32->uFluxCapControl[56] = 0; ui32->uFluxCapControl[57] = 0; ui32->uFluxCapControl[58] = 0; ui32->uFluxCapControl[59] = 0; ui32->uFluxCapControl[60] = 0; ui32->uFluxCapControl[61] = 0; ui32->uFluxCapControl[62] = 0; ui32->uFluxCapControl[63] = 0; 
	ui32->fFluxCapData[0] = 0.000000; ui32->fFluxCapData[1] = 0.000000; ui32->fFluxCapData[2] = 0.000000; ui32->fFluxCapData[3] = 0.000000; ui32->fFluxCapData[4] = 0.000000; ui32->fFluxCapData[5] = 0.000000; ui32->fFluxCapData[6] = 0.000000; ui32->fFluxCapData[7] = 0.000000; ui32->fFluxCapData[8] = 0.000000; ui32->fFluxCapData[9] = 0.000000; ui32->fFluxCapData[10] = 0.000000; ui32->fFluxCapData[11] = 0.000000; ui32->fFluxCapData[12] = 0.000000; ui32->fFluxCapData[13] = 0.000000; ui32->fFluxCapData[14] = 0.000000; ui32->fFluxCapData[15] = 0.000000; ui32->fFluxCapData[16] = 0.000000; ui32->fFluxCapData[17] = 0.000000; ui32->fFluxCapData[18] = 0.000000; ui32->fFluxCapData[19] = 0.000000; ui32->fFluxCapData[20] = 0.000000; ui32->fFluxCapData[21] = 0.000000; ui32->fFluxCapData[22] = 0.000000; ui32->fFluxCapData[23] = 0.000000; ui32->fFluxCapData[24] = 0.000000; ui32->fFluxCapData[25] = 0.000000; ui32->fFluxCapData[26] = 0.000000; ui32->fFluxCapData[27] = 0.000000; ui32->fFluxCapData[28] = 0.000000; ui32->fFluxCapData[29] = 0.000000; ui32->fFluxCapData[30] = 0.000000; ui32->fFluxCapData[31] = 0.000000; ui32->fFluxCapData[32] = 0.000000; ui32->fFluxCapData[33] = 0.000000; ui32->fFluxCapData[34] = 0.000000; ui32->fFluxCapData[35] = 0.000000; ui32->fFluxCapData[36] = 0.000000; ui32->fFluxCapData[37] = 0.000000; ui32->fFluxCapData[38] = 0.000000; ui32->fFluxCapData[39] = 0.000000; ui32->fFluxCapData[40] = 0.000000; ui32->fFluxCapData[41] = 0.000000; ui32->fFluxCapData[42] = 0.000000; ui32->fFluxCapData[43] = 0.000000; ui32->fFluxCapData[44] = 0.000000; ui32->fFluxCapData[45] = 0.000000; ui32->fFluxCapData[46] = 0.000000; ui32->fFluxCapData[47] = 0.000000; ui32->fFluxCapData[48] = 0.000000; ui32->fFluxCapData[49] = 0.000000; ui32->fFluxCapData[50] = 0.000000; ui32->fFluxCapData[51] = 0.000000; ui32->fFluxCapData[52] = 0.000000; ui32->fFluxCapData[53] = 0.000000; ui32->fFluxCapData[54] = 0.000000; ui32->fFluxCapData[55] = 0.000000; ui32->fFluxCapData[56] = 0.000000; ui32->fFluxCapData[57] = 0.000000; ui32->fFluxCapData[58] = 0.000000; ui32->fFluxCapData[59] = 0.000000; ui32->fFluxCapData[60] = 0.000000; ui32->fFluxCapData[61] = 0.000000; ui32->fFluxCapData[62] = 0.000000; ui32->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui32);
	delete ui32;


	m_fAPF_7_g = 0.600000;
	CUICtrl* ui33 = new CUICtrl;
	ui33->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui33->uControlId = 119;
	ui33->bLogSlider = false;
	ui33->bExpSlider = false;
	ui33->fUserDisplayDataLoLimit = -1.000000;
	ui33->fUserDisplayDataHiLimit = 1.000000;
	ui33->uUserDataType = floatData;
	ui33->fInitUserIntValue = 0;
	ui33->fInitUserFloatValue = 0.600000;
	ui33->fInitUserDoubleValue = 0;
	ui33->fInitUserUINTValue = 0;
	ui33->m_pUserCookedIntData = NULL;
	ui33->m_pUserCookedFloatData = &m_fAPF_7_g;
	ui33->m_pUserCookedDoubleData = NULL;
	ui33->m_pUserCookedUINTData = NULL;
	ui33->cControlUnits = "APF Bank 7";
	ui33->cVariableName = "m_fAPF_7_g";
	ui33->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui33->dPresetData[0] = 0.600000;ui33->dPresetData[1] = -0.500000;ui33->dPresetData[2] = -0.500000;ui33->dPresetData[3] = -0.500000;ui33->dPresetData[4] = 0.000000;ui33->dPresetData[5] = -0.500000;ui33->dPresetData[6] = 0.000000;ui33->dPresetData[7] = 0.000000;ui33->dPresetData[8] = 0.000000;ui33->dPresetData[9] = 0.000000;ui33->dPresetData[10] = 0.000000;ui33->dPresetData[11] = 0.000000;ui33->dPresetData[12] = 0.000000;ui33->dPresetData[13] = 0.000000;ui33->dPresetData[14] = 0.000000;ui33->dPresetData[15] = 0.000000;
	ui33->cControlName = "APF7 g";
	ui33->bOwnerControl = false;
	ui33->bMIDIControl = false;
	ui33->uMIDIControlCommand = 176;
	ui33->uMIDIControlName = 3;
	ui33->uMIDIControlChannel = 0;
	ui33->nGUIRow = nIndexer++;
	ui33->nGUIColumn = -1;
	ui33->bEnableParamSmoothing = false;
	ui33->fSmoothingTimeInMs = 100.00;
	ui33->uControlTheme[0] = 0; ui33->uControlTheme[1] = 0; ui33->uControlTheme[2] = 0; ui33->uControlTheme[3] = 0; ui33->uControlTheme[4] = 0; ui33->uControlTheme[5] = 0; ui33->uControlTheme[6] = 0; ui33->uControlTheme[7] = 0; ui33->uControlTheme[8] = 0; ui33->uControlTheme[9] = 0; ui33->uControlTheme[10] = 0; ui33->uControlTheme[11] = 0; ui33->uControlTheme[12] = 0; ui33->uControlTheme[13] = 0; ui33->uControlTheme[14] = 0; ui33->uControlTheme[15] = 0; ui33->uControlTheme[16] = 2; ui33->uControlTheme[17] = 0; ui33->uControlTheme[18] = 0; ui33->uControlTheme[19] = 0; ui33->uControlTheme[20] = 0; ui33->uControlTheme[21] = 0; ui33->uControlTheme[22] = 0; ui33->uControlTheme[23] = 0; ui33->uControlTheme[24] = 0; ui33->uControlTheme[25] = 0; ui33->uControlTheme[26] = 0; ui33->uControlTheme[27] = 0; ui33->uControlTheme[28] = 0; ui33->uControlTheme[29] = 0; ui33->uControlTheme[30] = 0; ui33->uControlTheme[31] = 0; 
	ui33->uFluxCapControl[0] = 0; ui33->uFluxCapControl[1] = 0; ui33->uFluxCapControl[2] = 0; ui33->uFluxCapControl[3] = 0; ui33->uFluxCapControl[4] = 0; ui33->uFluxCapControl[5] = 0; ui33->uFluxCapControl[6] = 0; ui33->uFluxCapControl[7] = 0; ui33->uFluxCapControl[8] = 0; ui33->uFluxCapControl[9] = 0; ui33->uFluxCapControl[10] = 0; ui33->uFluxCapControl[11] = 0; ui33->uFluxCapControl[12] = 0; ui33->uFluxCapControl[13] = 0; ui33->uFluxCapControl[14] = 0; ui33->uFluxCapControl[15] = 0; ui33->uFluxCapControl[16] = 0; ui33->uFluxCapControl[17] = 0; ui33->uFluxCapControl[18] = 0; ui33->uFluxCapControl[19] = 0; ui33->uFluxCapControl[20] = 0; ui33->uFluxCapControl[21] = 0; ui33->uFluxCapControl[22] = 0; ui33->uFluxCapControl[23] = 0; ui33->uFluxCapControl[24] = 0; ui33->uFluxCapControl[25] = 0; ui33->uFluxCapControl[26] = 0; ui33->uFluxCapControl[27] = 0; ui33->uFluxCapControl[28] = 0; ui33->uFluxCapControl[29] = 0; ui33->uFluxCapControl[30] = 0; ui33->uFluxCapControl[31] = 0; ui33->uFluxCapControl[32] = 0; ui33->uFluxCapControl[33] = 0; ui33->uFluxCapControl[34] = 0; ui33->uFluxCapControl[35] = 0; ui33->uFluxCapControl[36] = 0; ui33->uFluxCapControl[37] = 0; ui33->uFluxCapControl[38] = 0; ui33->uFluxCapControl[39] = 0; ui33->uFluxCapControl[40] = 0; ui33->uFluxCapControl[41] = 0; ui33->uFluxCapControl[42] = 0; ui33->uFluxCapControl[43] = 0; ui33->uFluxCapControl[44] = 0; ui33->uFluxCapControl[45] = 0; ui33->uFluxCapControl[46] = 0; ui33->uFluxCapControl[47] = 0; ui33->uFluxCapControl[48] = 0; ui33->uFluxCapControl[49] = 0; ui33->uFluxCapControl[50] = 0; ui33->uFluxCapControl[51] = 0; ui33->uFluxCapControl[52] = 0; ui33->uFluxCapControl[53] = 0; ui33->uFluxCapControl[54] = 0; ui33->uFluxCapControl[55] = 0; ui33->uFluxCapControl[56] = 0; ui33->uFluxCapControl[57] = 0; ui33->uFluxCapControl[58] = 0; ui33->uFluxCapControl[59] = 0; ui33->uFluxCapControl[60] = 0; ui33->uFluxCapControl[61] = 0; ui33->uFluxCapControl[62] = 0; ui33->uFluxCapControl[63] = 0; 
	ui33->fFluxCapData[0] = 0.000000; ui33->fFluxCapData[1] = 0.000000; ui33->fFluxCapData[2] = 0.000000; ui33->fFluxCapData[3] = 0.000000; ui33->fFluxCapData[4] = 0.000000; ui33->fFluxCapData[5] = 0.000000; ui33->fFluxCapData[6] = 0.000000; ui33->fFluxCapData[7] = 0.000000; ui33->fFluxCapData[8] = 0.000000; ui33->fFluxCapData[9] = 0.000000; ui33->fFluxCapData[10] = 0.000000; ui33->fFluxCapData[11] = 0.000000; ui33->fFluxCapData[12] = 0.000000; ui33->fFluxCapData[13] = 0.000000; ui33->fFluxCapData[14] = 0.000000; ui33->fFluxCapData[15] = 0.000000; ui33->fFluxCapData[16] = 0.000000; ui33->fFluxCapData[17] = 0.000000; ui33->fFluxCapData[18] = 0.000000; ui33->fFluxCapData[19] = 0.000000; ui33->fFluxCapData[20] = 0.000000; ui33->fFluxCapData[21] = 0.000000; ui33->fFluxCapData[22] = 0.000000; ui33->fFluxCapData[23] = 0.000000; ui33->fFluxCapData[24] = 0.000000; ui33->fFluxCapData[25] = 0.000000; ui33->fFluxCapData[26] = 0.000000; ui33->fFluxCapData[27] = 0.000000; ui33->fFluxCapData[28] = 0.000000; ui33->fFluxCapData[29] = 0.000000; ui33->fFluxCapData[30] = 0.000000; ui33->fFluxCapData[31] = 0.000000; ui33->fFluxCapData[32] = 0.000000; ui33->fFluxCapData[33] = 0.000000; ui33->fFluxCapData[34] = 0.000000; ui33->fFluxCapData[35] = 0.000000; ui33->fFluxCapData[36] = 0.000000; ui33->fFluxCapData[37] = 0.000000; ui33->fFluxCapData[38] = 0.000000; ui33->fFluxCapData[39] = 0.000000; ui33->fFluxCapData[40] = 0.000000; ui33->fFluxCapData[41] = 0.000000; ui33->fFluxCapData[42] = 0.000000; ui33->fFluxCapData[43] = 0.000000; ui33->fFluxCapData[44] = 0.000000; ui33->fFluxCapData[45] = 0.000000; ui33->fFluxCapData[46] = 0.000000; ui33->fFluxCapData[47] = 0.000000; ui33->fFluxCapData[48] = 0.000000; ui33->fFluxCapData[49] = 0.000000; ui33->fFluxCapData[50] = 0.000000; ui33->fFluxCapData[51] = 0.000000; ui33->fFluxCapData[52] = 0.000000; ui33->fFluxCapData[53] = 0.000000; ui33->fFluxCapData[54] = 0.000000; ui33->fFluxCapData[55] = 0.000000; ui33->fFluxCapData[56] = 0.000000; ui33->fFluxCapData[57] = 0.000000; ui33->fFluxCapData[58] = 0.000000; ui33->fFluxCapData[59] = 0.000000; ui33->fFluxCapData[60] = 0.000000; ui33->fFluxCapData[61] = 0.000000; ui33->fFluxCapData[62] = 0.000000; ui33->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui33);
	delete ui33;


	m_fAPF_8_Delay_mSec = 11.000000;
	CUICtrl* ui34 = new CUICtrl;
	ui34->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui34->uControlId = 128;
	ui34->bLogSlider = false;
	ui34->bExpSlider = false;
	ui34->fUserDisplayDataLoLimit = 0.000000;
	ui34->fUserDisplayDataHiLimit = 100.000000;
	ui34->uUserDataType = floatData;
	ui34->fInitUserIntValue = 0;
	ui34->fInitUserFloatValue = 11.000000;
	ui34->fInitUserDoubleValue = 0;
	ui34->fInitUserUINTValue = 0;
	ui34->m_pUserCookedIntData = NULL;
	ui34->m_pUserCookedFloatData = &m_fAPF_8_Delay_mSec;
	ui34->m_pUserCookedDoubleData = NULL;
	ui34->m_pUserCookedUINTData = NULL;
	ui34->cControlUnits = "mSec";
	ui34->cVariableName = "m_fAPF_8_Delay_mSec";
	ui34->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui34->dPresetData[0] = 11.000000;ui34->dPresetData[1] = 74.000000;ui34->dPresetData[2] = 74.000000;ui34->dPresetData[3] = 40.500000;ui34->dPresetData[4] = 0.000000;ui34->dPresetData[5] = 74.000000;ui34->dPresetData[6] = 0.000000;ui34->dPresetData[7] = 0.000000;ui34->dPresetData[8] = 0.000000;ui34->dPresetData[9] = 0.000000;ui34->dPresetData[10] = 0.000000;ui34->dPresetData[11] = 0.000000;ui34->dPresetData[12] = 0.000000;ui34->dPresetData[13] = 0.000000;ui34->dPresetData[14] = 0.000000;ui34->dPresetData[15] = 0.000000;
	ui34->cControlName = "APF8 Dly";
	ui34->bOwnerControl = false;
	ui34->bMIDIControl = false;
	ui34->uMIDIControlCommand = 176;
	ui34->uMIDIControlName = 3;
	ui34->uMIDIControlChannel = 0;
	ui34->nGUIRow = nIndexer++;
	ui34->nGUIColumn = -1;
	ui34->bEnableParamSmoothing = false;
	ui34->fSmoothingTimeInMs = 100.00;
	ui34->uControlTheme[0] = 0; ui34->uControlTheme[1] = 0; ui34->uControlTheme[2] = 0; ui34->uControlTheme[3] = 0; ui34->uControlTheme[4] = 0; ui34->uControlTheme[5] = 0; ui34->uControlTheme[6] = 0; ui34->uControlTheme[7] = 0; ui34->uControlTheme[8] = 0; ui34->uControlTheme[9] = 0; ui34->uControlTheme[10] = 0; ui34->uControlTheme[11] = 0; ui34->uControlTheme[12] = 0; ui34->uControlTheme[13] = 0; ui34->uControlTheme[14] = 0; ui34->uControlTheme[15] = 0; ui34->uControlTheme[16] = 2; ui34->uControlTheme[17] = 0; ui34->uControlTheme[18] = 0; ui34->uControlTheme[19] = 0; ui34->uControlTheme[20] = 0; ui34->uControlTheme[21] = 0; ui34->uControlTheme[22] = 0; ui34->uControlTheme[23] = 0; ui34->uControlTheme[24] = 0; ui34->uControlTheme[25] = 0; ui34->uControlTheme[26] = 0; ui34->uControlTheme[27] = 0; ui34->uControlTheme[28] = 0; ui34->uControlTheme[29] = 0; ui34->uControlTheme[30] = 0; ui34->uControlTheme[31] = 0; 
	ui34->uFluxCapControl[0] = 0; ui34->uFluxCapControl[1] = 0; ui34->uFluxCapControl[2] = 0; ui34->uFluxCapControl[3] = 0; ui34->uFluxCapControl[4] = 0; ui34->uFluxCapControl[5] = 0; ui34->uFluxCapControl[6] = 0; ui34->uFluxCapControl[7] = 0; ui34->uFluxCapControl[8] = 0; ui34->uFluxCapControl[9] = 0; ui34->uFluxCapControl[10] = 0; ui34->uFluxCapControl[11] = 0; ui34->uFluxCapControl[12] = 0; ui34->uFluxCapControl[13] = 0; ui34->uFluxCapControl[14] = 0; ui34->uFluxCapControl[15] = 0; ui34->uFluxCapControl[16] = 0; ui34->uFluxCapControl[17] = 0; ui34->uFluxCapControl[18] = 0; ui34->uFluxCapControl[19] = 0; ui34->uFluxCapControl[20] = 0; ui34->uFluxCapControl[21] = 0; ui34->uFluxCapControl[22] = 0; ui34->uFluxCapControl[23] = 0; ui34->uFluxCapControl[24] = 0; ui34->uFluxCapControl[25] = 0; ui34->uFluxCapControl[26] = 0; ui34->uFluxCapControl[27] = 0; ui34->uFluxCapControl[28] = 0; ui34->uFluxCapControl[29] = 0; ui34->uFluxCapControl[30] = 0; ui34->uFluxCapControl[31] = 0; ui34->uFluxCapControl[32] = 0; ui34->uFluxCapControl[33] = 0; ui34->uFluxCapControl[34] = 0; ui34->uFluxCapControl[35] = 0; ui34->uFluxCapControl[36] = 0; ui34->uFluxCapControl[37] = 0; ui34->uFluxCapControl[38] = 0; ui34->uFluxCapControl[39] = 0; ui34->uFluxCapControl[40] = 0; ui34->uFluxCapControl[41] = 0; ui34->uFluxCapControl[42] = 0; ui34->uFluxCapControl[43] = 0; ui34->uFluxCapControl[44] = 0; ui34->uFluxCapControl[45] = 0; ui34->uFluxCapControl[46] = 0; ui34->uFluxCapControl[47] = 0; ui34->uFluxCapControl[48] = 0; ui34->uFluxCapControl[49] = 0; ui34->uFluxCapControl[50] = 0; ui34->uFluxCapControl[51] = 0; ui34->uFluxCapControl[52] = 0; ui34->uFluxCapControl[53] = 0; ui34->uFluxCapControl[54] = 0; ui34->uFluxCapControl[55] = 0; ui34->uFluxCapControl[56] = 0; ui34->uFluxCapControl[57] = 0; ui34->uFluxCapControl[58] = 0; ui34->uFluxCapControl[59] = 0; ui34->uFluxCapControl[60] = 0; ui34->uFluxCapControl[61] = 0; ui34->uFluxCapControl[62] = 0; ui34->uFluxCapControl[63] = 0; 
	ui34->fFluxCapData[0] = 0.000000; ui34->fFluxCapData[1] = 0.000000; ui34->fFluxCapData[2] = 0.000000; ui34->fFluxCapData[3] = 0.000000; ui34->fFluxCapData[4] = 0.000000; ui34->fFluxCapData[5] = 0.000000; ui34->fFluxCapData[6] = 0.000000; ui34->fFluxCapData[7] = 0.000000; ui34->fFluxCapData[8] = 0.000000; ui34->fFluxCapData[9] = 0.000000; ui34->fFluxCapData[10] = 0.000000; ui34->fFluxCapData[11] = 0.000000; ui34->fFluxCapData[12] = 0.000000; ui34->fFluxCapData[13] = 0.000000; ui34->fFluxCapData[14] = 0.000000; ui34->fFluxCapData[15] = 0.000000; ui34->fFluxCapData[16] = 0.000000; ui34->fFluxCapData[17] = 0.000000; ui34->fFluxCapData[18] = 0.000000; ui34->fFluxCapData[19] = 0.000000; ui34->fFluxCapData[20] = 0.000000; ui34->fFluxCapData[21] = 0.000000; ui34->fFluxCapData[22] = 0.000000; ui34->fFluxCapData[23] = 0.000000; ui34->fFluxCapData[24] = 0.000000; ui34->fFluxCapData[25] = 0.000000; ui34->fFluxCapData[26] = 0.000000; ui34->fFluxCapData[27] = 0.000000; ui34->fFluxCapData[28] = 0.000000; ui34->fFluxCapData[29] = 0.000000; ui34->fFluxCapData[30] = 0.000000; ui34->fFluxCapData[31] = 0.000000; ui34->fFluxCapData[32] = 0.000000; ui34->fFluxCapData[33] = 0.000000; ui34->fFluxCapData[34] = 0.000000; ui34->fFluxCapData[35] = 0.000000; ui34->fFluxCapData[36] = 0.000000; ui34->fFluxCapData[37] = 0.000000; ui34->fFluxCapData[38] = 0.000000; ui34->fFluxCapData[39] = 0.000000; ui34->fFluxCapData[40] = 0.000000; ui34->fFluxCapData[41] = 0.000000; ui34->fFluxCapData[42] = 0.000000; ui34->fFluxCapData[43] = 0.000000; ui34->fFluxCapData[44] = 0.000000; ui34->fFluxCapData[45] = 0.000000; ui34->fFluxCapData[46] = 0.000000; ui34->fFluxCapData[47] = 0.000000; ui34->fFluxCapData[48] = 0.000000; ui34->fFluxCapData[49] = 0.000000; ui34->fFluxCapData[50] = 0.000000; ui34->fFluxCapData[51] = 0.000000; ui34->fFluxCapData[52] = 0.000000; ui34->fFluxCapData[53] = 0.000000; ui34->fFluxCapData[54] = 0.000000; ui34->fFluxCapData[55] = 0.000000; ui34->fFluxCapData[56] = 0.000000; ui34->fFluxCapData[57] = 0.000000; ui34->fFluxCapData[58] = 0.000000; ui34->fFluxCapData[59] = 0.000000; ui34->fFluxCapData[60] = 0.000000; ui34->fFluxCapData[61] = 0.000000; ui34->fFluxCapData[62] = 0.000000; ui34->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui34);
	delete ui34;


	m_fAPF_8_g = 0.600000;
	CUICtrl* ui35 = new CUICtrl;
	ui35->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui35->uControlId = 129;
	ui35->bLogSlider = false;
	ui35->bExpSlider = false;
	ui35->fUserDisplayDataLoLimit = -1.000000;
	ui35->fUserDisplayDataHiLimit = 1.000000;
	ui35->uUserDataType = floatData;
	ui35->fInitUserIntValue = 0;
	ui35->fInitUserFloatValue = 0.600000;
	ui35->fInitUserDoubleValue = 0;
	ui35->fInitUserUINTValue = 0;
	ui35->m_pUserCookedIntData = NULL;
	ui35->m_pUserCookedFloatData = &m_fAPF_8_g;
	ui35->m_pUserCookedDoubleData = NULL;
	ui35->m_pUserCookedUINTData = NULL;
	ui35->cControlUnits = "APF Bank 8";
	ui35->cVariableName = "m_fAPF_8_g";
	ui35->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui35->dPresetData[0] = 0.600000;ui35->dPresetData[1] = 0.500000;ui35->dPresetData[2] = 0.500000;ui35->dPresetData[3] = 0.500000;ui35->dPresetData[4] = 0.000000;ui35->dPresetData[5] = 0.500000;ui35->dPresetData[6] = 0.000000;ui35->dPresetData[7] = 0.000000;ui35->dPresetData[8] = 0.000000;ui35->dPresetData[9] = 0.000000;ui35->dPresetData[10] = 0.000000;ui35->dPresetData[11] = 0.000000;ui35->dPresetData[12] = 0.000000;ui35->dPresetData[13] = 0.000000;ui35->dPresetData[14] = 0.000000;ui35->dPresetData[15] = 0.000000;
	ui35->cControlName = "APF8 g";
	ui35->bOwnerControl = false;
	ui35->bMIDIControl = false;
	ui35->uMIDIControlCommand = 176;
	ui35->uMIDIControlName = 3;
	ui35->uMIDIControlChannel = 0;
	ui35->nGUIRow = nIndexer++;
	ui35->nGUIColumn = -1;
	ui35->bEnableParamSmoothing = false;
	ui35->fSmoothingTimeInMs = 100.00;
	ui35->uControlTheme[0] = 0; ui35->uControlTheme[1] = 0; ui35->uControlTheme[2] = 0; ui35->uControlTheme[3] = 0; ui35->uControlTheme[4] = 0; ui35->uControlTheme[5] = 0; ui35->uControlTheme[6] = 0; ui35->uControlTheme[7] = 0; ui35->uControlTheme[8] = 0; ui35->uControlTheme[9] = 0; ui35->uControlTheme[10] = 0; ui35->uControlTheme[11] = 0; ui35->uControlTheme[12] = 0; ui35->uControlTheme[13] = 0; ui35->uControlTheme[14] = 0; ui35->uControlTheme[15] = 0; ui35->uControlTheme[16] = 2; ui35->uControlTheme[17] = 0; ui35->uControlTheme[18] = 0; ui35->uControlTheme[19] = 0; ui35->uControlTheme[20] = 0; ui35->uControlTheme[21] = 0; ui35->uControlTheme[22] = 0; ui35->uControlTheme[23] = 0; ui35->uControlTheme[24] = 0; ui35->uControlTheme[25] = 0; ui35->uControlTheme[26] = 0; ui35->uControlTheme[27] = 0; ui35->uControlTheme[28] = 0; ui35->uControlTheme[29] = 0; ui35->uControlTheme[30] = 0; ui35->uControlTheme[31] = 0; 
	ui35->uFluxCapControl[0] = 0; ui35->uFluxCapControl[1] = 0; ui35->uFluxCapControl[2] = 0; ui35->uFluxCapControl[3] = 0; ui35->uFluxCapControl[4] = 0; ui35->uFluxCapControl[5] = 0; ui35->uFluxCapControl[6] = 0; ui35->uFluxCapControl[7] = 0; ui35->uFluxCapControl[8] = 0; ui35->uFluxCapControl[9] = 0; ui35->uFluxCapControl[10] = 0; ui35->uFluxCapControl[11] = 0; ui35->uFluxCapControl[12] = 0; ui35->uFluxCapControl[13] = 0; ui35->uFluxCapControl[14] = 0; ui35->uFluxCapControl[15] = 0; ui35->uFluxCapControl[16] = 0; ui35->uFluxCapControl[17] = 0; ui35->uFluxCapControl[18] = 0; ui35->uFluxCapControl[19] = 0; ui35->uFluxCapControl[20] = 0; ui35->uFluxCapControl[21] = 0; ui35->uFluxCapControl[22] = 0; ui35->uFluxCapControl[23] = 0; ui35->uFluxCapControl[24] = 0; ui35->uFluxCapControl[25] = 0; ui35->uFluxCapControl[26] = 0; ui35->uFluxCapControl[27] = 0; ui35->uFluxCapControl[28] = 0; ui35->uFluxCapControl[29] = 0; ui35->uFluxCapControl[30] = 0; ui35->uFluxCapControl[31] = 0; ui35->uFluxCapControl[32] = 0; ui35->uFluxCapControl[33] = 0; ui35->uFluxCapControl[34] = 0; ui35->uFluxCapControl[35] = 0; ui35->uFluxCapControl[36] = 0; ui35->uFluxCapControl[37] = 0; ui35->uFluxCapControl[38] = 0; ui35->uFluxCapControl[39] = 0; ui35->uFluxCapControl[40] = 0; ui35->uFluxCapControl[41] = 0; ui35->uFluxCapControl[42] = 0; ui35->uFluxCapControl[43] = 0; ui35->uFluxCapControl[44] = 0; ui35->uFluxCapControl[45] = 0; ui35->uFluxCapControl[46] = 0; ui35->uFluxCapControl[47] = 0; ui35->uFluxCapControl[48] = 0; ui35->uFluxCapControl[49] = 0; ui35->uFluxCapControl[50] = 0; ui35->uFluxCapControl[51] = 0; ui35->uFluxCapControl[52] = 0; ui35->uFluxCapControl[53] = 0; ui35->uFluxCapControl[54] = 0; ui35->uFluxCapControl[55] = 0; ui35->uFluxCapControl[56] = 0; ui35->uFluxCapControl[57] = 0; ui35->uFluxCapControl[58] = 0; ui35->uFluxCapControl[59] = 0; ui35->uFluxCapControl[60] = 0; ui35->uFluxCapControl[61] = 0; ui35->uFluxCapControl[62] = 0; ui35->uFluxCapControl[63] = 0; 
	ui35->fFluxCapData[0] = 0.000000; ui35->fFluxCapData[1] = 0.000000; ui35->fFluxCapData[2] = 0.000000; ui35->fFluxCapData[3] = 0.000000; ui35->fFluxCapData[4] = 0.000000; ui35->fFluxCapData[5] = 0.000000; ui35->fFluxCapData[6] = 0.000000; ui35->fFluxCapData[7] = 0.000000; ui35->fFluxCapData[8] = 0.000000; ui35->fFluxCapData[9] = 0.000000; ui35->fFluxCapData[10] = 0.000000; ui35->fFluxCapData[11] = 0.000000; ui35->fFluxCapData[12] = 0.000000; ui35->fFluxCapData[13] = 0.000000; ui35->fFluxCapData[14] = 0.000000; ui35->fFluxCapData[15] = 0.000000; ui35->fFluxCapData[16] = 0.000000; ui35->fFluxCapData[17] = 0.000000; ui35->fFluxCapData[18] = 0.000000; ui35->fFluxCapData[19] = 0.000000; ui35->fFluxCapData[20] = 0.000000; ui35->fFluxCapData[21] = 0.000000; ui35->fFluxCapData[22] = 0.000000; ui35->fFluxCapData[23] = 0.000000; ui35->fFluxCapData[24] = 0.000000; ui35->fFluxCapData[25] = 0.000000; ui35->fFluxCapData[26] = 0.000000; ui35->fFluxCapData[27] = 0.000000; ui35->fFluxCapData[28] = 0.000000; ui35->fFluxCapData[29] = 0.000000; ui35->fFluxCapData[30] = 0.000000; ui35->fFluxCapData[31] = 0.000000; ui35->fFluxCapData[32] = 0.000000; ui35->fFluxCapData[33] = 0.000000; ui35->fFluxCapData[34] = 0.000000; ui35->fFluxCapData[35] = 0.000000; ui35->fFluxCapData[36] = 0.000000; ui35->fFluxCapData[37] = 0.000000; ui35->fFluxCapData[38] = 0.000000; ui35->fFluxCapData[39] = 0.000000; ui35->fFluxCapData[40] = 0.000000; ui35->fFluxCapData[41] = 0.000000; ui35->fFluxCapData[42] = 0.000000; ui35->fFluxCapData[43] = 0.000000; ui35->fFluxCapData[44] = 0.000000; ui35->fFluxCapData[45] = 0.000000; ui35->fFluxCapData[46] = 0.000000; ui35->fFluxCapData[47] = 0.000000; ui35->fFluxCapData[48] = 0.000000; ui35->fFluxCapData[49] = 0.000000; ui35->fFluxCapData[50] = 0.000000; ui35->fFluxCapData[51] = 0.000000; ui35->fFluxCapData[52] = 0.000000; ui35->fFluxCapData[53] = 0.000000; ui35->fFluxCapData[54] = 0.000000; ui35->fFluxCapData[55] = 0.000000; ui35->fFluxCapData[56] = 0.000000; ui35->fFluxCapData[57] = 0.000000; ui35->fFluxCapData[58] = 0.000000; ui35->fFluxCapData[59] = 0.000000; ui35->fFluxCapData[60] = 0.000000; ui35->fFluxCapData[61] = 0.000000; ui35->fFluxCapData[62] = 0.000000; ui35->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui35);
	delete ui35;


	m_f_Feedback_pct = 0.000000;
	CUICtrl* ui36 = new CUICtrl;
	ui36->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui36->uControlId = 130;
	ui36->bLogSlider = false;
	ui36->bExpSlider = false;
	ui36->fUserDisplayDataLoLimit = -100.000000;
	ui36->fUserDisplayDataHiLimit = 100.000000;
	ui36->uUserDataType = floatData;
	ui36->fInitUserIntValue = 0;
	ui36->fInitUserFloatValue = 0.000000;
	ui36->fInitUserDoubleValue = 0;
	ui36->fInitUserUINTValue = 0;
	ui36->m_pUserCookedIntData = NULL;
	ui36->m_pUserCookedFloatData = &m_f_Feedback_pct;
	ui36->m_pUserCookedDoubleData = NULL;
	ui36->m_pUserCookedUINTData = NULL;
	ui36->cControlUnits = "%";
	ui36->cVariableName = "m_f_Feedback_pct";
	ui36->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui36->dPresetData[0] = 0.000000;ui36->dPresetData[1] = 95.000000;ui36->dPresetData[2] = 0.000000;ui36->dPresetData[3] = 0.000000;ui36->dPresetData[4] = 0.000000;ui36->dPresetData[5] = 0.000000;ui36->dPresetData[6] = 0.000000;ui36->dPresetData[7] = 0.000000;ui36->dPresetData[8] = 0.000000;ui36->dPresetData[9] = 0.000000;ui36->dPresetData[10] = 0.000000;ui36->dPresetData[11] = 0.000000;ui36->dPresetData[12] = 0.000000;ui36->dPresetData[13] = 0.000000;ui36->dPresetData[14] = 0.000000;ui36->dPresetData[15] = 0.000000;
	ui36->cControlName = "Pitch Emulator";
	ui36->bOwnerControl = false;
	ui36->bMIDIControl = false;
	ui36->uMIDIControlCommand = 176;
	ui36->uMIDIControlName = 3;
	ui36->uMIDIControlChannel = 0;
	ui36->nGUIRow = nIndexer++;
	ui36->nGUIColumn = -1;
	ui36->bEnableParamSmoothing = false;
	ui36->fSmoothingTimeInMs = 100.00;
	ui36->uControlTheme[0] = 0; ui36->uControlTheme[1] = 0; ui36->uControlTheme[2] = 0; ui36->uControlTheme[3] = 0; ui36->uControlTheme[4] = 0; ui36->uControlTheme[5] = 0; ui36->uControlTheme[6] = 0; ui36->uControlTheme[7] = 0; ui36->uControlTheme[8] = 0; ui36->uControlTheme[9] = 0; ui36->uControlTheme[10] = 0; ui36->uControlTheme[11] = 0; ui36->uControlTheme[12] = 0; ui36->uControlTheme[13] = 0; ui36->uControlTheme[14] = 0; ui36->uControlTheme[15] = 0; ui36->uControlTheme[16] = 2; ui36->uControlTheme[17] = 0; ui36->uControlTheme[18] = 0; ui36->uControlTheme[19] = 0; ui36->uControlTheme[20] = 0; ui36->uControlTheme[21] = 0; ui36->uControlTheme[22] = 0; ui36->uControlTheme[23] = 0; ui36->uControlTheme[24] = 0; ui36->uControlTheme[25] = 0; ui36->uControlTheme[26] = 0; ui36->uControlTheme[27] = 0; ui36->uControlTheme[28] = 0; ui36->uControlTheme[29] = 0; ui36->uControlTheme[30] = 0; ui36->uControlTheme[31] = 0; 
	ui36->uFluxCapControl[0] = 0; ui36->uFluxCapControl[1] = 0; ui36->uFluxCapControl[2] = 0; ui36->uFluxCapControl[3] = 0; ui36->uFluxCapControl[4] = 0; ui36->uFluxCapControl[5] = 0; ui36->uFluxCapControl[6] = 0; ui36->uFluxCapControl[7] = 0; ui36->uFluxCapControl[8] = 0; ui36->uFluxCapControl[9] = 0; ui36->uFluxCapControl[10] = 0; ui36->uFluxCapControl[11] = 0; ui36->uFluxCapControl[12] = 0; ui36->uFluxCapControl[13] = 0; ui36->uFluxCapControl[14] = 0; ui36->uFluxCapControl[15] = 0; ui36->uFluxCapControl[16] = 0; ui36->uFluxCapControl[17] = 0; ui36->uFluxCapControl[18] = 0; ui36->uFluxCapControl[19] = 0; ui36->uFluxCapControl[20] = 0; ui36->uFluxCapControl[21] = 0; ui36->uFluxCapControl[22] = 0; ui36->uFluxCapControl[23] = 0; ui36->uFluxCapControl[24] = 0; ui36->uFluxCapControl[25] = 0; ui36->uFluxCapControl[26] = 0; ui36->uFluxCapControl[27] = 0; ui36->uFluxCapControl[28] = 0; ui36->uFluxCapControl[29] = 0; ui36->uFluxCapControl[30] = 0; ui36->uFluxCapControl[31] = 0; ui36->uFluxCapControl[32] = 0; ui36->uFluxCapControl[33] = 0; ui36->uFluxCapControl[34] = 0; ui36->uFluxCapControl[35] = 0; ui36->uFluxCapControl[36] = 0; ui36->uFluxCapControl[37] = 0; ui36->uFluxCapControl[38] = 0; ui36->uFluxCapControl[39] = 0; ui36->uFluxCapControl[40] = 0; ui36->uFluxCapControl[41] = 0; ui36->uFluxCapControl[42] = 0; ui36->uFluxCapControl[43] = 0; ui36->uFluxCapControl[44] = 0; ui36->uFluxCapControl[45] = 0; ui36->uFluxCapControl[46] = 0; ui36->uFluxCapControl[47] = 0; ui36->uFluxCapControl[48] = 0; ui36->uFluxCapControl[49] = 0; ui36->uFluxCapControl[50] = 0; ui36->uFluxCapControl[51] = 0; ui36->uFluxCapControl[52] = 0; ui36->uFluxCapControl[53] = 0; ui36->uFluxCapControl[54] = 0; ui36->uFluxCapControl[55] = 0; ui36->uFluxCapControl[56] = 0; ui36->uFluxCapControl[57] = 0; ui36->uFluxCapControl[58] = 0; ui36->uFluxCapControl[59] = 0; ui36->uFluxCapControl[60] = 0; ui36->uFluxCapControl[61] = 0; ui36->uFluxCapControl[62] = 0; ui36->uFluxCapControl[63] = 0; 
	ui36->fFluxCapData[0] = 0.000000; ui36->fFluxCapData[1] = 0.000000; ui36->fFluxCapData[2] = 0.000000; ui36->fFluxCapData[3] = 0.000000; ui36->fFluxCapData[4] = 0.000000; ui36->fFluxCapData[5] = 0.000000; ui36->fFluxCapData[6] = 0.000000; ui36->fFluxCapData[7] = 0.000000; ui36->fFluxCapData[8] = 0.000000; ui36->fFluxCapData[9] = 0.000000; ui36->fFluxCapData[10] = 0.000000; ui36->fFluxCapData[11] = 0.000000; ui36->fFluxCapData[12] = 0.000000; ui36->fFluxCapData[13] = 0.000000; ui36->fFluxCapData[14] = 0.000000; ui36->fFluxCapData[15] = 0.000000; ui36->fFluxCapData[16] = 0.000000; ui36->fFluxCapData[17] = 0.000000; ui36->fFluxCapData[18] = 0.000000; ui36->fFluxCapData[19] = 0.000000; ui36->fFluxCapData[20] = 0.000000; ui36->fFluxCapData[21] = 0.000000; ui36->fFluxCapData[22] = 0.000000; ui36->fFluxCapData[23] = 0.000000; ui36->fFluxCapData[24] = 0.000000; ui36->fFluxCapData[25] = 0.000000; ui36->fFluxCapData[26] = 0.000000; ui36->fFluxCapData[27] = 0.000000; ui36->fFluxCapData[28] = 0.000000; ui36->fFluxCapData[29] = 0.000000; ui36->fFluxCapData[30] = 0.000000; ui36->fFluxCapData[31] = 0.000000; ui36->fFluxCapData[32] = 0.000000; ui36->fFluxCapData[33] = 0.000000; ui36->fFluxCapData[34] = 0.000000; ui36->fFluxCapData[35] = 0.000000; ui36->fFluxCapData[36] = 0.000000; ui36->fFluxCapData[37] = 0.000000; ui36->fFluxCapData[38] = 0.000000; ui36->fFluxCapData[39] = 0.000000; ui36->fFluxCapData[40] = 0.000000; ui36->fFluxCapData[41] = 0.000000; ui36->fFluxCapData[42] = 0.000000; ui36->fFluxCapData[43] = 0.000000; ui36->fFluxCapData[44] = 0.000000; ui36->fFluxCapData[45] = 0.000000; ui36->fFluxCapData[46] = 0.000000; ui36->fFluxCapData[47] = 0.000000; ui36->fFluxCapData[48] = 0.000000; ui36->fFluxCapData[49] = 0.000000; ui36->fFluxCapData[50] = 0.000000; ui36->fFluxCapData[51] = 0.000000; ui36->fFluxCapData[52] = 0.000000; ui36->fFluxCapData[53] = 0.000000; ui36->fFluxCapData[54] = 0.000000; ui36->fFluxCapData[55] = 0.000000; ui36->fFluxCapData[56] = 0.000000; ui36->fFluxCapData[57] = 0.000000; ui36->fFluxCapData[58] = 0.000000; ui36->fFluxCapData[59] = 0.000000; ui36->fFluxCapData[60] = 0.000000; ui36->fFluxCapData[61] = 0.000000; ui36->fFluxCapData[62] = 0.000000; ui36->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui36);
	delete ui36;


	m_fWetLevel_pct = 50.000000;
	CUICtrl* ui37 = new CUICtrl;
	ui37->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui37->uControlId = 131;
	ui37->bLogSlider = false;
	ui37->bExpSlider = false;
	ui37->fUserDisplayDataLoLimit = 0.000000;
	ui37->fUserDisplayDataHiLimit = 100.000000;
	ui37->uUserDataType = floatData;
	ui37->fInitUserIntValue = 0;
	ui37->fInitUserFloatValue = 50.000000;
	ui37->fInitUserDoubleValue = 0;
	ui37->fInitUserUINTValue = 0;
	ui37->m_pUserCookedIntData = NULL;
	ui37->m_pUserCookedFloatData = &m_fWetLevel_pct;
	ui37->m_pUserCookedDoubleData = NULL;
	ui37->m_pUserCookedUINTData = NULL;
	ui37->cControlUnits = "%";
	ui37->cVariableName = "m_fWetLevel_pct";
	ui37->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui37->dPresetData[0] = 50.000000;ui37->dPresetData[1] = 100.000000;ui37->dPresetData[2] = 0.000000;ui37->dPresetData[3] = 0.000000;ui37->dPresetData[4] = 0.000000;ui37->dPresetData[5] = 0.000000;ui37->dPresetData[6] = 0.000000;ui37->dPresetData[7] = 0.000000;ui37->dPresetData[8] = 0.000000;ui37->dPresetData[9] = 0.000000;ui37->dPresetData[10] = 0.000000;ui37->dPresetData[11] = 0.000000;ui37->dPresetData[12] = 0.000000;ui37->dPresetData[13] = 0.000000;ui37->dPresetData[14] = 0.000000;ui37->dPresetData[15] = 0.000000;
	ui37->cControlName = "Wet/Dry";
	ui37->bOwnerControl = false;
	ui37->bMIDIControl = false;
	ui37->uMIDIControlCommand = 176;
	ui37->uMIDIControlName = 3;
	ui37->uMIDIControlChannel = 0;
	ui37->nGUIRow = nIndexer++;
	ui37->nGUIColumn = -1;
	ui37->bEnableParamSmoothing = false;
	ui37->fSmoothingTimeInMs = 100.00;
	ui37->uControlTheme[0] = 0; ui37->uControlTheme[1] = 0; ui37->uControlTheme[2] = 0; ui37->uControlTheme[3] = 0; ui37->uControlTheme[4] = 0; ui37->uControlTheme[5] = 0; ui37->uControlTheme[6] = 0; ui37->uControlTheme[7] = 0; ui37->uControlTheme[8] = 0; ui37->uControlTheme[9] = 0; ui37->uControlTheme[10] = 0; ui37->uControlTheme[11] = 0; ui37->uControlTheme[12] = 0; ui37->uControlTheme[13] = 0; ui37->uControlTheme[14] = 0; ui37->uControlTheme[15] = 0; ui37->uControlTheme[16] = 2; ui37->uControlTheme[17] = 0; ui37->uControlTheme[18] = 0; ui37->uControlTheme[19] = 0; ui37->uControlTheme[20] = 0; ui37->uControlTheme[21] = 0; ui37->uControlTheme[22] = 0; ui37->uControlTheme[23] = 0; ui37->uControlTheme[24] = 0; ui37->uControlTheme[25] = 0; ui37->uControlTheme[26] = 0; ui37->uControlTheme[27] = 0; ui37->uControlTheme[28] = 0; ui37->uControlTheme[29] = 0; ui37->uControlTheme[30] = 0; ui37->uControlTheme[31] = 0; 
	ui37->uFluxCapControl[0] = 0; ui37->uFluxCapControl[1] = 0; ui37->uFluxCapControl[2] = 0; ui37->uFluxCapControl[3] = 0; ui37->uFluxCapControl[4] = 0; ui37->uFluxCapControl[5] = 0; ui37->uFluxCapControl[6] = 0; ui37->uFluxCapControl[7] = 0; ui37->uFluxCapControl[8] = 0; ui37->uFluxCapControl[9] = 0; ui37->uFluxCapControl[10] = 0; ui37->uFluxCapControl[11] = 0; ui37->uFluxCapControl[12] = 0; ui37->uFluxCapControl[13] = 0; ui37->uFluxCapControl[14] = 0; ui37->uFluxCapControl[15] = 0; ui37->uFluxCapControl[16] = 0; ui37->uFluxCapControl[17] = 0; ui37->uFluxCapControl[18] = 0; ui37->uFluxCapControl[19] = 0; ui37->uFluxCapControl[20] = 0; ui37->uFluxCapControl[21] = 0; ui37->uFluxCapControl[22] = 0; ui37->uFluxCapControl[23] = 0; ui37->uFluxCapControl[24] = 0; ui37->uFluxCapControl[25] = 0; ui37->uFluxCapControl[26] = 0; ui37->uFluxCapControl[27] = 0; ui37->uFluxCapControl[28] = 0; ui37->uFluxCapControl[29] = 0; ui37->uFluxCapControl[30] = 0; ui37->uFluxCapControl[31] = 0; ui37->uFluxCapControl[32] = 0; ui37->uFluxCapControl[33] = 0; ui37->uFluxCapControl[34] = 0; ui37->uFluxCapControl[35] = 0; ui37->uFluxCapControl[36] = 0; ui37->uFluxCapControl[37] = 0; ui37->uFluxCapControl[38] = 0; ui37->uFluxCapControl[39] = 0; ui37->uFluxCapControl[40] = 0; ui37->uFluxCapControl[41] = 0; ui37->uFluxCapControl[42] = 0; ui37->uFluxCapControl[43] = 0; ui37->uFluxCapControl[44] = 0; ui37->uFluxCapControl[45] = 0; ui37->uFluxCapControl[46] = 0; ui37->uFluxCapControl[47] = 0; ui37->uFluxCapControl[48] = 0; ui37->uFluxCapControl[49] = 0; ui37->uFluxCapControl[50] = 0; ui37->uFluxCapControl[51] = 0; ui37->uFluxCapControl[52] = 0; ui37->uFluxCapControl[53] = 0; ui37->uFluxCapControl[54] = 0; ui37->uFluxCapControl[55] = 0; ui37->uFluxCapControl[56] = 0; ui37->uFluxCapControl[57] = 0; ui37->uFluxCapControl[58] = 0; ui37->uFluxCapControl[59] = 0; ui37->uFluxCapControl[60] = 0; ui37->uFluxCapControl[61] = 0; ui37->uFluxCapControl[62] = 0; ui37->uFluxCapControl[63] = 0; 
	ui37->fFluxCapData[0] = 0.000000; ui37->fFluxCapData[1] = 0.000000; ui37->fFluxCapData[2] = 0.000000; ui37->fFluxCapData[3] = 0.000000; ui37->fFluxCapData[4] = 0.000000; ui37->fFluxCapData[5] = 0.000000; ui37->fFluxCapData[6] = 0.000000; ui37->fFluxCapData[7] = 0.000000; ui37->fFluxCapData[8] = 0.000000; ui37->fFluxCapData[9] = 0.000000; ui37->fFluxCapData[10] = 0.000000; ui37->fFluxCapData[11] = 0.000000; ui37->fFluxCapData[12] = 0.000000; ui37->fFluxCapData[13] = 0.000000; ui37->fFluxCapData[14] = 0.000000; ui37->fFluxCapData[15] = 0.000000; ui37->fFluxCapData[16] = 0.000000; ui37->fFluxCapData[17] = 0.000000; ui37->fFluxCapData[18] = 0.000000; ui37->fFluxCapData[19] = 0.000000; ui37->fFluxCapData[20] = 0.000000; ui37->fFluxCapData[21] = 0.000000; ui37->fFluxCapData[22] = 0.000000; ui37->fFluxCapData[23] = 0.000000; ui37->fFluxCapData[24] = 0.000000; ui37->fFluxCapData[25] = 0.000000; ui37->fFluxCapData[26] = 0.000000; ui37->fFluxCapData[27] = 0.000000; ui37->fFluxCapData[28] = 0.000000; ui37->fFluxCapData[29] = 0.000000; ui37->fFluxCapData[30] = 0.000000; ui37->fFluxCapData[31] = 0.000000; ui37->fFluxCapData[32] = 0.000000; ui37->fFluxCapData[33] = 0.000000; ui37->fFluxCapData[34] = 0.000000; ui37->fFluxCapData[35] = 0.000000; ui37->fFluxCapData[36] = 0.000000; ui37->fFluxCapData[37] = 0.000000; ui37->fFluxCapData[38] = 0.000000; ui37->fFluxCapData[39] = 0.000000; ui37->fFluxCapData[40] = 0.000000; ui37->fFluxCapData[41] = 0.000000; ui37->fFluxCapData[42] = 0.000000; ui37->fFluxCapData[43] = 0.000000; ui37->fFluxCapData[44] = 0.000000; ui37->fFluxCapData[45] = 0.000000; ui37->fFluxCapData[46] = 0.000000; ui37->fFluxCapData[47] = 0.000000; ui37->fFluxCapData[48] = 0.000000; ui37->fFluxCapData[49] = 0.000000; ui37->fFluxCapData[50] = 0.000000; ui37->fFluxCapData[51] = 0.000000; ui37->fFluxCapData[52] = 0.000000; ui37->fFluxCapData[53] = 0.000000; ui37->fFluxCapData[54] = 0.000000; ui37->fFluxCapData[55] = 0.000000; ui37->fFluxCapData[56] = 0.000000; ui37->fFluxCapData[57] = 0.000000; ui37->fFluxCapData[58] = 0.000000; ui37->fFluxCapData[59] = 0.000000; ui37->fFluxCapData[60] = 0.000000; ui37->fFluxCapData[61] = 0.000000; ui37->fFluxCapData[62] = 0.000000; ui37->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui37);
	delete ui37;


	m_fBitRate = 512;
	CUICtrl* ui38 = new CUICtrl;
	ui38->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui38->uControlId = 132;
	ui38->bLogSlider = false;
	ui38->bExpSlider = false;
	ui38->fUserDisplayDataLoLimit = 10.000000;
	ui38->fUserDisplayDataHiLimit = 512.000000;
	ui38->uUserDataType = intData;
	ui38->fInitUserIntValue = 512.000000;
	ui38->fInitUserFloatValue = 0;
	ui38->fInitUserDoubleValue = 0;
	ui38->fInitUserUINTValue = 0;
	ui38->m_pUserCookedIntData = &m_fBitRate;
	ui38->m_pUserCookedFloatData = NULL;
	ui38->m_pUserCookedDoubleData = NULL;
	ui38->m_pUserCookedUINTData = NULL;
	ui38->cControlUnits = "Units";
	ui38->cVariableName = "m_fBitRate";
	ui38->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui38->dPresetData[0] = 512.000000;ui38->dPresetData[1] = 75.000000;ui38->dPresetData[2] = 0.000000;ui38->dPresetData[3] = 0.000000;ui38->dPresetData[4] = 0.000000;ui38->dPresetData[5] = 0.000000;ui38->dPresetData[6] = 0.000000;ui38->dPresetData[7] = 0.000000;ui38->dPresetData[8] = 0.000000;ui38->dPresetData[9] = 0.000000;ui38->dPresetData[10] = 0.000000;ui38->dPresetData[11] = 0.000000;ui38->dPresetData[12] = 0.000000;ui38->dPresetData[13] = 0.000000;ui38->dPresetData[14] = 0.000000;ui38->dPresetData[15] = 0.000000;
	ui38->cControlName = "bitRate";
	ui38->bOwnerControl = false;
	ui38->bMIDIControl = false;
	ui38->uMIDIControlCommand = 176;
	ui38->uMIDIControlName = 3;
	ui38->uMIDIControlChannel = 0;
	ui38->nGUIRow = nIndexer++;
	ui38->nGUIColumn = -1;
	ui38->bEnableParamSmoothing = false;
	ui38->fSmoothingTimeInMs = 100.00;
	ui38->uControlTheme[0] = 0; ui38->uControlTheme[1] = 0; ui38->uControlTheme[2] = 0; ui38->uControlTheme[3] = 0; ui38->uControlTheme[4] = 0; ui38->uControlTheme[5] = 0; ui38->uControlTheme[6] = 0; ui38->uControlTheme[7] = 0; ui38->uControlTheme[8] = 0; ui38->uControlTheme[9] = 0; ui38->uControlTheme[10] = 0; ui38->uControlTheme[11] = 0; ui38->uControlTheme[12] = 0; ui38->uControlTheme[13] = 0; ui38->uControlTheme[14] = 0; ui38->uControlTheme[15] = 0; ui38->uControlTheme[16] = 2; ui38->uControlTheme[17] = 0; ui38->uControlTheme[18] = 0; ui38->uControlTheme[19] = 0; ui38->uControlTheme[20] = 0; ui38->uControlTheme[21] = 0; ui38->uControlTheme[22] = 0; ui38->uControlTheme[23] = 0; ui38->uControlTheme[24] = 0; ui38->uControlTheme[25] = 0; ui38->uControlTheme[26] = 0; ui38->uControlTheme[27] = 0; ui38->uControlTheme[28] = 0; ui38->uControlTheme[29] = 0; ui38->uControlTheme[30] = 0; ui38->uControlTheme[31] = 0; 
	ui38->uFluxCapControl[0] = 0; ui38->uFluxCapControl[1] = 0; ui38->uFluxCapControl[2] = 0; ui38->uFluxCapControl[3] = 0; ui38->uFluxCapControl[4] = 0; ui38->uFluxCapControl[5] = 0; ui38->uFluxCapControl[6] = 0; ui38->uFluxCapControl[7] = 0; ui38->uFluxCapControl[8] = 0; ui38->uFluxCapControl[9] = 0; ui38->uFluxCapControl[10] = 0; ui38->uFluxCapControl[11] = 0; ui38->uFluxCapControl[12] = 0; ui38->uFluxCapControl[13] = 0; ui38->uFluxCapControl[14] = 0; ui38->uFluxCapControl[15] = 0; ui38->uFluxCapControl[16] = 0; ui38->uFluxCapControl[17] = 0; ui38->uFluxCapControl[18] = 0; ui38->uFluxCapControl[19] = 0; ui38->uFluxCapControl[20] = 0; ui38->uFluxCapControl[21] = 0; ui38->uFluxCapControl[22] = 0; ui38->uFluxCapControl[23] = 0; ui38->uFluxCapControl[24] = 0; ui38->uFluxCapControl[25] = 0; ui38->uFluxCapControl[26] = 0; ui38->uFluxCapControl[27] = 0; ui38->uFluxCapControl[28] = 0; ui38->uFluxCapControl[29] = 0; ui38->uFluxCapControl[30] = 0; ui38->uFluxCapControl[31] = 0; ui38->uFluxCapControl[32] = 0; ui38->uFluxCapControl[33] = 0; ui38->uFluxCapControl[34] = 0; ui38->uFluxCapControl[35] = 0; ui38->uFluxCapControl[36] = 0; ui38->uFluxCapControl[37] = 0; ui38->uFluxCapControl[38] = 0; ui38->uFluxCapControl[39] = 0; ui38->uFluxCapControl[40] = 0; ui38->uFluxCapControl[41] = 0; ui38->uFluxCapControl[42] = 0; ui38->uFluxCapControl[43] = 0; ui38->uFluxCapControl[44] = 0; ui38->uFluxCapControl[45] = 0; ui38->uFluxCapControl[46] = 0; ui38->uFluxCapControl[47] = 0; ui38->uFluxCapControl[48] = 0; ui38->uFluxCapControl[49] = 0; ui38->uFluxCapControl[50] = 0; ui38->uFluxCapControl[51] = 0; ui38->uFluxCapControl[52] = 0; ui38->uFluxCapControl[53] = 0; ui38->uFluxCapControl[54] = 0; ui38->uFluxCapControl[55] = 0; ui38->uFluxCapControl[56] = 0; ui38->uFluxCapControl[57] = 0; ui38->uFluxCapControl[58] = 0; ui38->uFluxCapControl[59] = 0; ui38->uFluxCapControl[60] = 0; ui38->uFluxCapControl[61] = 0; ui38->uFluxCapControl[62] = 0; ui38->uFluxCapControl[63] = 0; 
	ui38->fFluxCapData[0] = 0.000000; ui38->fFluxCapData[1] = 0.000000; ui38->fFluxCapData[2] = 0.000000; ui38->fFluxCapData[3] = 0.000000; ui38->fFluxCapData[4] = 0.000000; ui38->fFluxCapData[5] = 0.000000; ui38->fFluxCapData[6] = 0.000000; ui38->fFluxCapData[7] = 0.000000; ui38->fFluxCapData[8] = 0.000000; ui38->fFluxCapData[9] = 0.000000; ui38->fFluxCapData[10] = 0.000000; ui38->fFluxCapData[11] = 0.000000; ui38->fFluxCapData[12] = 0.000000; ui38->fFluxCapData[13] = 0.000000; ui38->fFluxCapData[14] = 0.000000; ui38->fFluxCapData[15] = 0.000000; ui38->fFluxCapData[16] = 0.000000; ui38->fFluxCapData[17] = 0.000000; ui38->fFluxCapData[18] = 0.000000; ui38->fFluxCapData[19] = 0.000000; ui38->fFluxCapData[20] = 0.000000; ui38->fFluxCapData[21] = 0.000000; ui38->fFluxCapData[22] = 0.000000; ui38->fFluxCapData[23] = 0.000000; ui38->fFluxCapData[24] = 0.000000; ui38->fFluxCapData[25] = 0.000000; ui38->fFluxCapData[26] = 0.000000; ui38->fFluxCapData[27] = 0.000000; ui38->fFluxCapData[28] = 0.000000; ui38->fFluxCapData[29] = 0.000000; ui38->fFluxCapData[30] = 0.000000; ui38->fFluxCapData[31] = 0.000000; ui38->fFluxCapData[32] = 0.000000; ui38->fFluxCapData[33] = 0.000000; ui38->fFluxCapData[34] = 0.000000; ui38->fFluxCapData[35] = 0.000000; ui38->fFluxCapData[36] = 0.000000; ui38->fFluxCapData[37] = 0.000000; ui38->fFluxCapData[38] = 0.000000; ui38->fFluxCapData[39] = 0.000000; ui38->fFluxCapData[40] = 0.000000; ui38->fFluxCapData[41] = 0.000000; ui38->fFluxCapData[42] = 0.000000; ui38->fFluxCapData[43] = 0.000000; ui38->fFluxCapData[44] = 0.000000; ui38->fFluxCapData[45] = 0.000000; ui38->fFluxCapData[46] = 0.000000; ui38->fFluxCapData[47] = 0.000000; ui38->fFluxCapData[48] = 0.000000; ui38->fFluxCapData[49] = 0.000000; ui38->fFluxCapData[50] = 0.000000; ui38->fFluxCapData[51] = 0.000000; ui38->fFluxCapData[52] = 0.000000; ui38->fFluxCapData[53] = 0.000000; ui38->fFluxCapData[54] = 0.000000; ui38->fFluxCapData[55] = 0.000000; ui38->fFluxCapData[56] = 0.000000; ui38->fFluxCapData[57] = 0.000000; ui38->fFluxCapData[58] = 0.000000; ui38->fFluxCapData[59] = 0.000000; ui38->fFluxCapData[60] = 0.000000; ui38->fFluxCapData[61] = 0.000000; ui38->fFluxCapData[62] = 0.000000; ui38->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui38);
	delete ui38;


	m_fDownSampling = 1;
	CUICtrl* ui39 = new CUICtrl;
	ui39->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui39->uControlId = 133;
	ui39->bLogSlider = false;
	ui39->bExpSlider = false;
	ui39->fUserDisplayDataLoLimit = 1.000000;
	ui39->fUserDisplayDataHiLimit = 100.000000;
	ui39->uUserDataType = intData;
	ui39->fInitUserIntValue = 1.000000;
	ui39->fInitUserFloatValue = 0;
	ui39->fInitUserDoubleValue = 0;
	ui39->fInitUserUINTValue = 0;
	ui39->m_pUserCookedIntData = &m_fDownSampling;
	ui39->m_pUserCookedFloatData = NULL;
	ui39->m_pUserCookedDoubleData = NULL;
	ui39->m_pUserCookedUINTData = NULL;
	ui39->cControlUnits = "Units";
	ui39->cVariableName = "m_fDownSampling";
	ui39->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui39->dPresetData[0] = 1.000000;ui39->dPresetData[1] = 7.000000;ui39->dPresetData[2] = 0.000000;ui39->dPresetData[3] = 0.000000;ui39->dPresetData[4] = 0.000000;ui39->dPresetData[5] = 0.000000;ui39->dPresetData[6] = 0.000000;ui39->dPresetData[7] = 0.000000;ui39->dPresetData[8] = 0.000000;ui39->dPresetData[9] = 0.000000;ui39->dPresetData[10] = 0.000000;ui39->dPresetData[11] = 0.000000;ui39->dPresetData[12] = 0.000000;ui39->dPresetData[13] = 0.000000;ui39->dPresetData[14] = 0.000000;ui39->dPresetData[15] = 0.000000;
	ui39->cControlName = "Downsampling Ratio";
	ui39->bOwnerControl = false;
	ui39->bMIDIControl = false;
	ui39->uMIDIControlCommand = 176;
	ui39->uMIDIControlName = 3;
	ui39->uMIDIControlChannel = 0;
	ui39->nGUIRow = nIndexer++;
	ui39->nGUIColumn = -1;
	ui39->bEnableParamSmoothing = false;
	ui39->fSmoothingTimeInMs = 100.00;
	ui39->uControlTheme[0] = 0; ui39->uControlTheme[1] = 0; ui39->uControlTheme[2] = 0; ui39->uControlTheme[3] = 0; ui39->uControlTheme[4] = 0; ui39->uControlTheme[5] = 0; ui39->uControlTheme[6] = 0; ui39->uControlTheme[7] = 0; ui39->uControlTheme[8] = 0; ui39->uControlTheme[9] = 0; ui39->uControlTheme[10] = 0; ui39->uControlTheme[11] = 0; ui39->uControlTheme[12] = 0; ui39->uControlTheme[13] = 0; ui39->uControlTheme[14] = 0; ui39->uControlTheme[15] = 0; ui39->uControlTheme[16] = 2; ui39->uControlTheme[17] = 0; ui39->uControlTheme[18] = 0; ui39->uControlTheme[19] = 0; ui39->uControlTheme[20] = 0; ui39->uControlTheme[21] = 0; ui39->uControlTheme[22] = 0; ui39->uControlTheme[23] = 0; ui39->uControlTheme[24] = 0; ui39->uControlTheme[25] = 0; ui39->uControlTheme[26] = 0; ui39->uControlTheme[27] = 0; ui39->uControlTheme[28] = 0; ui39->uControlTheme[29] = 0; ui39->uControlTheme[30] = 0; ui39->uControlTheme[31] = 0; 
	ui39->uFluxCapControl[0] = 0; ui39->uFluxCapControl[1] = 0; ui39->uFluxCapControl[2] = 0; ui39->uFluxCapControl[3] = 0; ui39->uFluxCapControl[4] = 0; ui39->uFluxCapControl[5] = 0; ui39->uFluxCapControl[6] = 0; ui39->uFluxCapControl[7] = 0; ui39->uFluxCapControl[8] = 0; ui39->uFluxCapControl[9] = 0; ui39->uFluxCapControl[10] = 0; ui39->uFluxCapControl[11] = 0; ui39->uFluxCapControl[12] = 0; ui39->uFluxCapControl[13] = 0; ui39->uFluxCapControl[14] = 0; ui39->uFluxCapControl[15] = 0; ui39->uFluxCapControl[16] = 0; ui39->uFluxCapControl[17] = 0; ui39->uFluxCapControl[18] = 0; ui39->uFluxCapControl[19] = 0; ui39->uFluxCapControl[20] = 0; ui39->uFluxCapControl[21] = 0; ui39->uFluxCapControl[22] = 0; ui39->uFluxCapControl[23] = 0; ui39->uFluxCapControl[24] = 0; ui39->uFluxCapControl[25] = 0; ui39->uFluxCapControl[26] = 0; ui39->uFluxCapControl[27] = 0; ui39->uFluxCapControl[28] = 0; ui39->uFluxCapControl[29] = 0; ui39->uFluxCapControl[30] = 0; ui39->uFluxCapControl[31] = 0; ui39->uFluxCapControl[32] = 0; ui39->uFluxCapControl[33] = 0; ui39->uFluxCapControl[34] = 0; ui39->uFluxCapControl[35] = 0; ui39->uFluxCapControl[36] = 0; ui39->uFluxCapControl[37] = 0; ui39->uFluxCapControl[38] = 0; ui39->uFluxCapControl[39] = 0; ui39->uFluxCapControl[40] = 0; ui39->uFluxCapControl[41] = 0; ui39->uFluxCapControl[42] = 0; ui39->uFluxCapControl[43] = 0; ui39->uFluxCapControl[44] = 0; ui39->uFluxCapControl[45] = 0; ui39->uFluxCapControl[46] = 0; ui39->uFluxCapControl[47] = 0; ui39->uFluxCapControl[48] = 0; ui39->uFluxCapControl[49] = 0; ui39->uFluxCapControl[50] = 0; ui39->uFluxCapControl[51] = 0; ui39->uFluxCapControl[52] = 0; ui39->uFluxCapControl[53] = 0; ui39->uFluxCapControl[54] = 0; ui39->uFluxCapControl[55] = 0; ui39->uFluxCapControl[56] = 0; ui39->uFluxCapControl[57] = 0; ui39->uFluxCapControl[58] = 0; ui39->uFluxCapControl[59] = 0; ui39->uFluxCapControl[60] = 0; ui39->uFluxCapControl[61] = 0; ui39->uFluxCapControl[62] = 0; ui39->uFluxCapControl[63] = 0; 
	ui39->fFluxCapData[0] = 0.000000; ui39->fFluxCapData[1] = 0.000000; ui39->fFluxCapData[2] = 0.000000; ui39->fFluxCapData[3] = 0.000000; ui39->fFluxCapData[4] = 0.000000; ui39->fFluxCapData[5] = 0.000000; ui39->fFluxCapData[6] = 0.000000; ui39->fFluxCapData[7] = 0.000000; ui39->fFluxCapData[8] = 0.000000; ui39->fFluxCapData[9] = 0.000000; ui39->fFluxCapData[10] = 0.000000; ui39->fFluxCapData[11] = 0.000000; ui39->fFluxCapData[12] = 0.000000; ui39->fFluxCapData[13] = 0.000000; ui39->fFluxCapData[14] = 0.000000; ui39->fFluxCapData[15] = 0.000000; ui39->fFluxCapData[16] = 0.000000; ui39->fFluxCapData[17] = 0.000000; ui39->fFluxCapData[18] = 0.000000; ui39->fFluxCapData[19] = 0.000000; ui39->fFluxCapData[20] = 0.000000; ui39->fFluxCapData[21] = 0.000000; ui39->fFluxCapData[22] = 0.000000; ui39->fFluxCapData[23] = 0.000000; ui39->fFluxCapData[24] = 0.000000; ui39->fFluxCapData[25] = 0.000000; ui39->fFluxCapData[26] = 0.000000; ui39->fFluxCapData[27] = 0.000000; ui39->fFluxCapData[28] = 0.000000; ui39->fFluxCapData[29] = 0.000000; ui39->fFluxCapData[30] = 0.000000; ui39->fFluxCapData[31] = 0.000000; ui39->fFluxCapData[32] = 0.000000; ui39->fFluxCapData[33] = 0.000000; ui39->fFluxCapData[34] = 0.000000; ui39->fFluxCapData[35] = 0.000000; ui39->fFluxCapData[36] = 0.000000; ui39->fFluxCapData[37] = 0.000000; ui39->fFluxCapData[38] = 0.000000; ui39->fFluxCapData[39] = 0.000000; ui39->fFluxCapData[40] = 0.000000; ui39->fFluxCapData[41] = 0.000000; ui39->fFluxCapData[42] = 0.000000; ui39->fFluxCapData[43] = 0.000000; ui39->fFluxCapData[44] = 0.000000; ui39->fFluxCapData[45] = 0.000000; ui39->fFluxCapData[46] = 0.000000; ui39->fFluxCapData[47] = 0.000000; ui39->fFluxCapData[48] = 0.000000; ui39->fFluxCapData[49] = 0.000000; ui39->fFluxCapData[50] = 0.000000; ui39->fFluxCapData[51] = 0.000000; ui39->fFluxCapData[52] = 0.000000; ui39->fFluxCapData[53] = 0.000000; ui39->fFluxCapData[54] = 0.000000; ui39->fFluxCapData[55] = 0.000000; ui39->fFluxCapData[56] = 0.000000; ui39->fFluxCapData[57] = 0.000000; ui39->fFluxCapData[58] = 0.000000; ui39->fFluxCapData[59] = 0.000000; ui39->fFluxCapData[60] = 0.000000; ui39->fFluxCapData[61] = 0.000000; ui39->fFluxCapData[62] = 0.000000; ui39->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui39);
	delete ui39;


	m_fBitDepth = 32;
	CUICtrl* ui40 = new CUICtrl;
	ui40->uControlType = FILTER_CONTROL_CONTINUOUSLY_VARIABLE;
	ui40->uControlId = 134;
	ui40->bLogSlider = false;
	ui40->bExpSlider = false;
	ui40->fUserDisplayDataLoLimit = 1.000000;
	ui40->fUserDisplayDataHiLimit = 32.000000;
	ui40->uUserDataType = intData;
	ui40->fInitUserIntValue = 32.000000;
	ui40->fInitUserFloatValue = 0;
	ui40->fInitUserDoubleValue = 0;
	ui40->fInitUserUINTValue = 0;
	ui40->m_pUserCookedIntData = &m_fBitDepth;
	ui40->m_pUserCookedFloatData = NULL;
	ui40->m_pUserCookedDoubleData = NULL;
	ui40->m_pUserCookedUINTData = NULL;
	ui40->cControlUnits = "Units";
	ui40->cVariableName = "m_fBitDepth";
	ui40->cEnumeratedList = "SEL1,SEL2,SEL3";
	ui40->dPresetData[0] = 32.000000;ui40->dPresetData[1] = 2.000000;ui40->dPresetData[2] = 0.000000;ui40->dPresetData[3] = 0.000000;ui40->dPresetData[4] = 0.000000;ui40->dPresetData[5] = 0.000000;ui40->dPresetData[6] = 0.000000;ui40->dPresetData[7] = 0.000000;ui40->dPresetData[8] = 0.000000;ui40->dPresetData[9] = 0.000000;ui40->dPresetData[10] = 0.000000;ui40->dPresetData[11] = 0.000000;ui40->dPresetData[12] = 0.000000;ui40->dPresetData[13] = 0.000000;ui40->dPresetData[14] = 0.000000;ui40->dPresetData[15] = 0.000000;
	ui40->cControlName = "Bit Depth";
	ui40->bOwnerControl = false;
	ui40->bMIDIControl = false;
	ui40->uMIDIControlCommand = 176;
	ui40->uMIDIControlName = 3;
	ui40->uMIDIControlChannel = 0;
	ui40->nGUIRow = nIndexer++;
	ui40->nGUIColumn = -1;
	ui40->bEnableParamSmoothing = false;
	ui40->fSmoothingTimeInMs = 100.00;
	ui40->uControlTheme[0] = 0; ui40->uControlTheme[1] = 0; ui40->uControlTheme[2] = 0; ui40->uControlTheme[3] = 0; ui40->uControlTheme[4] = 0; ui40->uControlTheme[5] = 0; ui40->uControlTheme[6] = 0; ui40->uControlTheme[7] = 0; ui40->uControlTheme[8] = 0; ui40->uControlTheme[9] = 0; ui40->uControlTheme[10] = 0; ui40->uControlTheme[11] = 0; ui40->uControlTheme[12] = 0; ui40->uControlTheme[13] = 0; ui40->uControlTheme[14] = 0; ui40->uControlTheme[15] = 0; ui40->uControlTheme[16] = 2; ui40->uControlTheme[17] = 0; ui40->uControlTheme[18] = 0; ui40->uControlTheme[19] = 0; ui40->uControlTheme[20] = 0; ui40->uControlTheme[21] = 0; ui40->uControlTheme[22] = 0; ui40->uControlTheme[23] = 0; ui40->uControlTheme[24] = 0; ui40->uControlTheme[25] = 0; ui40->uControlTheme[26] = 0; ui40->uControlTheme[27] = 0; ui40->uControlTheme[28] = 0; ui40->uControlTheme[29] = 0; ui40->uControlTheme[30] = 0; ui40->uControlTheme[31] = 0; 
	ui40->uFluxCapControl[0] = 0; ui40->uFluxCapControl[1] = 0; ui40->uFluxCapControl[2] = 0; ui40->uFluxCapControl[3] = 0; ui40->uFluxCapControl[4] = 0; ui40->uFluxCapControl[5] = 0; ui40->uFluxCapControl[6] = 0; ui40->uFluxCapControl[7] = 0; ui40->uFluxCapControl[8] = 0; ui40->uFluxCapControl[9] = 0; ui40->uFluxCapControl[10] = 0; ui40->uFluxCapControl[11] = 0; ui40->uFluxCapControl[12] = 0; ui40->uFluxCapControl[13] = 0; ui40->uFluxCapControl[14] = 0; ui40->uFluxCapControl[15] = 0; ui40->uFluxCapControl[16] = 0; ui40->uFluxCapControl[17] = 0; ui40->uFluxCapControl[18] = 0; ui40->uFluxCapControl[19] = 0; ui40->uFluxCapControl[20] = 0; ui40->uFluxCapControl[21] = 0; ui40->uFluxCapControl[22] = 0; ui40->uFluxCapControl[23] = 0; ui40->uFluxCapControl[24] = 0; ui40->uFluxCapControl[25] = 0; ui40->uFluxCapControl[26] = 0; ui40->uFluxCapControl[27] = 0; ui40->uFluxCapControl[28] = 0; ui40->uFluxCapControl[29] = 0; ui40->uFluxCapControl[30] = 0; ui40->uFluxCapControl[31] = 0; ui40->uFluxCapControl[32] = 0; ui40->uFluxCapControl[33] = 0; ui40->uFluxCapControl[34] = 0; ui40->uFluxCapControl[35] = 0; ui40->uFluxCapControl[36] = 0; ui40->uFluxCapControl[37] = 0; ui40->uFluxCapControl[38] = 0; ui40->uFluxCapControl[39] = 0; ui40->uFluxCapControl[40] = 0; ui40->uFluxCapControl[41] = 0; ui40->uFluxCapControl[42] = 0; ui40->uFluxCapControl[43] = 0; ui40->uFluxCapControl[44] = 0; ui40->uFluxCapControl[45] = 0; ui40->uFluxCapControl[46] = 0; ui40->uFluxCapControl[47] = 0; ui40->uFluxCapControl[48] = 0; ui40->uFluxCapControl[49] = 0; ui40->uFluxCapControl[50] = 0; ui40->uFluxCapControl[51] = 0; ui40->uFluxCapControl[52] = 0; ui40->uFluxCapControl[53] = 0; ui40->uFluxCapControl[54] = 0; ui40->uFluxCapControl[55] = 0; ui40->uFluxCapControl[56] = 0; ui40->uFluxCapControl[57] = 0; ui40->uFluxCapControl[58] = 0; ui40->uFluxCapControl[59] = 0; ui40->uFluxCapControl[60] = 0; ui40->uFluxCapControl[61] = 0; ui40->uFluxCapControl[62] = 0; ui40->uFluxCapControl[63] = 0; 
	ui40->fFluxCapData[0] = 0.000000; ui40->fFluxCapData[1] = 0.000000; ui40->fFluxCapData[2] = 0.000000; ui40->fFluxCapData[3] = 0.000000; ui40->fFluxCapData[4] = 0.000000; ui40->fFluxCapData[5] = 0.000000; ui40->fFluxCapData[6] = 0.000000; ui40->fFluxCapData[7] = 0.000000; ui40->fFluxCapData[8] = 0.000000; ui40->fFluxCapData[9] = 0.000000; ui40->fFluxCapData[10] = 0.000000; ui40->fFluxCapData[11] = 0.000000; ui40->fFluxCapData[12] = 0.000000; ui40->fFluxCapData[13] = 0.000000; ui40->fFluxCapData[14] = 0.000000; ui40->fFluxCapData[15] = 0.000000; ui40->fFluxCapData[16] = 0.000000; ui40->fFluxCapData[17] = 0.000000; ui40->fFluxCapData[18] = 0.000000; ui40->fFluxCapData[19] = 0.000000; ui40->fFluxCapData[20] = 0.000000; ui40->fFluxCapData[21] = 0.000000; ui40->fFluxCapData[22] = 0.000000; ui40->fFluxCapData[23] = 0.000000; ui40->fFluxCapData[24] = 0.000000; ui40->fFluxCapData[25] = 0.000000; ui40->fFluxCapData[26] = 0.000000; ui40->fFluxCapData[27] = 0.000000; ui40->fFluxCapData[28] = 0.000000; ui40->fFluxCapData[29] = 0.000000; ui40->fFluxCapData[30] = 0.000000; ui40->fFluxCapData[31] = 0.000000; ui40->fFluxCapData[32] = 0.000000; ui40->fFluxCapData[33] = 0.000000; ui40->fFluxCapData[34] = 0.000000; ui40->fFluxCapData[35] = 0.000000; ui40->fFluxCapData[36] = 0.000000; ui40->fFluxCapData[37] = 0.000000; ui40->fFluxCapData[38] = 0.000000; ui40->fFluxCapData[39] = 0.000000; ui40->fFluxCapData[40] = 0.000000; ui40->fFluxCapData[41] = 0.000000; ui40->fFluxCapData[42] = 0.000000; ui40->fFluxCapData[43] = 0.000000; ui40->fFluxCapData[44] = 0.000000; ui40->fFluxCapData[45] = 0.000000; ui40->fFluxCapData[46] = 0.000000; ui40->fFluxCapData[47] = 0.000000; ui40->fFluxCapData[48] = 0.000000; ui40->fFluxCapData[49] = 0.000000; ui40->fFluxCapData[50] = 0.000000; ui40->fFluxCapData[51] = 0.000000; ui40->fFluxCapData[52] = 0.000000; ui40->fFluxCapData[53] = 0.000000; ui40->fFluxCapData[54] = 0.000000; ui40->fFluxCapData[55] = 0.000000; ui40->fFluxCapData[56] = 0.000000; ui40->fFluxCapData[57] = 0.000000; ui40->fFluxCapData[58] = 0.000000; ui40->fFluxCapData[59] = 0.000000; ui40->fFluxCapData[60] = 0.000000; ui40->fFluxCapData[61] = 0.000000; ui40->fFluxCapData[62] = 0.000000; ui40->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui40);
	delete ui40;


	m_uSwitch1 = 1;
	CUICtrl* ui41 = new CUICtrl;
	ui41->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui41->uControlId = 45;
	ui41->bLogSlider = false;
	ui41->bExpSlider = false;
	ui41->fUserDisplayDataLoLimit = 0.000000;
	ui41->fUserDisplayDataHiLimit = 1.000000;
	ui41->uUserDataType = UINTData;
	ui41->fInitUserIntValue = 0;
	ui41->fInitUserFloatValue = 0;
	ui41->fInitUserDoubleValue = 0;
	ui41->fInitUserUINTValue = 1.000000;
	ui41->m_pUserCookedIntData = NULL;
	ui41->m_pUserCookedFloatData = NULL;
	ui41->m_pUserCookedDoubleData = NULL;
	ui41->m_pUserCookedUINTData = &m_uSwitch1;
	ui41->cControlUnits = "";
	ui41->cVariableName = "m_uSwitch1";
	ui41->cEnumeratedList = "SWITCH_OFF,SWITCH_ON";
	ui41->dPresetData[0] = 1.000000;ui41->dPresetData[1] = 1.000000;ui41->dPresetData[2] = 0.000000;ui41->dPresetData[3] = 0.000000;ui41->dPresetData[4] = 0.000000;ui41->dPresetData[5] = 0.000000;ui41->dPresetData[6] = 0.000000;ui41->dPresetData[7] = 0.000000;ui41->dPresetData[8] = 0.000000;ui41->dPresetData[9] = 0.000000;ui41->dPresetData[10] = 0.000000;ui41->dPresetData[11] = 0.000000;ui41->dPresetData[12] = 0.000000;ui41->dPresetData[13] = 0.000000;ui41->dPresetData[14] = 0.000000;ui41->dPresetData[15] = 0.000000;
	ui41->cControlName = "Bitcrusher";
	ui41->bOwnerControl = false;
	ui41->bMIDIControl = false;
	ui41->uMIDIControlCommand = 176;
	ui41->uMIDIControlName = 3;
	ui41->uMIDIControlChannel = 0;
	ui41->nGUIRow = nIndexer++;
	ui41->nGUIColumn = -1;
	ui41->bEnableParamSmoothing = false;
	ui41->fSmoothingTimeInMs = 100.0;
	ui41->uControlTheme[0] = 0; ui41->uControlTheme[1] = 0; ui41->uControlTheme[2] = 0; ui41->uControlTheme[3] = 8355711; ui41->uControlTheme[4] = 139; ui41->uControlTheme[5] = 0; ui41->uControlTheme[6] = 0; ui41->uControlTheme[7] = 0; ui41->uControlTheme[8] = 0; ui41->uControlTheme[9] = 0; ui41->uControlTheme[10] = 0; ui41->uControlTheme[11] = 0; ui41->uControlTheme[12] = 0; ui41->uControlTheme[13] = 0; ui41->uControlTheme[14] = 0; ui41->uControlTheme[15] = 0; ui41->uControlTheme[16] = 0; ui41->uControlTheme[17] = 0; ui41->uControlTheme[18] = 0; ui41->uControlTheme[19] = 0; ui41->uControlTheme[20] = 0; ui41->uControlTheme[21] = 0; ui41->uControlTheme[22] = 0; ui41->uControlTheme[23] = 0; ui41->uControlTheme[24] = 0; ui41->uControlTheme[25] = 0; ui41->uControlTheme[26] = 0; ui41->uControlTheme[27] = 0; ui41->uControlTheme[28] = 0; ui41->uControlTheme[29] = 0; ui41->uControlTheme[30] = 0; ui41->uControlTheme[31] = 0; 
	ui41->uFluxCapControl[0] = 0; ui41->uFluxCapControl[1] = 0; ui41->uFluxCapControl[2] = 0; ui41->uFluxCapControl[3] = 0; ui41->uFluxCapControl[4] = 0; ui41->uFluxCapControl[5] = 0; ui41->uFluxCapControl[6] = 0; ui41->uFluxCapControl[7] = 0; ui41->uFluxCapControl[8] = 0; ui41->uFluxCapControl[9] = 0; ui41->uFluxCapControl[10] = 0; ui41->uFluxCapControl[11] = 0; ui41->uFluxCapControl[12] = 0; ui41->uFluxCapControl[13] = 0; ui41->uFluxCapControl[14] = 0; ui41->uFluxCapControl[15] = 0; ui41->uFluxCapControl[16] = 0; ui41->uFluxCapControl[17] = 0; ui41->uFluxCapControl[18] = 0; ui41->uFluxCapControl[19] = 0; ui41->uFluxCapControl[20] = 0; ui41->uFluxCapControl[21] = 0; ui41->uFluxCapControl[22] = 0; ui41->uFluxCapControl[23] = 0; ui41->uFluxCapControl[24] = 0; ui41->uFluxCapControl[25] = 0; ui41->uFluxCapControl[26] = 0; ui41->uFluxCapControl[27] = 0; ui41->uFluxCapControl[28] = 0; ui41->uFluxCapControl[29] = 0; ui41->uFluxCapControl[30] = 0; ui41->uFluxCapControl[31] = 0; ui41->uFluxCapControl[32] = 0; ui41->uFluxCapControl[33] = 0; ui41->uFluxCapControl[34] = 0; ui41->uFluxCapControl[35] = 0; ui41->uFluxCapControl[36] = 0; ui41->uFluxCapControl[37] = 0; ui41->uFluxCapControl[38] = 0; ui41->uFluxCapControl[39] = 0; ui41->uFluxCapControl[40] = 0; ui41->uFluxCapControl[41] = 0; ui41->uFluxCapControl[42] = 0; ui41->uFluxCapControl[43] = 0; ui41->uFluxCapControl[44] = 0; ui41->uFluxCapControl[45] = 0; ui41->uFluxCapControl[46] = 0; ui41->uFluxCapControl[47] = 0; ui41->uFluxCapControl[48] = 0; ui41->uFluxCapControl[49] = 0; ui41->uFluxCapControl[50] = 0; ui41->uFluxCapControl[51] = 0; ui41->uFluxCapControl[52] = 0; ui41->uFluxCapControl[53] = 0; ui41->uFluxCapControl[54] = 0; ui41->uFluxCapControl[55] = 0; ui41->uFluxCapControl[56] = 0; ui41->uFluxCapControl[57] = 0; ui41->uFluxCapControl[58] = 0; ui41->uFluxCapControl[59] = 0; ui41->uFluxCapControl[60] = 0; ui41->uFluxCapControl[61] = 0; ui41->uFluxCapControl[62] = 0; ui41->uFluxCapControl[63] = 0; 
	ui41->fFluxCapData[0] = 0.000000; ui41->fFluxCapData[1] = 0.000000; ui41->fFluxCapData[2] = 0.000000; ui41->fFluxCapData[3] = 0.000000; ui41->fFluxCapData[4] = 0.000000; ui41->fFluxCapData[5] = 0.000000; ui41->fFluxCapData[6] = 0.000000; ui41->fFluxCapData[7] = 0.000000; ui41->fFluxCapData[8] = 0.000000; ui41->fFluxCapData[9] = 0.000000; ui41->fFluxCapData[10] = 0.000000; ui41->fFluxCapData[11] = 0.000000; ui41->fFluxCapData[12] = 0.000000; ui41->fFluxCapData[13] = 0.000000; ui41->fFluxCapData[14] = 0.000000; ui41->fFluxCapData[15] = 0.000000; ui41->fFluxCapData[16] = 0.000000; ui41->fFluxCapData[17] = 0.000000; ui41->fFluxCapData[18] = 0.000000; ui41->fFluxCapData[19] = 0.000000; ui41->fFluxCapData[20] = 0.000000; ui41->fFluxCapData[21] = 0.000000; ui41->fFluxCapData[22] = 0.000000; ui41->fFluxCapData[23] = 0.000000; ui41->fFluxCapData[24] = 0.000000; ui41->fFluxCapData[25] = 0.000000; ui41->fFluxCapData[26] = 0.000000; ui41->fFluxCapData[27] = 0.000000; ui41->fFluxCapData[28] = 0.000000; ui41->fFluxCapData[29] = 0.000000; ui41->fFluxCapData[30] = 0.000000; ui41->fFluxCapData[31] = 0.000000; ui41->fFluxCapData[32] = 0.000000; ui41->fFluxCapData[33] = 0.000000; ui41->fFluxCapData[34] = 0.000000; ui41->fFluxCapData[35] = 0.000000; ui41->fFluxCapData[36] = 0.000000; ui41->fFluxCapData[37] = 0.000000; ui41->fFluxCapData[38] = 0.000000; ui41->fFluxCapData[39] = 0.000000; ui41->fFluxCapData[40] = 0.000000; ui41->fFluxCapData[41] = 0.000000; ui41->fFluxCapData[42] = 0.000000; ui41->fFluxCapData[43] = 0.000000; ui41->fFluxCapData[44] = 0.000000; ui41->fFluxCapData[45] = 0.000000; ui41->fFluxCapData[46] = 0.000000; ui41->fFluxCapData[47] = 0.000000; ui41->fFluxCapData[48] = 0.000000; ui41->fFluxCapData[49] = 0.000000; ui41->fFluxCapData[50] = 0.000000; ui41->fFluxCapData[51] = 0.000000; ui41->fFluxCapData[52] = 0.000000; ui41->fFluxCapData[53] = 0.000000; ui41->fFluxCapData[54] = 0.000000; ui41->fFluxCapData[55] = 0.000000; ui41->fFluxCapData[56] = 0.000000; ui41->fFluxCapData[57] = 0.000000; ui41->fFluxCapData[58] = 0.000000; ui41->fFluxCapData[59] = 0.000000; ui41->fFluxCapData[60] = 0.000000; ui41->fFluxCapData[61] = 0.000000; ui41->fFluxCapData[62] = 0.000000; ui41->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui41);
	delete ui41;


	m_uSwitch2 = 0;
	CUICtrl* ui42 = new CUICtrl;
	ui42->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui42->uControlId = 46;
	ui42->bLogSlider = false;
	ui42->bExpSlider = false;
	ui42->fUserDisplayDataLoLimit = 0.000000;
	ui42->fUserDisplayDataHiLimit = 1.000000;
	ui42->uUserDataType = UINTData;
	ui42->fInitUserIntValue = 0;
	ui42->fInitUserFloatValue = 0;
	ui42->fInitUserDoubleValue = 0;
	ui42->fInitUserUINTValue = 0.000000;
	ui42->m_pUserCookedIntData = NULL;
	ui42->m_pUserCookedFloatData = NULL;
	ui42->m_pUserCookedDoubleData = NULL;
	ui42->m_pUserCookedUINTData = &m_uSwitch2;
	ui42->cControlUnits = "";
	ui42->cVariableName = "m_uSwitch2";
	ui42->cEnumeratedList = "SWITCH_OFF,SWITCH_ON";
	ui42->dPresetData[0] = -0.000000;ui42->dPresetData[1] = 1.000000;ui42->dPresetData[2] = 0.000000;ui42->dPresetData[3] = 0.000000;ui42->dPresetData[4] = 0.000000;ui42->dPresetData[5] = 0.000000;ui42->dPresetData[6] = 0.000000;ui42->dPresetData[7] = 0.000000;ui42->dPresetData[8] = 0.000000;ui42->dPresetData[9] = 0.000000;ui42->dPresetData[10] = 0.000000;ui42->dPresetData[11] = 0.000000;ui42->dPresetData[12] = 0.000000;ui42->dPresetData[13] = 0.000000;ui42->dPresetData[14] = 0.000000;ui42->dPresetData[15] = 0.000000;
	ui42->cControlName = "Reverb";
	ui42->bOwnerControl = false;
	ui42->bMIDIControl = false;
	ui42->uMIDIControlCommand = 176;
	ui42->uMIDIControlName = 3;
	ui42->uMIDIControlChannel = 0;
	ui42->nGUIRow = nIndexer++;
	ui42->nGUIColumn = -1;
	ui42->bEnableParamSmoothing = false;
	ui42->fSmoothingTimeInMs = 100.0;
	ui42->uControlTheme[0] = 0; ui42->uControlTheme[1] = 0; ui42->uControlTheme[2] = 0; ui42->uControlTheme[3] = 8355711; ui42->uControlTheme[4] = 139; ui42->uControlTheme[5] = 0; ui42->uControlTheme[6] = 0; ui42->uControlTheme[7] = 0; ui42->uControlTheme[8] = 0; ui42->uControlTheme[9] = 0; ui42->uControlTheme[10] = 0; ui42->uControlTheme[11] = 0; ui42->uControlTheme[12] = 0; ui42->uControlTheme[13] = 0; ui42->uControlTheme[14] = 0; ui42->uControlTheme[15] = 0; ui42->uControlTheme[16] = 0; ui42->uControlTheme[17] = 0; ui42->uControlTheme[18] = 0; ui42->uControlTheme[19] = 0; ui42->uControlTheme[20] = 0; ui42->uControlTheme[21] = 0; ui42->uControlTheme[22] = 0; ui42->uControlTheme[23] = 0; ui42->uControlTheme[24] = 0; ui42->uControlTheme[25] = 0; ui42->uControlTheme[26] = 0; ui42->uControlTheme[27] = 0; ui42->uControlTheme[28] = 0; ui42->uControlTheme[29] = 0; ui42->uControlTheme[30] = 0; ui42->uControlTheme[31] = 0; 
	ui42->uFluxCapControl[0] = 0; ui42->uFluxCapControl[1] = 0; ui42->uFluxCapControl[2] = 0; ui42->uFluxCapControl[3] = 0; ui42->uFluxCapControl[4] = 0; ui42->uFluxCapControl[5] = 0; ui42->uFluxCapControl[6] = 0; ui42->uFluxCapControl[7] = 0; ui42->uFluxCapControl[8] = 0; ui42->uFluxCapControl[9] = 0; ui42->uFluxCapControl[10] = 0; ui42->uFluxCapControl[11] = 0; ui42->uFluxCapControl[12] = 0; ui42->uFluxCapControl[13] = 0; ui42->uFluxCapControl[14] = 0; ui42->uFluxCapControl[15] = 0; ui42->uFluxCapControl[16] = 0; ui42->uFluxCapControl[17] = 0; ui42->uFluxCapControl[18] = 0; ui42->uFluxCapControl[19] = 0; ui42->uFluxCapControl[20] = 0; ui42->uFluxCapControl[21] = 0; ui42->uFluxCapControl[22] = 0; ui42->uFluxCapControl[23] = 0; ui42->uFluxCapControl[24] = 0; ui42->uFluxCapControl[25] = 0; ui42->uFluxCapControl[26] = 0; ui42->uFluxCapControl[27] = 0; ui42->uFluxCapControl[28] = 0; ui42->uFluxCapControl[29] = 0; ui42->uFluxCapControl[30] = 0; ui42->uFluxCapControl[31] = 0; ui42->uFluxCapControl[32] = 0; ui42->uFluxCapControl[33] = 0; ui42->uFluxCapControl[34] = 0; ui42->uFluxCapControl[35] = 0; ui42->uFluxCapControl[36] = 0; ui42->uFluxCapControl[37] = 0; ui42->uFluxCapControl[38] = 0; ui42->uFluxCapControl[39] = 0; ui42->uFluxCapControl[40] = 0; ui42->uFluxCapControl[41] = 0; ui42->uFluxCapControl[42] = 0; ui42->uFluxCapControl[43] = 0; ui42->uFluxCapControl[44] = 0; ui42->uFluxCapControl[45] = 0; ui42->uFluxCapControl[46] = 0; ui42->uFluxCapControl[47] = 0; ui42->uFluxCapControl[48] = 0; ui42->uFluxCapControl[49] = 0; ui42->uFluxCapControl[50] = 0; ui42->uFluxCapControl[51] = 0; ui42->uFluxCapControl[52] = 0; ui42->uFluxCapControl[53] = 0; ui42->uFluxCapControl[54] = 0; ui42->uFluxCapControl[55] = 0; ui42->uFluxCapControl[56] = 0; ui42->uFluxCapControl[57] = 0; ui42->uFluxCapControl[58] = 0; ui42->uFluxCapControl[59] = 0; ui42->uFluxCapControl[60] = 0; ui42->uFluxCapControl[61] = 0; ui42->uFluxCapControl[62] = 0; ui42->uFluxCapControl[63] = 0; 
	ui42->fFluxCapData[0] = 0.000000; ui42->fFluxCapData[1] = 0.000000; ui42->fFluxCapData[2] = 0.000000; ui42->fFluxCapData[3] = 0.000000; ui42->fFluxCapData[4] = 0.000000; ui42->fFluxCapData[5] = 0.000000; ui42->fFluxCapData[6] = 0.000000; ui42->fFluxCapData[7] = 0.000000; ui42->fFluxCapData[8] = 0.000000; ui42->fFluxCapData[9] = 0.000000; ui42->fFluxCapData[10] = 0.000000; ui42->fFluxCapData[11] = 0.000000; ui42->fFluxCapData[12] = 0.000000; ui42->fFluxCapData[13] = 0.000000; ui42->fFluxCapData[14] = 0.000000; ui42->fFluxCapData[15] = 0.000000; ui42->fFluxCapData[16] = 0.000000; ui42->fFluxCapData[17] = 0.000000; ui42->fFluxCapData[18] = 0.000000; ui42->fFluxCapData[19] = 0.000000; ui42->fFluxCapData[20] = 0.000000; ui42->fFluxCapData[21] = 0.000000; ui42->fFluxCapData[22] = 0.000000; ui42->fFluxCapData[23] = 0.000000; ui42->fFluxCapData[24] = 0.000000; ui42->fFluxCapData[25] = 0.000000; ui42->fFluxCapData[26] = 0.000000; ui42->fFluxCapData[27] = 0.000000; ui42->fFluxCapData[28] = 0.000000; ui42->fFluxCapData[29] = 0.000000; ui42->fFluxCapData[30] = 0.000000; ui42->fFluxCapData[31] = 0.000000; ui42->fFluxCapData[32] = 0.000000; ui42->fFluxCapData[33] = 0.000000; ui42->fFluxCapData[34] = 0.000000; ui42->fFluxCapData[35] = 0.000000; ui42->fFluxCapData[36] = 0.000000; ui42->fFluxCapData[37] = 0.000000; ui42->fFluxCapData[38] = 0.000000; ui42->fFluxCapData[39] = 0.000000; ui42->fFluxCapData[40] = 0.000000; ui42->fFluxCapData[41] = 0.000000; ui42->fFluxCapData[42] = 0.000000; ui42->fFluxCapData[43] = 0.000000; ui42->fFluxCapData[44] = 0.000000; ui42->fFluxCapData[45] = 0.000000; ui42->fFluxCapData[46] = 0.000000; ui42->fFluxCapData[47] = 0.000000; ui42->fFluxCapData[48] = 0.000000; ui42->fFluxCapData[49] = 0.000000; ui42->fFluxCapData[50] = 0.000000; ui42->fFluxCapData[51] = 0.000000; ui42->fFluxCapData[52] = 0.000000; ui42->fFluxCapData[53] = 0.000000; ui42->fFluxCapData[54] = 0.000000; ui42->fFluxCapData[55] = 0.000000; ui42->fFluxCapData[56] = 0.000000; ui42->fFluxCapData[57] = 0.000000; ui42->fFluxCapData[58] = 0.000000; ui42->fFluxCapData[59] = 0.000000; ui42->fFluxCapData[60] = 0.000000; ui42->fFluxCapData[61] = 0.000000; ui42->fFluxCapData[62] = 0.000000; ui42->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui42);
	delete ui42;


	m_uSwitch4 = 0;
	CUICtrl* ui43 = new CUICtrl;
	ui43->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui43->uControlId = 48;
	ui43->bLogSlider = false;
	ui43->bExpSlider = false;
	ui43->fUserDisplayDataLoLimit = 0.000000;
	ui43->fUserDisplayDataHiLimit = 1.000000;
	ui43->uUserDataType = UINTData;
	ui43->fInitUserIntValue = 0;
	ui43->fInitUserFloatValue = 0;
	ui43->fInitUserDoubleValue = 0;
	ui43->fInitUserUINTValue = 0.000000;
	ui43->m_pUserCookedIntData = NULL;
	ui43->m_pUserCookedFloatData = NULL;
	ui43->m_pUserCookedDoubleData = NULL;
	ui43->m_pUserCookedUINTData = &m_uSwitch4;
	ui43->cControlUnits = "";
	ui43->cVariableName = "m_uSwitch4";
	ui43->cEnumeratedList = "SWITCH_OFF,SWITCH_ON";
	ui43->dPresetData[0] = -0.000000;ui43->dPresetData[1] = -0.000000;ui43->dPresetData[2] = 0.000000;ui43->dPresetData[3] = 0.000000;ui43->dPresetData[4] = 0.000000;ui43->dPresetData[5] = 0.000000;ui43->dPresetData[6] = 0.000000;ui43->dPresetData[7] = 0.000000;ui43->dPresetData[8] = 0.000000;ui43->dPresetData[9] = 0.000000;ui43->dPresetData[10] = 0.000000;ui43->dPresetData[11] = 0.000000;ui43->dPresetData[12] = 0.000000;ui43->dPresetData[13] = 0.000000;ui43->dPresetData[14] = 0.000000;ui43->dPresetData[15] = 0.000000;
	ui43->cControlName = "Delay";
	ui43->bOwnerControl = false;
	ui43->bMIDIControl = false;
	ui43->uMIDIControlCommand = 176;
	ui43->uMIDIControlName = 3;
	ui43->uMIDIControlChannel = 0;
	ui43->nGUIRow = nIndexer++;
	ui43->nGUIColumn = -1;
	ui43->bEnableParamSmoothing = false;
	ui43->fSmoothingTimeInMs = 100.0;
	ui43->uControlTheme[0] = 0; ui43->uControlTheme[1] = 0; ui43->uControlTheme[2] = 0; ui43->uControlTheme[3] = 8355711; ui43->uControlTheme[4] = 139; ui43->uControlTheme[5] = 0; ui43->uControlTheme[6] = 0; ui43->uControlTheme[7] = 0; ui43->uControlTheme[8] = 0; ui43->uControlTheme[9] = 0; ui43->uControlTheme[10] = 0; ui43->uControlTheme[11] = 0; ui43->uControlTheme[12] = 0; ui43->uControlTheme[13] = 0; ui43->uControlTheme[14] = 0; ui43->uControlTheme[15] = 0; ui43->uControlTheme[16] = 0; ui43->uControlTheme[17] = 0; ui43->uControlTheme[18] = 0; ui43->uControlTheme[19] = 0; ui43->uControlTheme[20] = 0; ui43->uControlTheme[21] = 0; ui43->uControlTheme[22] = 0; ui43->uControlTheme[23] = 0; ui43->uControlTheme[24] = 0; ui43->uControlTheme[25] = 0; ui43->uControlTheme[26] = 0; ui43->uControlTheme[27] = 0; ui43->uControlTheme[28] = 0; ui43->uControlTheme[29] = 0; ui43->uControlTheme[30] = 0; ui43->uControlTheme[31] = 0; 
	ui43->uFluxCapControl[0] = 0; ui43->uFluxCapControl[1] = 0; ui43->uFluxCapControl[2] = 0; ui43->uFluxCapControl[3] = 0; ui43->uFluxCapControl[4] = 0; ui43->uFluxCapControl[5] = 0; ui43->uFluxCapControl[6] = 0; ui43->uFluxCapControl[7] = 0; ui43->uFluxCapControl[8] = 0; ui43->uFluxCapControl[9] = 0; ui43->uFluxCapControl[10] = 0; ui43->uFluxCapControl[11] = 0; ui43->uFluxCapControl[12] = 0; ui43->uFluxCapControl[13] = 0; ui43->uFluxCapControl[14] = 0; ui43->uFluxCapControl[15] = 0; ui43->uFluxCapControl[16] = 0; ui43->uFluxCapControl[17] = 0; ui43->uFluxCapControl[18] = 0; ui43->uFluxCapControl[19] = 0; ui43->uFluxCapControl[20] = 0; ui43->uFluxCapControl[21] = 0; ui43->uFluxCapControl[22] = 0; ui43->uFluxCapControl[23] = 0; ui43->uFluxCapControl[24] = 0; ui43->uFluxCapControl[25] = 0; ui43->uFluxCapControl[26] = 0; ui43->uFluxCapControl[27] = 0; ui43->uFluxCapControl[28] = 0; ui43->uFluxCapControl[29] = 0; ui43->uFluxCapControl[30] = 0; ui43->uFluxCapControl[31] = 0; ui43->uFluxCapControl[32] = 0; ui43->uFluxCapControl[33] = 0; ui43->uFluxCapControl[34] = 0; ui43->uFluxCapControl[35] = 0; ui43->uFluxCapControl[36] = 0; ui43->uFluxCapControl[37] = 0; ui43->uFluxCapControl[38] = 0; ui43->uFluxCapControl[39] = 0; ui43->uFluxCapControl[40] = 0; ui43->uFluxCapControl[41] = 0; ui43->uFluxCapControl[42] = 0; ui43->uFluxCapControl[43] = 0; ui43->uFluxCapControl[44] = 0; ui43->uFluxCapControl[45] = 0; ui43->uFluxCapControl[46] = 0; ui43->uFluxCapControl[47] = 0; ui43->uFluxCapControl[48] = 0; ui43->uFluxCapControl[49] = 0; ui43->uFluxCapControl[50] = 0; ui43->uFluxCapControl[51] = 0; ui43->uFluxCapControl[52] = 0; ui43->uFluxCapControl[53] = 0; ui43->uFluxCapControl[54] = 0; ui43->uFluxCapControl[55] = 0; ui43->uFluxCapControl[56] = 0; ui43->uFluxCapControl[57] = 0; ui43->uFluxCapControl[58] = 0; ui43->uFluxCapControl[59] = 0; ui43->uFluxCapControl[60] = 0; ui43->uFluxCapControl[61] = 0; ui43->uFluxCapControl[62] = 0; ui43->uFluxCapControl[63] = 0; 
	ui43->fFluxCapData[0] = 0.000000; ui43->fFluxCapData[1] = 0.000000; ui43->fFluxCapData[2] = 0.000000; ui43->fFluxCapData[3] = 0.000000; ui43->fFluxCapData[4] = 0.000000; ui43->fFluxCapData[5] = 0.000000; ui43->fFluxCapData[6] = 0.000000; ui43->fFluxCapData[7] = 0.000000; ui43->fFluxCapData[8] = 0.000000; ui43->fFluxCapData[9] = 0.000000; ui43->fFluxCapData[10] = 0.000000; ui43->fFluxCapData[11] = 0.000000; ui43->fFluxCapData[12] = 0.000000; ui43->fFluxCapData[13] = 0.000000; ui43->fFluxCapData[14] = 0.000000; ui43->fFluxCapData[15] = 0.000000; ui43->fFluxCapData[16] = 0.000000; ui43->fFluxCapData[17] = 0.000000; ui43->fFluxCapData[18] = 0.000000; ui43->fFluxCapData[19] = 0.000000; ui43->fFluxCapData[20] = 0.000000; ui43->fFluxCapData[21] = 0.000000; ui43->fFluxCapData[22] = 0.000000; ui43->fFluxCapData[23] = 0.000000; ui43->fFluxCapData[24] = 0.000000; ui43->fFluxCapData[25] = 0.000000; ui43->fFluxCapData[26] = 0.000000; ui43->fFluxCapData[27] = 0.000000; ui43->fFluxCapData[28] = 0.000000; ui43->fFluxCapData[29] = 0.000000; ui43->fFluxCapData[30] = 0.000000; ui43->fFluxCapData[31] = 0.000000; ui43->fFluxCapData[32] = 0.000000; ui43->fFluxCapData[33] = 0.000000; ui43->fFluxCapData[34] = 0.000000; ui43->fFluxCapData[35] = 0.000000; ui43->fFluxCapData[36] = 0.000000; ui43->fFluxCapData[37] = 0.000000; ui43->fFluxCapData[38] = 0.000000; ui43->fFluxCapData[39] = 0.000000; ui43->fFluxCapData[40] = 0.000000; ui43->fFluxCapData[41] = 0.000000; ui43->fFluxCapData[42] = 0.000000; ui43->fFluxCapData[43] = 0.000000; ui43->fFluxCapData[44] = 0.000000; ui43->fFluxCapData[45] = 0.000000; ui43->fFluxCapData[46] = 0.000000; ui43->fFluxCapData[47] = 0.000000; ui43->fFluxCapData[48] = 0.000000; ui43->fFluxCapData[49] = 0.000000; ui43->fFluxCapData[50] = 0.000000; ui43->fFluxCapData[51] = 0.000000; ui43->fFluxCapData[52] = 0.000000; ui43->fFluxCapData[53] = 0.000000; ui43->fFluxCapData[54] = 0.000000; ui43->fFluxCapData[55] = 0.000000; ui43->fFluxCapData[56] = 0.000000; ui43->fFluxCapData[57] = 0.000000; ui43->fFluxCapData[58] = 0.000000; ui43->fFluxCapData[59] = 0.000000; ui43->fFluxCapData[60] = 0.000000; ui43->fFluxCapData[61] = 0.000000; ui43->fFluxCapData[62] = 0.000000; ui43->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui43);
	delete ui43;


	m_uModType = 0;
	CUICtrl* ui44 = new CUICtrl;
	ui44->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui44->uControlId = 41;
	ui44->bLogSlider = false;
	ui44->bExpSlider = false;
	ui44->fUserDisplayDataLoLimit = 0.000000;
	ui44->fUserDisplayDataHiLimit = 3.000000;
	ui44->uUserDataType = UINTData;
	ui44->fInitUserIntValue = 0;
	ui44->fInitUserFloatValue = 0;
	ui44->fInitUserDoubleValue = 0;
	ui44->fInitUserUINTValue = 0.000000;
	ui44->m_pUserCookedIntData = NULL;
	ui44->m_pUserCookedFloatData = NULL;
	ui44->m_pUserCookedDoubleData = NULL;
	ui44->m_pUserCookedUINTData = &m_uModType;
	ui44->cControlUnits = "";
	ui44->cVariableName = "m_uModType";
	ui44->cEnumeratedList = "Flanger,Vibrato,Chorus,Mixed";
	ui44->dPresetData[0] = -0.000000;ui44->dPresetData[1] = 0.000000;ui44->dPresetData[2] = 0.000000;ui44->dPresetData[3] = 0.000000;ui44->dPresetData[4] = 0.000000;ui44->dPresetData[5] = 0.000000;ui44->dPresetData[6] = 0.000000;ui44->dPresetData[7] = 0.000000;ui44->dPresetData[8] = 0.000000;ui44->dPresetData[9] = 0.000000;ui44->dPresetData[10] = 0.000000;ui44->dPresetData[11] = 0.000000;ui44->dPresetData[12] = 0.000000;ui44->dPresetData[13] = 0.000000;ui44->dPresetData[14] = 0.000000;ui44->dPresetData[15] = 0.000000;
	ui44->cControlName = "Mod Type";
	ui44->bOwnerControl = false;
	ui44->bMIDIControl = false;
	ui44->uMIDIControlCommand = 176;
	ui44->uMIDIControlName = 3;
	ui44->uMIDIControlChannel = 0;
	ui44->nGUIRow = nIndexer++;
	ui44->nGUIColumn = -1;
	ui44->bEnableParamSmoothing = false;
	ui44->fSmoothingTimeInMs = 100.0;
	ui44->uControlTheme[0] = 0; ui44->uControlTheme[1] = 0; ui44->uControlTheme[2] = 0; ui44->uControlTheme[3] = 8355711; ui44->uControlTheme[4] = 139; ui44->uControlTheme[5] = 0; ui44->uControlTheme[6] = 0; ui44->uControlTheme[7] = 0; ui44->uControlTheme[8] = 0; ui44->uControlTheme[9] = 0; ui44->uControlTheme[10] = 0; ui44->uControlTheme[11] = 0; ui44->uControlTheme[12] = 0; ui44->uControlTheme[13] = 0; ui44->uControlTheme[14] = 0; ui44->uControlTheme[15] = 0; ui44->uControlTheme[16] = 0; ui44->uControlTheme[17] = 0; ui44->uControlTheme[18] = 0; ui44->uControlTheme[19] = 0; ui44->uControlTheme[20] = 0; ui44->uControlTheme[21] = 0; ui44->uControlTheme[22] = 0; ui44->uControlTheme[23] = 0; ui44->uControlTheme[24] = 0; ui44->uControlTheme[25] = 0; ui44->uControlTheme[26] = 0; ui44->uControlTheme[27] = 0; ui44->uControlTheme[28] = 0; ui44->uControlTheme[29] = 0; ui44->uControlTheme[30] = 0; ui44->uControlTheme[31] = 0; 
	ui44->uFluxCapControl[0] = 0; ui44->uFluxCapControl[1] = 0; ui44->uFluxCapControl[2] = 0; ui44->uFluxCapControl[3] = 0; ui44->uFluxCapControl[4] = 0; ui44->uFluxCapControl[5] = 0; ui44->uFluxCapControl[6] = 0; ui44->uFluxCapControl[7] = 0; ui44->uFluxCapControl[8] = 0; ui44->uFluxCapControl[9] = 0; ui44->uFluxCapControl[10] = 0; ui44->uFluxCapControl[11] = 0; ui44->uFluxCapControl[12] = 0; ui44->uFluxCapControl[13] = 0; ui44->uFluxCapControl[14] = 0; ui44->uFluxCapControl[15] = 0; ui44->uFluxCapControl[16] = 0; ui44->uFluxCapControl[17] = 0; ui44->uFluxCapControl[18] = 0; ui44->uFluxCapControl[19] = 0; ui44->uFluxCapControl[20] = 0; ui44->uFluxCapControl[21] = 0; ui44->uFluxCapControl[22] = 0; ui44->uFluxCapControl[23] = 0; ui44->uFluxCapControl[24] = 0; ui44->uFluxCapControl[25] = 0; ui44->uFluxCapControl[26] = 0; ui44->uFluxCapControl[27] = 0; ui44->uFluxCapControl[28] = 0; ui44->uFluxCapControl[29] = 0; ui44->uFluxCapControl[30] = 0; ui44->uFluxCapControl[31] = 0; ui44->uFluxCapControl[32] = 0; ui44->uFluxCapControl[33] = 0; ui44->uFluxCapControl[34] = 0; ui44->uFluxCapControl[35] = 0; ui44->uFluxCapControl[36] = 0; ui44->uFluxCapControl[37] = 0; ui44->uFluxCapControl[38] = 0; ui44->uFluxCapControl[39] = 0; ui44->uFluxCapControl[40] = 0; ui44->uFluxCapControl[41] = 0; ui44->uFluxCapControl[42] = 0; ui44->uFluxCapControl[43] = 0; ui44->uFluxCapControl[44] = 0; ui44->uFluxCapControl[45] = 0; ui44->uFluxCapControl[46] = 0; ui44->uFluxCapControl[47] = 0; ui44->uFluxCapControl[48] = 0; ui44->uFluxCapControl[49] = 0; ui44->uFluxCapControl[50] = 0; ui44->uFluxCapControl[51] = 0; ui44->uFluxCapControl[52] = 0; ui44->uFluxCapControl[53] = 0; ui44->uFluxCapControl[54] = 0; ui44->uFluxCapControl[55] = 0; ui44->uFluxCapControl[56] = 0; ui44->uFluxCapControl[57] = 0; ui44->uFluxCapControl[58] = 0; ui44->uFluxCapControl[59] = 0; ui44->uFluxCapControl[60] = 0; ui44->uFluxCapControl[61] = 0; ui44->uFluxCapControl[62] = 0; ui44->uFluxCapControl[63] = 0; 
	ui44->fFluxCapData[0] = 0.000000; ui44->fFluxCapData[1] = 0.000000; ui44->fFluxCapData[2] = 0.000000; ui44->fFluxCapData[3] = 0.000000; ui44->fFluxCapData[4] = 0.000000; ui44->fFluxCapData[5] = 0.000000; ui44->fFluxCapData[6] = 0.000000; ui44->fFluxCapData[7] = 0.000000; ui44->fFluxCapData[8] = 0.000000; ui44->fFluxCapData[9] = 0.000000; ui44->fFluxCapData[10] = 0.000000; ui44->fFluxCapData[11] = 0.000000; ui44->fFluxCapData[12] = 0.000000; ui44->fFluxCapData[13] = 0.000000; ui44->fFluxCapData[14] = 0.000000; ui44->fFluxCapData[15] = 0.000000; ui44->fFluxCapData[16] = 0.000000; ui44->fFluxCapData[17] = 0.000000; ui44->fFluxCapData[18] = 0.000000; ui44->fFluxCapData[19] = 0.000000; ui44->fFluxCapData[20] = 0.000000; ui44->fFluxCapData[21] = 0.000000; ui44->fFluxCapData[22] = 0.000000; ui44->fFluxCapData[23] = 0.000000; ui44->fFluxCapData[24] = 0.000000; ui44->fFluxCapData[25] = 0.000000; ui44->fFluxCapData[26] = 0.000000; ui44->fFluxCapData[27] = 0.000000; ui44->fFluxCapData[28] = 0.000000; ui44->fFluxCapData[29] = 0.000000; ui44->fFluxCapData[30] = 0.000000; ui44->fFluxCapData[31] = 0.000000; ui44->fFluxCapData[32] = 0.000000; ui44->fFluxCapData[33] = 0.000000; ui44->fFluxCapData[34] = 0.000000; ui44->fFluxCapData[35] = 0.000000; ui44->fFluxCapData[36] = 0.000000; ui44->fFluxCapData[37] = 0.000000; ui44->fFluxCapData[38] = 0.000000; ui44->fFluxCapData[39] = 0.000000; ui44->fFluxCapData[40] = 0.000000; ui44->fFluxCapData[41] = 0.000000; ui44->fFluxCapData[42] = 0.000000; ui44->fFluxCapData[43] = 0.000000; ui44->fFluxCapData[44] = 0.000000; ui44->fFluxCapData[45] = 0.000000; ui44->fFluxCapData[46] = 0.000000; ui44->fFluxCapData[47] = 0.000000; ui44->fFluxCapData[48] = 0.000000; ui44->fFluxCapData[49] = 0.000000; ui44->fFluxCapData[50] = 0.000000; ui44->fFluxCapData[51] = 0.000000; ui44->fFluxCapData[52] = 0.000000; ui44->fFluxCapData[53] = 0.000000; ui44->fFluxCapData[54] = 0.000000; ui44->fFluxCapData[55] = 0.000000; ui44->fFluxCapData[56] = 0.000000; ui44->fFluxCapData[57] = 0.000000; ui44->fFluxCapData[58] = 0.000000; ui44->fFluxCapData[59] = 0.000000; ui44->fFluxCapData[60] = 0.000000; ui44->fFluxCapData[61] = 0.000000; ui44->fFluxCapData[62] = 0.000000; ui44->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui44);
	delete ui44;


	m_uLFOType = 0;
	CUICtrl* ui45 = new CUICtrl;
	ui45->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui45->uControlId = 42;
	ui45->bLogSlider = false;
	ui45->bExpSlider = false;
	ui45->fUserDisplayDataLoLimit = 0.000000;
	ui45->fUserDisplayDataHiLimit = 1.000000;
	ui45->uUserDataType = UINTData;
	ui45->fInitUserIntValue = 0;
	ui45->fInitUserFloatValue = 0;
	ui45->fInitUserDoubleValue = 0;
	ui45->fInitUserUINTValue = 0.000000;
	ui45->m_pUserCookedIntData = NULL;
	ui45->m_pUserCookedFloatData = NULL;
	ui45->m_pUserCookedDoubleData = NULL;
	ui45->m_pUserCookedUINTData = &m_uLFOType;
	ui45->cControlUnits = "";
	ui45->cVariableName = "m_uLFOType";
	ui45->cEnumeratedList = "Triangle,Sine";
	ui45->dPresetData[0] = -0.000000;ui45->dPresetData[1] = 0.000000;ui45->dPresetData[2] = 0.000000;ui45->dPresetData[3] = 0.000000;ui45->dPresetData[4] = 0.000000;ui45->dPresetData[5] = 0.000000;ui45->dPresetData[6] = 0.000000;ui45->dPresetData[7] = 0.000000;ui45->dPresetData[8] = 0.000000;ui45->dPresetData[9] = 0.000000;ui45->dPresetData[10] = 0.000000;ui45->dPresetData[11] = 0.000000;ui45->dPresetData[12] = 0.000000;ui45->dPresetData[13] = 0.000000;ui45->dPresetData[14] = 0.000000;ui45->dPresetData[15] = 0.000000;
	ui45->cControlName = "LFO";
	ui45->bOwnerControl = false;
	ui45->bMIDIControl = false;
	ui45->uMIDIControlCommand = 176;
	ui45->uMIDIControlName = 3;
	ui45->uMIDIControlChannel = 0;
	ui45->nGUIRow = nIndexer++;
	ui45->nGUIColumn = -1;
	ui45->bEnableParamSmoothing = false;
	ui45->fSmoothingTimeInMs = 100.0;
	ui45->uControlTheme[0] = 0; ui45->uControlTheme[1] = 0; ui45->uControlTheme[2] = 0; ui45->uControlTheme[3] = 8355711; ui45->uControlTheme[4] = 139; ui45->uControlTheme[5] = 0; ui45->uControlTheme[6] = 0; ui45->uControlTheme[7] = 0; ui45->uControlTheme[8] = 0; ui45->uControlTheme[9] = 0; ui45->uControlTheme[10] = 0; ui45->uControlTheme[11] = 0; ui45->uControlTheme[12] = 0; ui45->uControlTheme[13] = 0; ui45->uControlTheme[14] = 0; ui45->uControlTheme[15] = 0; ui45->uControlTheme[16] = 0; ui45->uControlTheme[17] = 0; ui45->uControlTheme[18] = 0; ui45->uControlTheme[19] = 0; ui45->uControlTheme[20] = 0; ui45->uControlTheme[21] = 0; ui45->uControlTheme[22] = 0; ui45->uControlTheme[23] = 0; ui45->uControlTheme[24] = 0; ui45->uControlTheme[25] = 0; ui45->uControlTheme[26] = 0; ui45->uControlTheme[27] = 0; ui45->uControlTheme[28] = 0; ui45->uControlTheme[29] = 0; ui45->uControlTheme[30] = 0; ui45->uControlTheme[31] = 0; 
	ui45->uFluxCapControl[0] = 0; ui45->uFluxCapControl[1] = 0; ui45->uFluxCapControl[2] = 0; ui45->uFluxCapControl[3] = 0; ui45->uFluxCapControl[4] = 0; ui45->uFluxCapControl[5] = 0; ui45->uFluxCapControl[6] = 0; ui45->uFluxCapControl[7] = 0; ui45->uFluxCapControl[8] = 0; ui45->uFluxCapControl[9] = 0; ui45->uFluxCapControl[10] = 0; ui45->uFluxCapControl[11] = 0; ui45->uFluxCapControl[12] = 0; ui45->uFluxCapControl[13] = 0; ui45->uFluxCapControl[14] = 0; ui45->uFluxCapControl[15] = 0; ui45->uFluxCapControl[16] = 0; ui45->uFluxCapControl[17] = 0; ui45->uFluxCapControl[18] = 0; ui45->uFluxCapControl[19] = 0; ui45->uFluxCapControl[20] = 0; ui45->uFluxCapControl[21] = 0; ui45->uFluxCapControl[22] = 0; ui45->uFluxCapControl[23] = 0; ui45->uFluxCapControl[24] = 0; ui45->uFluxCapControl[25] = 0; ui45->uFluxCapControl[26] = 0; ui45->uFluxCapControl[27] = 0; ui45->uFluxCapControl[28] = 0; ui45->uFluxCapControl[29] = 0; ui45->uFluxCapControl[30] = 0; ui45->uFluxCapControl[31] = 0; ui45->uFluxCapControl[32] = 0; ui45->uFluxCapControl[33] = 0; ui45->uFluxCapControl[34] = 0; ui45->uFluxCapControl[35] = 0; ui45->uFluxCapControl[36] = 0; ui45->uFluxCapControl[37] = 0; ui45->uFluxCapControl[38] = 0; ui45->uFluxCapControl[39] = 0; ui45->uFluxCapControl[40] = 0; ui45->uFluxCapControl[41] = 0; ui45->uFluxCapControl[42] = 0; ui45->uFluxCapControl[43] = 0; ui45->uFluxCapControl[44] = 0; ui45->uFluxCapControl[45] = 0; ui45->uFluxCapControl[46] = 0; ui45->uFluxCapControl[47] = 0; ui45->uFluxCapControl[48] = 0; ui45->uFluxCapControl[49] = 0; ui45->uFluxCapControl[50] = 0; ui45->uFluxCapControl[51] = 0; ui45->uFluxCapControl[52] = 0; ui45->uFluxCapControl[53] = 0; ui45->uFluxCapControl[54] = 0; ui45->uFluxCapControl[55] = 0; ui45->uFluxCapControl[56] = 0; ui45->uFluxCapControl[57] = 0; ui45->uFluxCapControl[58] = 0; ui45->uFluxCapControl[59] = 0; ui45->uFluxCapControl[60] = 0; ui45->uFluxCapControl[61] = 0; ui45->uFluxCapControl[62] = 0; ui45->uFluxCapControl[63] = 0; 
	ui45->fFluxCapData[0] = 0.000000; ui45->fFluxCapData[1] = 0.000000; ui45->fFluxCapData[2] = 0.000000; ui45->fFluxCapData[3] = 0.000000; ui45->fFluxCapData[4] = 0.000000; ui45->fFluxCapData[5] = 0.000000; ui45->fFluxCapData[6] = 0.000000; ui45->fFluxCapData[7] = 0.000000; ui45->fFluxCapData[8] = 0.000000; ui45->fFluxCapData[9] = 0.000000; ui45->fFluxCapData[10] = 0.000000; ui45->fFluxCapData[11] = 0.000000; ui45->fFluxCapData[12] = 0.000000; ui45->fFluxCapData[13] = 0.000000; ui45->fFluxCapData[14] = 0.000000; ui45->fFluxCapData[15] = 0.000000; ui45->fFluxCapData[16] = 0.000000; ui45->fFluxCapData[17] = 0.000000; ui45->fFluxCapData[18] = 0.000000; ui45->fFluxCapData[19] = 0.000000; ui45->fFluxCapData[20] = 0.000000; ui45->fFluxCapData[21] = 0.000000; ui45->fFluxCapData[22] = 0.000000; ui45->fFluxCapData[23] = 0.000000; ui45->fFluxCapData[24] = 0.000000; ui45->fFluxCapData[25] = 0.000000; ui45->fFluxCapData[26] = 0.000000; ui45->fFluxCapData[27] = 0.000000; ui45->fFluxCapData[28] = 0.000000; ui45->fFluxCapData[29] = 0.000000; ui45->fFluxCapData[30] = 0.000000; ui45->fFluxCapData[31] = 0.000000; ui45->fFluxCapData[32] = 0.000000; ui45->fFluxCapData[33] = 0.000000; ui45->fFluxCapData[34] = 0.000000; ui45->fFluxCapData[35] = 0.000000; ui45->fFluxCapData[36] = 0.000000; ui45->fFluxCapData[37] = 0.000000; ui45->fFluxCapData[38] = 0.000000; ui45->fFluxCapData[39] = 0.000000; ui45->fFluxCapData[40] = 0.000000; ui45->fFluxCapData[41] = 0.000000; ui45->fFluxCapData[42] = 0.000000; ui45->fFluxCapData[43] = 0.000000; ui45->fFluxCapData[44] = 0.000000; ui45->fFluxCapData[45] = 0.000000; ui45->fFluxCapData[46] = 0.000000; ui45->fFluxCapData[47] = 0.000000; ui45->fFluxCapData[48] = 0.000000; ui45->fFluxCapData[49] = 0.000000; ui45->fFluxCapData[50] = 0.000000; ui45->fFluxCapData[51] = 0.000000; ui45->fFluxCapData[52] = 0.000000; ui45->fFluxCapData[53] = 0.000000; ui45->fFluxCapData[54] = 0.000000; ui45->fFluxCapData[55] = 0.000000; ui45->fFluxCapData[56] = 0.000000; ui45->fFluxCapData[57] = 0.000000; ui45->fFluxCapData[58] = 0.000000; ui45->fFluxCapData[59] = 0.000000; ui45->fFluxCapData[60] = 0.000000; ui45->fFluxCapData[61] = 0.000000; ui45->fFluxCapData[62] = 0.000000; ui45->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui45);
	delete ui45;


	m_uLFOPhase = 0;
	CUICtrl* ui46 = new CUICtrl;
	ui46->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui46->uControlId = 43;
	ui46->bLogSlider = false;
	ui46->bExpSlider = false;
	ui46->fUserDisplayDataLoLimit = 0.000000;
	ui46->fUserDisplayDataHiLimit = 2.000000;
	ui46->uUserDataType = UINTData;
	ui46->fInitUserIntValue = 0;
	ui46->fInitUserFloatValue = 0;
	ui46->fInitUserDoubleValue = 0;
	ui46->fInitUserUINTValue = 0.000000;
	ui46->m_pUserCookedIntData = NULL;
	ui46->m_pUserCookedFloatData = NULL;
	ui46->m_pUserCookedDoubleData = NULL;
	ui46->m_pUserCookedUINTData = &m_uLFOPhase;
	ui46->cControlUnits = "";
	ui46->cVariableName = "m_uLFOPhase";
	ui46->cEnumeratedList = "Normal,Quad,Inverted";
	ui46->dPresetData[0] = -0.000000;ui46->dPresetData[1] = 0.000000;ui46->dPresetData[2] = 0.000000;ui46->dPresetData[3] = 0.000000;ui46->dPresetData[4] = 0.000000;ui46->dPresetData[5] = 0.000000;ui46->dPresetData[6] = 0.000000;ui46->dPresetData[7] = 0.000000;ui46->dPresetData[8] = 0.000000;ui46->dPresetData[9] = 0.000000;ui46->dPresetData[10] = 0.000000;ui46->dPresetData[11] = 0.000000;ui46->dPresetData[12] = 0.000000;ui46->dPresetData[13] = 0.000000;ui46->dPresetData[14] = 0.000000;ui46->dPresetData[15] = 0.000000;
	ui46->cControlName = "Phase";
	ui46->bOwnerControl = false;
	ui46->bMIDIControl = false;
	ui46->uMIDIControlCommand = 176;
	ui46->uMIDIControlName = 3;
	ui46->uMIDIControlChannel = 0;
	ui46->nGUIRow = nIndexer++;
	ui46->nGUIColumn = -1;
	ui46->bEnableParamSmoothing = false;
	ui46->fSmoothingTimeInMs = 100.0;
	ui46->uControlTheme[0] = 0; ui46->uControlTheme[1] = 0; ui46->uControlTheme[2] = 0; ui46->uControlTheme[3] = 8355711; ui46->uControlTheme[4] = 139; ui46->uControlTheme[5] = 0; ui46->uControlTheme[6] = 0; ui46->uControlTheme[7] = 0; ui46->uControlTheme[8] = 0; ui46->uControlTheme[9] = 0; ui46->uControlTheme[10] = 0; ui46->uControlTheme[11] = 0; ui46->uControlTheme[12] = 0; ui46->uControlTheme[13] = 0; ui46->uControlTheme[14] = 0; ui46->uControlTheme[15] = 0; ui46->uControlTheme[16] = 0; ui46->uControlTheme[17] = 0; ui46->uControlTheme[18] = 0; ui46->uControlTheme[19] = 0; ui46->uControlTheme[20] = 0; ui46->uControlTheme[21] = 0; ui46->uControlTheme[22] = 0; ui46->uControlTheme[23] = 0; ui46->uControlTheme[24] = 0; ui46->uControlTheme[25] = 0; ui46->uControlTheme[26] = 0; ui46->uControlTheme[27] = 0; ui46->uControlTheme[28] = 0; ui46->uControlTheme[29] = 0; ui46->uControlTheme[30] = 0; ui46->uControlTheme[31] = 0; 
	ui46->uFluxCapControl[0] = 0; ui46->uFluxCapControl[1] = 0; ui46->uFluxCapControl[2] = 0; ui46->uFluxCapControl[3] = 0; ui46->uFluxCapControl[4] = 0; ui46->uFluxCapControl[5] = 0; ui46->uFluxCapControl[6] = 0; ui46->uFluxCapControl[7] = 0; ui46->uFluxCapControl[8] = 0; ui46->uFluxCapControl[9] = 0; ui46->uFluxCapControl[10] = 0; ui46->uFluxCapControl[11] = 0; ui46->uFluxCapControl[12] = 0; ui46->uFluxCapControl[13] = 0; ui46->uFluxCapControl[14] = 0; ui46->uFluxCapControl[15] = 0; ui46->uFluxCapControl[16] = 0; ui46->uFluxCapControl[17] = 0; ui46->uFluxCapControl[18] = 0; ui46->uFluxCapControl[19] = 0; ui46->uFluxCapControl[20] = 0; ui46->uFluxCapControl[21] = 0; ui46->uFluxCapControl[22] = 0; ui46->uFluxCapControl[23] = 0; ui46->uFluxCapControl[24] = 0; ui46->uFluxCapControl[25] = 0; ui46->uFluxCapControl[26] = 0; ui46->uFluxCapControl[27] = 0; ui46->uFluxCapControl[28] = 0; ui46->uFluxCapControl[29] = 0; ui46->uFluxCapControl[30] = 0; ui46->uFluxCapControl[31] = 0; ui46->uFluxCapControl[32] = 0; ui46->uFluxCapControl[33] = 0; ui46->uFluxCapControl[34] = 0; ui46->uFluxCapControl[35] = 0; ui46->uFluxCapControl[36] = 0; ui46->uFluxCapControl[37] = 0; ui46->uFluxCapControl[38] = 0; ui46->uFluxCapControl[39] = 0; ui46->uFluxCapControl[40] = 0; ui46->uFluxCapControl[41] = 0; ui46->uFluxCapControl[42] = 0; ui46->uFluxCapControl[43] = 0; ui46->uFluxCapControl[44] = 0; ui46->uFluxCapControl[45] = 0; ui46->uFluxCapControl[46] = 0; ui46->uFluxCapControl[47] = 0; ui46->uFluxCapControl[48] = 0; ui46->uFluxCapControl[49] = 0; ui46->uFluxCapControl[50] = 0; ui46->uFluxCapControl[51] = 0; ui46->uFluxCapControl[52] = 0; ui46->uFluxCapControl[53] = 0; ui46->uFluxCapControl[54] = 0; ui46->uFluxCapControl[55] = 0; ui46->uFluxCapControl[56] = 0; ui46->uFluxCapControl[57] = 0; ui46->uFluxCapControl[58] = 0; ui46->uFluxCapControl[59] = 0; ui46->uFluxCapControl[60] = 0; ui46->uFluxCapControl[61] = 0; ui46->uFluxCapControl[62] = 0; ui46->uFluxCapControl[63] = 0; 
	ui46->fFluxCapData[0] = 0.000000; ui46->fFluxCapData[1] = 0.000000; ui46->fFluxCapData[2] = 0.000000; ui46->fFluxCapData[3] = 0.000000; ui46->fFluxCapData[4] = 0.000000; ui46->fFluxCapData[5] = 0.000000; ui46->fFluxCapData[6] = 0.000000; ui46->fFluxCapData[7] = 0.000000; ui46->fFluxCapData[8] = 0.000000; ui46->fFluxCapData[9] = 0.000000; ui46->fFluxCapData[10] = 0.000000; ui46->fFluxCapData[11] = 0.000000; ui46->fFluxCapData[12] = 0.000000; ui46->fFluxCapData[13] = 0.000000; ui46->fFluxCapData[14] = 0.000000; ui46->fFluxCapData[15] = 0.000000; ui46->fFluxCapData[16] = 0.000000; ui46->fFluxCapData[17] = 0.000000; ui46->fFluxCapData[18] = 0.000000; ui46->fFluxCapData[19] = 0.000000; ui46->fFluxCapData[20] = 0.000000; ui46->fFluxCapData[21] = 0.000000; ui46->fFluxCapData[22] = 0.000000; ui46->fFluxCapData[23] = 0.000000; ui46->fFluxCapData[24] = 0.000000; ui46->fFluxCapData[25] = 0.000000; ui46->fFluxCapData[26] = 0.000000; ui46->fFluxCapData[27] = 0.000000; ui46->fFluxCapData[28] = 0.000000; ui46->fFluxCapData[29] = 0.000000; ui46->fFluxCapData[30] = 0.000000; ui46->fFluxCapData[31] = 0.000000; ui46->fFluxCapData[32] = 0.000000; ui46->fFluxCapData[33] = 0.000000; ui46->fFluxCapData[34] = 0.000000; ui46->fFluxCapData[35] = 0.000000; ui46->fFluxCapData[36] = 0.000000; ui46->fFluxCapData[37] = 0.000000; ui46->fFluxCapData[38] = 0.000000; ui46->fFluxCapData[39] = 0.000000; ui46->fFluxCapData[40] = 0.000000; ui46->fFluxCapData[41] = 0.000000; ui46->fFluxCapData[42] = 0.000000; ui46->fFluxCapData[43] = 0.000000; ui46->fFluxCapData[44] = 0.000000; ui46->fFluxCapData[45] = 0.000000; ui46->fFluxCapData[46] = 0.000000; ui46->fFluxCapData[47] = 0.000000; ui46->fFluxCapData[48] = 0.000000; ui46->fFluxCapData[49] = 0.000000; ui46->fFluxCapData[50] = 0.000000; ui46->fFluxCapData[51] = 0.000000; ui46->fFluxCapData[52] = 0.000000; ui46->fFluxCapData[53] = 0.000000; ui46->fFluxCapData[54] = 0.000000; ui46->fFluxCapData[55] = 0.000000; ui46->fFluxCapData[56] = 0.000000; ui46->fFluxCapData[57] = 0.000000; ui46->fFluxCapData[58] = 0.000000; ui46->fFluxCapData[59] = 0.000000; ui46->fFluxCapData[60] = 0.000000; ui46->fFluxCapData[61] = 0.000000; ui46->fFluxCapData[62] = 0.000000; ui46->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui46);
	delete ui46;


	m_uSwitch5 = 0;
	CUICtrl* ui47 = new CUICtrl;
	ui47->uControlType = FILTER_CONTROL_RADIO_SWITCH_VARIABLE;
	ui47->uControlId = 3072;
	ui47->bLogSlider = false;
	ui47->bExpSlider = false;
	ui47->fUserDisplayDataLoLimit = 0.000000;
	ui47->fUserDisplayDataHiLimit = 1.000000;
	ui47->uUserDataType = UINTData;
	ui47->fInitUserIntValue = 0;
	ui47->fInitUserFloatValue = 0;
	ui47->fInitUserDoubleValue = 0;
	ui47->fInitUserUINTValue = 0.000000;
	ui47->m_pUserCookedIntData = NULL;
	ui47->m_pUserCookedFloatData = NULL;
	ui47->m_pUserCookedDoubleData = NULL;
	ui47->m_pUserCookedUINTData = &m_uSwitch5;
	ui47->cControlUnits = "";
	ui47->cVariableName = "m_uSwitch5";
	ui47->cEnumeratedList = "SWITCH_OFF,SWITCH_ON";
	ui47->dPresetData[0] = -0.000000;ui47->dPresetData[1] = 0.000000;ui47->dPresetData[2] = 0.000000;ui47->dPresetData[3] = 0.000000;ui47->dPresetData[4] = 0.000000;ui47->dPresetData[5] = 0.000000;ui47->dPresetData[6] = 0.000000;ui47->dPresetData[7] = 0.000000;ui47->dPresetData[8] = 0.000000;ui47->dPresetData[9] = 0.000000;ui47->dPresetData[10] = 0.000000;ui47->dPresetData[11] = 0.000000;ui47->dPresetData[12] = 0.000000;ui47->dPresetData[13] = 0.000000;ui47->dPresetData[14] = 0.000000;ui47->dPresetData[15] = 0.000000;
	ui47->cControlName = "Bypass";
	ui47->bOwnerControl = false;
	ui47->bMIDIControl = false;
	ui47->uMIDIControlCommand = 176;
	ui47->uMIDIControlName = 3;
	ui47->uMIDIControlChannel = 0;
	ui47->nGUIRow = nIndexer++;
	ui47->nGUIColumn = -1;
	ui47->bEnableParamSmoothing = false;
	ui47->fSmoothingTimeInMs = 100.0;
	ui47->uControlTheme[0] = 0; ui47->uControlTheme[1] = 0; ui47->uControlTheme[2] = 0; ui47->uControlTheme[3] = 8355711; ui47->uControlTheme[4] = 139; ui47->uControlTheme[5] = 0; ui47->uControlTheme[6] = 0; ui47->uControlTheme[7] = 0; ui47->uControlTheme[8] = 0; ui47->uControlTheme[9] = 0; ui47->uControlTheme[10] = 0; ui47->uControlTheme[11] = 0; ui47->uControlTheme[12] = 0; ui47->uControlTheme[13] = 0; ui47->uControlTheme[14] = 0; ui47->uControlTheme[15] = 0; ui47->uControlTheme[16] = 0; ui47->uControlTheme[17] = 0; ui47->uControlTheme[18] = 0; ui47->uControlTheme[19] = 0; ui47->uControlTheme[20] = 0; ui47->uControlTheme[21] = 0; ui47->uControlTheme[22] = 0; ui47->uControlTheme[23] = 0; ui47->uControlTheme[24] = 0; ui47->uControlTheme[25] = 0; ui47->uControlTheme[26] = 0; ui47->uControlTheme[27] = 0; ui47->uControlTheme[28] = 0; ui47->uControlTheme[29] = 0; ui47->uControlTheme[30] = 0; ui47->uControlTheme[31] = 0; 
	ui47->uFluxCapControl[0] = 0; ui47->uFluxCapControl[1] = 0; ui47->uFluxCapControl[2] = 0; ui47->uFluxCapControl[3] = 0; ui47->uFluxCapControl[4] = 0; ui47->uFluxCapControl[5] = 0; ui47->uFluxCapControl[6] = 0; ui47->uFluxCapControl[7] = 0; ui47->uFluxCapControl[8] = 0; ui47->uFluxCapControl[9] = 0; ui47->uFluxCapControl[10] = 0; ui47->uFluxCapControl[11] = 0; ui47->uFluxCapControl[12] = 0; ui47->uFluxCapControl[13] = 0; ui47->uFluxCapControl[14] = 0; ui47->uFluxCapControl[15] = 0; ui47->uFluxCapControl[16] = 0; ui47->uFluxCapControl[17] = 0; ui47->uFluxCapControl[18] = 0; ui47->uFluxCapControl[19] = 0; ui47->uFluxCapControl[20] = 0; ui47->uFluxCapControl[21] = 0; ui47->uFluxCapControl[22] = 0; ui47->uFluxCapControl[23] = 0; ui47->uFluxCapControl[24] = 0; ui47->uFluxCapControl[25] = 0; ui47->uFluxCapControl[26] = 0; ui47->uFluxCapControl[27] = 0; ui47->uFluxCapControl[28] = 0; ui47->uFluxCapControl[29] = 0; ui47->uFluxCapControl[30] = 0; ui47->uFluxCapControl[31] = 0; ui47->uFluxCapControl[32] = 0; ui47->uFluxCapControl[33] = 0; ui47->uFluxCapControl[34] = 0; ui47->uFluxCapControl[35] = 0; ui47->uFluxCapControl[36] = 0; ui47->uFluxCapControl[37] = 0; ui47->uFluxCapControl[38] = 0; ui47->uFluxCapControl[39] = 0; ui47->uFluxCapControl[40] = 0; ui47->uFluxCapControl[41] = 0; ui47->uFluxCapControl[42] = 0; ui47->uFluxCapControl[43] = 0; ui47->uFluxCapControl[44] = 0; ui47->uFluxCapControl[45] = 0; ui47->uFluxCapControl[46] = 0; ui47->uFluxCapControl[47] = 0; ui47->uFluxCapControl[48] = 0; ui47->uFluxCapControl[49] = 0; ui47->uFluxCapControl[50] = 0; ui47->uFluxCapControl[51] = 0; ui47->uFluxCapControl[52] = 0; ui47->uFluxCapControl[53] = 0; ui47->uFluxCapControl[54] = 0; ui47->uFluxCapControl[55] = 0; ui47->uFluxCapControl[56] = 0; ui47->uFluxCapControl[57] = 0; ui47->uFluxCapControl[58] = 0; ui47->uFluxCapControl[59] = 0; ui47->uFluxCapControl[60] = 0; ui47->uFluxCapControl[61] = 0; ui47->uFluxCapControl[62] = 0; ui47->uFluxCapControl[63] = 0; 
	ui47->fFluxCapData[0] = 0.000000; ui47->fFluxCapData[1] = 0.000000; ui47->fFluxCapData[2] = 0.000000; ui47->fFluxCapData[3] = 0.000000; ui47->fFluxCapData[4] = 0.000000; ui47->fFluxCapData[5] = 0.000000; ui47->fFluxCapData[6] = 0.000000; ui47->fFluxCapData[7] = 0.000000; ui47->fFluxCapData[8] = 0.000000; ui47->fFluxCapData[9] = 0.000000; ui47->fFluxCapData[10] = 0.000000; ui47->fFluxCapData[11] = 0.000000; ui47->fFluxCapData[12] = 0.000000; ui47->fFluxCapData[13] = 0.000000; ui47->fFluxCapData[14] = 0.000000; ui47->fFluxCapData[15] = 0.000000; ui47->fFluxCapData[16] = 0.000000; ui47->fFluxCapData[17] = 0.000000; ui47->fFluxCapData[18] = 0.000000; ui47->fFluxCapData[19] = 0.000000; ui47->fFluxCapData[20] = 0.000000; ui47->fFluxCapData[21] = 0.000000; ui47->fFluxCapData[22] = 0.000000; ui47->fFluxCapData[23] = 0.000000; ui47->fFluxCapData[24] = 0.000000; ui47->fFluxCapData[25] = 0.000000; ui47->fFluxCapData[26] = 0.000000; ui47->fFluxCapData[27] = 0.000000; ui47->fFluxCapData[28] = 0.000000; ui47->fFluxCapData[29] = 0.000000; ui47->fFluxCapData[30] = 0.000000; ui47->fFluxCapData[31] = 0.000000; ui47->fFluxCapData[32] = 0.000000; ui47->fFluxCapData[33] = 0.000000; ui47->fFluxCapData[34] = 0.000000; ui47->fFluxCapData[35] = 0.000000; ui47->fFluxCapData[36] = 0.000000; ui47->fFluxCapData[37] = 0.000000; ui47->fFluxCapData[38] = 0.000000; ui47->fFluxCapData[39] = 0.000000; ui47->fFluxCapData[40] = 0.000000; ui47->fFluxCapData[41] = 0.000000; ui47->fFluxCapData[42] = 0.000000; ui47->fFluxCapData[43] = 0.000000; ui47->fFluxCapData[44] = 0.000000; ui47->fFluxCapData[45] = 0.000000; ui47->fFluxCapData[46] = 0.000000; ui47->fFluxCapData[47] = 0.000000; ui47->fFluxCapData[48] = 0.000000; ui47->fFluxCapData[49] = 0.000000; ui47->fFluxCapData[50] = 0.000000; ui47->fFluxCapData[51] = 0.000000; ui47->fFluxCapData[52] = 0.000000; ui47->fFluxCapData[53] = 0.000000; ui47->fFluxCapData[54] = 0.000000; ui47->fFluxCapData[55] = 0.000000; ui47->fFluxCapData[56] = 0.000000; ui47->fFluxCapData[57] = 0.000000; ui47->fFluxCapData[58] = 0.000000; ui47->fFluxCapData[59] = 0.000000; ui47->fFluxCapData[60] = 0.000000; ui47->fFluxCapData[61] = 0.000000; ui47->fFluxCapData[62] = 0.000000; ui47->fFluxCapData[63] = 0.000000; 
	m_UIControlList.append(*ui47);
	delete ui47;


	m_uX_TrackPadIndex = -1;
	m_uY_TrackPadIndex = -1;

	m_AssignButton1Name = "B1";
	m_AssignButton2Name = "B2";
	m_AssignButton3Name = "B3";

	m_bLatchingAssignButton1 = false;
	m_bLatchingAssignButton2 = false;
	m_bLatchingAssignButton3 = false;

	m_nGUIType = -1;
	m_nGUIThemeID = -1;
	m_bUseCustomVSTGUI = false;

	m_uControlTheme[0] = 0; m_uControlTheme[1] = 0; m_uControlTheme[2] = 0; m_uControlTheme[3] = 0; m_uControlTheme[4] = 0; m_uControlTheme[5] = 0; m_uControlTheme[6] = 0; m_uControlTheme[7] = 0; m_uControlTheme[8] = 0; m_uControlTheme[9] = 0; m_uControlTheme[10] = 0; m_uControlTheme[11] = 0; m_uControlTheme[12] = 0; m_uControlTheme[13] = 0; m_uControlTheme[14] = 0; m_uControlTheme[15] = 0; m_uControlTheme[16] = 0; m_uControlTheme[17] = 0; m_uControlTheme[18] = 0; m_uControlTheme[19] = 0; m_uControlTheme[20] = 0; m_uControlTheme[21] = 0; m_uControlTheme[22] = 0; m_uControlTheme[23] = 0; m_uControlTheme[24] = 0; m_uControlTheme[25] = 0; m_uControlTheme[26] = 0; m_uControlTheme[27] = 0; m_uControlTheme[28] = 0; m_uControlTheme[29] = 0; m_uControlTheme[30] = 0; m_uControlTheme[31] = 0; m_uControlTheme[32] = 0; m_uControlTheme[33] = 0; m_uControlTheme[34] = 0; m_uControlTheme[35] = 0; m_uControlTheme[36] = 0; m_uControlTheme[37] = 0; m_uControlTheme[38] = 0; m_uControlTheme[39] = 0; m_uControlTheme[40] = 0; m_uControlTheme[41] = 0; m_uControlTheme[42] = 0; m_uControlTheme[43] = 0; m_uControlTheme[44] = 0; m_uControlTheme[45] = 0; m_uControlTheme[46] = 0; m_uControlTheme[47] = 0; m_uControlTheme[48] = 0; m_uControlTheme[49] = 0; m_uControlTheme[50] = 0; m_uControlTheme[51] = 0; m_uControlTheme[52] = 0; m_uControlTheme[53] = 0; m_uControlTheme[54] = 0; m_uControlTheme[55] = 0; m_uControlTheme[56] = 0; m_uControlTheme[57] = 0; m_uControlTheme[58] = 0; m_uControlTheme[59] = 0; m_uControlTheme[60] = 0; m_uControlTheme[61] = 0; m_uControlTheme[62] = 0; m_uControlTheme[63] = 0; 

	m_uPlugInEx[0] = 6813; m_uPlugInEx[1] = 430; m_uPlugInEx[2] = 2; m_uPlugInEx[3] = 0; m_uPlugInEx[4] = 0; m_uPlugInEx[5] = 0; m_uPlugInEx[6] = 1; m_uPlugInEx[7] = 0; m_uPlugInEx[8] = 0; m_uPlugInEx[9] = 0; m_uPlugInEx[10] = 0; m_uPlugInEx[11] = 0; m_uPlugInEx[12] = 0; m_uPlugInEx[13] = 0; m_uPlugInEx[14] = 0; m_uPlugInEx[15] = 0; m_uPlugInEx[16] = 0; m_uPlugInEx[17] = 0; m_uPlugInEx[18] = 0; m_uPlugInEx[19] = 0; m_uPlugInEx[20] = 0; m_uPlugInEx[21] = 0; m_uPlugInEx[22] = 0; m_uPlugInEx[23] = 0; m_uPlugInEx[24] = 0; m_uPlugInEx[25] = 0; m_uPlugInEx[26] = 0; m_uPlugInEx[27] = 0; m_uPlugInEx[28] = 0; m_uPlugInEx[29] = 0; m_uPlugInEx[30] = 0; m_uPlugInEx[31] = 0; m_uPlugInEx[32] = 0; m_uPlugInEx[33] = 0; m_uPlugInEx[34] = 0; m_uPlugInEx[35] = 0; m_uPlugInEx[36] = 0; m_uPlugInEx[37] = 0; m_uPlugInEx[38] = 0; m_uPlugInEx[39] = 0; m_uPlugInEx[40] = 0; m_uPlugInEx[41] = 0; m_uPlugInEx[42] = 0; m_uPlugInEx[43] = 0; m_uPlugInEx[44] = 0; m_uPlugInEx[45] = 0; m_uPlugInEx[46] = 0; m_uPlugInEx[47] = 0; m_uPlugInEx[48] = 0; m_uPlugInEx[49] = 0; m_uPlugInEx[50] = 0; m_uPlugInEx[51] = 0; m_uPlugInEx[52] = 0; m_uPlugInEx[53] = 0; m_uPlugInEx[54] = 0; m_uPlugInEx[55] = 0; m_uPlugInEx[56] = 0; m_uPlugInEx[57] = 0; m_uPlugInEx[58] = 0; m_uPlugInEx[59] = 0; m_uPlugInEx[60] = 0; m_uPlugInEx[61] = 0; m_uPlugInEx[62] = 0; m_uPlugInEx[63] = 0; 
	m_fPlugInEx[0] = 0.000000; m_fPlugInEx[1] = 5000.000000; m_fPlugInEx[2] = 0.000000; m_fPlugInEx[3] = 0.000000; m_fPlugInEx[4] = 0.000000; m_fPlugInEx[5] = 0.000000; m_fPlugInEx[6] = 0.000000; m_fPlugInEx[7] = 0.000000; m_fPlugInEx[8] = 0.000000; m_fPlugInEx[9] = 0.000000; m_fPlugInEx[10] = 0.000000; m_fPlugInEx[11] = 0.000000; m_fPlugInEx[12] = 0.000000; m_fPlugInEx[13] = 0.000000; m_fPlugInEx[14] = 0.000000; m_fPlugInEx[15] = 0.000000; m_fPlugInEx[16] = 0.000000; m_fPlugInEx[17] = 0.000000; m_fPlugInEx[18] = 0.000000; m_fPlugInEx[19] = 0.000000; m_fPlugInEx[20] = 0.000000; m_fPlugInEx[21] = 0.000000; m_fPlugInEx[22] = 0.000000; m_fPlugInEx[23] = 0.000000; m_fPlugInEx[24] = 0.000000; m_fPlugInEx[25] = 0.000000; m_fPlugInEx[26] = 0.000000; m_fPlugInEx[27] = 0.000000; m_fPlugInEx[28] = 0.000000; m_fPlugInEx[29] = 0.000000; m_fPlugInEx[30] = 0.000000; m_fPlugInEx[31] = 0.000000; m_fPlugInEx[32] = 0.000000; m_fPlugInEx[33] = 0.000000; m_fPlugInEx[34] = 0.000000; m_fPlugInEx[35] = 0.000000; m_fPlugInEx[36] = 0.000000; m_fPlugInEx[37] = 0.000000; m_fPlugInEx[38] = 0.000000; m_fPlugInEx[39] = 0.000000; m_fPlugInEx[40] = 0.000000; m_fPlugInEx[41] = 0.000000; m_fPlugInEx[42] = 0.000000; m_fPlugInEx[43] = 0.000000; m_fPlugInEx[44] = 0.000000; m_fPlugInEx[45] = 0.000000; m_fPlugInEx[46] = 0.000000; m_fPlugInEx[47] = 0.000000; m_fPlugInEx[48] = 0.000000; m_fPlugInEx[49] = 0.000000; m_fPlugInEx[50] = 0.000000; m_fPlugInEx[51] = 0.000000; m_fPlugInEx[52] = 0.000000; m_fPlugInEx[53] = 0.000000; m_fPlugInEx[54] = 0.000000; m_fPlugInEx[55] = 0.000000; m_fPlugInEx[56] = 0.000000; m_fPlugInEx[57] = 0.000000; m_fPlugInEx[58] = 0.000000; m_fPlugInEx[59] = 0.000000; m_fPlugInEx[60] = 0.000000; m_fPlugInEx[61] = 0.000000; m_fPlugInEx[62] = 0.000000; m_fPlugInEx[63] = 0.000000; 

	m_TextLabels[0] = ""; m_TextLabels[1] = ""; m_TextLabels[2] = ""; m_TextLabels[3] = ""; m_TextLabels[4] = ""; m_TextLabels[5] = ""; m_TextLabels[6] = ""; m_TextLabels[7] = ""; m_TextLabels[8] = ""; m_TextLabels[9] = ""; m_TextLabels[10] = ""; m_TextLabels[11] = ""; m_TextLabels[12] = ""; m_TextLabels[13] = ""; m_TextLabels[14] = ""; m_TextLabels[15] = ""; m_TextLabels[16] = ""; m_TextLabels[17] = ""; m_TextLabels[18] = ""; m_TextLabels[19] = ""; m_TextLabels[20] = ""; m_TextLabels[21] = ""; m_TextLabels[22] = ""; m_TextLabels[23] = ""; m_TextLabels[24] = ""; m_TextLabels[25] = ""; m_TextLabels[26] = ""; m_TextLabels[27] = ""; m_TextLabels[28] = ""; m_TextLabels[29] = ""; m_TextLabels[30] = ""; m_TextLabels[31] = ""; m_TextLabels[32] = ""; m_TextLabels[33] = ""; m_TextLabels[34] = ""; m_TextLabels[35] = ""; m_TextLabels[36] = ""; m_TextLabels[37] = ""; m_TextLabels[38] = ""; m_TextLabels[39] = ""; m_TextLabels[40] = ""; m_TextLabels[41] = ""; m_TextLabels[42] = ""; m_TextLabels[43] = ""; m_TextLabels[44] = ""; m_TextLabels[45] = ""; m_TextLabels[46] = ""; m_TextLabels[47] = ""; m_TextLabels[48] = ""; m_TextLabels[49] = ""; m_TextLabels[50] = ""; m_TextLabels[51] = ""; m_TextLabels[52] = ""; m_TextLabels[53] = ""; m_TextLabels[54] = ""; m_TextLabels[55] = ""; m_TextLabels[56] = ""; m_TextLabels[57] = ""; m_TextLabels[58] = ""; m_TextLabels[59] = ""; m_TextLabels[60] = ""; m_TextLabels[61] = ""; m_TextLabels[62] = ""; m_TextLabels[63] = ""; 

	m_uLabelCX[0] = 0; m_uLabelCX[1] = 0; m_uLabelCX[2] = 0; m_uLabelCX[3] = 0; m_uLabelCX[4] = 0; m_uLabelCX[5] = 0; m_uLabelCX[6] = 0; m_uLabelCX[7] = 0; m_uLabelCX[8] = 0; m_uLabelCX[9] = 0; m_uLabelCX[10] = 0; m_uLabelCX[11] = 0; m_uLabelCX[12] = 0; m_uLabelCX[13] = 0; m_uLabelCX[14] = 0; m_uLabelCX[15] = 0; m_uLabelCX[16] = 0; m_uLabelCX[17] = 0; m_uLabelCX[18] = 0; m_uLabelCX[19] = 0; m_uLabelCX[20] = 0; m_uLabelCX[21] = 0; m_uLabelCX[22] = 0; m_uLabelCX[23] = 0; m_uLabelCX[24] = 0; m_uLabelCX[25] = 0; m_uLabelCX[26] = 0; m_uLabelCX[27] = 0; m_uLabelCX[28] = 0; m_uLabelCX[29] = 0; m_uLabelCX[30] = 0; m_uLabelCX[31] = 0; m_uLabelCX[32] = 0; m_uLabelCX[33] = 0; m_uLabelCX[34] = 0; m_uLabelCX[35] = 0; m_uLabelCX[36] = 0; m_uLabelCX[37] = 0; m_uLabelCX[38] = 0; m_uLabelCX[39] = 0; m_uLabelCX[40] = 0; m_uLabelCX[41] = 0; m_uLabelCX[42] = 0; m_uLabelCX[43] = 0; m_uLabelCX[44] = 0; m_uLabelCX[45] = 0; m_uLabelCX[46] = 0; m_uLabelCX[47] = 0; m_uLabelCX[48] = 0; m_uLabelCX[49] = 0; m_uLabelCX[50] = 0; m_uLabelCX[51] = 0; m_uLabelCX[52] = 0; m_uLabelCX[53] = 0; m_uLabelCX[54] = 0; m_uLabelCX[55] = 0; m_uLabelCX[56] = 0; m_uLabelCX[57] = 0; m_uLabelCX[58] = 0; m_uLabelCX[59] = 0; m_uLabelCX[60] = 0; m_uLabelCX[61] = 0; m_uLabelCX[62] = 0; m_uLabelCX[63] = 0; 
	m_uLabelCY[0] = 0; m_uLabelCY[1] = 0; m_uLabelCY[2] = 0; m_uLabelCY[3] = 0; m_uLabelCY[4] = 0; m_uLabelCY[5] = 0; m_uLabelCY[6] = 0; m_uLabelCY[7] = 0; m_uLabelCY[8] = 0; m_uLabelCY[9] = 0; m_uLabelCY[10] = 0; m_uLabelCY[11] = 0; m_uLabelCY[12] = 0; m_uLabelCY[13] = 0; m_uLabelCY[14] = 0; m_uLabelCY[15] = 0; m_uLabelCY[16] = 0; m_uLabelCY[17] = 0; m_uLabelCY[18] = 0; m_uLabelCY[19] = 0; m_uLabelCY[20] = 0; m_uLabelCY[21] = 0; m_uLabelCY[22] = 0; m_uLabelCY[23] = 0; m_uLabelCY[24] = 0; m_uLabelCY[25] = 0; m_uLabelCY[26] = 0; m_uLabelCY[27] = 0; m_uLabelCY[28] = 0; m_uLabelCY[29] = 0; m_uLabelCY[30] = 0; m_uLabelCY[31] = 0; m_uLabelCY[32] = 0; m_uLabelCY[33] = 0; m_uLabelCY[34] = 0; m_uLabelCY[35] = 0; m_uLabelCY[36] = 0; m_uLabelCY[37] = 0; m_uLabelCY[38] = 0; m_uLabelCY[39] = 0; m_uLabelCY[40] = 0; m_uLabelCY[41] = 0; m_uLabelCY[42] = 0; m_uLabelCY[43] = 0; m_uLabelCY[44] = 0; m_uLabelCY[45] = 0; m_uLabelCY[46] = 0; m_uLabelCY[47] = 0; m_uLabelCY[48] = 0; m_uLabelCY[49] = 0; m_uLabelCY[50] = 0; m_uLabelCY[51] = 0; m_uLabelCY[52] = 0; m_uLabelCY[53] = 0; m_uLabelCY[54] = 0; m_uLabelCY[55] = 0; m_uLabelCY[56] = 0; m_uLabelCY[57] = 0; m_uLabelCY[58] = 0; m_uLabelCY[59] = 0; m_uLabelCY[60] = 0; m_uLabelCY[61] = 0; m_uLabelCY[62] = 0; m_uLabelCY[63] = 0; 

	m_pVectorJSProgram[JS_PROG_INDEX(0,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(0,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(1,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(2,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(3,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(4,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(5,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(6,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(7,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(8,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(9,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(10,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(11,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(12,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(13,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(14,6)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,0)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,1)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,2)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,3)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,4)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,5)] = 0.0000;
	m_pVectorJSProgram[JS_PROG_INDEX(15,6)] = 0.0000;


	m_JS_XCtrl.cControlName = "MIDI JS X";
	m_JS_XCtrl.uControlId = 32773;
	m_JS_XCtrl.uUserDataType = floatData;
	m_JS_XCtrl.bMIDIControl = false;
	m_JS_XCtrl.uMIDIControlCommand = 176;
	m_JS_XCtrl.uMIDIControlName = 16;
	m_JS_XCtrl.uMIDIControlChannel = 0;
	m_JS_XCtrl.fJoystickValue = 0.0;
	m_JS_XCtrl.bKorgVectorJoystickOrientation = true;
	m_JS_XCtrl.nGUIRow = nIndexer++;
	m_JS_XCtrl.bEnableParamSmoothing = false;
	m_JS_XCtrl.fSmoothingTimeInMs = 100.00;
	m_JS_XCtrl.dPresetData[0] = 0.000000;m_JS_XCtrl.dPresetData[1] = 0.000000;m_JS_XCtrl.dPresetData[2] = 0.000000;m_JS_XCtrl.dPresetData[3] = 0.000000;m_JS_XCtrl.dPresetData[4] = 0.000000;m_JS_XCtrl.dPresetData[5] = 0.000000;m_JS_XCtrl.dPresetData[6] = 0.000000;m_JS_XCtrl.dPresetData[7] = 0.000000;m_JS_XCtrl.dPresetData[8] = 0.000000;m_JS_XCtrl.dPresetData[9] = 0.000000;m_JS_XCtrl.dPresetData[10] = 0.000000;m_JS_XCtrl.dPresetData[11] = 0.000000;m_JS_XCtrl.dPresetData[12] = 0.000000;m_JS_XCtrl.dPresetData[13] = 0.000000;m_JS_XCtrl.dPresetData[14] = 0.000000;m_JS_XCtrl.dPresetData[15] = 0.000000;

	m_JS_YCtrl.cControlName = "MIDI JS Y";
	m_JS_YCtrl.uControlId = 32774;
	m_JS_XCtrl.uUserDataType = floatData;
	m_JS_YCtrl.bMIDIControl = false;
	m_JS_YCtrl.uMIDIControlCommand = 176;
	m_JS_YCtrl.uMIDIControlName = 17;
	m_JS_YCtrl.uMIDIControlChannel = 0;
	m_JS_YCtrl.fJoystickValue = 0.0;
	m_JS_YCtrl.bKorgVectorJoystickOrientation = true;
	m_JS_YCtrl.nGUIRow = nIndexer++;
	m_JS_YCtrl.bEnableParamSmoothing = false;
	m_JS_YCtrl.fSmoothingTimeInMs = 100.00;
	m_JS_YCtrl.dPresetData[0] = 0.000000;m_JS_YCtrl.dPresetData[1] = 0.000000;m_JS_YCtrl.dPresetData[2] = 0.000000;m_JS_YCtrl.dPresetData[3] = 0.000000;m_JS_YCtrl.dPresetData[4] = 0.000000;m_JS_YCtrl.dPresetData[5] = 0.000000;m_JS_YCtrl.dPresetData[6] = 0.000000;m_JS_YCtrl.dPresetData[7] = 0.000000;m_JS_YCtrl.dPresetData[8] = 0.000000;m_JS_YCtrl.dPresetData[9] = 0.000000;m_JS_YCtrl.dPresetData[10] = 0.000000;m_JS_YCtrl.dPresetData[11] = 0.000000;m_JS_YCtrl.dPresetData[12] = 0.000000;m_JS_YCtrl.dPresetData[13] = 0.000000;m_JS_YCtrl.dPresetData[14] = 0.000000;m_JS_YCtrl.dPresetData[15] = 0.000000;

	float* pJSProg = NULL;
	m_PresetNames[0] = "Factory Preset";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[0] = pJSProg;

	m_PresetNames[1] = "Epic Monster";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[1] = pJSProg;

	m_PresetNames[2] = "Short Reverb/Big Room";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[2] = pJSProg;

	m_PresetNames[3] = "Reverb/Small room(Maybe Drum Room)";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[3] = pJSProg;

	m_PresetNames[4] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[4] = pJSProg;

	m_PresetNames[5] = "Medium Room";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[5] = pJSProg;

	m_PresetNames[6] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[6] = pJSProg;

	m_PresetNames[7] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[7] = pJSProg;

	m_PresetNames[8] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[8] = pJSProg;

	m_PresetNames[9] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[9] = pJSProg;

	m_PresetNames[10] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[10] = pJSProg;

	m_PresetNames[11] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[11] = pJSProg;

	m_PresetNames[12] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[12] = pJSProg;

	m_PresetNames[13] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[13] = pJSProg;

	m_PresetNames[14] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[14] = pJSProg;

	m_PresetNames[15] = "";
	pJSProg = new float[MAX_JS_PROGRAM_STEPS*MAX_JS_PROGRAM_STEP_VARS];
	pJSProg[JS_PROG_INDEX(0,0)] = 0.000000;pJSProg[JS_PROG_INDEX(0,1)] = 0.000000;pJSProg[JS_PROG_INDEX(0,2)] = 0.000000;pJSProg[JS_PROG_INDEX(0,3)] = 0.000000;pJSProg[JS_PROG_INDEX(0,4)] = 0.000000;pJSProg[JS_PROG_INDEX(0,5)] = 0.000000;pJSProg[JS_PROG_INDEX(0,6)] = 0.000000;pJSProg[JS_PROG_INDEX(1,0)] = 0.000000;pJSProg[JS_PROG_INDEX(1,1)] = 0.000000;pJSProg[JS_PROG_INDEX(1,2)] = 0.000000;pJSProg[JS_PROG_INDEX(1,3)] = 0.000000;pJSProg[JS_PROG_INDEX(1,4)] = 0.000000;pJSProg[JS_PROG_INDEX(1,5)] = 0.000000;pJSProg[JS_PROG_INDEX(1,6)] = 0.000000;pJSProg[JS_PROG_INDEX(2,0)] = 0.000000;pJSProg[JS_PROG_INDEX(2,1)] = 0.000000;pJSProg[JS_PROG_INDEX(2,2)] = 0.000000;pJSProg[JS_PROG_INDEX(2,3)] = 0.000000;pJSProg[JS_PROG_INDEX(2,4)] = 0.000000;pJSProg[JS_PROG_INDEX(2,5)] = 0.000000;pJSProg[JS_PROG_INDEX(2,6)] = 0.000000;pJSProg[JS_PROG_INDEX(3,0)] = 0.000000;pJSProg[JS_PROG_INDEX(3,1)] = 0.000000;pJSProg[JS_PROG_INDEX(3,2)] = 0.000000;pJSProg[JS_PROG_INDEX(3,3)] = 0.000000;pJSProg[JS_PROG_INDEX(3,4)] = 0.000000;pJSProg[JS_PROG_INDEX(3,5)] = 0.000000;pJSProg[JS_PROG_INDEX(3,6)] = 0.000000;pJSProg[JS_PROG_INDEX(4,0)] = 0.000000;pJSProg[JS_PROG_INDEX(4,1)] = 0.000000;pJSProg[JS_PROG_INDEX(4,2)] = 0.000000;pJSProg[JS_PROG_INDEX(4,3)] = 0.000000;pJSProg[JS_PROG_INDEX(4,4)] = 0.000000;pJSProg[JS_PROG_INDEX(4,5)] = 0.000000;pJSProg[JS_PROG_INDEX(4,6)] = 0.000000;pJSProg[JS_PROG_INDEX(5,0)] = 0.000000;pJSProg[JS_PROG_INDEX(5,1)] = 0.000000;pJSProg[JS_PROG_INDEX(5,2)] = 0.000000;pJSProg[JS_PROG_INDEX(5,3)] = 0.000000;pJSProg[JS_PROG_INDEX(5,4)] = 0.000000;pJSProg[JS_PROG_INDEX(5,5)] = 0.000000;pJSProg[JS_PROG_INDEX(5,6)] = 0.000000;pJSProg[JS_PROG_INDEX(6,0)] = 0.000000;pJSProg[JS_PROG_INDEX(6,1)] = 0.000000;pJSProg[JS_PROG_INDEX(6,2)] = 0.000000;pJSProg[JS_PROG_INDEX(6,3)] = 0.000000;pJSProg[JS_PROG_INDEX(6,4)] = 0.000000;pJSProg[JS_PROG_INDEX(6,5)] = 0.000000;pJSProg[JS_PROG_INDEX(6,6)] = 0.000000;pJSProg[JS_PROG_INDEX(7,0)] = 0.000000;pJSProg[JS_PROG_INDEX(7,1)] = 0.000000;pJSProg[JS_PROG_INDEX(7,2)] = 0.000000;pJSProg[JS_PROG_INDEX(7,3)] = 0.000000;pJSProg[JS_PROG_INDEX(7,4)] = 0.000000;pJSProg[JS_PROG_INDEX(7,5)] = 0.000000;pJSProg[JS_PROG_INDEX(7,6)] = 0.000000;pJSProg[JS_PROG_INDEX(8,0)] = 0.000000;pJSProg[JS_PROG_INDEX(8,1)] = 0.000000;pJSProg[JS_PROG_INDEX(8,2)] = 0.000000;pJSProg[JS_PROG_INDEX(8,3)] = 0.000000;pJSProg[JS_PROG_INDEX(8,4)] = 0.000000;pJSProg[JS_PROG_INDEX(8,5)] = 0.000000;pJSProg[JS_PROG_INDEX(8,6)] = 0.000000;pJSProg[JS_PROG_INDEX(9,0)] = 0.000000;pJSProg[JS_PROG_INDEX(9,1)] = 0.000000;pJSProg[JS_PROG_INDEX(9,2)] = 0.000000;pJSProg[JS_PROG_INDEX(9,3)] = 0.000000;pJSProg[JS_PROG_INDEX(9,4)] = 0.000000;pJSProg[JS_PROG_INDEX(9,5)] = 0.000000;pJSProg[JS_PROG_INDEX(9,6)] = 0.000000;pJSProg[JS_PROG_INDEX(10,0)] = 0.000000;pJSProg[JS_PROG_INDEX(10,1)] = 0.000000;pJSProg[JS_PROG_INDEX(10,2)] = 0.000000;pJSProg[JS_PROG_INDEX(10,3)] = 0.000000;pJSProg[JS_PROG_INDEX(10,4)] = 0.000000;pJSProg[JS_PROG_INDEX(10,5)] = 0.000000;pJSProg[JS_PROG_INDEX(10,6)] = 0.000000;pJSProg[JS_PROG_INDEX(11,0)] = 0.000000;pJSProg[JS_PROG_INDEX(11,1)] = 0.000000;pJSProg[JS_PROG_INDEX(11,2)] = 0.000000;pJSProg[JS_PROG_INDEX(11,3)] = 0.000000;pJSProg[JS_PROG_INDEX(11,4)] = 0.000000;pJSProg[JS_PROG_INDEX(11,5)] = 0.000000;pJSProg[JS_PROG_INDEX(11,6)] = 0.000000;pJSProg[JS_PROG_INDEX(12,0)] = 0.000000;pJSProg[JS_PROG_INDEX(12,1)] = 0.000000;pJSProg[JS_PROG_INDEX(12,2)] = 0.000000;pJSProg[JS_PROG_INDEX(12,3)] = 0.000000;pJSProg[JS_PROG_INDEX(12,4)] = 0.000000;pJSProg[JS_PROG_INDEX(12,5)] = 0.000000;pJSProg[JS_PROG_INDEX(12,6)] = 0.000000;pJSProg[JS_PROG_INDEX(13,0)] = 0.000000;pJSProg[JS_PROG_INDEX(13,1)] = 0.000000;pJSProg[JS_PROG_INDEX(13,2)] = 0.000000;pJSProg[JS_PROG_INDEX(13,3)] = 0.000000;pJSProg[JS_PROG_INDEX(13,4)] = 0.000000;pJSProg[JS_PROG_INDEX(13,5)] = 0.000000;pJSProg[JS_PROG_INDEX(13,6)] = 0.000000;pJSProg[JS_PROG_INDEX(14,0)] = 0.000000;pJSProg[JS_PROG_INDEX(14,1)] = 0.000000;pJSProg[JS_PROG_INDEX(14,2)] = 0.000000;pJSProg[JS_PROG_INDEX(14,3)] = 0.000000;pJSProg[JS_PROG_INDEX(14,4)] = 0.000000;pJSProg[JS_PROG_INDEX(14,5)] = 0.000000;pJSProg[JS_PROG_INDEX(14,6)] = 0.000000;pJSProg[JS_PROG_INDEX(15,0)] = 0.000000;pJSProg[JS_PROG_INDEX(15,1)] = 0.000000;pJSProg[JS_PROG_INDEX(15,2)] = 0.000000;pJSProg[JS_PROG_INDEX(15,3)] = 0.000000;pJSProg[JS_PROG_INDEX(15,4)] = 0.000000;pJSProg[JS_PROG_INDEX(15,5)] = 0.000000;pJSProg[JS_PROG_INDEX(15,6)] = 0.000000;
	m_PresetJSPrograms[15] = pJSProg;

	// --- v6.8 thread-safe additions
	int nNumParams = m_UIControlList.count() + numAddtlParams;// numAddtlParams = 2 params from VJStick
	m_uControlListCount = m_UIControlList.count();

	// --- create fast lookup table of controls
	if(m_ppControlTable) delete [] m_ppControlTable;
	m_ppControlTable = new CUICtrl*[nNumParams];

	// --- create outbound GUI Parameters -----------------------------------------------
	if(m_pOutGUIParameters) delete [] m_pOutGUIParameters;
	m_pOutGUIParameters = new GUI_PARAMETER[nNumParams];
	memset(m_pOutGUIParameters, 0, sizeof(GUI_PARAMETER)*nNumParams);
	for(int i=0; i<m_UIControlList.count(); i++)
	{
		// --- save the CUICtrl* for fast lookups
		CUICtrl* pUICtrl = m_UIControlList.getAt(i);
		if(!pUICtrl) continue; // will never happen
		m_ppControlTable[i] = pUICtrl;

		// --- setup the outbound GUI Parameters
		m_pOutGUIParameters[i].fActualValue = 0.0;
		m_pOutGUIParameters[i].fNormalizedValue = 0.0;
		m_pOutGUIParameters[i].uControlId = pUICtrl->uControlId;
		m_pOutGUIParameters[i].nControlIndex = i;
		m_pOutGUIParameters[i].uSampleOffset = 0;
		m_pOutGUIParameters[i].bKorgVectorJoystickOrientation = false;
	}


	// **--0xEDA5--**
// ------------------------------------------------------------------------------- //

	return true;

}
