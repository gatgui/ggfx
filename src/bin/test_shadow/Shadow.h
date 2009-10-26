#ifndef __Shadow_h_
#define __Shadow_h_

#include <gmath/gmath>
#include <ggfx/light.h>

class Shadow {
    
  public:
  
    enum Type {
      ST_CUSTOM,
      ST_UNIFORM,
      ST_LiSPSM,
      ST_UNIFORM2,
      ST_LiSPSM2
    };
    
    Shadow();
    ~Shadow();
    
    void setType(Type t);
    
    void setSceneParameters(const gmath::AABox &receiverBB, const gmath::AABox &casterBB);
    
    void setShadowParameters(float sFar, float sOffset, float sSize, float sDLExtrude);
    
    void setDirLightParameters(const gmath::Vector3 &dir);
    
    void setPointLightParameters(const gmath::Vector3 &pos);
    
    void setSpotLightParameters(
      const gmath::Vector3 &pos, const gmath::Vector3 &dir, float inner, float outer);
    
    void setCameraParameters(
      float fovy, float aspect, float nplane, float fplane,
      const gmath::Matrix4 &view);
    
    void compute();
    
    const gmath::Matrix4 getLightProj() const;
    const gmath::Matrix4 getLightView() const;
    const gmath::ConvexHull3D getBodyB() const;
    const gmath::ConvexHull3D getBodyFocus() const;
    const gmath::ConvexHull3D getBodyLVS() const;
    const gmath::ConvexHull3D getBodyLight() const;
    const gmath::ConvexHull3D getBodyCam() const;
    const gmath::Frustum getLightFrustum() const;
    const gmath::Frustum getCameraFrustum() const;
    inline gmath::PointList getPointsLVS() const {return mBodyLVSPts;}
    inline gmath::PointList getPointsB() const {return mBodyBPts;}
    
    inline const gmath::Vector3& getE() const {return E;}
    
  protected:
    
    float getShadowDist();
    
    float getShadowOffset();
    
    gmath::Vector3 getUpForDir(const gmath::Vector3 &dir);
    
    void computeDirLightFrustum();
    
    void computeSpotLightFrustum();
    
    void computePointLightFrustum();
    
    void updateLightFrustum();
    
    void updateBodies();
  
  protected:
  
    gmath::Matrix4 CalcLightProj();
    void CalcSpotLightSpace(gmath::Matrix4 &Lv, gmath::Matrix4 &lp, const gmath::Vector3 &up);
    void CalcLightSpace(gmath::Matrix4 &Lv, gmath::Matrix4 &Lp);
    gmath::Matrix4 ScaleTranslateMatrix(const gmath::Matrix4 &LS, const gmath::PointList &B);
    void CalcFrustumPoints(const gmath::Matrix4 &iMpv, gmath::Vector3 pts[8], bool invZ=false);
    void CalcFrustumPlanes(const gmath::Vector3 pts[8], gmath::Plane pl[6]);
    void CalcB(gmath::PointList &B);
    void CalcLVS(gmath::PointList &LVS);
    gmath::Vector3 GetProjViewDir(const gmath::Matrix4 &LS);
    gmath::Matrix4 CalcLispMatrix(const gmath::Matrix4 &LS, const gmath::PointList &B);
    gmath::Vector3 GetNearCameraPointE();
    float CalcN(const gmath::Matrix4 &ls, const gmath::AABox &box);
    gmath::Vector3 CalcZ0(const gmath::Matrix4 &LS, const gmath::Vector3 &e, float zM, const gmath::Vector3 &eDir);
  
  protected:
  
    Type mType;
    
    float mShadowFar;
    float mShadowOffset;
    float mShadowMapSize;
    float mShadowDirLightExtrude;
    
    ggfx::LightType mLightType;
    gmath::Vector3 mLightDir;
    gmath::Vector3 mLightPos;
    float mLightSpotInner;
    float mLightSpotOuter;
    
    gmath::Matrix4 mCamView;
    gmath::Matrix4 mCamViewInverse;
    gmath::Vector3 mCamPos;
    gmath::Vector3 mCamDir;
    float mCamNear;
    float mCamFar;
    float mCamFov;
    float mCamAspect;
    
    gmath::AABox mSceneCasterBB;
    gmath::AABox mSceneReceiverBB;
    
    // ---
    
    gmath::Matrix4 mLightView;
    gmath::Matrix4 mLightProj;
    float mLightFov;
    float mLightNear;
    float mLightFar;
    gmath::ProjectionMode mLightMode;
    
    gmath::Frustum mCamFrustum;
    gmath::Frustum mLightFrustum;

    gmath::ConvexHull3D mCamHull;
    gmath::ConvexHull3D mLightHull;
    gmath::ConvexHull3D mBodyB; // cam/light/casterBB
    gmath::ConvexHull3D mBodyLVS; // light/casterBB
    gmath::ConvexHull3D mFocusBody;
    
    gmath::PointList mBodyBPts;
    gmath::PointList mBodyLVSPts;
    
    gmath::Vector3 E;
};


#endif

