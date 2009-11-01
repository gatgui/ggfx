
// specular mask textures and such
uniform sampler2D shadowMap;
uniform int shadowWidth;
uniform float shininess;

varying vec3 h;
varying vec3 n;
varying vec4 pL;
varying vec4 pT;

void main() {

  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  
  float sc  = pow(max(0.0, dot(nn, nh)), shininess);
  
  vec2 tc = 0.5 * (pT.xy / pT.w) + 0.5;
  
  float depth = 0.5 * pL.z / pL.w + 0.5;
  
  float lit = 0.0;
  if (tc.x<0.0 || tc.x>1.0 || tc.y<0.0 || tc.y>1.0) {
    lit = 1.0;
  } else {
    lit = (depth < texture2D(shadowMap, tc).x) ? 1.0 : 0.0;
  }
  
  gl_FragColor.rgb = lit * vec3(sc);
  gl_FragColor.a = 1.0;
  
}
