#version 410 core

in vec3 vLineColor;
flat in vec3 vC0;
flat in vec3 vC1;
flat in vec3 vC2;
flat in vec3 vC3;

out vec4 outColor;

// 0 = lines, 1 = points
uniform int uIsPoint;

void main()
{
  if (uIsPoint == 0)
  {
    // Lines
    outColor = vec4(vLineColor, 1.0);
    return;
  }

  // Points
  vec2 p = gl_PointCoord * 2.0 - 1.0;
  if (dot(p, p) > 1.0) discard;

  bool left   = gl_PointCoord.x < 0.5;
  bool bottom = gl_PointCoord.y < 0.5;
  vec3 c;
  if (left && bottom)       c = vC0; // BL
  else if (!left && bottom) c = vC1; // BR
  else if (left && !bottom) c = vC2; // TL
  else                      c = vC3; // TR

  outColor = vec4(c, 1.0);
}
