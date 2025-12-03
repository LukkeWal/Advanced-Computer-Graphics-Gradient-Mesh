#include "mainview.h"
#include <vector>

MainView::MainView(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_vboNet(QOpenGLBuffer::VertexBuffer)
{
  // nothing special here yet
}

MainView::~MainView()
{
  makeCurrent();
  m_vboNet.destroy();
  m_vaoNet.destroy();
  doneCurrent();
}

void MainView::initializeGL()
{
  initializeOpenGLFunctions();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // --- Shaders (same as before) ---
  m_program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    R"(#version 330 core
           layout(location = 0) in vec2 inPos;
           layout(location = 1) in vec3 inColor;
           out vec3 vColor;
           void main() {
               vec2 p = inPos * 2.0 - vec2(1.0, 1.0);
               gl_Position = vec4(p, 0.0, 1.0);
               vColor = inColor;
           })"
                                    );

  m_program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    R"(#version 330 core
           in vec3 vColor;
           out vec4 fragColor;
           void main() {
               fragColor = vec4(vColor, 1.0);
           })"
                                    );

  m_program.link();

  // --- Patch VAO/VBO (triangles) ---
  m_vaoPatch.create();
  m_vaoPatch.bind();

  m_vboPatch = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  m_vboPatch.create();
  m_vboPatch.bind();

  m_program.bind();
  m_program.enableAttributeArray(0);
  m_program.enableAttributeArray(1);
  m_program.setAttributeBuffer(
      0, GL_FLOAT,
      offsetof(Vertex, x),
      2, sizeof(Vertex)
      );
  m_program.setAttributeBuffer(
      1, GL_FLOAT,
      offsetof(Vertex, r),
      3, sizeof(Vertex)
      );
  m_program.release();

  m_vboPatch.release();
  m_vaoPatch.release();

  // --- Control net VAO/VBO (lines + points) ---
  m_vaoNet.create();
  m_vaoNet.bind();

  m_vboNet = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  m_vboNet.create();
  m_vboNet.bind();

  m_program.bind();
  m_program.enableAttributeArray(0);
  m_program.enableAttributeArray(1);
  m_program.setAttributeBuffer(
      0, GL_FLOAT,
      offsetof(Vertex, x),
      2, sizeof(Vertex)
      );
  m_program.setAttributeBuffer(
      1, GL_FLOAT,
      offsetof(Vertex, r),
      3, sizeof(Vertex)
      );
  m_program.release();

  m_vboNet.release();
  m_vaoNet.release();

  // --- Create test mesh and build geometry ---
  //m_mesh = GradientMesh::makeRegularTestMesh(3, 3); // 3x3 grid → 2x2 patches
  m_mesh = GradientMesh::makePaperTestMesh();

  buildPatchMesh();
  buildControlNetMesh();
}

void MainView::resizeGL(int w, int h)
{
  glViewport(0, 0, w, h);
}

void MainView::paintGL()
{
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  m_program.bind();

  // --- Draw patches (triangles) ---
  glEnable(GL_DEPTH_TEST);
  m_vaoPatch.bind();
  glDrawArrays(GL_TRIANGLES, 0, m_vertexCountPatch);
  m_vaoPatch.release();

  // --- Draw control net on top ---
  glDisable(GL_DEPTH_TEST);
  m_vaoNet.bind();

  glLineWidth(1.5f);
  glDrawArrays(GL_LINES, 0, m_netLineVertexCount);

  glPointSize(6.0f);
  glDrawArrays(GL_POINTS,
               m_netLineVertexCount,
               m_netPointVertexCount);

  m_vaoNet.release();

  m_program.release();
}

void MainView::buildControlNetMesh()
{
  std::vector<Vertex> vertices;
  vertices.reserve(m_mesh.rows() * m_mesh.cols() * 4); // rough

  auto white = [](float x, float y) {
    Vertex v;
    v.x = x;
    v.y = y;
    v.r = 1.0f;
    v.g = 1.0f;
    v.b = 1.0f;
    return v;
  };

  // --- Lines between neighbouring control points ---
  // Horizontal lines
  for (int r = 0; r < m_mesh.rows(); ++r)
  {
    for (int c = 0; c < m_mesh.cols() - 1; ++c)
    {
      const ControlPoint& a = m_mesh.at(r, c);
      const ControlPoint& b = m_mesh.at(r, c + 1);

      vertices.push_back(white(static_cast<float>(a.pos.x),
                               static_cast<float>(a.pos.y)));
      vertices.push_back(white(static_cast<float>(b.pos.x),
                               static_cast<float>(b.pos.y)));
    }
  }

  // Vertical lines
  for (int c = 0; c < m_mesh.cols(); ++c)
  {
    for (int r = 0; r < m_mesh.rows() - 1; ++r)
    {
      const ControlPoint& a = m_mesh.at(r, c);
      const ControlPoint& b = m_mesh.at(r + 1, c);

      vertices.push_back(white(static_cast<float>(a.pos.x),
                               static_cast<float>(a.pos.y)));
      vertices.push_back(white(static_cast<float>(b.pos.x),
                               static_cast<float>(b.pos.y)));
    }
  }

  m_netLineVertexCount = static_cast<int>(vertices.size());

  // --- Points at each control point ---
  for (int r = 0; r < m_mesh.rows(); ++r)
  {
    for (int c = 0; c < m_mesh.cols(); ++c)
    {
      const ControlPoint& p = m_mesh.at(r, c);
      vertices.push_back(white(static_cast<float>(p.pos.x),
                               static_cast<float>(p.pos.y)));
    }
  }

  m_netPointVertexCount =
      m_mesh.rows() * m_mesh.cols();

  // Upload to GPU
  m_vaoNet.bind();
  m_vboNet.bind();
  m_vboNet.allocate(vertices.data(),
                    static_cast<int>(vertices.size() * sizeof(Vertex)));
  m_vboNet.release();
  m_vaoNet.release();
}

void MainView::buildPatchMesh()
{
  const int resU = 40;
  const int resV = 40;

  std::vector<Vertex> vertices;

  std::vector<GradientPatch> patches = m_mesh.buildPatches();
  vertices.reserve(static_cast<std::size_t>(patches.size() * resU * resV * 6));

  auto addVertex = [&vertices](const ControlPoint& cp)
  {
    Vertex v;
    v.x = static_cast<float>(cp.pos.x);
    v.y = static_cast<float>(cp.pos.y);
    v.r = static_cast<float>(cp.col.r);
    v.g = static_cast<float>(cp.col.g);
    v.b = static_cast<float>(cp.col.b);
    vertices.push_back(v);
  };

  for (const GradientPatch& patch : patches)
  {
    for (int i = 0; i < resU; ++i)
    {
      double u0 = static_cast<double>(i) / resU;
      double u1 = static_cast<double>(i + 1) / resU;

      for (int j = 0; j < resV; ++j)
      {
        double v0 = static_cast<double>(j) / resV;
        double v1 = static_cast<double>(j + 1) / resV;

        ControlPoint c00 = patch.evaluate(u0, v0);
        ControlPoint c10 = patch.evaluate(u1, v0);
        ControlPoint c01 = patch.evaluate(u0, v1);
        ControlPoint c11 = patch.evaluate(u1, v1);

        // tri 1
        addVertex(c00);
        addVertex(c10);
        addVertex(c01);

        // tri 2
        addVertex(c10);
        addVertex(c11);
        addVertex(c01);
      }
    }
  }

  m_vertexCountPatch = static_cast<int>(vertices.size());

  m_vaoPatch.bind();
  m_vboPatch.bind();
  m_vboPatch.allocate(vertices.data(),
                      m_vertexCountPatch * sizeof(Vertex));
  m_vboPatch.release();
  m_vaoPatch.release();
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
      const ControlPoint& cp = m_mesh.at(r, c);

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

  ControlPoint& cp = m_mesh.at(m_selectedRow, m_selectedCol);
  cp.pos.x = wx;
  cp.pos.y = wy;

  m_mesh.updateDerivatives();

  // Rebuild geometry
  buildPatchMesh();
  buildControlNetMesh();

  update();
}

void MainView::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() != Qt::LeftButton)
    return;

  m_selectedRow = -1;
  m_selectedCol = -1;
}

