#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>

#include "model/gradientmesh.h"

class MainView : public QOpenGLWidget, protected QOpenGLFunctions
{
  Q_OBJECT
public:
  explicit MainView(QWidget *parent = nullptr);
  ~MainView() override;

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  struct Vertex {
    float x, y;
    float r, g, b;
  };

  void buildPatchMesh();
  void buildControlNetMesh();    // NEW

  QOpenGLShaderProgram m_program;

  // Patches (triangles)
  QOpenGLVertexArrayObject m_vaoPatch;
  QOpenGLBuffer            m_vboPatch;
  int m_vertexCountPatch = 0;

  // Control net (lines + points)
  QOpenGLVertexArrayObject m_vaoNet;
  QOpenGLBuffer            m_vboNet;
  int m_netLineVertexCount  = 0; // GL_LINES
  int m_netPointVertexCount = 0; // GL_POINTS

  GradientMesh m_mesh;

  // Selection
  int   m_selectedRow = -1;
  int   m_selectedCol = -1;
  float m_pickRadiusPx = 10.0f;
};
