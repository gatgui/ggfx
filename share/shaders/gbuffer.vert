
uniform mat4 inverseViewMatrix;

varying vec4 wPos;
varying vec3 wNrm;

void main() {
  
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  
  vec4 ePos = gl_ModelViewMatrix * gl_Vertex;
  vec3 eNrm = gl_NormalMatrix * gl_Normal;
  
  wPos = inverseViewMatrix * ePos;
  wNrm = (inverseViewMatrix * vec4(eNrm, 0.0)).xyz;
}

