/* Mirror.cpp
* Encapsulates functions for setting up and rendering a mirror
*
* Sam Serrels, Computer Graphics, 2015
*/
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include "mirror.h"
#include "main.h"
#include "Math.h"
#include "libENUgraphics\graphics_framework.h"

using namespace std;
using namespace graphics_framework;
using namespace glm;

target_camera bouncecam;

GLuint FramebufferName = 0;
GLuint renderedTexture = 0;
GLuint depthrenderbuffer = 0;
effect waterEffect;

// loads shaders and creates a FBO for reflected texture
void SetupMirror() {
  waterEffect.add_shader("shaders\\water.vert", GL_VERTEX_SHADER);
  waterEffect.add_shader("shaders\\water.frag", GL_FRAGMENT_SHADER);
  waterEffect.build();

  // Create the FBO
  glGenFramebuffers(1, &FramebufferName);
  // Bind to FBO
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FramebufferName);

  // Create the textures in the fbo
  glGenTextures(1, &renderedTexture);
  // bind it
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  // set dimensions,  Give an empty image to OpenGL ( the last "0" )
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  // tex filtering
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  // unbind
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);

  // Do the same for depth, but use a renderbuffer rather than texture
  glGenRenderbuffers(1, &depthrenderbuffer);
  glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
  // set dimensions, fill with undefined
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720);
  // attach to fbo depth output
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                            depthrenderbuffer);

  // unbind
  glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void RenderMirror(mesh &mirror) {
  // render the reflected scene into a texture
  {
    bouncecam.set_projection(quarter_pi<float>(), gfx->aspect, 2.414f, 2000.0f);

    vec3 mirrorPos = mirror.get_transform().position;
    vec3 mirrorNormal = normalize(GetUpVector(mirror.get_transform().orientation));
    mat4 reflectionMat = MirrorMatrix(mirrorNormal, mirrorPos);

    bouncecam.set_view(gfx->cam.get_view() * reflectionMat);
    // renderer::set_render_target(*mirrorFB);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);

    // Clear the depth and colour buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx->activeCam = &bouncecam;

    // Rerender scene
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    // glDisable(GL_CULL_FACE);
    for (auto &e : gfx->meshes) {
      gfx->rendermesh(e.second, gfx->checkedTexture);
    }
    // glDisable(GL_CULL_FACE);
    gfx->rendermesh(*gfx->desertM, gfx->sandTexture);
    gfx->renderSky();

    // end render
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gfx->activeCam = &gfx->cam;
  }

  // render texture onto plane
  renderer::bind(waterEffect);

  auto M = mirror.get_transform().get_transform_matrix();
  auto V = gfx->activeCam->get_view();
  auto P = gfx->activeCam->get_projection();
  auto MVP = P * V * M;

  auto RV = gfx->activeCam->get_view();
  auto RP = gfx->activeCam->get_projection();
  auto RMVP = RP * RV * M;

  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, renderedTexture);
  // Set our "renderedTexture" sampler to user Texture Unit 0
  glUniform1i(waterEffect.get_uniform_location("tex"), 0);

  // Set MVP matrix uniform
  glUniformMatrix4fv(waterEffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  glUniformMatrix4fv(waterEffect.get_uniform_location("reflected_MVP"), 1, GL_FALSE,
                     value_ptr(RMVP));
  glDisable(GL_CULL_FACE);
  renderer::render(mirror);
  glEnable(GL_CULL_FACE);
}