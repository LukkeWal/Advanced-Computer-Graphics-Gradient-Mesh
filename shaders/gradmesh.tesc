#version 430 core
layout(vertices = 4) out;

uniform float uTess;

flat in  int vPatchID[];
flat out int tcPatchID[];

void main()
{
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  tcPatchID[gl_InvocationID] = vPatchID[gl_InvocationID];  // pass through

  if (gl_InvocationID == 0) {
    float t = max(1.0, uTess);
    gl_TessLevelOuter[0] = t;
    gl_TessLevelOuter[1] = t;
    gl_TessLevelOuter[2] = t;
    gl_TessLevelOuter[3] = t;
    gl_TessLevelInner[0] = t;
    gl_TessLevelInner[1] = t;
  }
}
