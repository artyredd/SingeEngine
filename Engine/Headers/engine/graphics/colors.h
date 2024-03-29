#pragma once

#include "core/csharp.h"
#include "core/file.h"

typedef struct _color color;

struct _color
{
	float r;
	float g;
	float b;
	float a;
};

struct _colors {
	color Red;
	color Green;
	color Blue;
	color White;
	color Black;
	bool (*TryDeserialize)(const char* buffer, const ulong length, color* out_vector3);
	bool (*TrySerialize)(char* buffer, const ulong length, const color vector);
	bool (*TrySerializeStream)(File stream, const color vector);
};

extern const struct _colors Colors;
