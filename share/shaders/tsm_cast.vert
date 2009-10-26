
varying vec4 posL;
varying vec4 posT;

void main() {
  posT = gl_ModelViewProjectionMatrix * gl_Vertex;
  posL = gl_ModelViewMatrix * gl_Vertex;
  gl_Position = posT;
}
