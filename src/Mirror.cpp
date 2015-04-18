/* Mirror.cpp
* Encapsulates functions for setting up and rendering a mirror
*
* Sam Serrels, Computer Graphics, 2015
*/
#include <glm\glm.hpp>
#include <glm\gtx\rotate_vector.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_access.hpp>
#include "Rendering.h"
#include "mirror.h"
#include "main.h"
#include "Math.h"
#include "Enviroment.h"
#include "libENUgraphics\graphics_framework.h"

using namespace std;
using namespace graphics_framework;
using namespace glm;

target_camera bouncecam;
static bool frozen = false;
static bool debug = false;
static bool show = false;
static mat4 frozenviewMat;

void FreezeMirror(bool b) {
  frozenviewMat = gfx->cam.get_view();
  frozen = b;
}
bool FreezeMirror() { return frozen; }
void DebugMirror(bool b) { debug = b; }
bool DebugMirror() { return debug; }

void ShowMirror(bool b) { show = b; }
bool ShowMirror() { return show; }

// loads shaders and creates a FBO for reflected texture
void SetupMirror() {
  frozen = false;
}

static void MakeWarpedProjMat(mat4 &projmat, mat4 &viewMat, const mat4 &playerView, const vec3 &mirrorPos,
                              const vec3 &mirrorNormal) {

  const mat4 reflectionMat = MirrorMatrix(mirrorNormal, mirrorPos);
  viewMat = playerView * reflectionMat;
  vec4 clipPlane4d(mirrorNormal, -dot(mirrorPos, mirrorNormal));
  clipPlane4d = glm::transpose(glm::inverse(viewMat)) * clipPlane4d;

  // Calculate the clip-space corner point opposite the clipping plane
  // as (sgn(clipPlane.x), sgn(clipPlane.y), 1, 1) and
  // transform it into camera space by multiplying it
  // by the inverse of the projection matrix
  vec4 q;
  q.x = (sgn(clipPlane4d.x) + projmat[2][0]) / projmat[0][0];
  q.y = (sgn(clipPlane4d.y) + projmat[2][1]) / projmat[1][1];
  q.z = -1.0F;
  q.w = (1.0F + projmat[2][2]) / projmat[3][2];

  // Calculate the scaled plane vector
  vec4 c = clipPlane4d * (2.0f / dot(clipPlane4d, q));

  // Replace the third row of the projection matrix
  projmat[0][2] = c.x;
  projmat[1][2] = c.y;
  projmat[2][2] = c.z + 1.0F;
  projmat[3][2] = c.w;
}

void MirrorStencilPass(mesh &mirror) {
  // Read from depth
  glEnable(GL_DEPTH_TEST);
  // don't write to depth or colour
  glDrawBuffer(GL_NONE);
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glDepthMask(GL_FALSE);
  // write only to stencil
  glEnable(GL_STENCIL_TEST);
  glStencilMask(0xFF);
  // clear stencil
  glClear(GL_STENCIL_BUFFER_BIT);

  // setup stencil functions
  // don't comnpare to existing stencil data , just write
  glStencilFunc(GL_ALWAYS, 0, 0);
  // Write stencil for both front and back, set to zero if depth trest fails
  glStencilOpSeparate(GL_BACK, GL_ZERO, GL_ZERO, GL_INCR);
  // todo could use this for easy side detection and culling rather than a plane EQ
  glStencilOpSeparate(GL_FRONT, GL_ZERO, GL_ZERO, GL_INCR);
  glDisable(GL_CULL_FACE);

  // do stencil
  effect eff = gfx->nullEffect;
  // Bind effect
  renderer::bind(eff);
  auto M = mirror.get_transform().get_transform_matrix();
  auto V = gfx->activeCam->get_view();
  auto P = gfx->activeCam->get_projection();
  auto MVP = P * V * M;
  glUniformMatrix4fv(eff.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));

  renderer::render(mirror);

  // enable color and depth buffers.
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  glDepthMask(GL_TRUE);
  // glDisable(GL_BLEND);
  // glDisable(GL_DEPTH_TEST);
  // disable writing to stencil
  glStencilMask(0x00);
  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
  if (getRM()) {
    GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                            GL_COLOR_ATTACHMENT5};
    glDrawBuffers(5, DrawBuffers);
  } else {
    glDrawBuffer(GL_BACK);
  }

  /*
  //testing, render full quad to test stencil
  glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
  //glDrawBuffer(GL_COLOR_ATTACHMENT1); //todo: remove this hack.
  GLenum DrawBuffers[] = { GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT5 };
  glDrawBuffers(2, DrawBuffers);

  renderer::bind(gfx->simpleEffect);
  MVP = mat4(1.0f);
  glUniformMatrix4fv(gfx->simpleEffect.get_uniform_location("MVP"), 1, GL_FALSE, value_ptr(MVP));
  //glDisable(GL_CULL_FACE);
  renderer::render(gfx->planegeo);
  glEnable(GL_CULL_FACE);
  glDisable(GL_STENCIL_TEST);*/
}

void RenderMirror(mesh &mirror) {
  if (!show) {
    return;
  }

  // render the reflected scene
  mat4 playerView;

  if (frozen) {
    playerView = frozenviewMat;
  } else {
    playerView = gfx->cam.get_view();
  }

  const vec3 mirrorPos = mirror.get_transform().position;
  const vec3 mirrorNormal = normalize(GetUpVector(mirror.get_transform().orientation));

  // we don't have clipping, so don't render the opposite side
  const float plneq = dot(mirrorNormal, (vec3(inverse(playerView)[3]) - mirrorPos));

  // reflected viewmat
  mat4 viewMat;
  // reflected projmat
  mat4 projmat;

  if (plneq > 0) {
    MirrorStencilPass(mirror);
    // create intial unwarped projection
    bouncecam.set_projection(quarter_pi<float>(), gfx->aspect, 2.414f, 2000.0f);
    projmat = bouncecam.get_projection();
    // get reflected view, and warped projection
    MakeWarpedProjMat(projmat, viewMat, playerView, mirrorPos, mirrorNormal);

    // apply to camera
    bouncecam.set_view(viewMat);
    bouncecam.set_projection2(projmat);
    gfx->activeCam = &bouncecam;

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_FRONT);
    gfx->DrawScene();

    // revert rendering back to normal
    glCullFace(GL_BACK);

    glDisable(GL_STENCIL_TEST);
    glStencilMask(0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    glStencilMask(0x00);

    gfx->activeCam = &gfx->cam;
  }

  // if frozen, show frustum
  if (frozen) {
    vec4 fr[8] = {// near
                  vec4(-1, -1, -1, 1), vec4(1, -1, -1, 1), vec4(1, 1, -1, 1), vec4(-1, 1, -1, 1),
                  // far
                  vec4(-1, -1, 1, 1), vec4(1, -1, 1, 1), vec4(1, 1, 1, 1), vec4(-1, 1, 1, 1)};

    for (int i = 0; i < 8; i++) {
      fr[i] = inverse(projmat * viewMat) * fr[i];
      //
      fr[i].x /= fr[i].w;
      fr[i].y /= fr[i].w;
      fr[i].z /= fr[i].w;
      fr[i].w = 1.0f;
      gfx->DrawCross(vec3(fr[i]), 1.0f);
    }
    gfx->DrawLine(vec3(fr[0]), vec3(fr[1]));
    gfx->DrawLine(vec3(fr[1]), vec3(fr[2]));
    gfx->DrawLine(vec3(fr[2]), vec3(fr[3]));
    gfx->DrawLine(vec3(fr[3]), vec3(fr[0]));
    gfx->DrawLine(vec3(fr[4]), vec3(fr[5]));
    gfx->DrawLine(vec3(fr[5]), vec3(fr[6]));
    gfx->DrawLine(vec3(fr[6]), vec3(fr[7]));
    gfx->DrawLine(vec3(fr[7]), vec3(fr[4]));
    gfx->DrawLine(vec3(fr[0]), vec3(fr[4]));
    gfx->DrawLine(vec3(fr[1]), vec3(fr[5]));
    gfx->DrawLine(vec3(fr[2]), vec3(fr[6]));
    gfx->DrawLine(vec3(fr[3]), vec3(fr[7]));
    gfx->ProcessLines();
  }
}