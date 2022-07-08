#version 330 core

attribute vec2 coord2d;
varying vec4 f_color;
uniform float offset_x;
uniform float offset_y;
uniform float scale_x;
uniform float scale_y;

void main(void) {

  float logConst = 1.0 / log(10.0);
  float X = (coord2d.x);
  float Y = (coord2d.y) * 0.000025;

  gl_Position = vec4( ( ( X + offset_x) * scale_x), ( Y + offset_y ) * scale_y, 0, 1);
  f_color = vec4(coord2d.xy / 2.0 + 0.5, 1, 1);
}