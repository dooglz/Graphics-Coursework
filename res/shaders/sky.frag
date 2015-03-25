#version 430
//https://www.shadertoy.com/view/XsBXDc
precision highp float;
const float hverticalcuttoff = -0.2;
const vec2 iResolution = vec2(1280,720);
uniform float iGlobalTime;           // shader playback time (in seconds)
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform mat3 playerrot;
uniform mat4 MVP;
//direction the player is facing
uniform vec3 playerview;

// Incoming vertex colour
layout (location = 0) in vec4 in_colour;
layout (location = 1) in vec4 dots;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

vec3 up = vec3(0,1,0);
//vec3 topcol = vec3(0.55,0.75,0.83);
//vec3 bottomcol = vec3(0.5,0.78,0.98);

vec4 calculateSun(){
    //vec4 sun = vec4(0.929,0.866,0.619,0);
    vec4 sun = vec4(1.0,0,0,1.0);
    //sun = vec4(0,0,0,0);
    const vec2 fragcord = gl_FragCoord.xy;

    vec2 screenPercent = 1.0 - (fragcord / iResolution);
    //screenPercent.y, 0.0 = bottom of screen, 1.0 = top

    //	dots = vec4(topdot, bottomdot, topdot, bottomdot);
    float Sy = (screenPercent.y * (dots.w - dots.z)) + dots.z;	//updown
    float Sx = (screenPercent.x * (dots.x - dots.y)) + dots.y;	//leftright = broken for now

    const vec2 sunPos = vec2(0.25,0.25);

    float Sundist = pow(Sy - sunPos.x, 2) * 2000.0;
    float SunAmount = 1.0 /  Sundist;
    sun *= clamp(SunAmount, 0.0, 1.0);
    
    return sun;
}

void main()
{
    //float f = dot(playerview,up);
    // Simply set outgoing colour
    //out_colour = vec4(mix(bottomcol,topcol,f),1.0);
    out_colour = calculateSun();
    ////vec4 ocol;
    //vec2 fragcord =  gl_FragCoord.xy;
    //mainImage(ocol,fragcord);

   // out_colour = ocol;
}