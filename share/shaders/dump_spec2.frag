
uniform sampler2D shadowMaps[3];
uniform mat4 eyeToLightProjs[3];
uniform float splits[4];
uniform int shadowWidth;
uniform float shininess;

varying vec3 h;
varying vec3 n;
varying vec4 pE;

void main() {

  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  
  float sc = pow(max(0.0, dot(nn, nh)), shininess);
  
  float lit = 0.0;
  if (pE.z > splits[0]) {
    lit = 1.0;
  } else if (pE.z > splits[1]) {
    vec4 pL = eyeToLightProjs[0] * pE;
    vec4 tmp = 0.5 * (pL / pL.w) + 0.5;
    lit = (tmp.z < texture2D(shadowMaps[0], tmp.xy).r) ? 1.0 : 0.0;
    lit = (tmp.x<0.0 || tmp.x>1.0 || tmp.y<0.0 || tmp.y>1.0) ? 1.0 : lit;
  } else if (pE.z > splits[2]) {
    vec4 pL = eyeToLightProjs[1] * pE;
    vec4 tmp = 0.5 * (pL / pL.w) + 0.5;
    lit = (tmp.z < texture2D(shadowMaps[1], tmp.xy).r) ? 1.0 : 0.0;
    lit = (tmp.x<0.0 || tmp.x>1.0 || tmp.y<0.0 || tmp.y>1.0) ? 1.0 : lit;
  } else if (pE.z > splits[3]) {
    vec4 pL = eyeToLightProjs[2] * pE;
    vec4 tmp = 0.5 * (pL / pL.w) + 0.5;
    lit = (tmp.z < texture2D(shadowMaps[2], tmp.xy).r) ? 1.0 : 0.0;
    lit = (tmp.x<0.0 || tmp.x>1.0 || tmp.y<0.0 || tmp.y>1.0) ? 1.0 : lit;
  } else {
    lit = 1.0;
  }
  
  gl_FragColor.rgb = lit * vec3(sc);
  gl_FragColor.a = 1.0;
  
}

