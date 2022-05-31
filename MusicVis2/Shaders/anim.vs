#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const float amplitude = 0.5;
const float frequency = 41000.0;
const float PI = 3.14159;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;
uniform vec3 sample;

void main()
{
  float distance = length(aPos);
  float effect = amplitude*(sample.x/15000.0)*sin(-PI*distance*frequency+time);
  gl_Position = projection*view*model*vec4(aPos.x  ,aPos.y + effect*0.7 , aPos.z ,1);
  TexCoords=vec2(aTexCoords.x + effect ,aTexCoords.y + effect);

}