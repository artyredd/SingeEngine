#pragma once

#include "graphics/material.h"
#include "math/triangles.h"

struct _drawing
{
	void (*SetScene)(Scene scene);
	void (*DrawTriangle)(triangle triangle, Material material);
	void (*DrawDefaultTriangle)(triangle);
	void (*DrawCubeFromPoints)(vector3, vector3);
	void (*SetDefaultMaterial)(Material material);
};

extern const struct _drawing Drawing;