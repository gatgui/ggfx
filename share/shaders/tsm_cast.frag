
uniform float depthBias;

varying vec4 posL;
varying vec4 posT;

void main() {

  float depthVal = (posL.z / posL.w) + depthBias;
  depthVal = (0.5 * depthVal) + 0.5;
  
  gl_FragColor.r = depthVal;
  gl_FragColor.g = depthVal * depthVal; // for VSM
  gl_FragColor.b = depthVal;
  gl_FragColor.a = 1.0;
}
