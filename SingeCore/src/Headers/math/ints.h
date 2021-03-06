#pragma once
#include <stdlib.h>
#include "csharp.h"

struct _intMethods {
	bool (*TryDeserialize)(const char* buffer, size_t bufferLength, size_t* out_value);
};

extern const struct _intMethods Ints;