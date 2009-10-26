#ifndef __MeshData_h_
#define __MeshData_h_

std::string VERTEX_FORMAT = "position-float-3|normal-float-3|texcoord-float-2|texcoord-float-2|texcoord-float-2";

struct Vertex {
  float pos[3];
  float nrm[3];
  float texc0[2];
  float texc1[2];
  float texc2[2];
};
struct Tri {
  GLuint v0;
  GLuint v1;
  GLuint v2;
};
struct MaterialData {
  float diff[4];
  float ambi[4];
  float spec[4];
  float shininess;
};

gmath::Vector4 white(1,1,1,1);
gmath::Vector4 black(0,0,0,1);

MaterialData NoneMaterial = {
  {1, 1, 1, 1},
  {0, 0, 0, 1},
  {1, 1, 1, 1},
  0
};

float planeSize = 30.f;
Vertex PlaneVertexData[] = {
  {{-planeSize, 0,  planeSize}, {0, 1, 0}, {0, 0}, {0, 0}, {0, 0}},
  {{ planeSize, 0,  planeSize}, {0, 1, 0}, {1, 0}, {1, 0}, {1, 0}},
  {{ planeSize, 0, -planeSize}, {0, 1, 0}, {1, 1}, {1, 1}, {1, 1}},
  {{-planeSize, 0, -planeSize}, {0, 1, 0}, {0, 1}, {0, 1}, {0, 1}},
};
Tri PlaneFaceData[] = {
  {0, 1, 2},
  {0, 2, 3}
};
unsigned int PlaneVertexCount(sizeof(PlaneVertexData) / sizeof(Vertex));
unsigned int PlaneFaceCount(sizeof(PlaneFaceData) / sizeof(Tri));
MaterialData PlaneMaterial = {
  {0.8f, 0.2f, 0.5f, 1.0f},
  {0, 0, 0, 1.0f},
  {1, 1, 1, 1.0f},
  100
};

float cubeSize = 2.5f;
Vertex CubeVertexData[] = {
  // Front face
  {{-cubeSize, 0,           cubeSize}, { 0,  0,  1}, {0, 0}, {0, 0}, {0, 0}},
  {{ cubeSize, 0,           cubeSize}, { 0,  0,  1}, {1, 0}, {1, 0}, {1, 0}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 0,  0,  1}, {1, 1}, {1, 1}, {1, 1}},
  {{-cubeSize, 2*cubeSize,  cubeSize}, { 0,  0,  1}, {0, 1}, {0, 1}, {0, 1}},
  // Back face
  {{ cubeSize, 0,          -cubeSize}, { 0,  0, -1}, {0, 0}, {0, 0}, {0, 0}},
  {{-cubeSize, 0,          -cubeSize}, { 0,  0, -1}, {1, 0}, {1, 0}, {1, 0}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, { 0,  0, -1}, {1, 1}, {1, 1}, {1, 1}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 0,  0, -1}, {0, 1}, {0, 1}, {0, 1}},
  // Left face
  {{ cubeSize, 0,           cubeSize}, { 1,  0,  0}, {0, 0}, {0, 0}, {0, 0}},
  {{ cubeSize, 0,          -cubeSize}, { 1,  0,  0}, {1, 0}, {1, 0}, {1, 0}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 1,  0,  0}, {1, 1}, {1, 1}, {1, 1}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 1,  0,  0}, {0, 1}, {0, 1}, {0, 1}},
  // Right face
  {{-cubeSize, 0,          -cubeSize}, {-1,  0,  0}, {0, 0}, {0, 0}, {0, 0}},
  {{-cubeSize, 0,           cubeSize}, {-1,  0,  0}, {1, 0}, {1, 0}, {1, 0}},
  {{-cubeSize, 2*cubeSize,  cubeSize}, {-1,  0,  0}, {1, 1}, {1, 1}, {1, 1}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, {-1,  0,  0}, {0, 1}, {0, 1}, {0, 1}},
  // Top face
  {{-cubeSize, 2*cubeSize,  cubeSize}, { 0,  1,  0}, {0, 0}, {0, 0}, {0, 0}},
  {{ cubeSize, 2*cubeSize,  cubeSize}, { 0,  1,  0}, {1, 0}, {1, 0}, {1, 0}},
  {{ cubeSize, 2*cubeSize, -cubeSize}, { 0,  1,  0}, {1, 1}, {1, 1}, {1, 1}},
  {{-cubeSize, 2*cubeSize, -cubeSize}, { 0,  1,  0}, {0, 1}, {0, 1}, {0, 1}},
  // Bottom face
  {{ cubeSize,          0,  cubeSize}, { 0, -1,  0}, {0, 0}, {0, 0}, {0, 0}},
  {{-cubeSize,          0,  cubeSize}, { 0, -1,  0}, {1, 0}, {1, 0}, {1, 0}},
  {{-cubeSize,          0, -cubeSize}, { 0, -1,  0}, {1, 1}, {1, 1}, {1, 1}},
  {{ cubeSize,          0, -cubeSize}, { 0, -1,  0}, {0, 1}, {0, 1}, {0, 1}}
};
Tri CubeFaceData[] = {
  { 0,  1,  2},
  { 0,  2,  3},
  { 4,  5,  6},
  { 4,  6,  7},
  { 8,  9, 10},
  { 8, 10, 11},
  {12, 13, 14},
  {12, 14, 15},
  {16, 17, 18},
  {16, 18, 19},
  {20, 21, 22},
  {20, 22, 23}
};
unsigned int CubeVertexCount(sizeof(CubeVertexData) / sizeof(Vertex));
unsigned int CubeFaceCount(sizeof(CubeFaceData) / sizeof(Tri));
MaterialData CubeMaterial = {
  {0.4f, 0.2f, 0.8f, 1.0f},
  {0, 0, 0, 1.0f},
  {1, 1, 1, 1.0f},
  12
};

#endif

