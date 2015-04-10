#pragma once

class Enviroment
{
public:
  static void Load();
  static void RenderSky();
  static void Update();
  static void Update(float delta_time);

  static float dayscale;
  static bool daymode;
  static bool enabled;
  static float speed;
private:

};
