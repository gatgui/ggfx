uniform sampler2D shadowMap;

varying vec4 pls;

void main() {
  
  vec2 tc = (0.5 * (pls.xy / pls.w)) + 0.5;
  float depth = pls.z / pls.w;
  float depthRef = texture2D(shadowMap, tc).x;
  float lit = float(depth < depthRef);
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}

