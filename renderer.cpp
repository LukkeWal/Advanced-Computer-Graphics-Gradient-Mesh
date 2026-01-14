#include "renderer.h"

renderer::renderer(): m_vboNet(QOpenGLBuffer::VertexBuffer) {}

void renderer::cleanup()
{
  m_vboPatch.destroy();
  m_vaoPatch.destroy();
  m_vboNet.destroy();
  m_vaoNet.destroy();
  if (CsHandle) {
    glDeleteBuffers(1, &CsHandle);
    CsHandle = 0;
  }
}

void renderer::initializeShader(){
  auto ok =
      shader.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/gradmesh.vert") &&
      shader.addShaderFromSourceFile(QOpenGLShader::TessellationControl, ":/shaders/gradmesh.tesc") &&
      shader.addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, ":/shaders/gradmesh.tese") &&
      shader.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/gradmesh.frag") &&
      shader.link();

  if (!ok) {
    qDebug() << "Shader error:" << shader.log();
  }

  ok =
      shaderNet.addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/net.vert") &&
      shaderNet.addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/net.frag") &&
      shaderNet.link();

  if (!ok) {
    qDebug() << "Shader error:" << shaderNet.log();
  }
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
  const bool ok = initializeOpenGLFunctions();
  qDebug() << "initializeOpenGLFunctions() =" << ok;

  // SSBO handle for patch coefficients
  glGenBuffers(1, &CsHandle);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // Needed if your vertex shader sets gl_PointSize
  glEnable(GL_PROGRAM_POINT_SIZE);

  m_vaoPatch.create();
  m_vaoPatch.bind();
  m_vaoPatch.release();

  m_vaoNet.create();
  m_vaoNet.bind();

  if (m_vboNet.isCreated())
    m_vboNet.destroy();

  m_vboNet = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  m_vboNet.create();
  m_vboNet.bind();

  // Position (x,y) at location 0
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
      0, 2, GL_FLOAT, GL_FALSE,
      sizeof(Vertex),
      reinterpret_cast<void*>(offsetof(Vertex, x))
      );

  // Quadrant color 0 at location 1
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
      1, 3, GL_FLOAT, GL_FALSE,
      sizeof(Vertex),
      reinterpret_cast<void*>(offsetof(Vertex, r0))
      );

  // Quadrant color 1 at location 2
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
      2, 3, GL_FLOAT, GL_FALSE,
      sizeof(Vertex),
      reinterpret_cast<void*>(offsetof(Vertex, r1))
      );

  // Quadrant color 2 at location 3
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(
      3, 3, GL_FLOAT, GL_FALSE,
      sizeof(Vertex),
      reinterpret_cast<void*>(offsetof(Vertex, r2))
      );

  // Quadrant color 3 at location 4
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(
      4, 3, GL_FLOAT, GL_FALSE,
      sizeof(Vertex),
      reinterpret_cast<void*>(offsetof(Vertex, r3))
      );

  m_vboNet.release();
  m_vaoNet.release();
}


void renderer::buildControlNetMesh()
{
  std::vector<Vertex> vertices;
  vertices.reserve(mesh.rows() * mesh.cols() * 8);

  auto mk = [](float x, float y,
               const ColorRGB& c0,
               const ColorRGB& c1,
               const ColorRGB& c2,
               const ColorRGB& c3)
  {
    Vertex v{};
    v.x = x; v.y = y;

    v.r0 = (float)c0.r; v.g0 = (float)c0.g; v.b0 = (float)c0.b;
    v.r1 = (float)c1.r; v.g1 = (float)c1.g; v.b1 = (float)c1.b;
    v.r2 = (float)c2.r; v.g2 = (float)c2.g; v.b2 = (float)c2.b;
    v.r3 = (float)c3.r; v.g3 = (float)c3.g; v.b3 = (float)c3.b;
    return v;
  };

  auto white = [&](float x, float y) {
    ColorRGB w{1.0, 1.0, 1.0};
    return mk(x, y, w, w, w, w);
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

          if (alongU) {
            u = t;
            v = fixedVal;
          } else {
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
          const CMatrices Cs = patch.calculateC(); // compute once
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

  m_netLineVertexCount = static_cast<int>(vertices.size());

  // Points at each control point
  for (int r = 0; r < mesh.rows(); ++r)
  {
    for (int c = 0; c < mesh.cols(); ++c)
    {
      const ControlPoint& p = mesh.points[r][c];

      if (controlNetOnly)
      {
        const ColorRGB& CTL = p.cols[0][0];
        const ColorRGB& CTR = p.cols[0][1];
        const ColorRGB& CBL = p.cols[1][0];
        const ColorRGB& CBR = p.cols[1][1];

        vertices.push_back(mk((float)p.pos.x, (float)p.pos.y, CBL, CBR, CTL, CTR));
      }
      else
      {
        vertices.push_back(white((float)p.pos.x, (float)p.pos.y));
      }
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
  std::vector<GradientPatch> patches = mesh.calculatePatches();
  patchCount = static_cast<int>(patches.size());
  if (patchCount == 0) return;

  std::vector<CsGPU> gpu;
  gpu.resize(patches.size());

  auto toVec4 = [](const Vec3& v) -> Vec4 {
    return Vec4{ (float)v.x, (float)v.y, (float)v.z, 0.0f };
  };

  for (size_t p = 0; p < patches.size(); ++p)
  {
    CMatrices Cs = patches[p].calculateC();

    for (int r = 0; r < 4; ++r)
      for (int c = 0; c < 4; ++c)
      {
        int idx = r * 4 + c;
        gpu[p].Cred[idx]   = toVec4(Cs.C_red[r][c]);
        gpu[p].Cgreen[idx] = toVec4(Cs.C_green[r][c]);
        gpu[p].Cblue[idx]  = toVec4(Cs.C_blue[r][c]);
      }
  }

  // Upload SSBO
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, CsHandle);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               GLsizeiptr(gpu.size() * sizeof(CsGPU)),
               gpu.data(),
               GL_DYNAMIC_DRAW);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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
  shader.setUniformValue("uTess", float(resolution));

  if (!controlNetOnly)
  {
    glEnable(GL_DEPTH_TEST);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, CsHandle);

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    m_vaoPatch.bind();
    glDrawArraysInstanced(GL_PATCHES, 0, 4, patchCount); // 4 dummy verts, patchCount instances
    m_vaoPatch.release();

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  }
  shader.release();
  shaderNet.bind();
  shaderNet.setUniformValue("uPointSize", controlNetOnly ? 20.0f : 10.0f);

  // Draw control net on top
  glDisable(GL_DEPTH_TEST);
  m_vaoNet.bind();

  glLineWidth(1.5f);
  shaderNet.setUniformValue("uIsPoint", 0);
  glDrawArrays(GL_LINES, 0, m_netLineVertexCount);
  shaderNet.setUniformValue("uIsPoint", 1);
  glDrawArrays(GL_POINTS, m_netLineVertexCount, m_netPointVertexCount);

  m_vaoNet.release();
  shaderNet.release();
}
