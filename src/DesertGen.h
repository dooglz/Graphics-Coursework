#pragma once
namespace graphics_framework{
    class geometry;
}

class DesertGen
{
public:
    DesertGen();
    ~DesertGen();
    static graphics_framework::geometry* CreateDesert();
private:
    static graphics_framework::geometry _geo;
};

