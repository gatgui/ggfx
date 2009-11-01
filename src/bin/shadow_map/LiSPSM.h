#ifndef __LiSPSM_h_
#define __LiSPSM_h_

#include "ShadowAlgorithm.h"

class LiSPSM : public ShadowAlgorithm {
  public:
    
    LiSPSM(const ISceneData *scn=NULL);
    virtual ~LiSPSM();
    
    virtual bool update(unsigned int flags = 0);
    
    virtual void initializePrograms();
    
    void enablePCF(bool on);
    //void enableVSM();
    
  protected:
  
    static const gmath::Matrix4 NormalToLightSpace;
    static const gmath::Matrix4 LightToNormalSpace;
    static const gmath::Matrix4 InvZ;
    
    virtual void setupCasterProgram();
    virtual void setupReceiverProgram(bool noLighting=false);
    
    void calcB(gmath::PointList &pts);
    void calcLVS(gmath::PointList &pts);
    gmath::Vector3 calcNearCameraPoint(const gmath::PointList &pts);
    gmath::Vector3 calcProjViewDir(const gmath::Matrix4 &LS, const gmath::PointList &pts);
    gmath::Matrix4 calcUnitCubeTransform(const gmath::Matrix4 &LS, const gmath::PointList &pts);
    gmath::Vector3 calcZ0(const gmath::Matrix4 &LS, float z);
    float calcN(const gmath::Matrix4 &LS, const gmath::AABox &bBoxLS);
    gmath::Matrix4 calcLiSPSM(const gmath::Matrix4 &LS, const gmath::PointList &B);
    
  protected:
    
    gmath::Vector3 mE;
    bool mPCF;
};


#endif

