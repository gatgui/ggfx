uniform mat4 eyeToLightProj;

varying vec4 pls;

void main() {
  
  // position (eye space)
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  
  // vertex position (light space)
  pls = eyeToLightProj * p;
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

