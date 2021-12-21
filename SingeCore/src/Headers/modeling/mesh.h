#pragma once

#include "math/vectors.h"
#include "csharp.h"

// options
typedef struct _mesh* Mesh;

struct _mesh {
	char* Name;
	mat4 Transform;
	bool SmoothingEnabled;
	size_t VertexCount;
	float* Vertices;
	size_t TextureCount;
	float* TextureVertices;
	size_t NormalCount;
	float* Normals;
	Mesh Next;
};

Mesh CreateMesh();