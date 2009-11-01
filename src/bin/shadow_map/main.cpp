#include <ggfx/config.h>
#ifdef __APPLE__
# include <GLUT/glut.h>
#else
# ifdef WIN32
#   pragma warning(disable: 4505)
# endif
# include <GL/glut.h>
#endif
#include <ggfx/bufferobject.h>
#include <ggfx/vertexformat.h>
#include <ggfx/shader.h>
#include <ggfx/mesh.h>
#include <ggfx/system.h>
#include <ggfx/renderer.h>
#include "LiSPSM.h"
#include "TSM.h"
#include "PSSM.h"
#include "MeshData.h"

using namespace ggfx;
using namespace gmath;
using namespace std;

GLenum TexFormat = GL_RGBA16F_ARB;

// -----------------------------------------------------------------------------

class FrameState : public ShadowAlgorithm::ISceneData {
  
  protected:
    
    float  mCamFovy;
    float  mCamNear;
    float  mCamFar;
    
    Vector3 mCamAim;
    Vector3 mCamRot; // Yaw, Pitch, Roll in this order
    float  mCamDtt;
    Matrix4 mCamView;
    Matrix4 mCamViewInverse;
    Vector3 mCamPos;
    Vector3 mCamDir;
    
    Vector3 mCamAim2;
    Vector3 mCamRot2;
    float  mCamDtt2;
    Matrix4 mCamView2;
    Matrix4 mCamViewInverse2;
    Vector3 mCamPos2;
    Vector3 mCamDir2;
    
    AABox mCasterBB;
    AABox mReceiverBB;
    
    Vector3 mLightPos;
    Vector4 mLightPosGL;
    Vector3 mLightPosEye;
    Vector3 mLightDir;
    Vector3 mLightAim;
    float mLightOuter;
    float mLightInner;
    LightType mLightType;
    
    Vector3 *mCubeInstPos;
    unsigned int mCubeCount;
    
    SubMesh *mCube;
    SubMesh *mPlane;
    Mesh *mBunny;
    Mesh *mHorse;
    Vector3 mBunnyPos;
    Vector3 mHorsePos;
    
    GLuint mFBO;
    GLuint mDBO;
    GLuint mShadowMap;
    GLuint mColorMap0;
    GLuint mColorMap1;
    GLuint mColorMap2;
    GLsizei mShadowSize;
    
    // Deffered shading
    GLuint mGBuffer;
    GLuint mBlurBuffer;
    GLuint mColorBuffer;
    GLuint mPositionBuffer;
    GLuint mNormalBuffer;
    GLuint mShadowBuffer;
    GLuint mShadowBlurBuffer;
    GLuint mDepthBuffer;
    int mShowRTT;
    
    int mViewWidth;
    int mViewHeight;
    float mViewAspect;
    
    int mLastX;
    int mLastY;
    int mCurX;
    int mCurY;
    int mMouseButton;
    int mMouseState;
    int mKeyMod;
    enum {
      MO_CAM_ROTATE = 0,
      MO_CAM_TRANSLATE,
      MO_CAM_DOLLY,
      MO_LIGHT_TRANSLATE,
      MO_NONE
    } mMouseOp;
    
    bool mDirtyProj;
    bool mDirtyView;
    bool mDirtyLight;
    
    TSM mTSM;
    LiSPSM mLiSPSM;
    PSSM mPSSM;
    ShadowAlgorithm mStandard;
    ShadowAlgorithm *mShadow;
    bool mPCF;
    bool mVSM;
    bool mBlur;
    bool mTestRTT;
    float mDepthBias;
    bool mBackFaceCaster;
    float mShininess;
    Program *mDumpSpec;
    Program *mDumpSpecPSSM;
    Program *mGBufProg;
    Program *mCompose;
    
    Matrix4 mProjMatrix;
    Matrix4 mViewMatrix;
    
    VertexFormat *mLastVF;
    
    Program *mBlurX;
    Program *mBlurY;
    Program *mBlurDisc;
    
    int mCurSplit;
    bool mUseCam2;
    bool mDiffuse;
    bool mSpecular;
    bool mOneSplit;
    struct SplitLayer {
      bool mDiffuse;
      bool mSpecular;
    };
    SplitLayer mSplitLayers[3];
    
  protected:
    
    FrameState() {
      mUseCam2 = false;
      mCurSplit = 0;
      mCamFovy = 56;
      mCamNear = 2;
      mCamFar = 1000;
      mCamAim2 = Vector3(0,0,0);
      mCamDtt2 = 10;
      mCamRot2 = Vector3(0,0,0);
      mCamAim = Vector3(0,0,0);
      mCamDtt = 10;
      mCamRot = Vector3(0,0,0);
      mViewWidth = mViewHeight = 512;
      mLastX = mLastY = -1;
      mCurX = mCurY = -1;
      mMouseOp = MO_NONE;
      mLastVF = NULL;
      mLightType = LT_SPOT;
      mLightPos = Vector3(20,20,20);
      mLightAim = Vector3(0,0,0);
      mLightDir = (mLightAim - mLightPos).normalize();
      mLightInner = 80.0f;
      mLightOuter = 89.0f;
      mDirtyView = true;
      mDirtyProj = true;
      mDirtyLight = true;
      mTSM.setSceneData(this);
      mLiSPSM.setSceneData(this);
      mStandard.setSceneData(this);
      mPSSM.setSceneData(this);
      mFBO = 0;
      mDBO = 0;
      mShowRTT = 0;
      mShadow = &mTSM;
      mShadowMap = 0;
      mColorMap0 = 0;
      mColorMap1 = 0;
      mShadowSize = 1024;
      mPCF = false;
      mVSM = false;
      mDepthBias = 0.0f;
      mBackFaceCaster = false;
      mBlur = false;
      mTestRTT = false;
      mShininess = 36.0f;
      mDiffuse = false;
      mSpecular = false;
      mOneSplit = false;
      mSplitLayers[0].mDiffuse = false;
      mSplitLayers[0].mSpecular = false;
      mSplitLayers[1].mDiffuse = false;
      mSplitLayers[1].mSpecular = false;
      mSplitLayers[2].mDiffuse = false;
      mSplitLayers[2].mSpecular = false;
      mBunny = NULL;
      mHorse = NULL;
      mBunnyPos = Vector3(10.0f, 8.0f, 10.0f);
      mHorsePos = Vector3(0.0f, 4.0f, 0.0f);
    }
    
  public:
    
    static FrameState& Instance() {
      static FrameState fsInstance = FrameState();
      return fsInstance;
    }
    
    ~FrameState() {
    }
    
    // --- Init / Cleanup ---
    
    SubMesh* createIndexedStaticMesh(
      VertexFormat *fmt,
      void *vdata, unsigned int nv, unsigned int vsz,
      void *idata, unsigned int ni, unsigned int isz)
    {
      SubMesh *m = new SubMesh();
      if (m->createIndexed(SubMesh::PT_TRI, fmt, nv, SubMesh::IT_32, ni)) {
        m->getVertexBuffer()->upload(vsz, vdata, BufferObject::BOU_STATIC);
        m->getIndexBuffer()->upload(isz, idata, BufferObject::BOU_STATIC);
        return m;
      } else {
        delete m;
        return NULL;
      }
    }
    
    void checkFBO() {
      GLenum fbErr = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
      if (fbErr != GL_FRAMEBUFFER_COMPLETE_EXT) {
        switch (fbErr) {
          case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: cout << "FBO: Incomplete attachment" << endl; break;
          case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: cout << "FBO: Missing attachment" << endl; break;
          case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: cout << "FBO: Invalid dimensions" << endl; break;
          case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: cout << "FBO: Invalid formats" << endl; break;
          case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: cout << "FBO: Invalid draw buffer" << endl; break;
          case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: cout << "FBO: Invalid read buffer" << endl; break;
          case GL_FRAMEBUFFER_UNSUPPORTED_EXT: cout << "FBO: Unsupported" << endl; break;
          default: cout << "Unknown error" << endl;
        }
      }
    }
    
    void initialize() {
      // Setup mesh
#ifdef _DEBUG
      std::cout << "Create meshes..." << std::endl;
#endif
      mBunny = MeshManager::Instance().create("share/models/bunny-1500.obj", BufferObject::BOU_STATIC, "Bunny");
      mHorse = MeshManager::Instance().create("share/models/cow-1500.obj", BufferObject::BOU_STATIC, "Horse");
      VertexFormat *fmt = VertexFormat::Get(VERTEX_FORMAT);
      mPlane = createIndexedStaticMesh(fmt,
        PlaneVertexData, 4, sizeof(PlaneVertexData),
        PlaneFaceData, 6, sizeof(PlaneFaceData));
      mCasterBB.reset();
      void *ptr = NULL;
      if (mBunny->getSharedVertexBuffer()->lock(BufferObject::BOA_READ, &ptr)) {
        fmt = mBunny->getSharedFormat();
        if (fmt->hasAttribute(VAS_POSITION) &&
            fmt->getAttribute(VAS_POSITION).getType() == VAT_FLOAT) {
          
          void *pos = fmt->getAttributePtr(ptr, 0, VAS_POSITION);
          
          mCasterBB.setMin(Vector3((float*)pos)+mBunnyPos);
          mCasterBB.setMax(mCasterBB.getMin());
          
          for (size_t v=0; v<mBunny->getNumSharedVertices(); ++v) {
            mCasterBB.merge(Vector3((float*)pos)+mBunnyPos);
            fmt->incrementPtr(&pos);
          }
        }
        mBunny->getSharedVertexBuffer()->unlock();
        /*
        float *fptr = (float*)ptr;
        mCasterBB.setMin(Vector3(fptr)+mBunnyPos);
        mCasterBB.setMax(mCasterBB.getMin());
        for (size_t v=0; v<mBunny->getNumSharedVertices(); ++v) {
          mCasterBB.merge(Vector3(fptr)+mBunnyPos);
          fptr += 6;
        }
        mBunny->getSharedVertexBuffer()->unlock();
        */
      }
      if (mHorse->getSharedVertexBuffer()->lock(BufferObject::BOA_READ, &ptr)) {
        fmt = mHorse->getSharedFormat();
        if (fmt->hasAttribute(VAS_POSITION) &&
            fmt->getAttribute(VAS_POSITION).getType() == VAT_FLOAT) {
          
          void *pos = fmt->getAttributePtr(ptr, 0, VAS_POSITION);
          
          for (size_t v=0; v<mHorse->getNumSharedVertices(); ++v) {
            mCasterBB.merge(Vector3((float*)pos)+mHorsePos);
            fmt->incrementPtr(&pos);
          }
        }
        mHorse->getSharedVertexBuffer()->unlock();
        /*
        float *fptr = (float*)ptr;
        for (size_t v=0; v<mHorse->getNumSharedVertices(); ++v) {
          mCasterBB.merge(Vector3(fptr)+mHorsePos);
          fptr += 6;
        }
        mHorse->getSharedVertexBuffer()->unlock();
        */
      }
      mReceiverBB = mCasterBB;
      for (unsigned int i=0; i<PlaneVertexCount; ++i) {
        mReceiverBB.merge(Vector3(PlaneVertexData[i].pos));
      }
      // Setup shaders
#ifdef _DEBUG
      std::cout << "Setup shaders..." << std::endl;
#endif
      mTSM.initializePrograms();
      mLiSPSM.initializePrograms();
      mStandard.initializePrograms();
      mPSSM.initializePrograms();
      ProgramManager &mgr = ProgramManager::Instance();
      Shader *v = mgr.createShader("StdQuad_vs", Shader::ST_VERTEX, "share/shaders/std_quad.vert");
      Shader *x = mgr.createShader("BlurX_fs", Shader::ST_FRAGMENT, "share/shaders/blur_x.frag");
      Shader *y = mgr.createShader("BlurY_fs", Shader::ST_FRAGMENT, "share/shaders/blur_y.frag");
      Shader *c = mgr.createShader("Compose_fs", Shader::ST_FRAGMENT, "share/shaders/compose.frag");
      Shader *d = mgr.createShader("BlurDisc_fs", Shader::ST_FRAGMENT, "share/shaders/blur_disc.frag");
      mBlurX = mgr.createProgram("BlurX", v, x);
      mBlurY = mgr.createProgram("BlurY", v, y);
      mBlurDisc = mgr.createProgram("BlurDisc", v, d);
      mCompose = mgr.createProgram("Compose", v, c);
      Shader *sv = mgr.createShader("DumpSpec_vs", Shader::ST_VERTEX, "share/shaders/dump_spec.vert");
      Shader *sf = mgr.createShader("DumpSpec_fs", Shader::ST_FRAGMENT, "share/shaders/dump_spec.frag");
      mDumpSpec = mgr.createProgram("DumpSpec", sv, sf);
      sv = mgr.createShader("DumpSpecPSSM_vs", Shader::ST_VERTEX, "share/shaders/dump_spec2.vert");
      sf = mgr.createShader("DumpSpecPSSM_fs", Shader::ST_FRAGMENT, "share/shaders/dump_spec2.frag");
      mDumpSpecPSSM = mgr.createProgram("DumpSpecPSSM", sv, sf);
      sv = mgr.createShader("GBuffer_vs", Shader::ST_VERTEX, "share/shaders/gbuffer.vert");
      sf = mgr.createShader("GBuffer_fs", Shader::ST_FRAGMENT, "share/shaders/gbuffer.frag");
      mGBufProg = mgr.createProgram("GBuffer", sv, sf);
      // Setup textures
#ifdef _DEBUG
      std::cout << "Setup textures..." << std::endl;
#endif
      glEnable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      if (glGetError() != GL_NO_ERROR) {
        std::cout << "Error in first part of initialization" << std::endl;
      }
      glGenTextures(1, &mPositionBuffer);
      glBindTexture(GL_TEXTURE_2D, mPositionBuffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mPositionBuffer" << endl;
      glGenTextures(1, &mNormalBuffer);
      glBindTexture(GL_TEXTURE_2D, mNormalBuffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mNormalBuffer" << endl;
      glGenTextures(1, &mColorBuffer);
      glBindTexture(GL_TEXTURE_2D, mColorBuffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mColorBuffer" << endl;
      glGenTextures(1, &mShadowBuffer);
      glBindTexture(GL_TEXTURE_2D, mShadowBuffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mShadowBuffer" << endl;
      glGenTextures(1, &mShadowBlurBuffer);
      glBindTexture(GL_TEXTURE_2D, mShadowBlurBuffer);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mShadowBlurBuffer" << endl;
      glGenRenderbuffersEXT(1, &mDepthBuffer);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuffer);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mViewWidth, mViewHeight);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating render buffer: mDepthBuffer" << endl;
      glGenFramebuffersEXT(1, &mGBuffer);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGBuffer);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mPositionBuffer, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mNormalBuffer, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mColorBuffer, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT3_EXT, GL_TEXTURE_2D, mShadowBuffer, 0);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDepthBuffer);
      cout << "GBuffer" << endl;
      checkFBO();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      glGenFramebuffersEXT(1, &mBlurBuffer);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mBlurBuffer);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mShadowBuffer, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mShadowBlurBuffer, 0);
      cout << "BlurBuffer" << endl;
      checkFBO();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      // For shadow mapping
      // Render targets
      glGenTextures(1, &mColorMap0);
      glBindTexture(GL_TEXTURE_2D, mColorMap0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mShadowSize, mShadowSize, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mColorMap0" << endl;
      glGenTextures(1, &mColorMap1);
      glBindTexture(GL_TEXTURE_2D, mColorMap1);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mShadowSize, mShadowSize, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mColorMap1" << endl;
      glGenTextures(1, &mColorMap2);
      glBindTexture(GL_TEXTURE_2D, mColorMap2);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mShadowSize, mShadowSize, 0, GL_RGBA, GL_FLOAT, NULL);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating texture: mColorMap2" << endl;
      // Depth buffer
      glGenRenderbuffersEXT(1, &mDBO);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDBO);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mShadowSize, mShadowSize);
      if (glGetError() != GL_NO_ERROR) cout << "Error creating render buffer: mDBO" << endl;
      // Setup FBO
      glGenFramebuffersEXT(1, &mFBO);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBO);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mColorMap0, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, GL_TEXTURE_2D, mColorMap1, 0);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT2_EXT, GL_TEXTURE_2D, mColorMap2, 0);
      glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mDBO);
      cout << "ShadowBuffer" << endl;
      checkFBO();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      cout << "Initialization done !" << endl;
    }
    
    void cleanup() {
      cout << "Cleanup" << endl;
      glDeleteTextures(1, &mColorMap0);
      glDeleteTextures(1, &mColorMap1);
      glDeleteTextures(1, &mColorMap2);
      glDeleteTextures(1, &mColorBuffer);
      glDeleteTextures(1, &mPositionBuffer);
      glDeleteTextures(1, &mNormalBuffer);
      glDeleteTextures(1, &mShadowBuffer);
      glDeleteTextures(1, &mShadowBlurBuffer);
      glDeleteRenderbuffersEXT(1, &mDBO);
      glDeleteRenderbuffersEXT(1, &mDepthBuffer);
      glDeleteFramebuffersEXT(1, &mFBO);
      glDeleteFramebuffersEXT(1, &mGBuffer);
      delete ProgramManager::InstancePtr();
      delete[] mCubeInstPos;
      delete mPlane;
      delete mCube;
    }
    
    // ---
    
    void setAspect(int w, int h) {
      mViewWidth = w;
      mViewHeight = h;
      mViewAspect = float(w) / float(h);
      mDirtyProj = true;
      // resize GBuffer
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, mPositionBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, mNormalBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, mColorBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, mShadowBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, mShadowBlurBuffer);
      glTexImage2D(GL_TEXTURE_2D, 0, TexFormat, mViewWidth, mViewHeight, 0, GL_RGBA, GL_FLOAT, NULL);
      glBindTexture(GL_TEXTURE_2D, 0);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mDepthBuffer);
      glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, mViewWidth, mViewHeight);
      glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    }
    
    void setupGLViewport() {
      glViewport(0, 0, mViewWidth, mViewHeight);
    }
    
    void setupGLProjection() {
      glMatrixMode(GL_PROJECTION);
      glLoadMatrixf(mProjMatrix);
    }
    
    void beginCustomGLProjection(const Matrix4 &Mp) {
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadMatrixf(Mp);
    }
    
    void endCustomGLProjection() {
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
    }
    
    void setupGLView() {
      glMatrixMode(GL_MODELVIEW);
      glLoadMatrixf(mCamView);
    }
    
    void beginCustomGLView(const Matrix4 &Mv) {
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadMatrixf(Mv);
    }
    
    void endCustomGLView() {
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
    }
    
    void setupMaterial(const MaterialData &material) {
      glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diff);
      glMaterialfv(GL_FRONT, GL_SPECULAR, material.spec);
      glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambi);
      glMaterialf(GL_FRONT, GL_SHININESS, material.shininess);
    }
    
    void renderObjects(MaterialData *mat=NULL, bool diffuseOnly=false) {
      if (mat) {
        if (diffuseOnly) {
          glColor4fv(mat->diff);
        } else {
          setupMaterial(*mat);
        }
      }
      glPushMatrix();
        glTranslatef(mBunnyPos.x, mBunnyPos.y, mBunnyPos.z);
        glScalef(20.0f, 20.0f, 20.0f);
        mBunny->getSubMesh(0)->render();
      glPopMatrix();
      glPushMatrix();
        glTranslatef(mHorsePos.x, mHorsePos.y, mHorsePos.z);
        glScalef(12.0f, 12.0f, 12.0f);
        glRotatef(-90.0f, 1.0f, 0.0f, 0.0f);
        mHorse->getSubMesh(0)->render();
      glPopMatrix();
    }
    
    void renderPlane(MaterialData *mat=NULL, bool diffuseOnly=false) {
      if (mat) {
        if (diffuseOnly) {
          glColor4fv(mat->diff);
        } else {
          setupMaterial(*mat);
        }
      }
      mPlane->render();
    }
    
    void renderLightObject() {
      glPushMatrix();
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.8f, 0.8f, 0.0f);
        glTranslatef(mLightPos.x, mLightPos.y, mLightPos.z);
        glutSolidSphere(1, 16, 16);
        glPopAttrib();
      glPopMatrix();
    }
    
    void renderHull(const ConvexHull3D &b, const Vector3 &color) {
      int i, j;
      Vector4 c0 = Vector4(color, 0.25f);
      Vector3 c1 = color - Vector3(0.25f);
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for (i=0; i<b.getNumPolygons(); ++i) {
        const ConvexHull3D::Polygon &p = b.getPolygon(i);
        glBegin(GL_POLYGON);
          glColor4fv(c0);
          for (j=0; j<p.getNumVertices(); ++j) {
            glVertex3fv(p.getVertex(j));
          }
        glEnd();
      }
      glDisable(GL_BLEND);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      for (i=0; i<b.getNumPolygons(); ++i) {
        const ConvexHull3D::Polygon &p = b.getPolygon(i);
        glBegin(GL_POLYGON);
          glColor3fv(c1);
          for (j=0; j<p.getNumVertices(); ++j) {
            glVertex3fv(p.getVertex(j));
          }
        glEnd();
      }
      glPolygonMode(GL_FRONT, GL_FILL);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_LIGHTING);
    }
    
    void renderDebugView() {
      setupGLViewport();
      setupGLProjection(); // same projection as cam1
      beginCustomGLView(mCamView2);
      glDisable(GL_TEXTURE_2D);
      for (int i=0; i<8; ++i) {
        glActiveTextureARB(GLenum(GL_TEXTURE0_ARB+i));
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      glLightfv(GL_LIGHT0, GL_POSITION, mLightPosGL); // with 0 or 1 appended
      if (mLightType == LT_SPOT) {
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, mLightDir);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, mLightOuter);
      }
      renderLightObject();
      renderObjects(&CubeMaterial);
      renderPlane(&PlaneMaterial);
      mPSSM.setCurrentLayer(mCurSplit);
      renderHull(mPSSM.getCameraHull(), Vector3(1, 0, 0));
      renderHull(mPSSM.getBodyB(), Vector3(0, 0, 1));
      endCustomGLView();
    }
    
    void renderToQuad(GLint w, GLint h, int nsrc, GLuint *src, GLuint fbo, int dst, Program *prg) {
      int i;
      GLint db = GL_BACK;
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo);
      if (fbo > 0 && dst >= 0) {
        glGetIntegerv(GL_DRAW_BUFFER, &db);
        glDrawBuffer(GLenum(GL_COLOR_ATTACHMENT0_EXT+dst));
      }
      glPushAttrib(GL_VIEWPORT_BIT|GL_ENABLE_BIT);
      glDisable(GL_LIGHTING);
      glDisable(GL_DEPTH_TEST);
      glEnable(GL_TEXTURE_2D);
      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, w, h); // this should match target size
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glLoadIdentity();
      for (i=0; i<nsrc; ++i) {
        glActiveTextureARB(GLenum(GL_TEXTURE0_ARB+i));
        glBindTexture(GL_TEXTURE_2D, src[i]);
      }
      if (prg) {
        prg->bind();
        for (i=0; i<nsrc; ++i) {
          ostringstream oss;
          oss << "tex" << i;
          prg->uniform(oss.str())->set(GLint(i));
        }
        prg->uniform("textureWidth")->set(float(w));
        prg->uniform("textureHeight")->set(float(h));
        prg->uniform("invTextureWidth")->set(1.0f / float(w));
        prg->uniform("invTextureHeight")->set(1.0f / float(h));
      }
      glBegin(GL_QUADS);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 0);
        glVertex3i(-1, -1, -1);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 0);
        glVertex3i( 1, -1, -1);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 1, 1);
        glVertex3i( 1,  1, -1);
        glMultiTexCoord2fARB(GL_TEXTURE0_ARB, 0, 1);
        glVertex3i(-1,  1, -1);
      glEnd();
      if (prg) {
        prg->unbind();
      }
      for (i=0; i<nsrc; ++i) {
        glActiveTextureARB(GLenum(GL_TEXTURE0_ARB+i));
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glPopAttrib();
      if (fbo > 0 && dst >= 0) {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glDrawBuffer(db);
      }
    }
    
    void renderSplitShadowPass() {
      // we suppose nsplit = 3 for now
      GLint db;
      glGetIntegerv(GL_DRAW_BUFFER, &db);
      glPushAttrib(GL_VIEWPORT_BIT|GL_ENABLE_BIT);
      glClearColor(1,1,1,1);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBO);
      mPSSM.update();
      mPSSM.enableCasterProgram();
      for (int i=0; i<3; ++i) {
        mPSSM.setCurrentLayer(i);
        glDrawBuffer(GLenum(GL_COLOR_ATTACHMENT0_EXT+i));
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, mShadowSize, mShadowSize);
        beginCustomGLProjection(mPSSM.getProjectionMatrix());
        beginCustomGLView(mPSSM.getViewMatrix());
        if (mBackFaceCaster) {
          glFrontFace(GL_CW);
        }
        renderObjects();
        if (mBackFaceCaster) {
          glFrontFace(GL_CCW);
        }
        endCustomGLProjection();
        endCustomGLView();
      }
      mPSSM.disableCasterProgram();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      //glClearColor(0,0,0,1);
      glClearColor(0.5f, 0.5f, 0.5f ,1);
      glPopAttrib();
      glDrawBuffer(db);
      glEnable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
    }
    
    void renderShadowPass() {
      GLint db;
      glGetIntegerv(GL_DRAW_BUFFER, &db);
      glPushAttrib(GL_VIEWPORT_BIT|GL_ENABLE_BIT);
      glClearColor(1,1,1,1);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFBO);
      glDrawBuffer(GL_COLOR_ATTACHMENT0_EXT);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      glViewport(0, 0, mShadowSize, mShadowSize);
      mShadow->update();
      beginCustomGLProjection(mShadow->getProjectionMatrix());
      beginCustomGLView(mShadow->getViewMatrix());
      if (mBackFaceCaster) {
        glFrontFace(GL_CW);
      }
      mShadow->enableCasterProgram();
      renderObjects();
      if (mBackFaceCaster) {
        glFrontFace(GL_CCW);
      }
      mShadow->disableCasterProgram();
      endCustomGLProjection();
      endCustomGLView();
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
      //glClearColor(0,0,0,1);
      glClearColor(0.5f, 0.5f, 0.5f ,1);
      glPopAttrib();
      glDrawBuffer(db);
    }
    
    void renderAmbient() {
      // NOOP
    }
    
    void renderGBuffer() {
      GLenum db[4] = {
        GL_COLOR_ATTACHMENT0_EXT,
        GL_COLOR_ATTACHMENT1_EXT,
        GL_COLOR_ATTACHMENT2_EXT,
        GL_NONE
      };
      glDrawBuffers(4, db);
      glClearColor(0,0,0,1);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      mGBufProg->bind();
      mGBufProg->uniform("inverseViewMatrix")->set(mCamViewInverse);
      mGBufProg->uniform("diffuseColor")->set(Vector4(CubeMaterial.diff));
      renderObjects(&CubeMaterial);
      mGBufProg->uniform("diffuseColor")->set(Vector4(PlaneMaterial.diff));
      renderPlane(&PlaneMaterial);
      
      mGBufProg->uniform("diffuseColor")->set(Vector4(0.8f,0.8f,0.8f,1.0f));
      //renderLightObject();
      glPushMatrix();
        glTranslatef(mLightPos.x, mLightPos.y, mLightPos.z);
        glutSolidSphere(1, 16, 16);
      glPopMatrix();
      
      mGBufProg->unbind();
      db[0] = GL_BACK;
      db[1] = db[2] = GL_NONE;
      glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
      glDrawBuffers(4, db);
    }
    
    void renderDiffuse() {
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D); // this will not be the case necessarily all the time
      // without the next 6 lines, even with glDisable(GL_TEXTURE_2D), texture pop up
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, 0);
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glBindTexture(GL_TEXTURE_2D, 0);
      // blend color with lighting
      glEnable(GL_BLEND);
      glBlendFunc(GL_DST_COLOR, GL_ZERO); // if DST == 0,0,0 --> black... bu why would it be black
      // offset towards eye to avoid depth fighting
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(-1, -1);
      // do not write offseted depth of course
      glDepthMask(GL_FALSE);
      // render
      renderObjects(&CubeMaterial, true);
      renderPlane(&PlaneMaterial, true);
      // restore state
      glDepthMask(GL_TRUE);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glDisable(GL_BLEND);
      glEnable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap0);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap1);
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap2);
    }
    
    void renderSpecular(GLint shadowMap, const Matrix4 &LS, const Matrix4 &PPT) {
      // should not add specular to shadowed area... isn't it
      // then we should use again the depth map [and in TSM case, the Nt matrix]
      // again for split scene --> will need 3 pass to render
      // it is starting to be a bit expensive, should do the specular lighting
      // during the diffuse pass, iterate lights in shader ?
      // all on all it would be the same
      glDepthMask(GL_FALSE);
      glDisable(GL_TEXTURE_2D); // again this is true for the current scene
      glDisable(GL_LIGHTING);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE); // ADD
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(-1, -1);
      //glPolygonOffset(0, -5);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, shadowMap); //mColorMap0);
      mDumpSpec->bind();
      mDumpSpec->uniform("shininess")->set(CubeMaterial.shininess);
      mDumpSpec->uniform("LS")->set(LS);
      mDumpSpec->uniform("eyeToLightProj")->set(PPT*LS);
      mDumpSpec->uniform("shadowMap")->set(GLint(0));
      mDumpSpec->uniform("shadowSize")->set(GLint(mShadowSize));
      renderObjects();
      mDumpSpec->uniform("shininess")->set(PlaneMaterial.shininess);
      renderPlane();
      mDumpSpec->unbind();
      glDisable(GL_BLEND);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glEnable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);
    }
    
    void renderSpecularPSSM() {
      glDepthMask(GL_FALSE);
      glDisable(GL_TEXTURE_2D); // again this is true for the current scene
      glDisable(GL_LIGHTING);
      glEnable(GL_BLEND);
      glBlendFunc(GL_ONE, GL_ONE); // ADD
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(-1, -1);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap0);
      glActiveTextureARB(GL_TEXTURE1_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap1);
      glActiveTextureARB(GL_TEXTURE2_ARB);
      glBindTexture(GL_TEXTURE_2D, mColorMap2);
      mDumpSpecPSSM->bind();
      mPSSM.setCurrentLayer(0);
      mDumpSpecPSSM->uniform("splits", 0)->set(-mPSSM.getCameraNear());
      for (int i=0; i<3; ++i) {
        mPSSM.setCurrentLayer(i);
        mDumpSpecPSSM->uniform("splits", i+1)->set(-mPSSM.getCameraFar());
        Matrix4 LS = mPSSM.getProjectionMatrix() * mPSSM.getViewMatrix() * getCameraInverseViewMatrix();
        mDumpSpecPSSM->uniform("eyeToLightProjs", i)->set(LS);
        mDumpSpecPSSM->uniform("shadowMaps", i)->set(GLint(i));
      }
      mDumpSpecPSSM->uniform("shininess")->set(CubeMaterial.shininess);
      mDumpSpecPSSM->uniform("shadowSize")->set(GLint(mShadowSize));
      renderObjects();
      mDumpSpecPSSM->uniform("shininess")->set(PlaneMaterial.shininess);
      renderPlane();
      mDumpSpecPSSM->unbind();
      glDisable(GL_BLEND);
      glDisable(GL_POLYGON_OFFSET_FILL);
      glEnable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      glDepthMask(GL_TRUE);
    }
    
    void renderShadow(GLuint shadowMap) {
      glDisable(GL_BLEND);
      glDisable(GL_LIGHTING);
      glEnable(GL_TEXTURE_2D);
      setupMaterial(NoneMaterial);
      glColor4fv(white);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, shadowMap);
      mShadow->enableReceiverProgram(true);
      renderObjects();
      renderPlane();
      mShadow->disableReceiverProgram();
      glEnable(GL_LIGHTING);
    }
    
    void renderLighting(GLuint shadowMap) {
      glDisable(GL_BLEND);
      setupMaterial(NoneMaterial);
      glColor4fv(white);
      glActiveTextureARB(GL_TEXTURE0_ARB);
      glBindTexture(GL_TEXTURE_2D, shadowMap);
      mShadow->enableReceiverProgram();
      renderObjects();
      renderPlane();
      mShadow->disableReceiverProgram();
    }
    
    void renderScene() {
      setupGLViewport();
      setupGLProjection();
      setupGLView();
      glLightfv(GL_LIGHT0, GL_POSITION, mLightPosGL); // with 0 or 1 appended
      if (mLightType == LT_SPOT) {
        glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, mLightDir);
        glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, mLightOuter);
      }
      // === AMBIENT PASS
      renderAmbient();
      if (mShadow != &mPSSM) {
        // === LIGHTING (Add)
        renderLighting(mColorMap0);
        // === DIFFUSE PASS (Modulate)
        if (mDiffuse) {
          renderLightObject();
          renderDiffuse();
        }
        // === SPECULAR PASS (Add)
        if (mSpecular) {
          Matrix4 LS, PPT;
          if (mShadow == &mTSM) {
            LS = mShadow->getViewMatrix();
            PPT = mShadow->getProjectionMatrix();
          } else {
            LS = mShadow->getProjectionMatrix() * mShadow->getViewMatrix();
          }
          LS *= getCameraInverseViewMatrix();
          renderSpecular(mColorMap0, LS, PPT);
        }
      } else {
        glActiveTextureARB(GL_TEXTURE0_ARB);
        glBindTexture(GL_TEXTURE_2D, mColorMap0);
        glActiveTextureARB(GL_TEXTURE1_ARB);
        glBindTexture(GL_TEXTURE_2D, mColorMap1);
        glActiveTextureARB(GL_TEXTURE2_ARB);
        glBindTexture(GL_TEXTURE_2D, mColorMap2);
#ifdef PSSM_SINGLE_PASS_LIGHTING
        renderLighting(mColorMap0);
        if (mDiffuse) {
          renderLightObject();
          renderDiffuse();
        }
        if (mSpecular) {
          renderSpecularPSSM();
        }
#else
        // 3 pass
        int from = 0;
        int to = 2;
        if (mOneSplit) {
          from = mCurSplit;
          to = from;
        }
        for (int i=from; i<=to; ++i) {
          mPSSM.setCurrentLayer(i);
          GLuint shadowMap = (i==0 ? mColorMap0 : (i==1 ? mColorMap1 : mColorMap2));
          // need to bias a little so that there's no overlap
          float F = mPSSM.getCameraFar();
          float N = mPSSM.getCameraNear();
          float off = (F - N) * 0.02f; // 0.5f;
          //float off = 0.0f;
          beginCustomGLProjection(Matrix4::MakePerspective(mCamFovy, mViewAspect, N-off, F));
          glDepthRange(float(i)/3.0f, float(i+1)/3.0f);
          glMatrixMode(GL_MODELVIEW);
          // === LIGHTING
          renderLighting(shadowMap);
          // === DIFFUSE PASS (Modulate)
          if (mSplitLayers[i].mDiffuse) {
            renderLightObject();
            renderDiffuse();
          }
          // === SPECULAR PASS (Add)
          if (mSplitLayers[i].mSpecular) {
            Matrix4 LS = mPSSM.getProjectionMatrix() * mPSSM.getViewMatrix();
            LS *= getCameraInverseViewMatrix();
            renderSpecular(shadowMap, LS, Matrix4::IDENTITY);
          }
          endCustomGLProjection();
          glMatrixMode(GL_MODELVIEW);
          // specular pass should be deferred
          // as diffuse one
          // it would be far more effective
          // and avoid all blend && polygon offset mess
          // just render the scene for diffuse comtrib
          // and positon/normal etc with MRT !
        }
#endif
      }
      // Done
      //glDisable(GL_BLEND);
      //glEnable(GL_LIGHTING);
      //glDisable(GL_POLYGON_OFFSET_FILL);
      //glDepthMask(GL_TRUE);
    }
    
    void render() {
      
      if (mTestRTT) {
        
        // render shadow map using current approach
        // render lighting buffer [diffuse component]
        // accumulate for each light
        
        if (mShadow == &mPSSM) {
          renderSplitShadowPass();
          glActiveTextureARB(GL_TEXTURE0_ARB);
          glBindTexture(GL_TEXTURE_2D, mColorMap0);
          glActiveTextureARB(GL_TEXTURE1_ARB);
          glBindTexture(GL_TEXTURE_2D, mColorMap1);
          glActiveTextureARB(GL_TEXTURE2_ARB);
          glBindTexture(GL_TEXTURE_2D, mColorMap2);
        } else {
          renderShadowPass();
        }
        // bindFramebuffer
        
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGBuffer);
        setupGLViewport();
        setupGLProjection();
        setupGLView();
        
        //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mBlurBuffer);
        //glDrawBuffer(GL_COLOR_ATTACHMENT1_EXT); // -> mShadowBlurBuffer
        glDrawBuffer(GL_COLOR_ATTACHMENT3_EXT);
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        renderShadow(mColorMap0);
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        //glDrawBuffer(GL_BACK);
        
        //glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mGBuffer);
        renderGBuffer();
        
        glDisable(GL_DEPTH_TEST);
        
        if (mShowRTT == 0) {
          renderToQuad(mViewWidth, mViewHeight, 1, &mPositionBuffer, 0, -1, NULL);
        } else if (mShowRTT == 1) {
          renderToQuad(mViewWidth, mViewHeight, 1, &mNormalBuffer, 0, -1, NULL);
        } else if (mShowRTT == 2) {
          renderToQuad(mViewWidth, mViewHeight, 1, &mColorBuffer, 0, -1, NULL);
        } else if (mShowRTT == 3) {
          renderToQuad(mViewWidth, mViewHeight, 1, &mShadowBuffer, 0, -1, NULL);
        } else {
          
          GLuint shadow = mShadowBuffer;
          
          if (mBlur) {
            // POISSON
            renderToQuad(mViewWidth, mViewHeight, 1, &mShadowBuffer, mBlurBuffer, 1, mBlurDisc);
            shadow = mShadowBlurBuffer;
            // GAUSS
            //renderToQuad(mViewWidth, mViewHeight, 1, &mShadowBuffer, mBlurBuffer, 1, mBlurX);
            //renderToQuad(mViewWidth, mViewHeight, 1, &mShadowBlurBuffer, mBlurBuffer, 0, mBlurY);
            //shadow stays the same
          }
          
          GLuint textures[4] = {
            mPositionBuffer,
            mNormalBuffer,
            mColorBuffer,
            shadow
            //mShadowBlurBuffer
            //mShadowBuffer
          };
          mCompose->bind();
          mCompose->uniform("wLightPos")->set(mLightPos);
          mCompose->uniform("wEyePos")->set(mCamViewInverse.getColumn(3));
          mCompose->unbind(); // really
          renderToQuad(mViewWidth, mViewHeight, 4, textures, 0, -1, mCompose);
        }
        
        glEnable(GL_DEPTH_TEST);
        
      } else if (mUseCam2) {
        
        renderDebugView();
        
      } else {
        
        if (mShadow == &mPSSM) {
          renderSplitShadowPass();
        } else {
          renderShadowPass();
        }
        renderScene();
        
      }
    }
        
    void update() {
      if (mDirtyProj) {
        mProjMatrix = Matrix4::MakePerspective(mCamFovy, mViewAspect, mCamNear, mCamFar);
        mDirtyProj = false;
      }
      if (mDirtyView) {
        mCamView = Matrix4::MakeTranslate(Vector3(0,0,-mCamDtt));
        mCamView *= Matrix4::MakeRotate(-mCamRot[2], Vector3::UNIT_Z); // roll
        mCamView *= Matrix4::MakeRotate(-mCamRot[1], Vector3::UNIT_X); // pitch
        mCamView *= Matrix4::MakeRotate(-mCamRot[0], Vector3::UNIT_Y); // yaw
        mCamView *= Matrix4::MakeTranslate(-mCamAim);
        mCamViewInverse = mCamView.getFastInverse();
        mCamPos = Vector3(mCamViewInverse.getColumn(3));
        mCamDir = Vector3(-mCamViewInverse.getColumn(2));
        
        mCamView2 = Matrix4::MakeTranslate(Vector3(0,0,-mCamDtt2));
        mCamView2 *= Matrix4::MakeRotate(-mCamRot2[2], Vector3::UNIT_Z); // roll
        mCamView2 *= Matrix4::MakeRotate(-mCamRot2[1], Vector3::UNIT_X); // pitch
        mCamView2 *= Matrix4::MakeRotate(-mCamRot2[0], Vector3::UNIT_Y); // yaw
        mCamView2 *= Matrix4::MakeTranslate(-mCamAim2);
        mCamViewInverse2 = mCamView2.getFastInverse();
        mCamPos2 = Vector3(mCamViewInverse2.getColumn(3));
        mCamDir2 = Vector3(-mCamViewInverse2.getColumn(2));
        
        mDirtyView = false;
        mDirtyLight = true;
      }
      if (mDirtyLight) {
        mLightDir = (mLightAim - mLightPos).normalize();
        mLightPosEye = mCamView * mLightPos;
        mLightPosGL = Vector4(mLightPos, (mLightType==LT_DIR ? 0 : 1));
        mDirtyLight = false;
        
        mShadow->update();
      }
    }
    
  public:
  
    static void OnDraw() {
      FrameState &fs = FrameState::Instance();
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
      fs.render();
      glutSwapBuffers();
    }
    
    static void OnReshape(int w, int h) {
      FrameState &fs = FrameState::Instance();
      fs.setAspect(w, h);
      fs.update();
    }
    
    static void OnKeyboard(unsigned char key, int, int) {
      FrameState &fs = FrameState::Instance();
      switch (key) {
        case 27:
          exit(0);
        case 'g':
          fs.mBlur = !fs.mBlur; // should be a gaussian blur
          cout << "Blur:  " << (fs.mBlur ? "on" : "off") << endl;
          glutPostRedisplay();
          break;
        case 'b':
          fs.mBackFaceCaster = !fs.mBackFaceCaster;
          glutPostRedisplay();
          break;
        case 'o':
          fs.mOneSplit = !fs.mOneSplit;
          glutPostRedisplay();
          break;
        case 'd':
          fs.mDepthBias += (1.0f / (fs.mCamNear + fs.mCamFar));
          fs.mTSM.setDepthBias(fs.mDepthBias);
          fs.mLiSPSM.setDepthBias(fs.mDepthBias);
          fs.mStandard.setDepthBias(fs.mDepthBias);
          fs.mPSSM.setDepthBias(fs.mDepthBias);
          cout << "depth bias: " << fs.mDepthBias << endl;
          glutPostRedisplay();
          break;
        case 'D':
          fs.mDepthBias -= (1.0f / (fs.mCamNear + fs.mCamFar));
          fs.mTSM.setDepthBias(fs.mDepthBias);
          fs.mLiSPSM.setDepthBias(fs.mDepthBias);
          fs.mStandard.setDepthBias(fs.mDepthBias);
          fs.mPSSM.setDepthBias(fs.mDepthBias);
          cout << "depth bias: " << fs.mDepthBias << endl;
          glutPostRedisplay();
          break;
        case 'f':
          fs.mTSM.setFocusFactor(fs.mTSM.getFocusFactor()+0.05f);
          cout << "factor: " << fs.mTSM.getFocusFactor() << endl;
          glutPostRedisplay();
          break;
        case 'F':
          fs.mTSM.setFocusFactor(fs.mTSM.getFocusFactor()-0.05f);
          cout << "factor: " << fs.mTSM.getFocusFactor() << endl;
          glutPostRedisplay();
          break;
        case 's':
          if (fs.mShadow == &fs.mStandard) {
            cout << "LiSPSM" << endl;
            fs.mShadow = &fs.mLiSPSM;
          } else if (fs.mShadow == &fs.mLiSPSM) {
            cout << "TSM" << endl;
            fs.mShadow = &fs.mTSM;
          } else if (fs.mShadow == &fs.mTSM) {
            cout << "PSSM" << endl;
            fs.mShadow = &fs.mPSSM;
          } else {
            cout << "Standard" << endl;
            fs.mShadow = &fs.mStandard;
          }
          glutPostRedisplay();
          break;
        case 'v':
          fs.mVSM = !fs.mVSM;
          fs.mPCF = false;
          fs.mTSM.enableVSM(fs.mVSM);
          fs.mLiSPSM.enablePCF(false);
          glutPostRedisplay();
          break;
        case 'p':
          fs.mPCF = !fs.mPCF;
          fs.mVSM = false;
          fs.mTSM.enablePCF(fs.mPCF);
          fs.mLiSPSM.enablePCF(fs.mPCF);
          glutPostRedisplay();
          break;
        case 'l':
          switch (fs.mLightType) {
            case LT_DIR:
              fs.mLightType = LT_POINT;
              break;
            case LT_POINT:
              fs.mLightType = LT_SPOT;
              break;
            case LT_SPOT:
              fs.mLightType = LT_DIR;
            default:
              break;
          }
          fs.mDirtyLight = true;
          fs.update();
          glutPostRedisplay();
          break;
        case 't':
          //fs.mTestRTT = !fs.mTestRTT;
          if (fs.mTestRTT) {
            fs.mTestRTT = false;
            if (fs.mShadow == &fs.mPSSM) {
              fs.mUseCam2 = true;
            } else {
              fs.mUseCam2 = false;
            }
          } else {
            if (fs.mUseCam2 == true) {
              fs.mUseCam2 = false;
            } else {
              fs.mTestRTT = true;
            }
          }
          glutPostRedisplay();
          break;
        case '+':
          if (fs.mTestRTT) {
            if (++fs.mShowRTT >= 5) {
              fs.mShowRTT = 4;
            }
          } else {
            if (++fs.mCurSplit >= 3) {
              fs.mCurSplit=2;
            }
          }
          glutPostRedisplay();
          break;
        case '-':
          if (fs.mTestRTT) {
            if (--fs.mShowRTT < 0) {
              fs.mShowRTT = 0;
            }
          } else {
            if (--fs.mCurSplit < 0) {
              fs.mCurSplit=0;
            }
          }
          glutPostRedisplay();
          break;
        case '1':
          if (fs.mShadow == &fs.mPSSM) {
            fs.mSplitLayers[fs.mCurSplit].mDiffuse = !fs.mSplitLayers[fs.mCurSplit].mDiffuse;
          } else {
            fs.mDiffuse = !fs.mDiffuse;
            fs.mSplitLayers[0].mDiffuse = fs.mDiffuse;
            fs.mSplitLayers[1].mDiffuse = fs.mDiffuse;
            fs.mSplitLayers[2].mDiffuse = fs.mDiffuse;
          }
          glutPostRedisplay();
          break;
        case '2':
          if (fs.mShadow == &fs.mPSSM) {
            fs.mSplitLayers[fs.mCurSplit].mSpecular = !fs.mSplitLayers[fs.mCurSplit].mSpecular;
          } else {
            fs.mSpecular = !fs.mSpecular;
            fs.mSplitLayers[0].mSpecular = fs.mSpecular;
            fs.mSplitLayers[1].mSpecular = fs.mSpecular;
            fs.mSplitLayers[2].mSpecular = fs.mSpecular;
          }
          glutPostRedisplay();
          break;
        default:
          break;
      }
    }
    
    static void OnMouseClick(int button, int state, int x, int y) {
      FrameState &fs = FrameState::Instance();
      fs.mMouseButton = button;
      fs.mMouseState = state;
      fs.mKeyMod = glutGetModifiers();
      fs.mCurX = x;
      fs.mCurY = y;
      fs.mLastX = x;
      fs.mLastY = y;
      if (fs.mMouseState == GLUT_DOWN) {
        if (fs.mKeyMod & GLUT_ACTIVE_ALT) {
          switch (fs.mMouseButton) {
            case GLUT_LEFT_BUTTON:
              fs.mMouseOp = MO_CAM_ROTATE;
              break;
            case GLUT_MIDDLE_BUTTON:
              fs.mMouseOp = MO_CAM_TRANSLATE;
              break;
            case GLUT_RIGHT_BUTTON:
              fs.mMouseOp = MO_CAM_DOLLY;
              break;
            default:
              fs.mMouseOp = MO_NONE;
          }
        } else {
          if (fs.mMouseButton == GLUT_LEFT_BUTTON) {
            Matrix4 *v = (fs.mUseCam2 ? &(fs.mCamView2) : &(fs.mCamView)); 
            float sy = fs.getCameraNear() * Tand(0.5f * fs.getCameraFovy());
            float sx = fs.getViewAspect() * sy;
            float nx = ((2.0f * x) / float(fs.mViewWidth)) - 1.0f;
            float ny = ((2.0f * (fs.mViewHeight - y)) / float(fs.mViewHeight)) - 1.0f;
            Vector3 org = Vector3(0,0,0);
            Vector3 dir = (Vector3(sx*nx, sy*ny, -fs.getCameraNear()) - org).normalize();
            //Vector3 lpos = fs.getCameraViewMatrix() * fs.getLightPosition();
            Vector3 lpos = (*v) * fs.getLightPosition();
            Plane pl(Vector3::UNIT_Z, lpos.z);
            Ray ray(org, dir);
            float t;
            ray.intersect(pl, &t);
            Vector3 tmp = org - t*dir;
            float d = (tmp - lpos).getLength();
            if (d <= 1.0f) {
              fs.mMouseOp = MO_LIGHT_TRANSLATE; 
            }
          }
        }
      } else {
        fs.mMouseOp = MO_NONE;
      }
    }
    
    static void OnMouseMove(int x, int y) {
      FrameState &fs = FrameState::Instance();
      fs.mLastX = fs.mCurX;
      fs.mLastY = fs.mCurY;
      fs.mCurX = x;
      fs.mCurY = y;
    }
    
    static void OnMouseDrag(int x, int y) {
      FrameState &fs = FrameState::Instance();
      fs.mLastX = fs.mCurX;
      fs.mLastY = fs.mCurY;
      fs.mCurX = x;
      fs.mCurY = y;
      float dx = float(fs.mCurX - fs.mLastX);
      float dy = float(fs.mLastY - fs.mCurY);
      
      if (fs.mMouseOp == MO_CAM_ROTATE) {
        Vector3 *r = (fs.mUseCam2 ? &(fs.mCamRot2) : &(fs.mCamRot)); 
        //fs.mCamRot[1] += 0.25f * dy; // pitch
        //fs.mCamRot[0] -= 0.25f * dx; // yaw
        (*r)[1] += 0.25f * dy; // pitch
        (*r)[0] -= 0.25f * dx; // yaw
        fs.mDirtyView = true;
        fs.update();
        glutPostRedisplay();
      
      } else if (fs.mMouseOp == MO_CAM_TRANSLATE) {
        float *d = (fs.mUseCam2 ? &(fs.mCamDtt2) : &(fs.mCamDtt));
        Vector3 *a = (fs.mUseCam2 ? &(fs.mCamAim2) : &(fs.mCamAim));
        Matrix4 *vi = (fs.mUseCam2 ? &(fs.mCamViewInverse2) : &(fs.mCamViewInverse));
        //float ny = fs.mCamDtt * Tand(0.5f * fs.mCamFovy);
        float ny = (*d) * Tand(0.5f * fs.mCamFovy);
        float piy = ny / (0.5f * float(fs.mViewHeight));
        float pix = piy * fs.getViewAspect();
        dx *= -pix;
        dy *= -piy;
        //Vector3 x(fs.getCameraInverseViewMatrix().getColumn(0));
        //Vector3 y(fs.getCameraInverseViewMatrix().getColumn(1));
        Vector3 x(vi->getColumn(0));
        Vector3 y(vi->getColumn(1));
        x *= dx;
        y *= dy;
        //fs.mCamAim += (x + y);
        *a += (x + y);
        fs.mDirtyView = true;
        fs.update();
        glutPostRedisplay();
      
      } else if (fs.mMouseOp == MO_CAM_DOLLY) {
        float *d = (fs.mUseCam2 ? &(fs.mCamDtt2) : &(fs.mCamDtt));
        float dolly = (Abs(dx) > Abs(dy) ? dx : dy);
        //if (fs.mCamDtt > 0.000001) {
          //dolly *= (-1 * fs.mCamDtt) / 500;
        if (*d > 0.000001) {
          dolly *= (-1 * (*d)) / 500;
        } else {
          dolly *= -0.001f;
        }
        //fs.mCamDtt += dolly;
        *d += dolly;
        //if (fs.mCamDtt < 0) {
          //fs.mCamDtt = 0;
        if (*d < 0) {
          *d = 0;
        }
        fs.mDirtyView = true;
        fs.update();
        glutPostRedisplay();
        
      } else if (fs.mMouseOp == MO_LIGHT_TRANSLATE) {
        Matrix4 *v = (fs.mUseCam2 ? &(fs.mCamView2) : &(fs.mCamView));
        Matrix4 *iv = (fs.mUseCam2 ? &(fs.mCamViewInverse2) : &(fs.mCamViewInverse));
        //Vector3 lpos = fs.getCameraViewMatrix() * fs.mLightPos;
        Vector3 lpos = (*v) * fs.mLightPos;
        float sy  = -lpos.z * Tand(0.5f * fs.getCameraFovy());
        float sx  = fs.getViewAspect() * sy;
        float nx0 = ((2.0f * (fs.mLastX - fs.mViewWidth)) / float(fs.mViewWidth)) - 1.0f;
        float ny0 = ((2.0f * (fs.mViewHeight - fs.mLastY)) / float(fs.mViewHeight)) - 1.0f;
        float nx1 = ((2.0f * (fs.mCurX - fs.mViewWidth)) / float(fs.mViewWidth)) - 1.0f;
        float ny1 = ((2.0f * (fs.mViewHeight - fs.mCurY)) / float(fs.mViewHeight)) - 1.0f;
        float dx = sx * (nx1 - nx0);
        float dy = sy * (ny1 - ny0);
        //Vector3 x(fs.getCameraInverseViewMatrix().getColumn(0));
        //Vector3 y(fs.getCameraInverseViewMatrix().getColumn(1));
        Vector3 x(iv->getColumn(0));
        Vector3 y(iv->getColumn(1));
        fs.mLightPos += (dx * x) + (dy * y);
        fs.mDirtyLight = true;
        fs.update();
        glutPostRedisplay();
      
      }
    }
    
  public:
    
    virtual const AABox& getSceneBox() const {
      return mReceiverBB;
    }
    
    virtual float getCameraFovy() const {
      return mCamFovy;
    }
    
    virtual float getViewAspect() const {
      return mViewAspect;
    }
    
    virtual float getCameraNear() const {
      return mCamNear;
    }
    
    virtual float getCameraFar() const {
      return mCamFar;
    }
    
    virtual const Matrix4& getCameraViewMatrix() const {
      return mCamView;
    }
    
    virtual const Matrix4& getCameraInverseViewMatrix() const {
      return mCamViewInverse;
    }
    
    virtual const Vector3& getCameraPosition() const {
      return mCamPos;
    }
    
    virtual const Vector3& getCameraViewDirection() const {
      return mCamDir;
    }
    
    virtual const Vector3& getLightPosition() const {
      return mLightPos;
    }
    
    virtual const Vector3& getLightDirection() const {
      return mLightDir;
    }
    
    virtual const float getSpotLightOuterAngle() const {
      return mLightOuter;
    }
    
    virtual const float getSpotLightInnerAngle() const {
      return mLightInner;
    }
    
    virtual LightType getLightType() const {
      return mLightType;
    }
    
    virtual float getDirectionalLightDistance() const {
      return 3000.0f;
    }
    
    virtual int getShadowMapSize() const {
      return int(mShadowSize);
    }
    
    virtual const Vector3& getLightPositionEye() const {
      return mLightPosEye;
    }
    
    virtual int getShadowMapTexUnit() const {
      return 0;
    }
};


// -----------------------------------------------------------------------------

static void Cleanup() {
  FrameState::Instance().cleanup();
}

static void InitGL() {
  //ggfx::InitExtensions();
  ggfx::System *s = new ggfx::System();
  s->initialize("./share/plugins/ggfx");
  ggfx::Renderer *r = new ggfx::Renderer();
  r->initialize();
  
  glClearColor(0.5f, 0.5f, 0.5f ,1);
  glClearDepth(1);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);
  glPolygonMode(GL_FRONT, GL_FILL);
  glShadeModel(GL_SMOOTH);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
  glEnable(GL_TEXTURE_2D);
  FrameState::Instance().initialize();
}

int main(int argc, char **argv) {
  atexit(Cleanup);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
  glutInitWindowPosition(100,100);
  glutInitWindowSize(512,512);
  glutCreateWindow("ShadowMap test");
  InitGL();
  glutReshapeFunc(&FrameState::OnReshape);
  glutDisplayFunc(&FrameState::OnDraw);
  glutKeyboardFunc(&FrameState::OnKeyboard);
  glutMouseFunc(&FrameState::OnMouseClick);
  glutMotionFunc(&FrameState::OnMouseDrag);
  glutPassiveMotionFunc(&FrameState::OnMouseMove);
  glutMainLoop();
  return 0;
}

