uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pls;

// how to initialize it as a constant ?
vec2 filterTaps[12];

void main() {
  
  filterTaps[0] = vec2(-0.326212, -0.405805);
  filterTaps[1] = vec2(-0.840144, -0.07358);
  filterTaps[2] = vec2(-0.695914,  0.457137);
  filterTaps[3] = vec2(-0.203345,  0.620716);
  filterTaps[4] = vec2( 0.96234,  -0.194983);
  filterTaps[5] = vec2( 0.473434, -0.480026);
  filterTaps[6] = vec2( 0.519456,  0.767022);
  filterTaps[7] = vec2( 0.185461, -0.893124);
  filterTaps[8] = vec2( 0.507431,  0.064425);
  filterTaps[9] = vec2( 0.89642,   0.412458);
  filterTaps[10] = vec2(-0.32194,  -0.932615);
  filterTaps[11] = vec2(-0.791559, -0.597705);
  
  float ts  = 1.0 / float(shadowWidth);
  
  vec2 tc = (0.5 * (pls.xy / pls.w)) + vec2(0.5);
  // vec2 tc = pls.xy / pls.w; NO !!! [-1, 1]
  
  float lit = 0.0;  
  //float depth = (0.5 * (pls.z / pls.w)) + 0.5;
  float depth = pls.z / pls.w;
  for (int i=0; i<12; ++i) {
    lit += float(depth < texture2D(shadowMap, tc + (ts * filterTaps[i])).r);
  }
  lit /= 12.0;
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}


