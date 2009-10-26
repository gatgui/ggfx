uniform sampler2D shadowMap;
uniform int shadowWidth;

varying vec4 pls;
varying vec3 l;
varying vec3 h;
varying vec3 n;

void main() {

  vec3 nn = normalize(n);
  vec3 nh = normalize(h);
  vec3 nl = normalize(l);
  
  float dc  = max(0.0, dot(nn, nl));
  float sc  = pow(max(0.0, dot(nn, nh)), 36.0);
  
  //vec3 diffuse  = vec3(0.8, 0.2, 0.5);
  //vec3 specular = vec3(1.0, 1.0, 1.0);
  
  vec2 tc = 0.5 * ((pls.xy / pls.w) + vec2(1.0));
  
  // compute samples coordinates
  //float depth = (0.5 * (pls.z / pls.w)) + 0.5;
  float depth = pls.z / pls.w;
  vec2 texcoord, t00, t10, t01, t11;
  texcoord.x = (tc.x * float(shadowWidth)) - 0.5;
  texcoord.y = (tc.y * float(shadowWidth)) - 0.5;
  float x0 = floor(texcoord.x);
  float x1 = ceil(texcoord.x);
  float fx = fract(texcoord.x);
  float y0 = floor(texcoord.y);
  float y1 = ceil(texcoord.y);
  float fy = fract(texcoord.y);
  float iw = 1.0 / float(shadowWidth);
  t00 = vec2((x0+0.5)*iw, (y0+0.5)*iw);
  t10 = vec2((x1+0.5)*iw, (y0+0.5)*iw);
  t01 = vec2((x0+0.5)*iw, (y1+0.5)*iw);
  t11 = vec2((x1+0.5)*iw, (y1+0.5)*iw);
  // grab samples
  float z00 = texture2D(shadowMap, t00).r;
  float z10 = texture2D(shadowMap, t10).r;
  float z01 = texture2D(shadowMap, t01).r;
  float z11 = texture2D(shadowMap, t11).r;
  float v00 = (z00 <= depth) ? 0.0 : 1.0;
  float v10 = (z10 <= depth) ? 0.0 : 1.0;
  float v01 = (z01 <= depth) ? 0.0 : 1.0;
  float v11 = (z11 <= depth) ? 0.0 : 1.0;
  // texcoords outside texture range consider lit
  v00 = (t00.x <= 1.0 && t00.y <= 1.0) ? v00 : 1.0;
  v10 = (t10.x <= 1.0 && t10.y <= 1.0) ? v10 : 1.0;
  v01 = (t01.x <= 1.0 && t01.y <= 1.0) ? v01 : 1.0;
  v11 = (t11.x <= 1.0 && t11.y <= 1.0) ? v11 : 1.0;
  float v0 = (1.0 - fx) * v00 + fx * v10;
  float v1 = (1.0 - fx) * v01 + fx * v11;
  float lit = (1.0 - fy) * v0 + fy * v1;
  
  //gl_FragColor.rgb = lit * (dc*diffuse + sc*specular);
  gl_FragColor.rgb = vec3(lit * (dc + sc));
  
  /*
  float hts = 0.5 / float(shadowWidth);
  vec4 depth = vec4(0.5*((pls.z/pls.w)+1.0));
  vec4 depthRef;
  depthRef.x = texture2D(shadowMap, tc+vec2(-hts,-hts)).r;
  depthRef.y = texture2D(shadowMap, tc+vec2( hts,-hts)).r;
  depthRef.z = texture2D(shadowMap, tc+vec2( hts, hts)).r;
  depthRef.w = texture2D(shadowMap, tc+vec2(-hts, hts)).r;
  vec4 lit = vec4(lessThan(depth, depthRef));
  
  gl_FragColor.rgb = dot(vec4(0.25), lit) * (dc*diffuse + sc*specular);
  */
  
  gl_FragColor.a = 1.0;
}
