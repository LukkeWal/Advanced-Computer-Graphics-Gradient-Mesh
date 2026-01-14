#version 430 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inCol;

out vec3 vColor;
flat out int vPatchID;

void main()
{
  vColor = inCol;
  vPatchID = gl_InstanceID;
  gl_Position = vec4(inPos, 0.0, 1.0);
}
