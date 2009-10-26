#include <ggfx/config.h>
#include <ggfx/system.h>
#include <ggfx/renderer.h>
#ifndef __APPLE__
# ifdef WIN32
#   pragma warning(disable: 4505)
# endif
# include <GL/glut.h>
#else
# include <GLUT/glut.h>
#endif
using namespace std;
using namespace gmath;
using namespace ggfx;

struct OutParams {
  float top;
  float bottom;
  float n;
  Vector2 l;
  Vector2 q;
  Vector2 left;
  Vector2 right;
  Vector2 t0;
  Vector2 t1;
  Vector2 t2;
  Vector2 t3;
  Matrix4 Nt;
  Matrix4 Nt_step[8]; // include Nt
};

// nMid: center of near plane
// fMid: center of far plane
// fFoc: center of focus far plane
// Pts: points of the camera frustum in perspective space
//
// NOTE: if camera frustum is clipped
// nMid, fMid etc have no meaning
// so as fFoc
// in the same way the point list is the clipped body point list
// 
// More general approach:
// 
// compute as before basic modelview and proj light matrix
// camFrustum.clip(lightFrustum) --> BodyPoints --> to LS
// compute center of B, C
// for nMid and fMid use eyePos and C
// focus region ?
//   push camera near and far as to touch B --> distance from near to far = f' (f the original far - near)
//   delta = f'/f * delta
//   [thus new delta' in LS (to compute)]
void ComputeTSM(const Vector2 &nMid, const Vector2 &fMid, const Vector2 &fFoc, const ConvexHull2D::PointArray &pts, OutParams *out=NULL) {
  /* TO DO BEFORE
  
  // first suppose cam frustum inside light one
  // ... compute mLightFrustum (thus we have mLightProj & mLightView)
  // ... compute mCamFrustum
  Matrix4 LS = mLightProj * mLightView;
  // Compute line [width two points] goind from near plane center to
  // far plane center in PPS
  Vector2 fMid(LS * (mCamPos + mCamFar  * mCamDir));
  Vector2 nMid(LS * (mCamPos + mCamNear * mCamDir));
  // Compute convex hull 2d in PPS of the camera frustum
  size_t i, j;
  PointList C = LS * PointList(ConvexBody(mCamFrustum));
  ConvexHull2D::PointArray pts;
  for (i=0; i<C.size(); ++i) {
    pts.push_back(Vector2(C[i].x, C[i].y));
  }
  
  */
  size_t i, j;
  ConvexHull2D hull(pts);
  cout << "Hull: " << hull << endl;
  // Find top line and base line
  Vector2 l = (fMid - nMid).normalize();
  float top, bottom;
  top = bottom = 0;
  if (fMid == nMid) {
    // far plane BB
    cout << "Doh !!!" << endl;
    // TODO !!!
    // compute AAB, and transformToUnit Cube... over !
    return;
  } else {
    float cur;
    top = (hull[0] - nMid).dot(l);
    bottom = top;
    for (j=1; j<hull.size(); ++j) {
      cur = (hull[j] - nMid).dot(l);
      if (cur < top) {
        top = cur;
      } else if (cur > bottom) {
        bottom = cur;
      }
    }
  }
  cout << "top: " << top << ", bottom: " << bottom << endl;
  // Find center of projection
  float lambda = fabs(top - bottom); // lambda
  cout << "lambda: " << lambda << endl;
  float delta = (fFoc - nMid).dot(l) - top; // delta' (not delta)
  //float delta = 0.8f * lambda; // delta'
  cout << "delta: " << delta << endl;
  float epsilon = -0.6f; // == (1 - (0.8 * 2)) (as top -> 1, bottom -> -1)
  cout << "epsilon: " << epsilon << endl;
  float n = top + ((lambda * delta * (1 + epsilon)) / (lambda * (1 - epsilon) - 2*delta));
  Vector2 q = nMid + (top - n) * l;
  cout << "n: " << n << " -> q: " << q << endl;
  // Find left most and right most convex hull point that maximize the angle
  Vector2 rightMost = fMid;
  Vector2 leftMost = fMid;
  for (i=0; i<hull.size(); ++i) {
    float tmp = ConvexHull2D::IsLeft(q, fMid, hull[i]);
    if (fabs(tmp) < 0.000001) {
      // same line
      continue;
    } else if (tmp > 0) {
      // on left side
      if (ConvexHull2D::IsLeft(q, leftMost, hull[i]) >= 0) {
        leftMost = hull[i];
      }
    } else if (tmp < 0) {
      // on right side
      if (ConvexHull2D::IsLeft(q, rightMost, hull[i]) <= 0) {
        rightMost = hull[i];
      }
    }
  }
  cout << "RM: " << rightMost << ", LM: " << leftMost << endl;
  // Compute the 4 vertices of the trapezoid
  Vector2 t0, t1, t2, t3; // t2, t3 (near), t0, t1 (far)
  Vector2 e;
  float dp;
  e = (rightMost - q).normalize();
  dp = e.dot(l);
  if (fabs(dp) < 0.000001) {
    t3 = q + n*l;
    t0 = q + (n+lambda)*l;
  } else {
    dp = 1.0f / dp;
    t3 = q + (n * dp)*e;
    t0 = q + ((lambda + n) * dp)*e;
  }
  e = (leftMost - q).normalize();
  dp = e.dot(l);
  if (fabs(dp) < 0.000001) {
    t2 = q + n*l;
    t1 = q + (n+lambda)*l;
  } else {
    dp = 1.0f / dp;
    t2 = q + (n * dp)*e;
    t1 = q + ((lambda + n) * dp)*e;
  }
  cout << "t0 = " << t0 << endl;
  cout << "t1 = " << t1 << endl;
  cout << "t2 = " << t2 << endl;
  cout << "t3 = " << t3 << endl;
  // Compute transform to map trapezoid to unit square
  Vector4 t[4] = {
    Vector4(t0.x, t0.y, 1, 1),
    Vector4(t1.x, t1.y, 1, 1),
    Vector4(t2.x, t2.y, 1, 1),
    Vector4(t3.x, t3.y, 1, 1)
  };
  Vector4 o(q.x, q.y, 1, 1);
  Vector4 u, v;
  Matrix4 Nt, T1, R, T2, H, S1, N, T3, S2;
  u = 0.5f * (t[2] + t[3]);
  T1 = Matrix4::MakeTranslate(Vector3(-u.x, -u.y, 0));
  u = (t[2] - t[3]);
  u /= float(sqrt(u.x*u.x + u.y*u.y));
  if (out) out->Nt_step[0] = T1;
  R = Matrix4::IDENTITY;
  R(0,0) = u.x;
  R(0,1) = u.y;
  R(1,0) = u.y;
  R(1,1) = -u.x;
  Nt = R * T1;
  if (out) out->Nt_step[1] = Nt;
  u = Nt * o;
  T2 = Matrix4::MakeTranslate(Vector3(-u.x, -u.y, 0));
  Nt = T2 * Nt;
  if (out) out->Nt_step[2] = Nt;
  u = 0.5f * (Nt * (t[2] + t[3]));
  H = Matrix4::IDENTITY;
  H(0,1) = - u.x / u.y;
  Nt = H * Nt;
  if (out) out->Nt_step[3] = Nt;
  u = Nt * t[2];
  S1 = Matrix4::MakeScale(Vector3(1.0f/u.x, 1.0f/u.y, 1));
  if (out) out->Nt_step[4] = S1 * Nt;
  N = Matrix4(1,0,0,0, 0,1,0,1, 0,0,1,0, 0,1,0,0);
  Nt = N * S1 * Nt;
  if (out) out->Nt_step[5] = Nt;
  u = Nt * t[0];
  v = Nt * t[2];
  T3(1,3) = -0.5f * (u.y/u.w + v.y/v.w);
  Nt = T3 * Nt;
  if (out) out->Nt_step[6] = Nt;
  u = Nt * t[0];
  S2 = Matrix4::MakeScale(Vector3(1, -u.w/u.y, 1));
  Nt = S2 * Nt;
  if (out) out->Nt_step[7] = Nt;
  
  
  if (out) {
    out->bottom = bottom;
    out->top = top;
    out->n = n;
    out->q = q;
    out->l = l;
    out->t0 = t0;
    out->t1 = t1;
    out->t2 = t2;
    out->t3 = t3;
    out->left = leftMost;
    out->right = rightMost;
    out->Nt = Nt;
  }
}

// ---

ConvexHull2D::PointArray pts;
ConvexHull2D hull;
Vector2 nearP[4];
Vector2 farP[4];
Vector2 farMid, nearMid, farFoc;
OutParams out;
int tstep = -1;
float delta = 0.2; // no real meaning here, this is tweaking

static void drawFrustum() {
  glColor3f(1,0,0);
  glBegin(GL_LINE_LOOP);
    glVertex2fv(nearP[0]);
    glVertex2fv(nearP[1]);
    glVertex2fv(nearP[2]);
    glVertex2fv(nearP[3]);
  glEnd();
  glBegin(GL_LINE_LOOP);
    glVertex2fv(farP[0]);
    glVertex2fv(farP[1]);
    glVertex2fv(farP[2]);
    glVertex2fv(farP[3]);
  glEnd();
  glBegin(GL_LINES);
    for (int i=0; i<4; ++i) {
      glVertex2fv(nearP[i]);
      glVertex2fv(farP[i]);
    }
  glEnd();
}

static void drawHull() {
  glColor3f(0,0,1);
  glBegin(GL_LINE_LOOP);
    for (size_t i=0; i<hull.size(); ++i) {
      glVertex2fv(hull[i]);
    }
  glEnd();
}

static void drawLine(const Vector2 &v0, const Vector2 &v1, bool verbose = false) {
  Vector2 e = (v1 - v0).normalize();
  Vector2 i0, i1;
  // check intersection with left edge
  if (fabs(e.dot(Vector2::UNIT_X)) < 0.000001) {
    // vector is // to X axis
    float t0 = (-(-1) - v0.dot( Vector2::UNIT_Y)) / (e.dot( Vector2::UNIT_Y));
    float t1 = (-( 1) - v0.dot(-Vector2::UNIT_Y)) / (e.dot(-Vector2::UNIT_Y));
    i0 = v0 + t0*e;
    i1 = v0 + t1*e;
  } else {
    if (fabs(e.dot(Vector2::UNIT_Y)) < 0.000001) {
      // vector is // to Y axis
      float t0 = (1 - v0.dot( Vector2::UNIT_X)) / (e.dot( Vector2::UNIT_X));
      float t1 = (1 - v0.dot(-Vector2::UNIT_X)) / (e.dot(-Vector2::UNIT_X));
      i0 = v0 + t0*e;
      i1 = v0 + t1*e;
    } else {
      // general case, test all 4 unit square edges
      bool done0 = false;
      bool done1 = false;
      float t;
      Vector2 tmp;
      t = (1 - v0.dot( Vector2::UNIT_X)) / (e.dot( Vector2::UNIT_X));
      tmp = v0 + t*e;
      if (tmp.y >= -1 && tmp.y <= 1) {
        if (verbose) {
          cout << "Intersect right edge at: " << tmp << endl;
        }
        done0 = true;
        i0 = tmp;
      }
      t = (1 - v0.dot(-Vector2::UNIT_X)) / (e.dot(-Vector2::UNIT_X));
      tmp = v0 + t*e;
      if (tmp.y >= -1 && tmp.y <= 1) {
        if (verbose) {
          cout << "Intersect left edge at: " << tmp << endl;
        }
        if (done0) {
          done1 = true;
          i1 = tmp;
        } else {
          done0 = true;
          i0 = tmp;
        }
      }
      if (!done1) {
        t = (1 - v0.dot( Vector2::UNIT_Y)) / (e.dot( Vector2::UNIT_Y));
        tmp = v0 + t*e;
        if (tmp.x >= -1 && tmp.x <= 1) {
          if (verbose) {
            cout << "Intersect top edge at: " << tmp << " (t=" << t << ")" << endl;
          }
          if (done0) {
            done1 = true;
            i1 = tmp;
          } else {
            done0 = true;
            i0 = tmp;
          }
        }
        if (!done1) {
          t = (1 - v0.dot(-Vector2::UNIT_Y)) / (e.dot(-Vector2::UNIT_Y));
          i1 = v0 + t*e;
          if (verbose) {
            cout << "Intersect bottom edge at: " << i1 << " (t=" << t << ")" << endl;
          }
        }
      }
    }
  }
  if (verbose) {
    cout << "Line from " << i0 << " to " << i1 << endl;
  }
  glBegin(GL_LINES);
    glVertex2fv(i0);
    glVertex2fv(i1);
  glEnd();
}

static void drawPerpendicular(const Vector2 &org, const Vector2 &n, float d) {
  Vector2 pt0 = org + d*n;
  Vector2 pt1 = pt0 + Vector2(-n.y, n.x);
  drawLine(pt0, pt1);
}

static void drawPoints() {
  glColor3f(0,0,0);
  glBegin(GL_POINTS);
    glVertex2fv(farMid);
    glVertex2fv(nearMid);
    glVertex2fv(farFoc);
  glEnd();
  drawLine(nearMid, farMid);
}

static void drawOutput() {
  glColor3f(0,0,0);
  drawPerpendicular(nearMid, out.l, out.top);
  drawPerpendicular(nearMid, out.l, out.bottom);
  drawPerpendicular(nearMid, out.l, (out.top - out.n));
  drawLine(out.q, out.left);
  drawLine(out.q, out.right);
  glColor3f(0,0.7,0);
  glPushMatrix();
    //glMultMatrixf(out.Nt);
    if (tstep >= 0 && tstep < 8) {
      glMultMatrixf(out.Nt_step[tstep]);
    }
    glBegin(GL_LINE_LOOP);
      glVertex2fv(out.t0);
      glVertex2fv(out.t1);
      glVertex2fv(out.t2);
      glVertex2fv(out.t3);
    glEnd();
  glPopMatrix();
}

static void drawUnitSquare() {
  glColor3f(0,0,0);
  glBegin(GL_LINE_LOOP);
    glVertex2f(-1, -1);
    glVertex2f( 1, -1);
    glVertex2f( 1,  1);
    glVertex2f(-1,  1);
  glEnd();
}

static void display() {
  glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
    glTranslatef(0, 0, -1);
    drawUnitSquare();
    drawFrustum();
    drawHull();
    drawPoints();
    drawOutput();
  glPopMatrix();
  glutSwapBuffers();
}

static void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float n = 3.0f;
  glOrtho(-n, n, -n, n, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

static void keyboard(unsigned char key, int, int) {
  if (key == 27) {
    exit(0);
  } else if (key == '+') {
    if (++tstep > 7) {
      tstep = 7;
    }
    glutPostRedisplay();
  } else if (key == '-') {
    if (--tstep < -1) {
      tstep = -1;
    }
    glutPostRedisplay();
  }
}

int main(int argc, char **argv) {
  
  ConvexHull2D::PointArray pts;
  
  nearP[0] = Vector2(-0.1f, -0.15f);
  nearP[1] = Vector2( 0.0f, -0.2f);
  nearP[2] = Vector2( 0.0f, -0.1f);
  nearP[3] = Vector2(-0.07f, -0.08f);
  
  farP[0] = Vector2(-0.15f, 0.22f);
  farP[1] = Vector2( 0.3f, 0.1f);
  farP[2] = Vector2( 0.2f, 0.3f);
  farP[3] = Vector2(-0.1f, 0.4f);
  
  nearMid = 0.25f * (nearP[0]+nearP[1]+nearP[2]+nearP[3]);
  farMid = 0.25f * (farP[0]+farP[1]+farP[2]+farP[3]);
  //farFoc = Vector2(0.1, 0.1);
  farFoc = nearMid + (delta * (farMid-nearMid).normalize());
  cout << "Far focus: " << farFoc << endl;
  
  for (int i=0; i<4; ++i) {
    pts.push_back(farP[i]);
    pts.push_back(nearP[i]);
  }
  
  hull.compute(pts);
  
  ComputeTSM(nearMid, farMid, farFoc, pts, &out);
  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowPosition(50,50);
  glutInitWindowSize(512, 512);
  glutCreateWindow("Plot");
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glClearColor(1,1,1,1);
  glClearDepth(1);
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMainLoop();
  return 0;
}
