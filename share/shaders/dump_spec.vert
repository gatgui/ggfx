uniform mat4 eyeToLightProj;
uniform mat4 LS;

varying vec3 h;
varying vec3 n;
varying vec4 pL;
varying vec4 pT;

void main() {
  
  // position (eye space)
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  
  // eye vector (eye space)
  vec3 e = -normalize(p.xyz);
  
  // ModelView 3x3 Inverse Transpose
  n = gl_NormalMatrix * gl_Normal;
  //n = (gl_ModelViewMatrix * vec4(gl_Normal, 0.0)).xyz;
  
  // Should have one vp per light type !
  // Idem for diffuse contribution pass
  // light vector (eye space)
  vec3 l = normalize(gl_LightSource[0].position.xyz - p.xyz);
  //l = normalize(lightPosEye - p.xyz);
  
  // half vector (eye space)
  h = normalize(l + e);
  
  // vertex position (light space)
  pT = eyeToLightProj * p;
  pL = LS * p;
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
