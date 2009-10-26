// engine
#include <ggfx/ggfx>

// glut
#ifdef __APPLE__
# include <GLUT/glut.h>
#else
# ifdef WIN32
#   pragma warning(disable: 4505)
# endif // WIN32
# include <GL/glut.h>
#endif

// gettimeofday
#ifndef _MSC_VER
# include <sys/time.h>
#else
# define WIN32_LEAN_AND_MEAN
# include <time.h>
# include <windows.h> // for timeval struct
# define EPOCHFILETIME (116444736000000000i64)
//#define EPOCHFILETIME 116444736000000000LL /*__GNUC__*/
struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
};
struct timeval {
  long tv_sec;
  long tv_usec;
};
__inline int gettimeofday(struct timeval *tv, struct timezone *tz) {
  static int tzflags = 0;
  FILETIME ft;
  LARGE_INTEGER li;
  __int64 t;
  if (tv != NULL) {
    GetSystemTimeAsFileTime(&ft);
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;
    t  = li.QuadPart; // in 100-nanosecs intervals
    t -= EPOCHFILETIME; // offset to epoch
    t /= 10; // in microsecs
    tv->tv_sec  = (long)(t / 1000000);
    tv->tv_usec = (long)(t % 1000000);
  }
  if (tz != NULL) {
    if (!tzflags) {
      _tzset();
      ++tzflags;
    }
    tz->tz_minuteswest = _timezone / 60;
    tz->tz_dsttime = _daylight;
  }
  return 0;
}
#endif

using namespace ggfx;
using namespace gmath;
using namespace std;

// ---

class GLUTState {
  private:
    
    GLUTState()
      :mWidth(-1), mHeight(-1), mLastX(-1), mLastY(-1), mCurX(-1), mCurY(-1),
       mButton(-1), mState(-1), mModifier(-1), mKey(0) {
    }
    
    ~GLUTState() {
    }
    
    int mWidth;
    int mHeight;
    int mLastX;
    int mLastY;
    int mCurX;
    int mCurY;
    int mButton;
    int mState;
    int mModifier;
    float mAspect;
    unsigned char mKey;
    Matrix4 mCamMV;
    Matrix4 mCamIMV;
    enum CameraManip {
      CAMERA_ROTATE = 0,
      CAMERA_TRANSLATE,
      CAMERA_ZOOM,
      CAMERA_NOOP
    };
    CameraManip mCamManip;
    
    Scene *mScene;
    Node *mTeapot;
    Light *mLight;
    Mesh *mMesh;
    Material *mTeapotMaterial;
    Material *mXMaterial;
    Material *mYMaterial;
    Material *mZMaterial;
    Material *mCMaterial;
    Renderer *mRenderer;
    System *mSystem;
    Vector3 mCamRot;
    Vector3 mCamAim;
    float mCamDist;
    float mCamNear;
    float mCamFar;
    float mCamFovy;
    Vector4 mLightPos;
    GLUquadric *mCylinder;
    bool mAnimate;
    
    float mLastTime;
    float mCurTime;
    struct timeval mInitTime;
    
  public:
    
    static GLUTState& Get() {
      static GLUTState gs = GLUTState();
      return gs;
    }

    float getTime() const {
      struct timeval tv;
      gettimeofday(&tv, NULL);
      return (float(tv.tv_sec - mInitTime.tv_sec) +
              (0.000001f * float(tv.tv_usec - mInitTime.tv_usec)));
    }

    float getDeltaTime() const {
      return (mCurTime - mLastTime);
    }
    
    void onInitialize() {
      mSystem = new System();
      mSystem->initialize("./share/plugins/ggfx");
      mRenderer = new Renderer();
      mRenderer->initialize();
      mRenderer->setStencilTestEnabled(true);
      glPolygonMode(GL_FRONT, GL_FILL);
      glShadeModel(GL_SMOOTH);
      glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
      glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
      glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
      glEnable(GL_TEXTURE_2D);
      
      mCamRot = Vector3(0, 0, 0);
      mCamDist = 10.0f;
      mCamAim = Vector3(0, 5, 10);
      mCamNear = 1.0f;
      mCamFar = 400.0f;
      mCamFovy = 56.0f;
      mLightPos = Vector4(20, 20, 20, 1);
      mScene = new Scene();
      mLight = mScene->createLight("MainLight", LT_POINT, Vector3(50,50,50));
      Node *n = mScene->createNode("TeapotTrans");
      n->setPosition(Vector3(0,5,0));
      n->setOrientation(Vector3(1,1,1).normalize(), 45.0f);
      mTeapot = mScene->createNode("Teapot", n);
      mTeapot->setPosition(Vector3(0,5,0));
      mTeapot->setOrientation(Vector3::UNIT_Y, -45.0f);
      mTeapot->setScale(Vector3(1,1,1));
      mTeapot->setTransformAsRest();
      mTeapotMaterial = new Material("TeapotMaterial");
      mTeapotMaterial->setDiffuse(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
      mXMaterial = new Material("XMaterial");
      mXMaterial->setDiffuse(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
      mXMaterial->setShininess(120.0f);
      mYMaterial = new Material("YMaterial");
      mYMaterial->setDiffuse(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
      mYMaterial->setShininess(120.0f);
      mZMaterial = new Material("ZMaterial");
      mZMaterial->setDiffuse(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
      mZMaterial->setShininess(120.0f);
      mCMaterial = new Material("CMaterial");
      mCMaterial->setDiffuse(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
      mCMaterial->setShininess(120.0f);
      mCylinder = gluNewQuadric();
      //mMesh = MeshManager::Instance().create("./share/models/bunny-1500.obj", BufferObject::BOU_STATIC, "aMesh");
      //if (mMesh) {
      //  mMesh->setMaterial(mTeapotMaterial, false);
      //}
      // drawing boxes
      // drawing axes
      gettimeofday(&mInitTime, NULL);
      mLastTime = mCurTime = getTime();
      mAnimate = false;
      updateCameraView();
      mCamManip = CAMERA_NOOP;
    }
    
    void onCleanup() {
      gluDeleteQuadric(mCylinder);
      delete mTeapotMaterial;
      delete mXMaterial;
      delete mYMaterial;
      delete mZMaterial;
      delete mCMaterial;
      delete mScene;
      delete mRenderer;
    }
    
    void onIdle() {
      mLastTime = mCurTime;
      mCurTime = getTime();
      if (mAnimate) {
        Quat q;
        Convert::ToQuat(Vector3::UNIT_Y, 1.0f, q);
        float omega = 1.0f;
        float phi = 0.0f;
        float mag = 2.0f;
        Vector3 s, t;
        s.x = s.y = 1.0f;
        s.z = 1.0f + 0.5f * Sin(2.0*omega*mCurTime - phi);
        t.x = mag * Cos(omega*mCurTime + phi);
        t.y = mag * Sin(omega*mCurTime + phi);
        mTeapot->setPosition(mTeapot->getRestPosition() + t);
        mTeapot->rotate(q);
        mTeapot->setScale(s);
        //mTeapot->cumulateTransform(t, q, Vector3::UNIT_SCALE, 1.0f);
        glutPostRedisplay();
      }
    }
    
    void updateCameraView() {
      mCamMV  = Matrix4::MakeTranslate(Vector3(0,0,-mCamDist));
      mCamMV *= Matrix4::MakeRotate(-mCamRot.z, Vector3::UNIT_Z);
      mCamMV *= Matrix4::MakeRotate(-mCamRot.x, Vector3::UNIT_X);
      mCamMV *= Matrix4::MakeRotate(-mCamRot.y, Vector3::UNIT_Y);
      mCamMV *= Matrix4::MakeTranslate(-mCamAim);
      mCamIMV = mCamMV.getFastInverse();
      glutPostRedisplay();
    }
    
    void onDisplay() {
      // mRenderer->getMainRenderSurface()->clear(true, true, true);
      // mRenderer->getMainRenderSurface()->getViewport(0)->render(mRenderer);
      glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
      // Setup base projection and modelview
      glViewport(0, 0, mWidth, mHeight);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluPerspective(mCamFovy, mAspect, mCamNear, mCamFar);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
      // Setup camera
      //glTranslatef(0.0f, 0.0f, -mCamDist);
      //glRotatef(-mCamRot.z, 0.0f, 0.0f, 1.0f);
      //glRotatef(-mCamRot.x, 1.0f, 0.0f, 0.0f);
      //glRotatef(-mCamRot.y, 0.0f, 1.0f, 0.0f);
      //glTranslatef(-mCamAim.x, -mCamAim.y, -mCamAim.z);
      glMultMatrixf(mCamMV);
      // Setup light
      mRenderer->setupLight(0, mLight);
      // Draw axis
      mRenderer->setStencilFunc(CF_ALWAYS);
      mRenderer->setStencilRef(1);
      mRenderer->setStencilMask(1);
      mRenderer->setStencilFail(SO_KEEP);
      mRenderer->setStencilDepthFail(SO_KEEP);
      mRenderer->setStencilDepthPass(SO_REPLACE);
      mRenderer->setNormalizeEnabled(true);
      mRenderer->setFaceCullingEnabled(false);
      Scene::NodeIt it = mScene->getNodeIterator();
      while (it.isValid()) {
        glPushMatrix();
        Matrix4 mtx;
        Convert::ToMatrix4(it.getValue()->getWorldOrientation(), mtx);
        mtx.setColumn(3, Vector4(it.getValue()->getWorldPosition(), 1.0f));
        glMultMatrixf(mtx);
          mRenderer->setupMaterial(PF_FRONT_AND_BACK, mCMaterial);
          // mRenderer->pushAndScale();
          glPushMatrix();
            glScalef(0.2f, 0.2f, 0.2f);
            glutSolidSphere(1.0, 16, 16);
          glPopMatrix();
          mRenderer->setupMaterial(PF_FRONT_AND_BACK, mXMaterial);
          glPushMatrix();
            glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
            // base cylinder
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              gluCylinder(mCylinder, 0.5, 0.5, 0.7, 16, 1);
            glPopMatrix();
            // top cone
            glTranslatef(0.0f, 0.0f, 0.7f);
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              glutSolidCone(1.0, 0.3, 16, 1);
            glPopMatrix();
          glPopMatrix();
          mRenderer->setupMaterial(PF_FRONT_AND_BACK, mYMaterial);
          glPushMatrix();
            glRotatef(90.0f, -1.0f, 0.0f, 0.0f);
            // base cylinder
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              gluCylinder(mCylinder, 0.5, 0.5, 0.7, 16, 1);
            glPopMatrix();
            // top cone
            glTranslatef(0.0f, 0.0f, 0.7f);
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              glutSolidCone(1.0, 0.3, 16, 1);
            glPopMatrix();
          glPopMatrix();
          mRenderer->setupMaterial(PF_FRONT_AND_BACK, mZMaterial);
          glPushMatrix();
            // base cylinder
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              gluCylinder(mCylinder, 0.5, 0.5, 0.7, 16, 1);
            glPopMatrix();
            // top cone
            glTranslatef(0.0f, 0.0f, 0.7f);
            glPushMatrix();
              glScalef(0.2f, 0.2f, 1.0f);
              glutSolidCone(1.0, 0.3, 16, 1);
            glPopMatrix();
          glPopMatrix();
        glPopMatrix();
        it.next();
      }
      mRenderer->setFaceCullingEnabled(true);
      mRenderer->setNormalizeEnabled(false);
      // Draw scene
      // Discard every pixels where stencil is not zero
      mRenderer->setStencilFunc(CF_EQUAL);
      mRenderer->setStencilRef(0);
      mRenderer->setStencilDepthPass(SO_KEEP);
      glPushMatrix();
        const Vector3 &lp = mLight->getWorldPosition();
        glTranslatef(lp.x, lp.y, lp.z);
        glDisable(GL_LIGHTING);
        glColor3f(1,1,0);
        glutSolidSphere(2.0, 16, 16);
        glEnable(GL_LIGHTING);
      glPopMatrix();
      glPushMatrix();
        glMultMatrixf(mTeapot->getWorldMatrix());
        mRenderer->setNormalizeEnabled(true);
        if (!mMesh) {
          mRenderer->setupMaterial(PF_FRONT, mTeapotMaterial);
          mRenderer->setFrontFace(FF_CW);
          glutSolidTeapot(2.5);
          mRenderer->setFrontFace(FF_CCW);
        } else {
          for (size_t i=0; i<mMesh->getNumSubMeshes(); ++i) {
            if (mMesh->getSubMesh(i)->getMaterial() != NULL) {
              mRenderer->setupMaterial(PF_FRONT, mMesh->getSubMesh(i)->getMaterial());
            } else {
              mRenderer->setupMaterial(PF_FRONT, mTeapotMaterial);
            }
            mMesh->getSubMesh(i)->render();
          }
        }
        mRenderer->setNormalizeEnabled(false);
      glPopMatrix();
      // Done
      glutSwapBuffers();
    }
    
    void onReshape(int w, int h) {
      //
      mWidth = w;
      mHeight = h;
      mAspect = float(mWidth) / mHeight;
    }
    
    void onKeyboard(unsigned char key, int, int) {
      switch (key) {
        case 27:
          exit(0);
        case 'a':
          mAnimate = !mAnimate;
          glutPostRedisplay();
          break;
        default:
          break;
      }
      mKey = key;
    }
    
    void onMouseClick(int button, int state, int x, int y) {
      //
      mLastX = mCurX = x;
      mLastY = mCurY = y;
      mModifier = glutGetModifiers();
      mButton = button;
      mState = state;
      if (mState == GLUT_DOWN) {
        if (mModifier & GLUT_ACTIVE_ALT) {
          if (mButton == GLUT_LEFT_BUTTON) {
            mCamManip = CAMERA_ROTATE;
          } else if (mButton == GLUT_MIDDLE_BUTTON) {
            mCamManip = CAMERA_TRANSLATE;
          } else if (mButton == GLUT_RIGHT_BUTTON) {
            mCamManip = CAMERA_ZOOM;
          } else {
            mCamManip = CAMERA_NOOP;
          }
        }
      } else {
        mCamManip = CAMERA_NOOP;
      }
    }
    
    void onMouseMove(int x, int y) {
      // 
      mLastX = mCurX;
      mLastY = mCurY;
      mCurX = x;
      mCurY = y;
    }
    
    void onMouseDrag(int x, int y) {
      // 
      mLastX = mCurX;
      mLastY = mCurY;
      mCurX = x;
      mCurY = y;
      
      float dx = mCurX - mLastX;
      float dy = mLastY - mCurY;
      
      if (mCamManip == CAMERA_ROTATE) {
        mCamRot.x += 0.25f * dy;
        mCamRot.y -= 0.25f * dx;
        updateCameraView();
      
      } else if (mCamManip == CAMERA_TRANSLATE) {
        float ny = mCamDist * Tand(0.5f * mCamFovy);
        float piy = ny / (0.5f * mHeight);
        float pix = piy * mAspect;
        dx *= -pix;
        dy *= -piy;
        Vector3 x(mCamIMV.getColumn(0));
        Vector3 y(mCamIMV.getColumn(1));
        x *= dx;
        y *= dy;
        mCamAim += (x + y);
        updateCameraView();
        
      } else if (mCamManip == CAMERA_ZOOM) {
        float dolly = (Abs(dx) > Abs(dy) ? dx : dy);
        if (mCamDist > 0.000001) {
          dolly *= (-1 * mCamDist) / 500;
        } else {
          dolly *= -0.001f;
        }
        mCamDist += dolly;
        if (mCamDist < 0) {
          mCamDist = 0;
        }
        updateCameraView();
        
      }
    }

};

// --- GLUT

static void Init() {
  GLUTState::Get().onInitialize();
}

static void Cleanup() {
  GLUTState::Get().onCleanup();
}

static void GLUT_Display() {
  GLUTState::Get().onDisplay();
}

static void GLUT_Reshape(int w, int h) {
  GLUTState::Get().onReshape(w, h);
}

static void GLUT_MouseClick(int btn, int state, int x, int y) {
  GLUTState::Get().onMouseClick(btn, state, x, y);
}

static void GLUT_MouseMove(int x, int y) {
  GLUTState::Get().onMouseMove(x, y);
}

static void GLUT_MouseDrag(int x, int y) {
  GLUTState::Get().onMouseDrag(x, y);
}

static void GLUT_Keyboard(unsigned char key, int x, int y) {
  GLUTState::Get().onKeyboard(key, x, y);
}

static void GLUT_Idle() {
  GLUTState::Get().onIdle();
}

int main(int argc, char **argv) {
  atexit(Cleanup);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_STENCIL);
  glutInitWindowPosition(50, 50);
  glutInitWindowSize(512, 512);
  glutCreateWindow("Test3D");
  Init();
  glutDisplayFunc(GLUT_Display);
  glutReshapeFunc(GLUT_Reshape);
  glutKeyboardFunc(GLUT_Keyboard);
  glutMouseFunc(GLUT_MouseClick);
  glutMotionFunc(GLUT_MouseDrag);
  glutPassiveMotionFunc(GLUT_MouseMove);
  glutIdleFunc(GLUT_Idle);
  glutMainLoop();
  return 0;
}
