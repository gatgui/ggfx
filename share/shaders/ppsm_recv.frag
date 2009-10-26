
uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 lpos;
varying vec4 hpos;
varying vec3 N;
varying vec3 L;

float computeLighting(in float depth, in sampler2D shadow, in vec2 sc) {
  float ts = 1.0 / float(shadowWidth);
  vec2 tc = (float(shadowWidth) * sc) - 0.5;
  vec2 f  = fract(tc);
  vec2 t00 = ts * (floor(tc) + 0.5);
  vec2 t11 = ts * (ceil(tc) + 0.5);
  vec2 t01 = vec2(t00.x, t11.y);
  vec2 t10 = vec2(t11.x, t00.y);
  // Set contribution to 1.0 for pixels outside shadow map
  float lit = 0.0;
  if (t00.x>1.0 || t00.y>1.0) {
    // All samples outside
    lit = 1.0;
  } else {
    // At least one sample inside
    vec4 samples = vec4(1.0, 1.0, 1.0, 1.0);
    samples[0] = float(texture2D(shadow, t00).r > depth);
    if (t11.x>1.0) {
      if (t11.y<=1.0) {
        // t11, t10 out
        samples[2] = float(texture2D(shadow, t01).r > depth);
      }
    } else {
      if (t11.y>1.0) {
        // t11, t01 out
        samples[1] = float(texture2D(shadow, t10).r > depth);
      } else {
        samples[1] = float(texture2D(shadow, t10).r > depth);
        samples[2] = float(texture2D(shadow, t01).r > depth);
        samples[3] = float(texture2D(shadow, t11).r > depth);
      }
    }
    // Bi-linear 
    float v0 = (1.0 - f.x) * samples[0] + f.x * samples[1];
    float v1 = (1.0 - f.x) * samples[2] + f.x * samples[3];
    lit = (1.0 - f.y) * v0 + f.y * v1;
  }
    
  // Standard SM test
  //lit = float(texture2D(shadow, sc).r > depth);
  
  return lit;
}

void main() {

  vec3 nN = normalize(N);
  vec3 nL = normalize(L);
  
  float diff = max(0.0, dot(nN, nL));
  float lit = 0.0;
  
  vec4 nP = 0.5 * (lpos / lpos.w) + 0.5;
  
  lit = computeLighting(nP.z, shadowMap, nP.xy);
  
  gl_FragColor.rgb = lit * vec3(diff);
  gl_FragColor.a = 1.0;
}

