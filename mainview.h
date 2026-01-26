#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>

#include "renderer.h"
#include "MeshPresets.h"

class MainView : public QOpenGLWidget
{
  Q_OBJECT
public:
  explicit MainView(QWidget *parent = nullptr);
  ~MainView() override;
  void meshChanged(MeshPreset meshPreset);
  void resolutionChanged(int resolution);
  void curveControlChanged(bool curveControl);
  void showGradientChanged(bool showGradient);

protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;

private:
  renderer meshRenderer;
  Mesh m_mesh;
  enum class SelectionType { NONE, POSITION, U_TANGENT, V_TANGENT };
  SelectionType m_selectionType = SelectionType::NONE;

  // Selection
  int   m_selectedRow = -1;
  int   m_selectedCol = -1;
  float m_pickRadiusPx = 10.0f;
};
