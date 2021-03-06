//////////////////////////////////////////////////////////////////
//
// shadowreceiver.vert
//
// Hamilton Chong
// (c) 2006
//
//////////////////////////////////////////////////////////////////


// I N P U T   V A R I A B L E S /////////////////////////////////

uniform mat4 uModelViewProjection;	// modelview projection matrix
uniform mat4 uModel;                    // model matrix
uniform mat4 uTextureViewProjection;    // shadow map's view projection matrix
uniform vec4 uLightPosition;            // light position in object space


// O U T P U T   V A R I A B L E S ///////////////////////////////

varying vec4   pShadowCoord;    // vertex position in shadow map coordinates
varying float  pDiffuse;        // diffuse shading value

// M A I N ///////////////////////////////////////////////////////

void main()
{
    // compute diffuse shading
    vec3 lightDirection = normalize(uLightPosition.xyz - gl_Vertex.xyz);
    pDiffuse = dot(gl_Normal.xyz, lightDirection);

    // compute shadow map lookup coordinates
    pShadowCoord = uTextureViewProjection * (uModel * gl_Vertex);

    // compute vertex's homogenous screen-space coordinates
    //gl_Position = uModelViewProjection * gl_Vertex;	// uncomment if other passes use shaders
    gl_Position = ftransform();	  // uncomment if other passes use fixed function pipeline
}