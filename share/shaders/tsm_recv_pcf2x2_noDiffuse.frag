uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pL;
varying vec4 pT;

void main() {
  
  float ts  = 1.0 / float(shadowWidth);
  
  vec2 tc = 0.5 * (pT.xy / pT.w) + 0.5;
  
  float depth = 0.5 * (pL.z / pL.w) + 0.5;
  
  // Percentage-closer filtering
  vec2 sc = (float(shadowWidth) * tc) - 0.5;
  vec2 f = fract(sc);
  vec2 t00 = ts * (floor(sc) + 0.5);
  vec2 t11 = ts * (ceil(sc) + 0.5);
  vec2 t10 = vec2(t11.x, t00.y);
  vec2 t01 = vec2(t00.x, t11.y);
  float lit = 1.0;
  if (t00.x>1.0 || t00.y>1.0) {
    lit = 1.0;
  } else {
    vec4 samples = vec4(1.0, 1.0, 1.0, 1.0);
    samples[0] = float(texture2D(shadowMap, t00).r > depth);
    if (t11.x > 1.0) {
      if (t11.y <= 1.0) {
        samples[2] = float(texture2D(shadowMap, t01).r > depth);
      }
    } else {
      if (t11.y > 1.0) {
        samples[1] = float(texture2D(shadowMap, t10).r > depth);
      } else {
        samples[1] = float(texture2D(shadowMap, t10).r > depth);
        samples[2] = float(texture2D(shadowMap, t01).r > depth);
        samples[3] = float(texture2D(shadowMap, t11).r > depth);
      }
    }
    // bi-linear filtering
    float v0 = (1.0 - f.x) * samples[0] + f.x * samples[1];
    float v1 = (1.0 - f.x) * samples[2] + f.x * samples[3];
    lit = (1.0 - f.y) * v0 + f.y * v1;
    // no filterting
    //lit = 0.25 * (samples[0] + samples[1] + samples[2] + samples[3]);
  }
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}

