#pragma once
#include "libENUgraphics\point_light.h"
namespace graphics_framework{
  class effect;
}

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
#define FINALBUFFER GL_COLOR_ATTACHMENT4
#define INFOBUFFER GL_COLOR_ATTACHMENT5
#define DEPTH_STENCIL_BUFFER GL_DEPTH_STENCIL_ATTACHMENT

//Mode setting
void stencilworkaround(bool b);
bool stencilworkaround();
void EnableSSBO(bool b);
bool EnableSSBO();
void SetMode(const RenderMode rm, const DefferedMode dm);
RenderMode getRM();
DefferedMode getDM();

//
graphics_framework::effect& BasicEffect();
graphics_framework::effect& NormalEffect();

//Render order funcs
void BeginOpaque();
void EndOpaque();
void BeginTransparent();
void EndTransparent();
void BeginPost();
void EndPost();
void NewFrame();
// Send all light data to on the GPU
void UpdateLights();

//internal funcs
static void UpdateLights_SSBO();
static void CreateDeferredFbo();
static void CombineToFinalBuffer();
static void CombineToOuput();
static void FlipToOutput();
static void StencilPass(const graphics_framework::point_light& p);
static void PointLightPass(const graphics_framework::point_light& p);
static void DirectionalLightPass();
static void CreateSSBOs();