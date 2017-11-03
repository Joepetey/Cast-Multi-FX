#pragma once
#include <cmath>
#include <stdint.h>
#include <cstring>
#include "plugin.h"

class BitCrusher {
private:
	float sampleL, sampleR;
	int crush, downsample, counter;
	inline int keep_bits_from_16(int input, int keep_bits) {
		// Only keep the last 16 bits.
		input &= 0xffff;
		input &= (-1 << (16 - keep_bits));
		// Only keep the last 16 bits.
		input &= 0xffff;
		return input;
	}

public:
	BitCrusher();
	virtual ~BitCrusher() {}
	virtual void init();
	virtual bool process_audioframe(float **pInput, float** pOutput, int numsamples);
};