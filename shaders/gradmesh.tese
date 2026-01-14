#version 430 core
layout(quads, fractional_even_spacing, ccw) in;

out vec3 vColor;

flat in int tcPatchID[];


struct PatchCsGPU {
  vec4 Cred[16];
  vec4 Cgreen[16];
  vec4 Cblue[16];
};

layout(std430, binding = 0) buffer PatchBuffer {
  PatchCsGPU patches[];
};

vec4 hermite(float t)
{
  float t2 = t * t;
  float t3 = t2 * t;

  float h00 = 2.0*t3 - 3.0*t2 + 1.0;
  float h10 = t3 - 2.0*t2 + t;
  float h01 = -2.0*t3 + 3.0*t2;
  float h11 = t3 - t2;

  // order: [pos0, tan0, tan1, pos1]
  return vec4(h00, h10, h11, h01);
}

vec3 evalChannel(vec4 Hu, vec4 Hv, vec4 C[16])
{
  // HT(u) * C
  vec3 HTuC0 = Hu.x * C[0].xyz  + Hu.y * C[4].xyz  + Hu.z * C[8].xyz  + Hu.w * C[12].xyz;
  vec3 HTuC1 = Hu.x * C[1].xyz  + Hu.y * C[5].xyz  + Hu.z * C[9].xyz  + Hu.w * C[13].xyz;
  vec3 HTuC2 = Hu.x * C[2].xyz  + Hu.y * C[6].xyz  + Hu.z * C[10].xyz + Hu.w * C[14].xyz;
  vec3 HTuC3 = Hu.x * C[3].xyz  + Hu.y * C[7].xyz  + Hu.z * C[11].xyz + Hu.w * C[15].xyz;

  // (HT(u) * C) * H(v)
  return Hv.x * HTuC0 + Hv.y * HTuC1 + Hv.z * HTuC2 + Hv.w * HTuC3;
}

void main()
{
  int pid = tcPatchID[0];

  float u = gl_TessCoord.x;
  float v = gl_TessCoord.y;

  vec4 Hu = hermite(u);
  vec4 Hv = hermite(v);

  vec3 R = evalChannel(Hu, Hv, patches[pid].Cred);
  vec3 G = evalChannel(Hu, Hv, patches[pid].Cgreen);
  vec3 B = evalChannel(Hu, Hv, patches[pid].Cblue);

  vec2 pos = R.xy;
  vColor = vec3(R.z, G.z, B.z);
  vec2 ndc = pos * 2.0 - 1.0;
  gl_Position = vec4(ndc, 0.0, 1.0);
}
