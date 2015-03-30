/* Mirror.h
* Encapsulates functions for setting up and rendering a mirror
*
* Sam Serrels, Computer Graphics, 2015
*/

#pragma once
#include "libENUgraphics\mesh.h"

using namespace graphics_framework;

// loads shaders and creates a FBO for reflected texture
void SetupMirror();
void RenderMirror(mesh &mesh);
void FreezeMirror(bool b);