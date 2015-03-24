#version 430
precision highp float;
const float hverticalcuttoff = -0.2;
const vec2 iResolution = vec2(1280,720);
uniform float iGlobalTime;           // shader playback time (in seconds)
uniform sampler2D iChannel0;          // input channel. XX = 2D/Cube
uniform mat3 playerrot;
//direction the player is facing
uniform vec3 playerview;

// Incoming vertex colour
layout (location = 0) in vec4 in_colour;

// Outgoing pixel colour
layout (location = 0) out vec4 out_colour;

vec3 up = vec3(0,1,0);
//vec3 topcol = vec3(0.55,0.75,0.83);
//vec3 bottomcol = vec3(0.5,0.78,0.98);


float noise(vec3 x) {
  vec3 p = floor(x);
  vec3 f = fract(x);
  f = f * f * (3.0 - 2.0 * f);

  vec2 uv = (p.xy + vec2(37.0, 17.0) * p.z) + f.xy;
  vec2 rg = texture2D(iChannel0, (uv + 0.5) / 256.0, -100.0).yx;
  //vec2 rg = vec2(0.0,0.0);
  return mix(rg.x, rg.y, f.z);
}

float map(vec3 p) {
  vec3 q = p + 0.2 * vec3(3.0, 0.3, 5.0) * mod(iGlobalTime*0.2, 3600.0) * 2.0;
  float n = 0.0, f = 0.5;
  n += f * noise(q);
  q *= 3.001;
  f *= 0.333;
  n += f * noise(q);
  q *= 3.002;
  f *= 0.332;
  n += f * noise(q);
  return n;
}

float scene(vec3 p) {
  return p.y + 2.0 - 0.003 * map(vec3(p.x, 0.0, p.z));
  /*
      +0.0175*(1.0-sin(0.5*p.z+7.5*iGlobalTime))
      -0.01*(1.0-sin(1.0*p.z+15.0*iGlobalTime))
      -0.005*(1.0-sin(2.0*p.z+30.0*iGlobalTime))
      -0.0025*(1.0-sin(4.0*p.z+60.0*iGlobalTime));
*/
}

vec3 normal(vec3 p, float d) {
  float e = 0.05;
  float dx = scene(vec3(e, 0.0, 0.0) + p) - d;
  float dy = scene(vec3(0.0, e, 0.0) + p) - d;
  float dz = scene(vec3(0.0, 0.0, e) + p) - d;
  return normalize(vec3(dx, dy, dz));
}

float clouds(float skyPow,vec3 nml)
{
    float cloudFac = pow(abs(skyPow), 0.8)*1.0;
	float a = 0;
	//a += cloudFac*map(nml*2.0/nml.y);
   // a += cloudFac*map(nml*5.0/nml.y);
    float ufCloudCover = 0.5;
    // 0% = 0.0, 10% = 0.25, 20% = 0.35, 30% = 0.4
    // 50% = 0.45, 75% = 0.75, 100% = 1.5
    
    float cc = 0.0;
    if (ufCloudCover < 0.1) {
        cc = 0.0 + 0.2 * (ufCloudCover - 0.0)/0.1;
    } else if (ufCloudCover < 0.2) {
        cc = 0.2 + 0.1 * (ufCloudCover - 0.1)/0.1;
    } else if (ufCloudCover < 0.3) {
        cc = 0.3 + 0.1 * (ufCloudCover - 0.2)/0.1;
    } else if (ufCloudCover < 0.5) {
        cc = 0.4 + 0.1 * (ufCloudCover - 0.3)/0.3;
    } else if (ufCloudCover < 0.75) {
        cc = 0.5 + 0.2 * pow((ufCloudCover - 0.5)/0.25, 2.0);
    } else {
        cc = 0.7 + 0.75 * pow((ufCloudCover - 0.75)/0.25, 2.0);
    }
    float cloud = 0.0;
    cloud += min(1.0, (1.0-smoothstep(0.0, cc, map(nml/nml.y)))) * 0.4;
    cloud += min(1.0, (1.0-smoothstep(0.0, cc, map(nml*1.03/nml.y)))) * 0.4;
    cloud += min(1.0, (1.0-smoothstep(0.0, cc, map(nml*3.0/nml.y)))) * 0.3;
    return cloudFac*cloud + a;
}

vec3 shadeBg(vec3 nml, vec2 fragCoord, vec2 aspect, vec2 uv)
{
	vec3 bgLight = normalize(vec3(
        sin(iGlobalTime*0.5)*0.1,
		cos(iGlobalTime*0.1)*0.6-0.3,
        -1.0
	));
    float sunD = dot(bgLight, nml) > 0.995 ? 1.0 : 0.0;
	vec3 sun = vec3(6.5, 3.5, 2.0);
	float skyPow = dot(nml, vec3(0.0, -1.0, 0.0));
    float centerPow = 0.0; //-dot(uv,uv);
    float horizonPow = pow(1.0-abs(skyPow), 3.0)*(5.0+centerPow);
	float sunPow = dot(nml, bgLight);
	float sp = max(sunPow, 0.0);
    float scattering = clamp(1.0 - abs(2.0*(-bgLight.y)), 0.0, 1.0);
    
	vec3 bgCol;
    //? sky ambient 2
    bgCol += max(0.0, skyPow)*2.0*vec3(0.8);
    //horizon
	bgCol += 0.5*vec3(0.8)*(horizonPow);
    //sun
	bgCol += sun*(sunD+pow( sp, max(128.0, abs(bgLight.y)*512.0) ));
    //sun bloom
	bgCol += vec3(0.4,0.2,0.15)*(pow( sp, 8.0) + pow( sp, max(8.0, abs(bgLight.y)*128.0) ));
    //sky ambient
    bgCol *= mix(vec3(0.7, 0.85, 0.95), vec3(1.0, 0.45, 0.1), scattering);
    //nighttime fade
    bgCol *= 1.0 - clamp(bgLight.y*3.0, 0.0, 0.6);
  
    bgCol += clouds(skyPow,nml);
	return pow(max(vec3(0.0), bgCol), vec3(2.6));
}

mat3 rotationXY(vec2 angle) {
  float cp = cos(angle.x);
  float sp = sin(angle.x);
  float cy = cos(angle.y);
  float sy = sin(angle.y);

  return mat3(cy, -sy, 0.0, sy, cy, 0.0, 0.0, 0.0, 1.0) *
         mat3(cp, 0.0, -sp, 0.0, 1.0, 0.0, sp, 0.0, cp);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 aspect = vec2(iResolution.x/iResolution.y, 1.0);
    vec2 uv = (2.0 * (fragCoord.xy / iResolution.xy) - 1.0) * aspect;
	//Don't render stuff in the lower half of the screen
    if(uv.y < hverticalcuttoff){
    	//fragColor =  vec4(0.0);
       // return;
    }

    mat3 rot = rotationXY( vec2( 0.2+0.2*cos(0.5*iGlobalTime), -0.15*sin(0.5+0.5*iGlobalTime) ) );
   // vec3 d = rot * normalize(vec3(uv, 1.0));
	vec3 fragnormal = playerrot * normalize(vec3(uv, 1.0));

    vec3 col = shadeBg(-fragnormal, fragCoord,aspect,uv);
    
    vec4 noise = (texture2D(iChannel0, mod(fragCoord.xy/256.0, 1.0))-0.5) / 64.0;
    fragColor = pow(vec4( noise.rgb + (1.0 - exp(-1.3 * col.rgb)), 1.0 ), vec4(1.3));
}


void main()
{
    //float f = dot(playerview,up);
    // Simply set outgoing colour
    //out_colour = vec4(mix(bottomcol,topcol,f),1.0);
    vec4 ocol;
	vec2 fragcord =  gl_FragCoord.xy;
	mainImage(ocol,fragcord);

    out_colour = ocol;
}