
uniform float textureWidth;
uniform float textureHeight;
uniform float invTextureWidth;
uniform float invTextureHeight;
uniform sampler2D tex0;

vec2 filterTaps[12];

varying vec2 tc0;

void main() {
  
  filterTaps[ 0] = vec2(-0.326212, -0.405805);
  filterTaps[ 1] = vec2(-0.840144, -0.07358);
  filterTaps[ 2] = vec2(-0.695914,  0.457137);
  filterTaps[ 3] = vec2(-0.203345,  0.620716);
  filterTaps[ 4] = vec2( 0.96234,  -0.194983);
  filterTaps[ 5] = vec2( 0.473434, -0.480026);
  filterTaps[ 6] = vec2( 0.519456,  0.767022);
  filterTaps[ 7] = vec2( 0.185461, -0.893124);
  filterTaps[ 8] = vec2( 0.507431,  0.064425);
  filterTaps[ 9] = vec2( 0.89642,   0.412458);
  filterTaps[10] = vec2(-0.32194,  -0.932615);
  filterTaps[11] = vec2(-0.791559, -0.597705);
  
  float sum = 0.0;
  float scale = 5.0;
  vec2 step = vec2(invTextureWidth, invTextureHeight);
  vec2 tap;
  
  for (int i = 0; i < 12; ++i) {
    tap = tc0 + scale * (step * filterTaps[i]);
    sum += texture2D(tex0, tap).x;
  }
  
  sum /= 12.0;
  
  gl_FragColor = vec4(sum);
}

