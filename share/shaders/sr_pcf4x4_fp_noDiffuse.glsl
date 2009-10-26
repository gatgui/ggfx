uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pls;


void main() {
  
  float ts  = 1.0 / float(shadowWidth);
  float hts = 0.5 * ts;
  float tts = ts / 3.0;
  
  vec2 tc = 0.5 * ((pls.xy / pls.w) + vec2(1.0));
  
  float lit = 0.0;
  //float depth = (0.5 * (pls.z / pls.w)) + 0.5;
  float depth = (pls.z / pls.w);
  float xoff = -hts;
  
  for (int i=0; i<4; ++i, xoff+=tts) {
    float yoff = -hts;
    for (int j=0; j<4; ++j, yoff+=tts) {
      lit += float(depth < texture2D(shadowMap, tc + vec2(xoff, yoff)).r);
    }
  }
  lit /= 16.0;
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}

