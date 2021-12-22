#pragma once
#include "csharp.h"

#ifndef cglm_types_h
typedef _declspec(align(16)) float vec4[4];
typedef float vec3[3];
typedef float vec2[2];
#endif // !cglm_h

#ifndef cglm_mat_h
typedef _declspec(align(16)) vec4 mat4[4];
#endif // !cglm_mat_h

#define Vector2Equals(left,right) (left[0] ==  right[0] && left[1] == right[1])

#define Vector3Equals(left,right) (left[0] ==  right[0] && left[1] == right[1] && left[2] == right[2])

static void CopyTo(const float* source, float* destination, const size_t count)
{
	for (size_t i = 0; i < count; i++)
	{
		destination[i] = source[i];
	}
}

#define Vector3CopyTo(source,destination) CopyTo(source,destination, 3)
#define Vector2CopyTo(source,destination) CopyTo(source,destination, 2)

#define SetX(vector, x) vector[0] = x
#define SetY(vector, y) vector[1] = y
#define SetZ(vector, z) vector[2] = z
#define SetW(vector, w) vector[3] = w

#define SetVector2(vec2, x, y) vec2[0] = x; vec2[1] = y
#define SetVector3(vec3, x, y, z) vec3[0] = x; vec3[1] = y; vec3[2] = z
#define SetVector4(vec4, x, y, z, w) vec4[0] = x; vec4[1] = y; vec4[2] = z; vec4[3] = w

#define InitializeVector3(vec3) SetVector3(vec3,0,0,0);
#define InitializeVector4(vec3) SetVector4(vec3,0,0,0,0);
#define InitializeMat4(mat4) glm_mat4_identity(mat4)

static struct _vectorDirections {
	vec3 Zero;
	vec3 Up;
	vec3 Down;
	vec3 Right;
	vec3 Left;
	vec3 Forward;
	vec3 Back;
	/// <summary>
	/// Initalizes the provided vec3 wit all 0's
	/// </summary>
	void(*Initialize)(vec3);
} Vector3 = {
	.Up = { 0, 1, 0},
	.Down = { 0, -1, 0},
	.Left = { -1, 0, 0},
	.Right = { 1, 1, 0},
	.Forward = { 0, 0, 1},
	.Back = { 0, 1, -1}
};

bool TryParseVector3(char* buffer, vec3 out_vector);

bool TryParseVector2(char* buffer, vec3 out_vector);

bool RunVectorUnitTests();