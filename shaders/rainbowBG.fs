#version 460 core
out vec4 FragColor;

in vec3 ourPosition;
uniform vec3 leftGradientColor;
uniform vec3 rightGradientColor;
void main() {
  float leftPos = (-ourPosition.x  + 1) * 0.5f;
  float rightPos = (ourPosition.x + 1) * 0.5f;
  FragColor = vec4(leftPos * leftGradientColor + rightPos * rightGradientColor, 1.0);
  
}
