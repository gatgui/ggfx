#ifndef __Shadow2_h_
#define __Shadow2_h_

#include <gmath/gmath>
#include <ggfx/light.h>

class Shadow2 {
    
  public:
  
    struct TSMData {
      float top;
      float bottom;
      float n;
      gmath::ConvexHull2D::PointArray B;
      gmath::ConvexHull2D hull;
      gmath::Vector2 nMid;
      gmath::Vector2 fMid;
      gmath::Vector2 fFoc;
      gmath::Vector2 l;
      gmath::Vector2 q;
      gmath::Vector2 left;
      gmath::Vector2 right;
      gmath::Vector2 t0;
      gmath::Vector2 t1;
      gmath::Vector2 t2;
      gmath::Vector2 t3;
      gmath::Matrix4 Nt;
      gmath::Matrix4 Nt_step[9]; // first is IDENTITY, include Nt
      int step;
    };
    
    Shadow2();
    ~Shadow2();
    
    void setSceneParameters(const gmath::AABox &receiverBB, const gmath::AABox &casterBB);
    void setShadowParameters(float sFar, float sOffset, float sSize, float sDLExtrude);
    void setDirLightParameters(const gmath::Vector3 &dir);
    void setPointLightParameters(const gmath::Vector3 &pos);
    void setSpotLightParameters(const gmath::Vector3 &pos, const gmath::Vector3 &dir, float inner, float outer);
    void setCameraParameters(float fovy, float aspect, float nplane, float fplane, const gmath::Matrix4 &view);
    
    void calcB(gmath::PointList *B, gmath::ConvexHull3D *BodyB);
    void calcLVS(gmath::PointList *LVS, gmath::ConvexHull3D *BodyLVS);
    
    void compute();
    
    inline void setTSMFocus(float n) { // percentage of camera near-far 
      mFocusZone = n;
    }
    inline void setTSMStep(int i) {
      if (i < 0) i=0; // no TSM
      if (i > 8) i=8; // full TSM
      //mTSMStep = i;
      mTSMD.step = i;
    }
    inline int getTSMStep() const {
      //return mTSMStep;
      return mTSMD.step;
    }
    inline const TSMData& getTSMData() const {
      return mTSMD;
    }
    
    inline const gmath::Matrix4& getLightProj() const {
      return mLightProj;
    }
    inline const gmath::Matrix4& getLightView() const {
      return mLightView;
    }
    inline const gmath::Frustum& getLightFrustum() const {
      return mLightFrustum;
    }
    inline const gmath::Vector3& getE() const {
      return mE;
    }
    //inline const gmath::Matrix4& getNt() const {
    //  return mNt;
    //}
    inline void useLiSPSM(bool on) {
      mLiSPSM = on;
    }
    inline void useTSM(bool on) {
      mTSM = on;
    }
    inline void tryTSMCorrection(bool on) {
      mTSMAdjust = on;
    }
    inline bool tryTSMCorrection() const {
      return mTSMAdjust;
    }
    
  protected:
    
    static const gmath::Matrix4 NormalToLightSpace;
    static const gmath::Matrix4 LightToNormalSpace;
    static const gmath::Matrix4 InvZ;
    
    float getShadowDist() const;
    float getShadowOffset() const;
    void calcShadowMatrix(gmath::Matrix4 *P, gmath::Matrix4 *V, gmath::Frustum *frustum);
    
    // for LiSPSM
    gmath::Vector3 calcNearCameraPoint(const gmath::PointList &pts);
    gmath::Vector3 calcProjViewDir(const gmath::Matrix4 &LS, const gmath::PointList &pts);
    gmath::Matrix4 calcUnitCubeTransform(const gmath::Matrix4 &LS, const gmath::PointList &pts);
    gmath::Matrix4 calcLiSPSM(const gmath::Matrix4 &LS, const gmath::PointList &B);
    float calcN(const gmath::Matrix4 &LS, const gmath::AABox &bBoxLS);
    gmath::Vector3 calcZ0(const gmath::Matrix4 &LS, float z);
    
    // for TSM
    /*
    gmath::Matrix4 calcNt(const gmath::Vector4 &t0, const gmath::Vector4 &t1,
                          const gmath::Vector4 &t2, const gmath::Vector4 &t3);
    */
    
  protected:
    
    bool mLiSPSM;
    bool mTSM;
    bool mTSMAdjust;
    
    // shadow params
    float mShadowFar;
    float mShadowOffset;
    float mShadowMapSize;
    float mShadowDirLightExtrude;
    // light params
    ggfx::LightType mLightType;
    gmath::Vector3 mLightDir;
    gmath::Vector3 mLightPos;
    float mLightSpotInner;
    float mLightSpotOuter;
    // camera params
    gmath::Matrix4 mCamView;
    gmath::Matrix4 mCamViewInverse;
    gmath::Vector3 mCamPos;
    gmath::Vector3 mCamDir;
    gmath::Frustum mCamFrustum;
    float mCamNear;
    float mCamFar;
    float mCamFov;
    float mCamAspect;
    // scene params
    gmath::AABox mSceneCasterBB;
    gmath::AABox mSceneReceiverBB;
    
    // compute light params
    gmath::Matrix4 mLightView;
    gmath::Matrix4 mLightProj;
    
    // TSM
    //int mTSMStep;
    //gmath::Matrix4 mTSMMatrices[9];
    //gmath::Matrix4 mNt;
    TSMData mTSMD;
    float mFocusZone;
    
    //temp
    gmath::PointList mFocusPts;
    gmath::Frustum mLightFrustum;
    gmath::Vector3 mE;
};



#endif

