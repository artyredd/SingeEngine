#pragma once

#include "modeling/model.h"
#include "math/vectors.h"
#include "graphics/shaders.h"
#include "graphics/transform.h"
#include "graphics/sharedBuffer.h"

/// <summary>
/// The default Shader that should be used 
/// </summary>
Shader DefaultShader;

static unsigned int VertexShaderPosition = 0;
static unsigned int UVShaderPosition = 1;
static unsigned int NormalShaderPosition = 2;

typedef struct _renderMesh* RenderMesh;

struct _renderMesh {
	Shader Shader;

	SharedHandle VertexBuffer;
	SharedHandle UVBuffer;
	SharedHandle NormalBuffer;

	size_t NumberOfTriangles;
	Transform Transform;
	void(*Draw)(RenderMesh, mat4 position);
	void(*Dispose)(RenderMesh);
};

// Attempts to register the model with the underlying graphics device
bool TryBindMesh(Mesh mesh, RenderMesh* out_model);

// Creates a new instance of the provided rendermesh with it's own transform
RenderMesh InstanceMesh(RenderMesh);