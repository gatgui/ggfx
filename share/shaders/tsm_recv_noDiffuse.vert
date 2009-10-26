uniform mat4 eyeToLightProj;
uniform mat4 LS;

varying vec4 pL;
varying vec4 pT; // with Nt applied

void main() {
  
  // position (eye space)
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  
  // vertex position (light space)
  pT = eyeToLightProj * p;
  pL = LS * p;
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}
