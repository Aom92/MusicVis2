#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

const float amplitude = 0.00125;
const float frequency = 0.04;
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
  float effect = amplitude*(sample.x/100000.0)*sin(time * frequency);
  //float effect = 2*sin(time*0.25);
  float effect2 = amplitude*sin(-PI*frequency+time);
  gl_Position = projection*view*model*vec4(aPos.x ,aPos.y + effect2, aPos.z ,1);
  TexCoords=vec2(aTexCoords.x  ,aTexCoords.y);

}