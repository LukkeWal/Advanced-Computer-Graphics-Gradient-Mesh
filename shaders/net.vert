#version 410 core

layout(location=0) in vec2 inPos;
layout(location=1) in vec3 inC0;
layout(location=2) in vec3 inC1;
layout(location=3) in vec3 inC2;
layout(location=4) in vec3 inC3;

out vec3 vLineColor; // for lines
flat out vec3 vC0;
flat out vec3 vC1;
flat out vec3 vC2;
flat out vec3 vC3;

uniform float uPointSize;

void main()
{
  vec2 ndc = inPos * 2.0 - 1.0;
  gl_Position = vec4(ndc, 0.0, 1.0);

  // For lines, just use one color
  vLineColor = inC0;

  // For points, keep all four
  vC0 = inC0;
  vC1 = inC1;
  vC2 = inC2;
  vC3 = inC3;

  gl_PointSize = uPointSize;
}
