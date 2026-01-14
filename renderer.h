#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>

#include "model/mesh.h"

#include <QOpenGLFunctions_4_3_Core>

class renderer : protected QOpenGLFunctions_4_3_Core
{
public:
  renderer();
  void cleanup();
  void initializeGL();
  void initializeShader();
  void setMesh(const Mesh &m);
  void draw();
  void setResolution(int newRes);
  void setCurveControl(bool doCurveControl);
  void setcontrolNetOnly(bool newControlNetOnly);
private:
  struct Vertex {
    float x, y;
    float r0, g0, b0; // 0 0
    float r1, g1, b1; // 0 1
    float r2, g2, b2; // 1 0
    float r3, g3, b3; // 1 1
  };

  struct Vec4 { float x,y,z,w; };

  struct CsGPU {
    Vec4 Cred[16];
    Vec4 Cgreen[16];
    Vec4 Cblue[16];
  };

  int resolution = 40;
  bool curveControl = false;
  bool controlNetOnly = false;
  void buildControlNetMesh();
  void buildPatchMesh();

  QOpenGLShaderProgram shader;
  QOpenGLShaderProgram shaderNet;

  QOpenGLVertexArrayObject m_vaoPatch;
  QOpenGLBuffer            m_vboPatch;
  int m_vertexCountPatch = 0;

  // Control net (lines + points)
  QOpenGLVertexArrayObject m_vaoNet;
  QOpenGLBuffer            m_vboNet;
  int m_netLineVertexCount  = 0; // GL_LINES
  int m_netPointVertexCount = 0; // GL_POINTS

  Mesh mesh;

  GLuint CsHandle = 0;
  int patchCount = 0;
};
