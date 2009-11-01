uniform sampler2D shadowMap;

varying vec4 pL;
varying vec4 pT;
varying vec3 l;
varying vec3 n;

void main() {

  vec3 nn = normalize(n);
  vec3 nl = normalize(l);
  
  float dc  = max(0.0, dot(nn, nl));
  
  vec2 tc = 0.5 * (pT.xy / pT.w) + 0.5;
  
  float depthVal = 0.5 * (pL.z / pL.w) + 0.5;
  
  float lit = float(depthVal < texture2D(shadowMap, tc).r);
  
  gl_FragColor.rgb = lit * vec3(dc);
  gl_FragColor.a = 1.0;
}