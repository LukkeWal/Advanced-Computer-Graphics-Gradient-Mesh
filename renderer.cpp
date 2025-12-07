#include "renderer.h"

renderer::renderer(): m_vboNet(QOpenGLBuffer::VertexBuffer) {}

void renderer::cleanup()
{
  m_vboPatch.destroy();
  m_vaoPatch.destroy();
  m_vboNet.destroy();
  m_vaoNet.destroy();
}

void renderer::initializeShader(){
  shader.addShaderFromSourceCode(QOpenGLShader::Vertex,
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

  shader.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                 R"(#version 330 core
           in vec3 vColor;
           out vec4 fragColor;
           void main() {
               fragColor = vec4(vColor, 1.0);
           })"
                                 );

  shader.link();
}

void renderer::initializeGL()
{
  initializeOpenGLFunctions();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // --- Patch VAO/VBO (triangles) ---
  m_vaoPatch.create();
  m_vaoPatch.bind();

  m_vboPatch = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  m_vboPatch.create();
  m_vboPatch.bind();

  shader.bind();
  shader.enableAttributeArray(0);
  shader.enableAttributeArray(1);
  shader.setAttributeBuffer(
      0, GL_FLOAT,
      offsetof(Vertex, x),
      2, sizeof(Vertex)
      );
  shader.setAttributeBuffer(
      1, GL_FLOAT,
      offsetof(Vertex, r),
      3, sizeof(Vertex)
      );
  shader.release();

  m_vboPatch.release();
  m_vaoPatch.release();

  // --- Control net VAO/VBO (lines + points) ---
  m_vaoNet.create();
  m_vaoNet.bind();

  m_vboNet = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  m_vboNet.create();
  m_vboNet.bind();

  shader.bind();
  shader.enableAttributeArray(0);
  shader.enableAttributeArray(1);
  shader.setAttributeBuffer(
      0, GL_FLOAT,
      offsetof(Vertex, x),
      2, sizeof(Vertex)
      );
  shader.setAttributeBuffer(
      1, GL_FLOAT,
      offsetof(Vertex, r),
      3, sizeof(Vertex)
      );
  shader.release();

  m_vboNet.release();
  m_vaoNet.release();
}

void renderer::buildControlNetMesh()
{
  std::vector<Vertex> vertices;
  vertices.reserve(mesh.rows() * mesh.cols() * 4); // rough

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
  for (int r = 0; r < mesh.rows(); ++r)
  {
    for (int c = 0; c < mesh.cols() - 1; ++c)
    {
      const ControlPoint& a = mesh.points[r][c];
      const ControlPoint& b = mesh.points[r][c + 1];

      vertices.push_back(white(static_cast<float>(a.pos.x),
                               static_cast<float>(a.pos.y)));
      vertices.push_back(white(static_cast<float>(b.pos.x),
                               static_cast<float>(b.pos.y)));
    }
  }

  // Vertical lines
  for (int c = 0; c < mesh.cols(); ++c)
  {
    for (int r = 0; r < mesh.rows() - 1; ++r)
    {
      const ControlPoint& a = mesh.points[r][c];
      const ControlPoint& b = mesh.points[r + 1][c];

      vertices.push_back(white(static_cast<float>(a.pos.x),
                               static_cast<float>(a.pos.y)));
      vertices.push_back(white(static_cast<float>(b.pos.x),
                               static_cast<float>(b.pos.y)));
    }
  }

  m_netLineVertexCount = static_cast<int>(vertices.size());

  // --- Points at each control point ---
  for (int r = 0; r < mesh.rows(); ++r)
  {
    for (int c = 0; c < mesh.cols(); ++c)
    {
      const ControlPoint& p = mesh.points[r][c];
      vertices.push_back(white(static_cast<float>(p.pos.x),
                               static_cast<float>(p.pos.y)));
    }
  }

  m_netPointVertexCount =
      mesh.rows() * mesh.cols();

  // Upload to GPU
  m_vaoNet.bind();
  m_vboNet.bind();
  m_vboNet.allocate(vertices.data(),
                    static_cast<int>(vertices.size() * sizeof(Vertex)));
  m_vboNet.release();
  m_vaoNet.release();
}

void renderer::buildPatchMesh()
{
  const int resU = 40;
  const int resV = 40;

  for (int c = 0; c < mesh.cols(); ++c)
  {
    for (int r = 0; r < mesh.rows(); ++r)
    {
      qDebug() << "P: " << mesh.points[r][c].print();
    }
  }

  std::vector<Vertex> vertices;

  std::vector<GradientPatch> patches = mesh.calculatePatches();
  vertices.reserve(static_cast<std::size_t>(patches.size() * resU * resV * 6));

  auto addVertex = [&vertices](const DrawPoint& p)
  {
    Vertex v;
    v.x = static_cast<float>(p.pos.x);
    v.y = static_cast<float>(p.pos.y);
    v.r = static_cast<float>(p.col.r);
    v.g = static_cast<float>(p.col.g);
    v.b = static_cast<float>(p.col.b);
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

        DrawPoint c00 = patch.evaluate(u0, v0);
        DrawPoint c10 = patch.evaluate(u1, v0);
        DrawPoint c01 = patch.evaluate(u0, v1);
        DrawPoint c11 = patch.evaluate(u1, v1);

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

  // Upload to GPU
  m_vaoPatch.bind();
  m_vboPatch.bind();
  m_vboPatch.allocate(
      vertices.data(),
      static_cast<int>(vertices.size() * sizeof(Vertex))
      );
  m_vboPatch.release();
  m_vaoPatch.release();
}

void renderer::setMesh(const Mesh &m){
  mesh = m;
  buildPatchMesh();
  buildControlNetMesh();
}

void renderer::draw(){
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  shader.bind();

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

  shader.release();
}
