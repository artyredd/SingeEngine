#pragma once

#include "csharp.h"

struct _randomMethods
{
	// the seed that should be used for the random number generator
	// default: 42
	size_t Seed;
	// Whether or not a random seed should be chosen when the program starts
	// default: true
	bool RandomSeedOnStart;
	// Gets a random bool
	bool (*NextBool)(void);
	// Gets a random value between -1.0 and 1.0
	float (*NextFloat)(void);
	float (*BetweenFloat)(float lower, float upper);
	// Gets a random value between -INT_MAX, INT_MAX
	int (*NextInt)(void);
	int(*BetweenInt)(int lower, int upper);
	// Gets a random value between 0 and INT_MAX
	unsigned int(*NextUInt)(void);
	// Gets a random value between 0 and MAX_SIZE_T
	size_t(*NextSize_t)(void);
	size_t(*BetweenSize_t)(size_t lower, size_t upper);
};

extern struct _randomMethods Random;