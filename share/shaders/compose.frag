
uniform float textureWidth;
uniform float textureHeight;
uniform float invTextureWidth;
uniform float invTextureHeight;

uniform sampler2D tex0; // position
uniform sampler2D tex1; // normal
uniform sampler2D tex2; // color + shininess
uniform sampler2D tex3; // shadow

// need light position [in fact might depend on the light type]
uniform vec3 wLightPos;
uniform vec3 wEyePos;

varying vec2 tc0;

void main() {
  
  vec3  p = texture2D(tex0, tc0).xyz;
  vec3  n = texture2D(tex1, tc0).xyz;
  vec4 tmp = texture2D(tex2, tc0);
  vec3  c = tmp.xyz;
  float s = tmp.w;
  float lit = texture2D(tex3, tc0).x; // shadow percentage
  
  vec3  l = normalize(wLightPos - p);
  vec3  e = normalize(wEyePos - p);
  vec3  h = normalize(l + e);
  
  float dc = max(0.0, dot(n,l));
  float sc = 0.0;
  
  //if (dc > 0.0) { // if it is the light itself ... ?
    sc = pow(max(0.0, dot(n,h)), s);
    gl_FragColor.xyz = lit * ((dc * c) + vec3(sc));
  //} else {
  //  gl_FragColor.xyz = vec3(0.0);
  //}
  
  gl_FragColor.w = 1.0;
  
}

