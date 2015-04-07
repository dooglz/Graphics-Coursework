#version 330

uniform sampler2D gPositionMap;
uniform sampler2D gColorMap;
uniform sampler2D gNormalMap;
uniform sampler2D gInfoMap;
uniform vec2 gScreenSize;

vec2 CalcTexCoord() { return gl_FragCoord.xy / gScreenSize; }

out vec4 FragColor;

void main() {
	vec2 TexCoord = CalcTexCoord();
	vec3 WorldPos = texture(gPositionMap, TexCoord).xyz;
	vec3 Color = texture(gColorMap, TexCoord).xyz;
	vec3 Normal = texture(gNormalMap, TexCoord).xyz;
	Normal = normalize(Normal);
	vec3 Info = texture(gInfoMap, TexCoord).xyz;

  FragColor = vec4(Color * 0.4 + Normal * 0.3 + WorldPos * 0.3, 1.0);
  //FragColor = vec4(0.3, 1.0, 0.4, 1.0);
  //FragColor = vec4(Info, 1.0);
}
