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
  if (selectedMesh == MeshPreset::SHARP){
    m_mesh.setSharpMesh();
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
    if (event->button() != Qt::LeftButton) return;

    QPointF p = event->position();
    float mouseX = p.x();
    float mouseY = p.y();

    m_selectedRow = -1;
    m_selectedCol = -1;
    m_selectionType = SelectionType::NONE;

    float bestDist2 = m_pickRadiusPx * m_pickRadiusPx;
    const float handleScale = 1.0f / 5.0f; // Must match renderer.cpp

    for (int r = 0; r < m_mesh.rows(); ++r) {
        for (int c = 0; c < m_mesh.cols(); ++c) {
            const ControlPoint& cp = m_mesh.points[r][c];

            auto toScreen = [&](Vec2 worldPos) {
                return QPointF(worldPos.x * width(), (1.0 - worldPos.y) * height());
            };

            // 1. Check Position
            QPointF sPos = toScreen(cp.pos);
            float d2 = QPointF::dotProduct(sPos - p, sPos - p);
            if (d2 < bestDist2) {
                bestDist2 = d2; m_selectedRow = r; m_selectedCol = c;
                m_selectionType = SelectionType::POSITION;
            }

            // 2. Check U Tangent Handle
            QPointF sUTan = toScreen(cp.pos + cp.uPosTangent * handleScale);
            d2 = QPointF::dotProduct(sUTan - p, sUTan - p);
            if (d2 < bestDist2) {
                bestDist2 = d2; m_selectedRow = r; m_selectedCol = c;
                m_selectionType = SelectionType::U_TANGENT;
            }

            // 3. Check V Tangent Handle
            QPointF sVTan = toScreen(cp.pos + cp.vPosTangent * handleScale);
            d2 = QPointF::dotProduct(sVTan - p, sVTan - p);
            if (d2 < bestDist2) {
                bestDist2 = d2; m_selectedRow = r; m_selectedCol = c;
                m_selectionType = SelectionType::V_TANGENT;
            }
        }
    }
}

void MainView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) || m_selectedRow < 0) return;

    double wx = event->position().x() / width();
    double wy = 1.0 - (event->position().y() / height());

    ControlPoint& cp = m_mesh.points[m_selectedRow][m_selectedCol];
    const float handleScale = 1.0f / 5.0f;

    if (m_selectionType == SelectionType::POSITION) {
        cp.pos = Vec2(wx, wy);
    }
    else if (m_selectionType == SelectionType::U_TANGENT) {
        // Calculate new tangent based on vector from point to mouse
        cp.uPosTangent = (Vec2(wx, wy) - cp.pos) * (1.0 / handleScale);
    }
    else if (m_selectionType == SelectionType::V_TANGENT) {
        cp.vPosTangent = (Vec2(wx, wy) - cp.pos) * (1.0 / handleScale);
    }

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

