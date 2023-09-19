#version 330 core

attribute vec2 coord2d;
varying vec4 f_color;
uniform float offset_x;
uniform float offset_y;
uniform float scale_x;
uniform float scale_y;

#define PI_2 6.283185307179586476925286766559;

vec2 toPolar(vec2 cartesian){
	float distance = length(cartesian) * 2.0;
	float angle = atan(cartesian.y  , cartesian.x ) ;
	return vec2(angle , distance );
}

void main(void) {


  vec2 uv = (coord2d - 0.5 * (1280, 720) ) / 720;

  vec2 polar = toPolar(coord2d);

  gl_Position = vec4( polar.x  * scale_x, polar.y  * scale_y, 1.0, 1);
  //gl_Position = vec4( polar.y * cos(polar.x), polar.y * sin(polar.x), 1.0, 1);

  f_color = vec4( polar, 0,1 );


  
}