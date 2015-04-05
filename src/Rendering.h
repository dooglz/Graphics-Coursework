#pragma once

const static enum RenderMode{
  FORWARD,
  DEFFERED
};
const static enum DefferedMode{
  DEBUG_PASSTHROUGH,
  DEBUG_COMBINE,
  NORMAL
};

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

void BeginOpaque();
void EndOpaque();

void BeginTransparent();
void EndTransparent();

void BeginPost();
void EndPost();