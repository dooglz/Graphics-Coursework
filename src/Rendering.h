#pragma once
typedef enum RenderMode{
  FORWARD,
  DEFFERED
}RenderMode;

typedef enum DefferedMode{
  DEBUG_PASSTHROUGH,
  DEBUG_COMBINE,
  NORMAL
}DefferedMode;

enum GBUFFER_TEXTURE_TYPE {
  GBUFFER_TEXTURE_TYPE_POSITION,
  GBUFFER_TEXTURE_TYPE_DIFFUSE,
  GBUFFER_TEXTURE_TYPE_NORMAL,
  GBUFFER_TEXTURE_TYPE_TEXCOORD,
  GBUFFER_NUM_TEXTURES
};

#define POSITIONBUFFER GL_COLOR_ATTACHMENT0
#define DIFFUSEBUFFER GL_COLOR_ATTACHMENT1
#define NORMALBUFFER GL_COLOR_ATTACHMENT2
#define TEXCOORDBUFFER GL_COLOR_ATTACHMENT3
#define DEPTHBUFFER GL_DEPTH_ATTACHMENT //possibly should be GL_DEPTH_ATTACHMENT
#define FINALBUFFER GL_COLOR_ATTACHMENT4

void SetMode(const RenderMode rm, const DefferedMode dm);
RenderMode getRM();
DefferedMode getDM();

void BeginOpaque();
void EndOpaque();

void BeginTransparent();
void EndTransparent();

void BeginPost();
void EndPost();

static void CreateDeferredFbo();
static void CombineToFinalBuffer();
static void CombineToOuput();
static void FlipToOutput();
static void StencilPass(unsigned int PointLightIndex);
static void PointLightPass(unsigned int PointLightIndex);
static void DirectionalLightPass();