
uniform float textureWidth;
uniform float textureHeight;
uniform float invTextureWidth;
uniform float invTextureHeight;
uniform sampler2D tex0;

varying vec2 tc0;

// factors for a 9x9 gauss filter

// THIS FAILS on OSX with an ATI X1600
//const float factors[5] = {
//  0.3191538243,
//  0.3191538243 * 0.7261490371,
//  0.3191538243 * 0.2780373005,
//  0.3191538243 * 0.05613476283,
//  0.3191538243 * 0.005976022895
//};

float factors[5];

void main() {
  
  factors[0] = 0.3191538243;
  factors[1] = 0.3191538243 * 0.7261490371;
  factors[2] = 0.3191538243 * 0.2780373005;
  factors[3] = 0.3191538243 * 0.05613476283;
  factors[4] = 0.3191538243 * 0.005976022895;
  
  float v = tc0.y * textureHeight;
  gl_FragColor =
    factors[4] * texture2D(tex0, vec2(tc0.x, (v-4.0)*invTextureHeight)) +
    factors[3] * texture2D(tex0, vec2(tc0.x, (v-3.0)*invTextureHeight)) +
    factors[2] * texture2D(tex0, vec2(tc0.x, (v-2.0)*invTextureHeight)) +
    factors[1] * texture2D(tex0, vec2(tc0.x, (v-1.0)*invTextureHeight)) +
    factors[1] * texture2D(tex0, vec2(tc0.x, (v+1.0)*invTextureHeight)) +
    factors[2] * texture2D(tex0, vec2(tc0.x, (v+2.0)*invTextureHeight)) +
    factors[3] * texture2D(tex0, vec2(tc0.x, (v+3.0)*invTextureHeight)) +
    factors[4] * texture2D(tex0, vec2(tc0.x, (v+4.0)*invTextureHeight)) +
    factors[0] * texture2D(tex0, tc0);
}

