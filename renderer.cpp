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
  shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/gradmesh.vert");
  shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/gradmesh.frag");
  shader.link();
}

void renderer::setResolution(int newRes){
  resolution = newRes;
  buildPatchMesh();
  if(curveControl)
    buildControlNetMesh();
}

void renderer::setCurveControl(bool doCurveControl){
  curveControl = doCurveControl;
  buildControlNetMesh();
}

void renderer::setcontrolNetOnly(bool newControlNetOnly){
  controlNetOnly = newControlNetOnly;
  buildControlNetMesh();
}

void renderer::initializeGL()
{
  initializeOpenGLFunctions();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

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
  vertices.reserve(mesh.rows() * mesh.cols() * 8); // rough

  auto white = [](float x, float y) {
    Vertex v;
    v.x = x; v.y = y;
    v.r = 1.0f; v.g = 1.0f; v.b = 1.0f;
    return v;
  };

  auto addPolylineAsLines = [&](const std::vector<DrawPoint>& pts)
  {
    if (pts.size() < 2) return;
    for (size_t i = 1; i < pts.size(); ++i)
    {
      const auto& a = pts[i - 1].pos;
      const auto& b = pts[i].pos;
      vertices.push_back(white(static_cast<float>(a.x), static_cast<float>(a.y)));
      vertices.push_back(white(static_cast<float>(b.x), static_cast<float>(b.y)));
    }
  };

  if (!curveControl)
  {
    // Horizontal lines
    for (int r = 0; r < mesh.rows(); ++r)
    {
      for (int c = 0; c < mesh.cols() - 1; ++c)
      {
        const ControlPoint& a = mesh.points[r][c];
        const ControlPoint& b = mesh.points[r][c + 1];
        vertices.push_back(white((float)a.pos.x, (float)a.pos.y));
        vertices.push_back(white((float)b.pos.x, (float)b.pos.y));
      }
    }

    // Vertical lines
    for (int c = 0; c < mesh.cols(); ++c)
    {
      for (int r = 0; r < mesh.rows() - 1; ++r)
      {
        const ControlPoint& a = mesh.points[r][c];
        const ControlPoint& b = mesh.points[r + 1][c];
        vertices.push_back(white((float)a.pos.x, (float)a.pos.y));
        vertices.push_back(white((float)b.pos.x, (float)b.pos.y));
      }
    }
  }
  else
  {
    const int pr = mesh.rows() - 1; // patches in rows
    const int pc = mesh.cols() - 1; // patches in cols
    const int S  = std::max(2, resolution);

    std::vector<GradientPatch> patches = mesh.calculatePatches();
    if ((int)patches.size() == pr * pc)
    {
      auto patchAt = [&](int r, int c) -> const GradientPatch& {
        return patches[r * pc + c];
      };

      auto sampleEdge = [&](const GradientPatch& patch,
                            const CMatrices& Cs,
                            bool uFixed, double fixedVal,
                            bool alongU) -> std::vector<DrawPoint>
      {
        std::vector<DrawPoint> pts;
        pts.reserve((size_t)S + 1);

        for (int k = 0; k <= S; ++k)
        {
          double t = (double)k / (double)S;
          double u = 0.0, v = 0.0;

          if (alongU) { // u varies, v fixed
            u = t;
            v = fixedVal;
          } else {      // v varies, u fixed
            u = fixedVal;
            v = t;
          }

          pts.push_back(patch.evaluate(u, v, Cs));
        }
        return pts;
      };

      for (int r = 0; r < pr; ++r)
      {
        for (int c = 0; c < pc; ++c)
        {
          const GradientPatch& patch = patchAt(r, c);
          const CMatrices Cs = patch.calculateC(); // compute ONCE per patch
          addPolylineAsLines(sampleEdge(patch, Cs, false, 1.0, true));
          addPolylineAsLines(sampleEdge(patch, Cs, true, 1.0, false));
          addPolylineAsLines(sampleEdge(patch, Cs, true, 0.0, false));
          addPolylineAsLines(sampleEdge(patch, Cs, false, 0.0, true));
        }
      }
    }
    else
    {
      for (int r = 0; r < mesh.rows(); ++r)
      {
        for (int c = 0; c < mesh.cols() - 1; ++c)
        {
          const ControlPoint& a = mesh.points[r][c];
          const ControlPoint& b = mesh.points[r][c + 1];
          vertices.push_back(white((float)a.pos.x, (float)a.pos.y));
          vertices.push_back(white((float)b.pos.x, (float)b.pos.y));
        }
      }
      for (int c = 0; c < mesh.cols(); ++c)
      {
        for (int r = 0; r < mesh.rows() - 1; ++r)
        {
          const ControlPoint& a = mesh.points[r][c];
          const ControlPoint& b = mesh.points[r + 1][c];
          vertices.push_back(white((float)a.pos.x, (float)a.pos.y));
          vertices.push_back(white((float)b.pos.x, (float)b.pos.y));
        }
      }
    }
  }

  // Line vertices done
  m_netLineVertexCount = static_cast<int>(vertices.size());

  // Points at each control point (unchanged)
  for (int r = 0; r < mesh.rows(); ++r)
  {
    for (int c = 0; c < mesh.cols(); ++c)
    {
      const ControlPoint& p = mesh.points[r][c];
      if (controlNetOnly){
        Vertex v;
        v.x = static_cast<float>(p.pos.x);
        v.y = static_cast<float>(p.pos.y);
        v.r = static_cast<float>(p.col.r);
        v.g = static_cast<float>(p.col.g);
        v.b = static_cast<float>(p.col.b);

        vertices.push_back(v);
      } else
        vertices.push_back(white((float)p.pos.x, (float)p.pos.y));
    }
  }
  m_netPointVertexCount = mesh.rows() * mesh.cols();

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
  const int resU = resolution;
  const int resV = resolution;

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
    CMatrices Cs = patch.calculateC();
    for (int i = 0; i < resU; ++i)
    {
      double u0 = static_cast<double>(i) / resU;
      double u1 = static_cast<double>(i + 1) / resU;

      for (int j = 0; j < resV; ++j)
      {
        double v0 = static_cast<double>(j) / resV;
        double v1 = static_cast<double>(j + 1) / resV;


        DrawPoint c00 = patch.evaluate(u0, v0, Cs);
        DrawPoint c10 = patch.evaluate(u1, v0, Cs);
        DrawPoint c01 = patch.evaluate(u0, v1, Cs);
        DrawPoint c11 = patch.evaluate(u1, v1, Cs);

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

  if (!controlNetOnly)
  {
    // Draw patches (triangles)
    glEnable(GL_DEPTH_TEST);
    m_vaoPatch.bind();
    glDrawArrays(GL_TRIANGLES, 0, m_vertexCountPatch);
    m_vaoPatch.release();
  }

  // Draw control net on top
  glDisable(GL_DEPTH_TEST);
  m_vaoNet.bind();

  glLineWidth(1.5f);
  glDrawArrays(GL_LINES, 0, m_netLineVertexCount);

  if(controlNetOnly)
    glPointSize(20.0f);
  else
    glPointSize(10.0f);
  glDrawArrays(GL_POINTS,
               m_netLineVertexCount,
               m_netPointVertexCount);

  m_vaoNet.release();

  shader.release();
}
