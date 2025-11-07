#version 460
out vec4 FragColor;
uniform vec3 resolution = vec3(600.,600., 0.);
uniform float time = 0.;

vec3 palette(float t) {
  vec3 a = vec3(0.5);
  vec3 b = a;
  vec3 c = 2*a;
  vec3 d = vec3(0.263, 0.416, 0.557);
  return a + b*cos( 6.28318 * (c*t + d));
}

float perlinNoise(vec2 pos) {
  return 1.0;
} 
float manhattanDistance(vec2 v) {
  return abs(v.x) + abs(v.y); 
}
float chebDistance(vec2 v) {
  return max(v.x,v.y);
}
float PI = 3.141592653589793;
void main() {
    
  vec2 uv = (gl_FragCoord.xy * 2. - resolution.xy) / resolution.y;
  vec2 uv0 = uv;
  vec3 finalColor = vec3(0.);
  
  for (int i = 0; i < 3; i++) {
    float d = manhattanDistance(uv) + 1/(0.05 +manhattanDistance(uv0 * cos(uv0)));
    d *= length(uv) + 1/(0.05 + length(uv0 * cos(uv0)));
    d = fract(d + mod(time*.1, 1));

    finalColor += d * vec3(0., 0.25, 0.1);
    uv = fract(uv * 2) - 0.5;
  }
  

  FragColor = vec4(finalColor, 1.0);
}

