#include "PSSM.h"
using namespace std;
using namespace gmath;
using namespace ggfx;

PSSM::PSSM(const ISceneData *scn)
  :ShadowAlgorithm(scn), mCurrentLayer(0), mNSplit(3), mLambda(0.5f) {
  resizeSplits();
}

PSSM::~PSSM() {
}

void PSSM::calcB(PointList &pts, ConvexHull3D *B) {
  ConvexHull3D body(mCameraHull);
  if (mLightType != LT_DIR) {
    body.add(mLightPos);
    body.clip(mScn->getSceneBox());
    body.clip(mLightFrustum);
    pts.build(body);
  } else {
    body.clip(mScn->getSceneBox());
    pts.buildAndIncludeDir(body, mScn->getSceneBox(), -mLightDir);
  }
  if (B) {
    *B = body;
  }
}

Matrix4 PSSM::calcUnitCubeTransform(const Matrix4 &LS, const PointList &pts) {
  // that's basicaly an orthograhic projection !
  
  PointList ptsLS = LS * pts;
  
  Vector3 s = ptsLS.getAAB().getMax() + ptsLS.getAAB().getMin();
  Vector3 id = ptsLS.getAAB().getMax() - ptsLS.getAAB().getMin();
  id.x = 1.0f / id.x;
  id.y = 1.0f / id.y;
  id.z = 1.0f / id.z;
  
  Vector3 trans = - s * id;
  Vector3 scale = 2 * id;
  
  return Matrix4(
    scale.x, 0, 0, trans.x,
    0, scale.y, 0, trans.y,
    //0, 0, scale.z, trans.z, // should be -scale.z then ?
    0, 0, 1, 0,
    0, 0, 0, 1);
}

void PSSM::computeNearFar(const Vector3 &from, const Vector3 &dir, float &nplane, float &fplane) {
  AABox box = mScn->getSceneBox();
  float orgN = nplane;
  float orgF = fplane;
  float N = std::numeric_limits<float>::max();
  float F = std::numeric_limits<float>::min();
  for (unsigned char i=0; i<8; ++i) {
    Vector3 diff = box.getCorner(i) - from;
    float z = diff.dot(dir);
    if (z < N) {
      N = z;
    }
    if (z > F) {
      F = z;
    }
  }
  if (N < orgN) {
    N = orgN;
  }
  if (F > orgF) {
    F = orgF;
  }
  nplane = N;
  fplane = F;
  /*
  N = (box.getCorner(0) - from).dot(dir);
  if (N < 0.0f) {
    N = nplane;
  }
  F = N;
  for (unsigned char i=0; i<8; ++i) {
    Vector3 diff = box.getCorner(i) - from;
    float z = diff.dot(dir);
    if (z > 0.0) {
      if (z < N) {
        N = z;
      } else if (z > F) {
        F = z;
      }
    }
  }
  nplane = N;
  fplane = F;
  */
}

void PSSM::calcLightViewProj(const gmath::Vector3 &viewUp) {
  mLightView = Matrix4::MakeLookIn(mLightPos, mLightDir, viewUp);
  float N = mScn->getCameraNear();
  float F = 100000.0f;
  computeNearFar(mLightPos, mLightDir, N, F);
  if (mLightType == LT_DIR) {
    mLightFrustum.setMode(PM_ORTHOGRAPHIC);
    mLightFrustum.setFovy(90.0f);
  } else if (mLightType == LT_POINT) {
    mLightFrustum.setMode(PM_PERSPECTIVE);
    mLightFrustum.setFovy(120.0f);
  } else {
    mLightFrustum.setMode(PM_PERSPECTIVE);
    mLightFrustum.setFovy(1.2f*mScn->getSpotLightOuterAngle());
  }
  mLightFrustum.setNearClip(N);
  //mLightFrustum.setNearClip(mScn->getCameraNear());
  mLightFrustum.setFarClip(F);
  mLightFrustum.setAspect(1.0f);
  //mLightFrustum.setAspect(mScn->getViewAspect());
  mLightFrustum.transform(mLightView.getFastInverse());
  mLightProj = mLightFrustum.getProjectionMatrix();
  mLightFar = F;
}

// should proceed in an other way
// do X render with adjusting near far in accordance to splits
// also add an polygon offset for blending 

void PSSM::splitCameraFrustum(unsigned int flags) {
  //static const Matrix4 InvZ = Matrix4::MakeScale(Vector3(1, 1, -1));
  float N = mScn->getCameraNear(); // beware: > 0
  float F = mScn->getCameraFar();
  computeNearFar(mScn->getCameraPosition(), mScn->getCameraViewDirection(), N, F);
  float curNear = N; // NO !
  float FdivN = F / N;
  float FminN = F - N;
  Matrix4 IV = mScn->getCameraInverseViewMatrix();
  ConvexHull3D globalCamHull = mCameraHull;
  // For each layer
  for (int i=0; i<mNSplit; ++i) {
    SplitLayer &layer = getLayer(i);
    // compute camera frustum
    layer.camNear = curNear;
    float factor = float(i+1) / mNSplit;
    float slog = N * Pow(FdivN, factor);
    float suni = N + FminN * factor;
    layer.camFar = mLambda*slog + (1.0f-mLambda)*suni; // add a use tweakable param
    layer.camHull = globalCamHull;
    // Cut camera hull
    Plane nearPlane = IV * Plane(Vector3::UNIT_Z, Vector3(0,0,-layer.camNear));
    Plane farPlane = IV * Plane(-Vector3::UNIT_Z, Vector3(0,0,-layer.camFar));
    layer.camHull.clip(nearPlane);
    layer.camHull.clip(farPlane);
    // Update light parameters for the layer hull
    mCameraHull = layer.camHull;
    updateLightParameters();
    layer.lightPos = mLightPos;
    layer.lightDir = mLightDir;
    if ((flags & USE_CUSTOM_UP) == 0) {
      layer.lightUp = Vector3::UNIT_Y;
      if (Abs(layer.lightUp.dot(layer.lightDir)) >= 1.0f) {
        layer.lightUp = Vector3::UNIT_Z;
      }
    } else {
      layer.lightUp = mLightViewUp;
    }
    // Compute view and projection matrix as in standard SM
    calcLightViewProj(layer.lightUp);
    layer.lightFrustum = mLightFrustum;
    layer.proj = mLightProj;
    layer.view = mLightView;
    // Focus step
    PointList B;
    calcB(B, &(layer.B));
    if (B.size() > 0) { // this can happen
      Matrix4 LS = layer.proj * layer.view;
      layer.proj = calcUnitCubeTransform(LS, B) * layer.proj;
    }
    // should be done differently: no z modification !
    // Done. Next layer near is current layer far
    curNear = layer.camFar;
  }
  //mSplits[0].camNear = mScn->getCameraNear();
  //mSplits.back().camFar = mScn->getCameraFar();
  // restore global camera hull [is it necessary]
  mCameraHull = globalCamHull;
}

bool PSSM::update(unsigned int flags) {
  //cout << "PSSM::update" << endl;
  if (!mScn) {
    return false;
  }
  if ((flags & NO_CAMERA_UPDATE) == 0) {
    updateCameraHull();
  }
  splitCameraFrustum(flags); // this will compute proj and view for each split
}

void PSSM::setCurrentLayer(int i) {
  //cout << "PSSM::setCurrentLayer" << endl;
  mCurrentLayer = i;
  mLightProj = getLayer(i).proj;
  mLightView = getLayer(i).view;
}

void PSSM::initializePrograms() {
  //cout << "PSSM::initializePrograms" << endl;
  ProgramManager &mgr = ProgramManager::Instance();
#ifdef PSSM_SINGLE_PASS_LIGHTING
  mCasterProg = mgr.createProgram("PSSM_Caster",
                                  "share/shaders/ppsm_cast.vert",
                                  "share/shaders/ppsm_cast.frag");
  mReceiverProg = mgr.createProgram("PSSM_Receiver",
                                    "share/shaders/ppsm_recv2.vert",
                                    "share/shaders/ppsm_recv2.frag");
  mReceiverProgNL = mgr.createProgram("PSSM_ReceiverNL",
                                      "share/shaders/ppsm_recv2_noDiffuse.vert",
                                      "share/shaders/ppsm_recv2_noDiffuse.frag");
#else
  mCasterProg = mgr.createProgram("PSSM_Caster",
                                  "share/shaders/ppsm_cast.vert",
                                  "share/shaders/ppsm_cast.frag");
  mReceiverProg = mgr.createProgram("PSSM_Receiver",
                                    "share/shaders/ppsm_recv.vert",
                                    "share/shaders/ppsm_recv.frag");
#endif
}

void PSSM::setupCasterProgram() {
  //cout << "PSSM::setupCasterProgram" << endl;
  mCasterProg->uniform("depthBias")->set(mDepthBias);
}

void PSSM::setupReceiverProgram(bool noLighting) {
  //cout << "PSSM::setupReceiverProgram" << endl;
#ifdef PSSM_SINGLE_PASS_LIGHTING
  GLint baseUnit = GLint(mScn->getShadowMapTexUnit());
  Matrix4 mat0 = getLayer(0).proj * getLayer(0).view * mScn->getCameraInverseViewMatrix();
  Matrix4 mat1 = getLayer(1).proj * getLayer(1).view * mScn->getCameraInverseViewMatrix();
  Matrix4 mat2 = getLayer(2).proj * getLayer(2).view * mScn->getCameraInverseViewMatrix();
  //mReceiverProg->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  //mReceiverProg->uniform("shadowMaps", 0)->set(GLint(baseUnit));
  //mReceiverProg->uniform("shadowMaps", 1)->set(GLint(baseUnit+1));
  //mReceiverProg->uniform("shadowMaps", 2)->set(GLint(baseUnit+2));
  //mReceiverProg->uniform("eyeToLightProjs", 0)->set(mat0);
  //mReceiverProg->uniform("eyeToLightProjs", 1)->set(mat1);
  //mReceiverProg->uniform("eyeToLightProjs", 2)->set(mat2);
  //mReceiverProg->uniform("splits", 0)->set(-getLayer(0).camNear);
  //mReceiverProg->uniform("splits", 1)->set(-getLayer(0).camFar);
  //mReceiverProg->uniform("splits", 2)->set(-getLayer(1).camFar);
  //mReceiverProg->uniform("splits", 3)->set(-getLayer(2).camFar);
  mBoundReceiver->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  mBoundReceiver->uniform("shadowMaps", 0)->set(GLint(baseUnit));
  mBoundReceiver->uniform("shadowMaps", 1)->set(GLint(baseUnit+1));
  mBoundReceiver->uniform("shadowMaps", 2)->set(GLint(baseUnit+2));
  mBoundReceiver->uniform("eyeToLightProjs", 0)->set(mat0);
  mBoundReceiver->uniform("eyeToLightProjs", 1)->set(mat1);
  mBoundReceiver->uniform("eyeToLightProjs", 2)->set(mat2);
  mBoundReceiver->uniform("splits", 0)->set(-getLayer(0).camNear);
  mBoundReceiver->uniform("splits", 1)->set(-getLayer(0).camFar);
  mBoundReceiver->uniform("splits", 2)->set(-getLayer(1).camFar);
  mBoundReceiver->uniform("splits", 3)->set(-getLayer(2).camFar);
#else
  GLint baseUnit = GLint(mScn->getShadowMapTexUnit());
  Matrix4 mat = mLightProj * mLightView * mScn->getCameraInverseViewMatrix();
  mReceiverProg->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  mReceiverProg->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  mReceiverProg->uniform("shadowMap")->set(baseUnit+mCurrentLayer);
  mReceiverProg->uniform("eyeToLightProj")->set(mat);
#endif
}

