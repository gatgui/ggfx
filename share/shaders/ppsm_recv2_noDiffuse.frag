uniform int shadowWidth;
uniform mat4 eyeToLightProjs[3];
uniform sampler2D shadowMaps[3];
uniform float splits[4];

varying vec4 epos;
varying vec4 hpos;

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
    //lit = dot(vec4(0.25), samples);
  }
    
  // Standard SM test
  //lit = float(texture2D(shadow, sc).r > depth);
  
  return lit;
}

void main() {
  
  float lit = 0.0;
  
  if (epos.z > splits[0]) {
    lit = 1.0;
  } else if (epos.z > splits[1]) {
    vec4 lpos = eyeToLightProjs[0] * epos;
    vec4 nP = 0.5 * (lpos / lpos.w) + 0.5;
    lit = computeLighting(nP.z, shadowMaps[0], nP.xy);
  } else if (epos.z > splits[2]) {
    vec4 lpos = eyeToLightProjs[1] * epos;
    vec4 nP = 0.5 * (lpos / lpos.w) + 0.5;
    lit = computeLighting(nP.z, shadowMaps[1], nP.xy);
  } else if (epos.z > splits[3]) {
    vec4 lpos = eyeToLightProjs[2] * epos;
    vec4 nP = 0.5 * (lpos / lpos.w) + 0.5;
    lit = computeLighting(nP.z, shadowMaps[2], nP.xy);
  } else {
    lit = 1.0;
  }
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}

