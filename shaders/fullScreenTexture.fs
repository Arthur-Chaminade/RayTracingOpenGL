#version 460 core

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec3 resolution;

void main()
{
  //Convert pixel coordinates to texture coordinates
  vec2 texCoords = gl_FragCoord.xy/resolution.xy;
  FragColor = texture(screenTexture, texCoords);
}
