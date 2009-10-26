#include "TSM.h"
using namespace gmath;
using namespace ggfx;
using namespace std;

TSM::TSM(const ISceneData *scn)
  : ShadowAlgorithm(scn), mFocusFactor(0.4f), mPCF(false), mVSM(false) {
}

TSM::~TSM() {
}

void TSM::enablePCF(bool on) {
  /*
  ProgramManager &mgr = ProgramManager::Instance();
  if (on) {
    if (mPCF == true) { // already on
      return;
    }
    if (mVSM == true) {
      mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs_vsm"));
      mVSM = false;
    } else {
      mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs"));
    }
    mReceiverProg->attach(mgr.getShader("TSM_Receiver_fs_pcf2x2"));
    mPCF = true;
    cout << "TSM / PCF" << endl;
  } else {
    if (mPCF == false) { // already off
      return;
    }
    mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs_pcf2x2"));
    mReceiverProg->attach(mgr.getShader("TSM_Receiver_fs"));
    cout << "TSM / No filter" << endl;
    mPCF = false;
  }
  mReceiverProg->link();
  */
}

void TSM::enableVSM(bool on) {
  /*
  ProgramManager &mgr = ProgramManager::Instance();
  if (on) {
    if (mVSM == true) { // already on
      return;
    }
    if (mPCF == true) {
      mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs_pcf2x2"));
      mPCF = false;
    } else {
      mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs"));
    }
    mReceiverProg->attach(mgr.getShader("TSM_Receiver_fs_vsm"));
    cout << "TSM / VSM" << endl;
    mVSM = true;
  } else {
    if (mVSM == false) { // already false
      return;
    }
    mReceiverProg->detach(mgr.getShader("TSM_Receiver_fs_vsm"));
    mReceiverProg->attach(mgr.getShader("TSM_Receiver_fs"));
    cout << "TSM / No filter" << endl;
    mVSM = false;
  }
  mReceiverProg->link();
  */
}

void TSM::initializePrograms() {
  ProgramManager &mgr = ProgramManager::Instance();
  mCasterProg = mgr.createProgram("TSM_Caster",
                                  "share/shaders/tsm_cast.vert",
                                  "share/shaders/tsm_cast.frag");
  mReceiverProg = mgr.createProgram("TSM_Receiver",
                                    "share/shaders/tsm_recv.vert",
                                    "share/shaders/tsm_recv_pcf2x2.frag");
  mReceiverProgNL = mgr.createProgram("TSM_ReceiverNL",
                                      "share/shaders/tsm_recv_noDiffuse.vert",
                                      "share/shaders/tsm_recv_pcf2x2_noDiffuse.frag");
  //mReceiverProg = mgr.createProgram("TSM_Receiver", "share/shaders/tsm_recv.vert", "share/shaders/tsm_recv.frag");
  //mgr.createShader("TSM_Receiver_fs_pcf2x2", Shader::ST_FRAGMENT, "share/shaders/tsm_recv_pcf2x2.frag");
  //mgr.createShader("TSM_Receiver_fs_vsm", Shader::ST_FRAGMENT, "share/shaders/tsm_recv_vsm.frag");
}

void TSM::setupCasterProgram() {
  mCasterProg->uniform("depthBias")->set(mDepthBias);
}

void TSM::setupReceiverProgram(bool noLighting) {
  Matrix4 LS = getViewMatrix() * mScn->getCameraInverseViewMatrix();
  Matrix4 eyeToLightProj = getProjectionMatrix() * LS;
  //mReceiverProg->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  //mReceiverProg->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  //mReceiverProg->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  //mReceiverProg->uniform("eyeToLightProj")->set(eyeToLightProj);
  //mReceiverProg->uniform("LS")->set(LS);
  mBoundReceiver->uniform("shadowMap")->set(GLint(mScn->getShadowMapTexUnit()));
  mBoundReceiver->uniform("shadowWidth")->set(GLint(mScn->getShadowMapSize()));
  mBoundReceiver->uniform("eyeToLightProj")->set(eyeToLightProj);
  mBoundReceiver->uniform("LS")->set(LS);
  if (noLighting == false) {
    mBoundReceiver->uniform("lightPosEye")->set(mScn->getLightPositionEye());
  }
}

void TSM::calcB(PointList &pts) {
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
}

void TSM::calcReferencePoints(
  const PointList &B, Vector3 &nearP, Vector3 &focusP, Vector3 &farP)
{
  size_t i;
  nearP = mScn->getCameraPosition();
  // compute main body direction
  Vector3 dir;
  for (i=0; i<B.size(); ++i) {
    dir += (B[i] - nearP).normalize();
  }
  dir.normalize();
  // compute closest and furthest point from camera position
  float newNear, newFar, newFocus;
  newNear = newFar = (B[0] - nearP).dot(dir);
  for (i=1; i<B.size(); ++i) {
    float cur = (B[i] - nearP).dot(dir);
    if (cur < newNear) {
      newNear = cur;
    } else if (cur > newFar) {
      newFar = cur;
    }
  }
  newFocus = newNear + (mFocusFactor * (newFar - newNear));
  // compute reference points
  farP = nearP + newFar*dir;
  focusP = nearP + newFocus*dir;
  nearP = nearP + newNear*dir;
}

bool TSM::calcFrustum2D(
  const PointList &B, const Vector3 &nearP, const Vector3 &focusP, const Vector3 &farP,
  Vector2 &center, Vector2 &t0, Vector2 &t1, Vector2 &t2, Vector2 &t3)
{
  size_t i;
  ConvexHull2D::PointArray pts;
  for (i=0; i<B.size(); ++i) {
    pts.push_back(Vector2(B[i].x, B[i].y));
  }
  Vector2 nP(nearP.x, nearP.y);
  Vector2 mP(focusP.x, focusP.y);
  Vector2 fP(farP.x, farP.y);
  Vector2 l = (fP - nP).normalize();
  // Compute top and bottom line
  ConvexHull2D hull(pts);
  float top, bottom, cur;
  if (nP == fP) {
    cout << "Doh !!!!" << endl;
    return false;
  }
  top = bottom = (hull[0] - nP).dot(l);
  for (i=0; i<hull.size(); ++i) {
    cur = (hull[i] - nP).dot(l);
    if (cur < top) {
      top = cur;
    } else if (cur > bottom) {
      bottom = cur;
    }
  }
  // Compute center of projection
  float lambda = fabs(top - bottom);
  float delta = (mP - nP).dot(l) - top; // delta'
  float eps = -1.0f + (2.0f * mFocusFactor);
  float n = fabs(top + ((lambda*delta*(1+eps))/(lambda*(1-eps)-2*delta)));
  center = nP + (top-n)*l;
  // Compute tangents point to hull
  Vector2 rightMost = fP;
  Vector2 leftMost = fP;
  Vector2 ref = center + l;
  for (i=0; i<hull.size(); ++i) {
    float tmp = ConvexHull2D::IsLeft(center, ref, hull[i]);
    if (fabs(tmp) < 0.000001) {
      continue;
    } else if (tmp > 0) {
      if (ConvexHull2D::IsLeft(center, leftMost, hull[i]) >= 0) {
        leftMost = hull[i];
      }
    } else {
      if (ConvexHull2D::IsLeft(center, rightMost, hull[i]) <= 0) {
        rightMost = hull[i];
      }
    }
  }
  // Compute the 4 trapezoid points
  Vector2 e;
  float dp;
  e = (rightMost - center).normalize();
  dp = e.dot(l);
  if (fabs(dp) < 0.000001) {
    t3 = center + n*l;
    t0 = center + (n+lambda)*l;
  } else {
    dp = 1.0f / dp;
    t3 = center + (n*dp)*e;
    t0 = center + ((n+lambda)*dp)*e;
  }
  e = (leftMost - center).normalize();
  dp = e.dot(l);
  if (fabs(dp) < 0.000001) {
    t2 = center + n*l;
    t1 = center + (n+lambda)*l;
  } else {
    dp = 1.0f / dp;
    t2 = center + (n*dp)*e;
    t1 = center + ((n+lambda)*dp)*e;
  }
  return true;
}

Matrix4 TSM::calcNt(
  const Vector2 &inCenter, const Vector2 &inT0, const Vector2 &inT1,
  const Vector2 &inT2, const Vector2 &inT3)
{
  Vector4 t0 = Vector4(inT0.x, inT0.y, 1, 1);
  Vector4 t1 = Vector4(inT1.x, inT1.y, 1, 1);
  Vector4 t2 = Vector4(inT2.x, inT2.y, 1, 1);
  Vector4 t3 = Vector4(inT3.x, inT3.y, 1, 1);
  Vector4 c  = Vector4(inCenter.x, inCenter.y, 1, 1);
  Vector4 u  = 0.5f * (t2 + t3);
  Vector4 v;
  Matrix4 Nt;
  
  // translation to bring center of near segment to (0,0)
  Nt = Matrix4::MakeTranslate(Vector3(-u.x, -u.y, 0));
  // rotation to make nP fP point up
  u = t2 - t3;
  u /= float(sqrt(u.x*u.x + u.y*u.y));
  Nt = Matrix4(-u.x,-u.y,0,0, u.y,-u.x,0,0, 0,0,1,0, 0,0,0,1) * Nt;
  // translate to put center of projection at (0,0)
  u = Nt * c;
  Nt = Matrix4::MakeTranslate(Vector3(-u.x,-u.y,0)) * Nt;
  // compute shear to make trapezoid symetric
  u = 0.5f * (Nt * (t2 + t3));
  Nt = Matrix4(1,-u.x/u.y,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1) * Nt;
  // scale to make near segment unit
  u = Nt * t2;
  Nt = Matrix4::MakeScale(Vector3(1/u.x,1/u.y,1)) * Nt;
  // make trapezoid square
  Nt = Matrix4(1,0,0,0, 0,1,0,1, 0,0,1,0, 0,1,0,0) * Nt;
  // move back square in unit square
  u = Nt * t0;
  v = Nt * t2;
  float t = -0.5f * (u.y/u.w + v.y/v.w);
  Nt = Matrix4::MakeTranslate(Vector3(0,t,0)) * Nt;
  // scale square to unit square
  u = Nt * t0;
  Nt = Matrix4::MakeScale(Vector3(1,-u.w/u.y,1)) * Nt;
  
  return Nt;
}

bool TSM::update(unsigned int flags) {
  if (!mScn) {
    return false;
  }
  if ((flags & USE_CUSTOM_UP) == 0) {
    // if user didn't specify a custom up, set our own
    setLightViewUp(mScn->getCameraViewDirection());
    flags = flags | USE_CUSTOM_UP;
  }
  ShadowAlgorithm::update(flags);
  if (mScn->getSceneBox().isNull()) {
    return false;
  }
  // Compute B
  PointList B;
  calcB(B);
  if (B.size() == 0) {
    return false;
  }
  // Compute TSM
  Matrix4 LS = mLightProj * mLightView;
  //   calc reference points
  Vector3 nearP, focusP, farP;
  calcReferencePoints(B, nearP,focusP,farP); // hum
  //   transform to PPS
  nearP = LS * nearP;
  focusP = LS * focusP;
  farP = LS * farP;
  B = LS * B;
  //   calc trapezoid
  Vector2 c, t0, t1, t2, t3;
  if (!calcFrustum2D(B,nearP,focusP,farP, c,t0,t1,t2,t3)) {
    return false;
  }
  //   calc PPS transform and set it as projection matrix
  mLightView = LS;
  mLightProj = calcNt(c,t0,t1,t2,t3);
  return true;
}


