#include "BitCrusher.h"


BitCrusher::BitCrusher()
{
}

void BitCrusher::init() {
	crush = 16;
	downsample = 1;
	sampleL = 0.0;
	sampleR = 0.0;
	counter = downsample;
}

bool BitCrusher::process_audioframe(float **pInput, float** pOutput, int numsamples) {
	for (int i = 0; i < numsamples; i++) {
		// Only play back every <downsample> samples.
		if (--counter == 0) {
			sampleL = pInput[0][i];
			sampleR = pInput[1][i];
			counter = downsample;
		}
		// Bitcrush the samples.
		int in_left = (int)round((sampleL + 1.0) * 0.5 * 65536.0);
		int in_right = (int)round((sampleR + 1.0) * 0.5 * 65536.0);
		int out_left = keep_bits_from_16(in_left, crush);
		int out_right = keep_bits_from_16(in_right, crush);
		pOutput[0][i] = (((float)out_left / 65536.0) - 0.5) * 2.0;
		pOutput[1][i] = (((float)out_right / 65536.0) - 0.5) * 2.0;
	}
	return true;
}