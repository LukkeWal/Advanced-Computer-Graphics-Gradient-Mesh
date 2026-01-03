#include "mainview.h"

MainView::MainView(QWidget *parent): QOpenGLWidget(parent){}

MainView::~MainView()
{
  makeCurrent();
  meshRenderer.cleanup();
  doneCurrent();
}

void MainView::initializeGL()
{
  meshRenderer.initializeShader();
  meshRenderer.initializeGL();
  m_mesh = Mesh();
  m_mesh.setPaperTestMesh();
  meshRenderer.setMesh(m_mesh);
}

void MainView::meshChanged(MeshPreset selectedMesh)
{
  if (selectedMesh ==  MeshPreset::BASIC){
    m_mesh.setBasicTestMesh();
    meshRenderer.setMesh(m_mesh);
  }
  if (selectedMesh == MeshPreset::PAPER){
    m_mesh.setPaperTestMesh();
    meshRenderer.setMesh(m_mesh);
  }
  if (selectedMesh == MeshPreset::APPLE){
    m_mesh.setAppleMesh();
    meshRenderer.setMesh(m_mesh);
  }
  update();
}

void MainView::resolutionChanged(int resolution){
  meshRenderer.setResolution(resolution);
  update();
}

void MainView::curveControlChanged(bool curveControl){
  meshRenderer.setCurveControl(curveControl);
  update();
}

void MainView::showGradientChanged(bool showGradient){
  meshRenderer.setcontrolNetOnly(!showGradient);
  update();
}

void MainView::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
}

void MainView::paintGL()
{
  meshRenderer.draw();
}

void MainView::mousePressEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton)
    return;

  QPointF p = event->position(); // Qt6; use event->pos() for Qt5
  float mouseX = static_cast<float>(p.x());
  float mouseY = static_cast<float>(p.y());

  int bestRow = -1;
  int bestCol = -1;
  float bestDist2 = m_pickRadiusPx * m_pickRadiusPx;

  for (int r = 0; r < m_mesh.rows(); ++r)
  {
    for (int c = 0; c < m_mesh.cols(); ++c)
    {
      const ControlPoint& cp = m_mesh.points[r][c];

      float sx = static_cast<float>(cp.pos.x) * width();
      float sy = (1.0f - static_cast<float>(cp.pos.y)) * height();

      float dx = mouseX - sx;
      float dy = mouseY - sy;
      float d2 = dx*dx + dy*dy;

      if (d2 < bestDist2)
      {
        bestDist2 = d2;
        bestRow = r;
        bestCol = c;
      }
    }
  }

  m_selectedRow = bestRow;
  m_selectedCol = bestCol;
}

void MainView::mouseMoveEvent(QMouseEvent *event)
{
  if (!(event->buttons() & Qt::LeftButton))
    return;

  if (m_selectedRow < 0 || m_selectedCol < 0)
    return;

  QPointF p = event->position();
  double x = p.x();
  double y = p.y();

  // screen -> world
  double wx = x / static_cast<double>(width());
  double wy = 1.0 - y / static_cast<double>(height());

  // clamp a bit so we stay in visible region
  if (wx < 0.0) wx = 0.0;
  if (wx > 1.0) wx = 1.0;
  if (wy < 0.0) wy = 0.0;
  if (wy > 1.0) wy = 1.0;

  ControlPoint& cp = m_mesh.points[m_selectedRow][m_selectedCol];
  cp.pos.x = wx;
  cp.pos.y = wy;

  // Rebuild geometry
  m_mesh.updateDerivatives();
  meshRenderer.setMesh(m_mesh);
  update();
}

void MainView::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton)
    return;

  m_selectedRow = -1;
  m_selectedCol = -1;
}

