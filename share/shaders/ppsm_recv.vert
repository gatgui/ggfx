
uniform mat4 eyeToLightProj;

varying vec4 lpos;
varying vec4 hpos;
varying vec3 N;
varying vec3 L;

void main() {
  
  vec4 epos = gl_ModelViewMatrix * gl_Vertex;
  
  N = gl_NormalMatrix * gl_Normal;
  
  L = normalize(gl_LightSource[0].position.xyz - epos.xyz);
  
  hpos = gl_ModelViewProjectionMatrix * gl_Vertex;
  
  lpos = eyeToLightProj * epos;
  
  gl_Position = hpos;
}


