#include "engine/graphics/light.h"
#include "core/memory.h"
#include "engine/defaults.h"
#include "string.h"
#include "GL/glew.h"

struct _shadowMapSettings ShadowMaps = {
	.ResolutionX = SHADOW_MAP_RESOLUTION_X,
	.ResolutionY = SHADOW_MAP_RESOLUTION_Y
};

static Light Create(LightType type);
static void Dispose(Light);
static void CreateFrameBuffer(Light light);

const struct _lightMethods Lights = {
	.Create = &Create,
	.Dispose = &Dispose,
	.CreateFrameBuffer = &CreateFrameBuffer
};

DEFINE_TYPE_ID(Light);

static Light Create(LightType type)
{
	Memory.RegisterTypeName(nameof(Light), &LightTypeId);

	Light light = Memory.Alloc(sizeof(struct _light), LightTypeId);

	light->Ambient = (color){
		DEFAULT_AMBIENT_LIGHT_INTENSITY,
		DEFAULT_AMBIENT_LIGHT_INTENSITY,
		DEFAULT_AMBIENT_LIGHT_INTENSITY,
		1
	};

	light->Diffuse = (color)
	{
		DEFAULT_DIFFUSE_LIGHT_INTENSITY,
		DEFAULT_DIFFUSE_LIGHT_INTENSITY,
		DEFAULT_DIFFUSE_LIGHT_INTENSITY,
		1
	};

	light->Specular = (color)
	{
		DEFAULT_SPECULAR_LIGHT_INTENSITY,
		DEFAULT_SPECULAR_LIGHT_INTENSITY,
		DEFAULT_SPECULAR_LIGHT_INTENSITY,
		1
	};

	light->Radius = DEFAULT_LIGHT_RADIUS;
	light->Range = DEFAULT_LIGHT_RANGE;
	light->EdgeSoftness = DEFAULT_LIGHT_EDGE_SOFTNESS;
	light->Type = type;
	light->Enabled = true;
	light->Intensity = DEFAULT_LIGHT_INTENSITY;

	light->Transform = Transforms.Create();

	light->Orthographic = true;

	CreateFrameBuffer(light);

	return light;
}

static void CreateFrameBuffer(Light light)
{
	if (light->FrameBuffer isnt null)
	{
		FrameBuffers.Dispose(light->FrameBuffer);
	}

	// create a frame buffer that has no color buffer (.None)
	FrameBuffer frameBuffer = FrameBuffers.Create(FrameBufferTypes.None);

	// mark it as the current framebuffer
	FrameBuffers.Use(frameBuffer);

	TextureType type = TextureTypes.Default;

	RawTexture depthBuffer;
	RawTextures.TryCreateBufferTexture(type, TextureFormats.DepthComponent, BufferFormats.Float, ShadowMaps.ResolutionX, ShadowMaps.ResolutionY, &depthBuffer);

	FrameBuffers.AttachTexture(frameBuffer, depthBuffer, 0);

	RawTextures.Dispose(depthBuffer);

	frameBuffer->ClearMask = ClearMasks.Depth;

	light->FrameBuffer = frameBuffer;
}

static void Dispose(Light light)
{
	if (light is null) return;

	Transforms.Dispose(light->Transform);
	FrameBuffers.Dispose(light->FrameBuffer);

	Memory.Free(light, LightTypeId);
}