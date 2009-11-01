uniform mat4 eyeToLightProj;
uniform mat4 LS;

varying vec3 l;
varying vec3 n;
varying vec4 pL;
varying vec4 pT; // with Nt applied

void main() {
  
  // position (eye space)
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  
  // ModelView 3x3 Inverse Transpose
  n = gl_NormalMatrix * gl_Normal;
  
  // light vector (eye space)
  l = normalize(gl_LightSource[0].position.xyz - p.xyz);
  
  // vertex position (light space)
  pT = eyeToLightProj * p;
  pL = LS * p;
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}