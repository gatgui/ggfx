uniform mat4 eyeToLightProj;
uniform vec3 lightPosEye;

varying vec3 l;
varying vec3 h;
varying vec3 n;
varying vec4 pls;

void main() {
  
  // position (eye space)
  vec4 p = gl_ModelViewMatrix * gl_Vertex;
  
  // eye vector (eye space)
  vec3 e = -normalize(p.xyz);
  
  // ModelView 3x3 Inverse Transpose
  //n = gl_NormalMatrix * gl_Normal;
  n = (gl_ModelViewMatrix * vec4(gl_Normal, 0.0)).xyz;
  
  // light vector (eye space)
  //l = normalize(gl_LightSource[0].position.xyz - p.xyz);
  l = normalize(lightPosEye - p.xyz);
  
  // half vector (eye space)
  h = normalize(l + e);
  
  // vertex position (light space)
  //pls = gl_TextureMatrix[0] * p;
  pls = eyeToLightProj * p;
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

