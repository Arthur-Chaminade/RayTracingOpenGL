#version 460 core
layout (location = 0) in vec3 aPos;

uniform float scale;
out vec3 ourPosition; // determine the position and the value relative to the bg for the color gradient

void main() {
  gl_Position = vec4(scale * aPos, 1.0);
  ourPosition = gl_Position.xyz;
}
