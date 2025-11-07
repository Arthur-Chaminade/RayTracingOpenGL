#version 460
out vec4 FragColor;
uniform vec3 resolution = vec3(600.,600., 0.);
uniform float time = 0.;
uniform vec3 mousePos = vec3(0., 0., 0.);

double Mandlebrot(vec2 pos) {
  int max_iter = 100;
  double realZ = 0;
  double imaginaryZ = 0;
  double nextRealZ = 0;
  double color = 0;
  for (int i= 0; i < max_iter; i++) {
    
    if (color< 2.)
    {
      nextRealZ = realZ * realZ - imaginaryZ * imaginaryZ;
      imaginaryZ = 2 * realZ * imaginaryZ;
      realZ = nextRealZ + pos.x;
      imaginaryZ = imaginaryZ + pos.y;
      color = length(vec2(realZ,imaginaryZ));
    }
    else 
    {
      color = 0;
      break;
    }
  }
  return color * 0.5;
}

vec3 palette(float t) {
  vec3 a = vec3(0.5);
  vec3 b = a;
  vec3 c = 2*a;
  vec3 d = vec3(0.263, 0.416, 0.557);
  return a + b*cos( 6.28318 * (c*t + d));
}
void main() {
    
  vec2 uv = (gl_FragCoord.xy * 2. - resolution.xy) / resolution.y;
  vec2 uv0 = uv;
  vec3 finalColor = vec3(0.);
  for (int i= 0; i < 4; i++)
  {
    uv = fract(uv * 1.5) - 0.5;

    vec3 col = palette(length(uv0) + time*.4);
    float d = length(uv) * exp(-length(uv0));
    d = sin(d*8. + time * i)/8;
    d = abs(d);
    d= 0.02/d;
    finalColor += col * d;

  }
  

  FragColor = vec4(finalColor, 1.0);
}
