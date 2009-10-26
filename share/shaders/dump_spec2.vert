
varying vec3 h;
varying vec3 n;
varying vec4 pE;

void main() {
  
  // position (eye space)
  pE = gl_ModelViewMatrix * gl_Vertex;
  
  // eye vector (eye space)
  vec3 e = -normalize(pE.xyz);
  
  // ModelView 3x3 Inverse Transpose
  n = gl_NormalMatrix * gl_Normal;
  
  // light vector (eye space)
  vec3 l = normalize(gl_LightSource[0].position.xyz - pE.xyz);
  
  // half vector (eye space)
  h = normalize(l + e);
  
  // output position (projection space)
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

