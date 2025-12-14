#version 330 core
layout(location = 0) in vec2 inPos;
layout(location = 1) in vec3 inColor;

out vec3 vColor;

void main() {
   vec2 p = inPos * 2.0 - vec2(1.0, 1.0);
   gl_Position = vec4(p, 0.0, 1.0);
   vColor = inColor;
}
