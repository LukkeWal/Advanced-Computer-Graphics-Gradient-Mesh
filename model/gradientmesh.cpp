#include "gradientmesh.h"
#include <QDebug>

GradientMesh::GradientMesh(int rows, int cols)
    : m_rows(rows)
    , m_cols(cols)
    , m_points(static_cast<std::size_t>(rows * cols))
    , m_duGeom(static_cast<std::size_t>(rows * cols))
    , m_dvGeom(static_cast<std::size_t>(rows * cols))
    , m_duColor(static_cast<std::size_t>(rows * cols))
    , m_dvColor(static_cast<std::size_t>(rows * cols))
{
}

ControlPoint& GradientMesh::at(int row, int col)
{
  return m_points[index(row, col)];
}

const ControlPoint& GradientMesh::at(int row, int col) const
{
  return m_points[index(row, col)];
}

std::vector<GradientPatch> GradientMesh::buildPatches() const
{
  std::vector<GradientPatch> patches;

  if (m_rows < 2 || m_cols < 2)
    return patches;

  for (int r = 0; r < m_rows - 1; ++r)
  {
    for (int c = 0; c < m_cols - 1; ++c)
    {
      const ControlPoint& p00 = at(r    , c    ); // (u=0,v=0)
      const ControlPoint& p01 = at(r    , c + 1); // (u=1,v=0)
      const ControlPoint& p10 = at(r + 1, c    ); // (u=0,v=1)
      const ControlPoint& p11 = at(r + 1, c + 1); // (u=1,v=1)

      // per-vertex derivatives
      Vec2 Su00 = m_duGeom[index(r    , c    )];
      Vec2 Su01 = m_duGeom[index(r    , c + 1)];
      Vec2 Su10 = m_duGeom[index(r + 1, c    )];
      Vec2 Su11 = m_duGeom[index(r + 1, c + 1)];

      Vec2 Sv00 = m_dvGeom[index(r    , c    )];
      Vec2 Sv01 = m_dvGeom[index(r    , c + 1)];
      Vec2 Sv10 = m_dvGeom[index(r + 1, c    )];
      Vec2 Sv11 = m_dvGeom[index(r + 1, c + 1)];

      GradientPatch gp;

      // ----- GEOMETRY -----
      gp.geom.C[0][0] = p00.pos;
      gp.geom.C[2][0] = p10.pos;
      gp.geom.C[0][2] = p01.pos;
      gp.geom.C[2][2] = p11.pos;

      gp.geom.C[1][0] = Su00;
      gp.geom.C[3][0] = Su10;
      gp.geom.C[1][2] = Su01;
      gp.geom.C[3][2] = Su11;

      gp.geom.C[0][1] = Sv00;
      gp.geom.C[2][1] = Sv10;
      gp.geom.C[0][3] = Sv01;
      gp.geom.C[2][3] = Sv11;

      // twists = 0 (Ferguson)
      gp.geom.C[1][1] = Vec2(0.0, 0.0);
      gp.geom.C[1][3] = Vec2(0.0, 0.0);
      gp.geom.C[3][1] = Vec2(0.0, 0.0);
      gp.geom.C[3][3] = Vec2(0.0, 0.0);

      // ----- COLOUR (same pattern) -----
      ColorRGB C00 = p00.col;
      ColorRGB C01 = p01.col;
      ColorRGB C10 = p10.col;
      ColorRGB C11 = p11.col;

      ColorRGB Cu00 = m_duColor[index(r    , c    )];
      ColorRGB Cu01 = m_duColor[index(r    , c + 1)];
      ColorRGB Cu10 = m_duColor[index(r + 1, c    )];
      ColorRGB Cu11 = m_duColor[index(r + 1, c + 1)];

      ColorRGB Cv00 = m_dvColor[index(r    , c    )];
      ColorRGB Cv01 = m_dvColor[index(r    , c + 1)];
      ColorRGB Cv10 = m_dvColor[index(r + 1, c    )];
      ColorRGB Cv11 = m_dvColor[index(r + 1, c + 1)];

      gp.color.C[0][0] = C00;
      gp.color.C[2][0] = C10;
      gp.color.C[0][2] = C01;
      gp.color.C[2][2] = C11;

      gp.color.C[1][0] = Cu00;
      gp.color.C[3][0] = Cu10;
      gp.color.C[1][2] = Cu01;
      gp.color.C[3][2] = Cu11;

      gp.color.C[0][1] = Cv00;
      gp.color.C[2][1] = Cv10;
      gp.color.C[0][3] = Cv01;
      gp.color.C[2][3] = Cv11;

      gp.color.C[1][1] = ColorRGB(0.0, 0.0, 0.0);
      gp.color.C[1][3] = ColorRGB(0.0, 0.0, 0.0);
      gp.color.C[3][1] = ColorRGB(0.0, 0.0, 0.0);
      gp.color.C[3][3] = ColorRGB(0.0, 0.0, 0.0);

      patches.push_back(gp);
    }
  }

  return patches;
}

// Regular test mesh: unit square in geometry, smooth colour gradient
GradientMesh GradientMesh::makeRegularTestMesh(int rows, int cols)
{
  GradientMesh gm(rows, cols);
  qDebug() << "called with rows = " << rows << "and cols = " << cols;

  for (int r = 0; r < rows; ++r)
  {
    for (int c = 0; c < cols; ++c)
    {
      double u = (cols == 1) ? 0.0 : static_cast<double>(c) / (cols - 1);
      double v = (rows == 1) ? 0.0 : static_cast<double>(r) / (rows - 1);

      ControlPoint cp;
      cp.pos = Vec2(u, v);
      cp.col = ColorRGB(u, v, 1);
      gm.at(r, c) = cp;
    }
  }
  gm.updateDerivatives();
  return gm;
}

// Regular test mesh: unit square in geometry, smooth colour gradient
GradientMesh GradientMesh::makePaperTestMesh()
{
  int rows = 3;
  int cols = 4;
  GradientMesh gm(rows, cols);
  for (int r = 0; r < rows; ++r)
  {
    for (int c = 0; c < cols; ++c)
    {
      double u = (cols == 1) ? 0.0 : static_cast<double>(c) / (cols - 1);
      double v = (rows == 1) ? 0.0 : static_cast<double>(r) / (rows - 1);

      ControlPoint cp;
      cp.pos = Vec2(u, v);
      cp.col = ColorRGB(0, 0, 0);
      if (r == 1 && c == 1)
        cp.col = ColorRGB(1, 1, 1);
      if(r == 0 && c == 3)
        cp.col = ColorRGB(1, 0, 0);
      if(r == 1 && c == 3)
        cp.col = ColorRGB(0, 1, 0);
      if(r == 2 && c == 3)
        cp.col = ColorRGB(0, 0, 1);
      gm.at(r, c) = cp;
    }
  }
  gm.updateDerivatives();
  return gm;
}


void GradientMesh::updateDerivatives()
{
  if (m_rows == 0 || m_cols == 0)
    return;

  const double tangentScale = 0.0;

  // --- Geometry derivatives ---

  // Horizontal (u direction)
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      Vec2 du(0.0, 0.0);

      if (m_cols > 1)
      {
        if (c == 0)
          du = at(r, 1).pos - at(r, 0).pos;
        else if (c == m_cols - 1)
          du = at(r, m_cols - 1).pos - at(r, m_cols - 2).pos;
        else
          du = 0.5 * (at(r, c + 1).pos - at(r, c - 1).pos);
      }

      m_duGeom[index(r, c)] = tangentScale * du;
    }
  }

  // Vertical (v direction)
  for (int c = 0; c < m_cols; ++c)
  {
    for (int r = 0; r < m_rows; ++r)
    {
      Vec2 dv(0.0, 0.0);

      if (m_rows > 1)
      {
        if (r == 0)
          dv = at(1, c).pos - at(0, c).pos;
        else if (r == m_rows - 1)
          dv = at(m_rows - 1, c).pos - at(m_rows - 2, c).pos;
        else
          dv = 0.5 * (at(r + 1, c).pos - at(r - 1, c).pos);
      }

      m_dvGeom[index(r, c)] = tangentScale * dv;
    }
  }

  // --- Colour derivatives: set to 0 ---

  for (int r = 0; r < m_rows; ++r) {
    for (int c = 0; c < m_cols; ++c) {
      m_duColor[index(r, c)] = ColorRGB(0.0, 0.0, 0.0);
      m_dvColor[index(r, c)] = ColorRGB(0.0, 0.0, 0.0);
    }
  }
}
