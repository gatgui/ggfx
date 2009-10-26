uniform sampler2D shadowMap;

varying vec4 pL;
varying vec4 pT;

void main() {
  
  vec2 tc = 0.5 * (pT.xy / pT.w) + 0.5;
  
  float depthVal = 0.5 * (pL.z / pL.w) + 0.5;
  
  float lit = float(depthVal < texture2D(shadowMap, tc).r);
  
  gl_FragColor.rgb = vec3(lit);
  gl_FragColor.a = 1.0;
}


