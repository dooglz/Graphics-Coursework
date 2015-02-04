// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define VC_EXTRALEAN
#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#define GLM_FORCE_RADIANS

#if !(defined(DEBUG) | defined(_DEBUG))
#define NDEBUG
#endif

// TODO: reference additional headers your program requires here
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <GL\glew.h>
#include <GLFW\glfw3.h>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <chrono>
#include <string>
#include <memory>
#include <map>
#include <initializer_list>
#include <random>
#include <functional>
#include <cassert>
#include <algorithm>