#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>

#include "model/mesh.h"

class renderer: protected QOpenGLFunctions
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
    float r, g, b;
  };
  int resolution = 40;
  bool curveControl = false;
  bool controlNetOnly = false;
  void buildControlNetMesh();
  void buildPatchMesh();

  QOpenGLShaderProgram shader;

  QOpenGLVertexArrayObject m_vaoPatch;
  QOpenGLBuffer            m_vboPatch;
  int m_vertexCountPatch = 0;

  // Control net (lines + points)
  QOpenGLVertexArrayObject m_vaoNet;
  QOpenGLBuffer            m_vboNet;
  int m_netLineVertexCount  = 0; // GL_LINES
  int m_netPointVertexCount = 0; // GL_POINTS

  Mesh mesh;
};
