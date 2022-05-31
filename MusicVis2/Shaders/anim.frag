#version 330 core
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture1;
uniform float time;

void main()
{
 
  vec4 texColor= texture(texture1,TexCoords);
  texColor.r = 0.9*sin(2.5*time);
  //texColor.g = 0.10*sin(1.5 * time);
  //texColor.g = 0.10;
    if(texColor.a < 0.1)
        discard;
 
    FragColor = texColor;

}