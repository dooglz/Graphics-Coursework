/* DesertGen.h
* Generates a Desert
*
* Sam Serrels, Computer Graphics, 2015
*/
#pragma once

// forward decalre includes
namespace graphics_framework {
class geometry;
class mesh;
}

class DesertGen {
public:
  DesertGen();
  ~DesertGen();
  static void CreateDesert();
  static graphics_framework::mesh farMesh;

private:
  static graphics_framework::geometry _farGeo;
  static graphics_framework::geometry _nearGeo;
  static void makefarGeometry();
};
