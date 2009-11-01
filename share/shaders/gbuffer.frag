
uniform vec4 diffuseColor;

varying vec4 wPos;
varying vec3 wNrm;

void main() {
  
  gl_FragData[0] = wPos;
  
  gl_FragData[1].xyz = normalize(wNrm);
  gl_FragData[1].w = 1.0;
  
  gl_FragData[2].xyz = diffuseColor.xyz; //gl_FrontMaterial.diffuse.xyz;
  gl_FragData[2].w = gl_FrontMaterial.shininess;
  
}


