#include "Rendering.h"
#include "main.h"
#include <GL\glew.h>
#include "libENUgraphics\effect.h"
#include "libENUgraphics\renderer.h"
#include "libENUgraphics\directional_light.h"
#include "libENUgraphics\spot_light.h"
#include "libENUgraphics\point_light.h"
#include <glm\glm.hpp>
#include <glm\gtx\transform.hpp>

using namespace glm;
static GLuint depthVisFbo = -1;
static GLuint fbo = -1;
static GLuint fbo_textures[GBUFFER_NUM_TEXTURES];
static GLuint fbo_depthTexture = -1;
static GLuint fbo_infoTexture = -1;
static GLuint fbo_finalTexture = -1;
static RenderMode renderMode;
static DefferedMode defMode;

RenderMode getRM() { return renderMode; }
DefferedMode getDM() { return defMode; }

static const float lightScale = 30.0f;
static const vec3 lightcolour = vec3(0.2f, 1.0f, 0.0f);
static const vec3 lightcolour2 = vec3(1.0f, 0.8f, 0.8f);

// combines all buffers to one buffer
static graphics_framework::effect df_lighttest;
static graphics_framework::effect depthstencilvisEffect;
static graphics_framework::effect pointLightPassEffect;
static graphics_framework::effect directionalLightPassEffect;

void SetMode(const RenderMode rm, const DefferedMode dm) {
  renderMode = rm;
  defMode = dm;
  if (rm && fbo == -1) {
    CreateDeferredFbo();
  }
}

void BeginOpaque() {
  if (renderMode) {
    // Deferred, render to fbo
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    // clear the final buffer
    glDrawBuffer(FINALBUFFER);
    glClearColor(0, 0, 0, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    // set drawmode
    GLenum DrawBuffers[] = {POSITIONBUFFER, DIFFUSEBUFFER, NORMALBUFFER, TEXCOORDBUFFER, INFOBUFFER};
    glDrawBuffers(5, DrawBuffers);
    // Only the geometry pass updates the depth buffer
    glDepthMask(GL_TRUE);
    // clear all buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

  } else {
    // forward, render to 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
  }
}

void EndOpaque() {

  if (renderMode) {
    // stencil pass nneds depth buffer, but doesn't write disable writing to depth
    glDepthMask(GL_FALSE);

    if (defMode == DEBUG_PASSTHROUGH) {
      CombineToOuput();
    } else if (defMode == DEBUG_COMBINE) {
      /*if (df_lighttest.get_program() == NULL) {
        df_lighttest.add_shader("shaders\\null.vert", GL_VERTEX_SHADER);
        df_lighttest.add_shader("shaders\\df_lighttest.frag", GL_FRAGMENT_SHADER);
        df_lighttest.build();
      }
      CombineToFinalBuffer();
      FlipToOutput();
      */
      glEnable(GL_STENCIL_TEST);
      for (point_light *p : gfx->PLights) {
        StencilPass(*p);
        PointLightPass(*p);
      }

      // The directional light does not need a stencil test because its volume is unlimited
      // and the final pass simply copies the texture.
      glDisable(GL_STENCIL_TEST);
      DirectionalLightPass();
      // light pass
      CombineToOuput();
    } else {
      glEnable(GL_STENCIL_TEST);
      for (point_light *p : gfx->PLights) {
        StencilPass(*p);
        PointLightPass(*p);
      }

      // The directional light does not need a stencil test because its volume is unlimited
      // and the final pass simply copies the texture.
      glDisable(GL_STENCIL_TEST);
      DirectionalLightPass();
      // light pass
      FlipToOutput();
    }

  } else {
  }
}

void BeginTransparent() { return; }

void EndTransparent() { return; }
void BeginPost() { return; }
void EndPost() { return; }

void NewFrame() {
  if (renderMode) {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
    glDrawBuffer(INFOBUFFER);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);
  }
}

void CreateDeferredFbo() {
  // Create the FBO
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  // Create the gbuffer textures
  glGenTextures(GBUFFER_NUM_TEXTURES, fbo_textures);
  glGenTextures(1, &fbo_depthTexture);
  glGenTextures(1, &fbo_finalTexture);

  for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
    glBindTexture(GL_TEXTURE_2D, fbo_textures[i]);
    // setup dimensions, fill with Nulll
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    // attach to one of the fbo outputs
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, fbo_textures[i], 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }
  // info texture
  {
    glBindTexture(GL_TEXTURE_2D, fbo_infoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, INFOBUFFER, GL_TEXTURE_2D, fbo_infoTexture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  }

  // depth
  {
    glBindTexture(GL_TEXTURE_2D, fbo_depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, SCREENWIDTH, SCREENHEIGHT, 0, GL_DEPTH_STENCIL,
                 GL_FLOAT_32_UNSIGNED_INT_24_8_REV, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, DEPTH_STENCIL_BUFFER, GL_TEXTURE_2D, fbo_depthTexture, 0);
  }

  // final
  glBindTexture(GL_TEXTURE_2D, fbo_finalTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, FINALBUFFER, GL_TEXTURE_2D, fbo_finalTexture, 0);

  // set drawmode
  GLenum DrawBuffers[] = {POSITIONBUFFER, DIFFUSEBUFFER, NORMALBUFFER, TEXCOORDBUFFER, INFOBUFFER};
  glDrawBuffers(5, DrawBuffers);

  GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

  if (Status != GL_FRAMEBUFFER_COMPLETE) {
    printf("FB error, status: 0x%x\n", Status);
    assert(false);
  }

  // restore default FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// Combines the fbo textures in one shader, output to FINALBUFFER
void CombineToFinalBuffer() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
  const graphics_framework::effect &eff = df_lighttest;
  // Bind effect
  graphics_framework::renderer::bind(eff);

  glDrawBuffer(FINALBUFFER);
  glClear(GL_COLOR_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, fbo_textures[GBUFFER_TEXTURE_TYPE_POSITION + i]);
  }
  // info
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_2D, fbo_infoTexture);

  glUniform2f(eff.get_uniform_location("gScreenSize"), (float)SCREENWIDTH, (float)SCREENHEIGHT);
  glUniform1i(eff.get_uniform_location("gPositionMap"), GBUFFER_TEXTURE_TYPE_POSITION);
  glUniform1i(eff.get_uniform_location("gColorMap"), GBUFFER_TEXTURE_TYPE_DIFFUSE);
  glUniform1i(eff.get_uniform_location("gNormalMap"), GBUFFER_TEXTURE_TYPE_NORMAL);
  glUniform1i(eff.get_uniform_location("gInfoMap"), 5);

  auto MVP = mat4(1.0f);
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // glDisable(GL_DEPTH_TEST);
  // glDepthMask(GL_FALSE);
  graphics_framework::renderer::render(gfx->planegeo);
  glEnable(GL_CULL_FACE);
}

static GLuint dsView_depthtex = -1;
static GLuint dsView_stenciltex = -1;
void ShowDepthStencil() {
  if (!GLEW_ARB_stencil_texturing) {
    return;
  }

  if (dsView_depthtex == -1) {
    
    glGenTextures(1, &dsView_depthtex);
    glBindTexture(GL_TEXTURE_2D, dsView_depthtex);
    glTextureView(dsView_depthtex, GL_TEXTURE_2D, fbo_depthTexture, GL_R32F, 0, 1, 0, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);


    GLenum error = glGetError();
    // If there is an error display message
    if (error) {
      // Display error
      std::cerr << "texview failed " << gluErrorString(error) << std::endl;
    }
    printf("DONE \n");
  }
  /*
  if (dsView_stenciltex == -1)
  {
    glGenTextures(1, &dsView_stenciltex);
    glBindTexture(GL_TEXTURE_2D, dsView_stenciltex);
    glTextureView(dsView_stenciltex, GL_TEXTURE_2D, fbo_depthTexture, GL_DEPTH32F_STENCIL8, 0, 1, 0, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_STENCIL_INDEX);

   // unsigned char * pixels = new unsigned char[SCREENWIDTH * SCREENHEIGHT * 32];
    //glGetTexImage(GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &pixels);
    //return;
  }*/

  // fuck this shit
  // return;
  static GLuint depthtex;
  static GLuint stenciltex;
  if (depthVisFbo == -1) {
    // Create the FBO
    glGenFramebuffers(1, &depthVisFbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthVisFbo);
    // Create the textures
    glGenTextures(1, &depthtex);
    glGenTextures(1, &stenciltex);

    glBindTexture(GL_TEXTURE_2D, depthtex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depthtex, 0);
    glBindTexture(GL_TEXTURE_2D, stenciltex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCREENWIDTH, SCREENHEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, stenciltex, 0);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
      printf("FB error, status: 0x%x\n", Status);
      assert(false);
    }
  }

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, depthVisFbo);

  // set drawmode
  GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, DrawBuffers);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // bind and read depth buffer from fbo
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, dsView_depthtex);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, dsView_stenciltex);

  if (depthstencilvisEffect.get_program() == NULL) {
    depthstencilvisEffect.add_shader("shaders\\null.vert", GL_VERTEX_SHADER);
    depthstencilvisEffect.add_shader("shaders\\depthstencilvis.frag", GL_FRAGMENT_SHADER);
    depthstencilvisEffect.build();
  }

  const graphics_framework::effect &eff = depthstencilvisEffect;
  graphics_framework::renderer::bind(eff);
  glUniform2f(eff.get_uniform_location("gScreenSize"), (float)SCREENWIDTH, (float)SCREENHEIGHT);
  glUniform1i(eff.get_uniform_location("depthTex"), 0);
  glUniform1i(eff.get_uniform_location("stencilTex"), 1);
  // render!
  auto MVP = mat4(1.0f);
  // Set MVP matrix uniform
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  // glDisable(GL_DEPTH_TEST);
  // glDepthMask(GL_FALSE);
  graphics_framework::renderer::render(gfx->planegeo);

  // restore default FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

// blits the buffers to 4 quadrants on the the 0 buffer
void CombineToOuput() {
  ShowDepthStencil();

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

  const GLsizei HalfWidth = (GLsizei)(SCREENWIDTH / 2.0f);
  const GLsizei HalfHeight = (GLsizei)(SCREENHEIGHT / 2.0f);
  const GLsizei QuaterWidth = (GLsizei)(SCREENWIDTH / 4.0f);
  const GLsizei QuaterHeight = (GLsizei)(SCREENHEIGHT / 4.0f);
  const GLsizei ThirdWidth = (GLsizei)(SCREENWIDTH / 3.0f);
  const GLsizei ThirdHeight = (GLsizei)(SCREENHEIGHT / 3.0f);

  const int amount = 6;
  const int buffers[amount] = {POSITIONBUFFER, DIFFUSEBUFFER, NORMALBUFFER, TEXCOORDBUFFER, FINALBUFFER, INFOBUFFER};

  const GLsizei positions[] = {
      0, 0, QuaterWidth, HalfHeight, QuaterWidth, 0, 2 * QuaterWidth, HalfHeight, 2 * QuaterWidth, 0, 3 * QuaterWidth,
      HalfHeight,
      // row2
      0, HalfHeight, QuaterWidth, SCREENHEIGHT, QuaterWidth, HalfHeight, 2 * QuaterWidth, SCREENHEIGHT, 2 * QuaterWidth,
      HalfHeight, 3 * QuaterWidth, SCREENHEIGHT,
  };
  unsigned int i = 0;
  for (int buffer : buffers) {
    glReadBuffer(buffer);
    glBlitFramebuffer(0, 0, SCREENWIDTH, SCREENHEIGHT, positions[i], positions[i + 1], positions[i + 2],
                      positions[i + 3], GL_COLOR_BUFFER_BIT, GL_LINEAR);
    i += 4;
  }
  if (GLEW_ARB_stencil_texturing) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, depthVisFbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, SCREENWIDTH, SCREENHEIGHT, 3 * QuaterWidth, 0, SCREENWIDTH, HalfHeight, GL_COLOR_BUFFER_BIT,
                      GL_LINEAR);

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, SCREENWIDTH, SCREENHEIGHT, 3 * QuaterWidth, HalfHeight, SCREENWIDTH, SCREENHEIGHT,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}

void FlipToOutput() {
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
  glReadBuffer(FINALBUFFER);

  glBlitFramebuffer(0, 0, SCREENWIDTH, SCREENHEIGHT, 0, 0, SCREENWIDTH, SCREENHEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

// calculates the size of the bounding box for the specified light source
float CalcPointLightBSphere(const point_light &Light) {
  float MaxChannel = fmax(fmax(Light.get_light_colour().x, Light.get_light_colour().y), Light.get_light_colour().z);

  float ret =
      (-Light.get_linear_attenuation() +
       sqrtf(Light.get_linear_attenuation() * Light.get_linear_attenuation() -
             4 * Light.get_quadratic_attenuation() * (Light.get_quadratic_attenuation() - 256 * MaxChannel * 1.0f))) /
      2 * Light.get_quadratic_attenuation();
  return ret;
}

void StencilPass(const graphics_framework::point_light &p) {
  // effect eff = nullEffect;
  effect eff = gfx->nullEffect;
  // Bind effect
  renderer::bind(eff);

  // Disable color/depth write and enable stencil
  glDrawBuffer(GL_NONE);
  glEnable(GL_DEPTH_TEST);

  glDisable(GL_CULL_FACE);
  glClear(GL_STENCIL_BUFFER_BIT);

  // We need the stencil test to be enabled but we want it
  // to succeed always. Only the depth test matters.
  glStencilFunc(GL_ALWAYS, 0, 0);

  glStencilOpSeparate(GL_BACK, GL_KEEP, GL_INCR_WRAP, GL_KEEP);
  glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_DECR_WRAP, GL_KEEP);

  auto VP = gfx->activeCam->get_projection() * gfx->activeCam->get_view();
  auto M = glm::translate(p.get_position()) * glm::scale(vec3(p.GetRange()));
  auto MVP = VP * M;
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  renderer::render(gfx->sphereMesh);
}

void PointLightPass(const graphics_framework::point_light &p) {
  glDrawBuffer(FINALBUFFER);
  // bind the fbo textures as input textures
  for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, fbo_textures[GBUFFER_TEXTURE_TYPE_POSITION + i]);
  }
  // info
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_2D, fbo_infoTexture);

  if (pointLightPassEffect.get_program() == NULL) {
    pointLightPassEffect.add_shader("shaders\\null.vert", GL_VERTEX_SHADER);
    pointLightPassEffect.add_shader("shaders\\point_light_pass.frag", GL_FRAGMENT_SHADER);
    pointLightPassEffect.build();
  }

  effect eff = pointLightPassEffect;
  // Bind effect
  renderer::bind(eff);

  glUniform3fv(eff.get_uniform_location("gEyeWorldPos"), 1, &gfx->activeCam->get_position()[0]);
  glUniform2f(eff.get_uniform_location("gScreenSize"), (float)SCREENWIDTH, (float)SCREENHEIGHT);
  glUniform1i(eff.get_uniform_location("gPositionMap"), GBUFFER_TEXTURE_TYPE_POSITION);
  glUniform1i(eff.get_uniform_location("gColorMap"), GBUFFER_TEXTURE_TYPE_DIFFUSE);
  glUniform1i(eff.get_uniform_location("gNormalMap"), GBUFFER_TEXTURE_TYPE_NORMAL);
  glUniform1i(eff.get_uniform_location("gInfoMap"), 5);
  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);

  glUniform3fv(eff.get_uniform_location("gPointLight.Base.Color"), 1, &p.get_light_colour()[0]);
  glUniform1f(eff.get_uniform_location("gPointLight.Base.AmbientIntensity"), 0.0f);
  glUniform1f(eff.get_uniform_location("gPointLight.Base.DiffuseIntensity"), 0.9f);
  glUniform1f(eff.get_uniform_location("gPointLight.Atten.Constant"), 0.0f);
  glUniform1f(eff.get_uniform_location("gPointLight.Atten.Linear"), 0.0f);
  glUniform1f(eff.get_uniform_location("gPointLight.Atten.Exp"), 0.01f);

  glUniform3fv(eff.get_uniform_location("gPointLight.Position"), 1, &p.get_position()[0]);
  auto VP = gfx->activeCam->get_projection() * gfx->activeCam->get_view();
  auto M = glm::translate(p.get_position()) * glm::scale(vec3(p.GetRange()));
  auto MVP = VP * M;
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  renderer::render(gfx->sphereMesh);

  glDisable(GL_BLEND);
}

void DirectionalLightPass() {
  glDrawBuffer(FINALBUFFER);
  // bind the fbo textures as input textures
  for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, fbo_textures[GBUFFER_TEXTURE_TYPE_POSITION + i]);
  }
  // info
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_2D, fbo_infoTexture);

  if (directionalLightPassEffect.get_program() == NULL) {
    directionalLightPassEffect.add_shader("shaders\\null.vert", GL_VERTEX_SHADER);
    directionalLightPassEffect.add_shader("shaders\\dir_light_pass.frag", GL_FRAGMENT_SHADER);
    directionalLightPassEffect.build();
  }

  effect eff = directionalLightPassEffect;
  renderer::bind(eff);
  glUniform2f(eff.get_uniform_location("gScreenSize"), (float)SCREENWIDTH, (float)SCREENHEIGHT);
  glUniform1i(eff.get_uniform_location("gPositionMap"), GBUFFER_TEXTURE_TYPE_POSITION);
  glUniform1i(eff.get_uniform_location("gColorMap"), GBUFFER_TEXTURE_TYPE_DIFFUSE);
  glUniform1i(eff.get_uniform_location("gNormalMap"), GBUFFER_TEXTURE_TYPE_NORMAL);
  glUniform1i(eff.get_uniform_location("gInfoMap"), 4);

  glUniform3fv(eff.get_uniform_location("gEyeWorldPos"), 1, &gfx->activeCam->get_position()[0]);
  //
  glUniform1f(eff.get_uniform_location("gMatSpecularIntensity"), 0.0f);
  glUniform1f(eff.get_uniform_location("gSpecularPower"), 1.0f);

  glUniform1f(eff.get_uniform_location("gDirectionalLight.Base.DiffuseIntensity"), 1.0f);

  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendEquation(GL_FUNC_ADD);
  glBlendFunc(GL_ONE, GL_ONE);

  // render fullscreen quad
  auto MVP = mat4(1.0f);
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

  for (directional_light *d : gfx->DLights) {
    glUniform1f(eff.get_uniform_location("gDirectionalLight.Base.AmbientIntensity"), 0.3f);
    glUniform3fv(eff.get_uniform_location("gDirectionalLight.Direction"), 1, &(*d).get_direction()[0]);
    glUniform3fv(eff.get_uniform_location("gDirectionalLight.Base.Color"), 1, &(*d).get_light_colour()[0]);
    graphics_framework::renderer::render(gfx->planegeo);
  }

  glDisable(GL_BLEND);
}