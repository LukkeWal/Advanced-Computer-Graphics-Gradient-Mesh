#include "mesh.h"
#include "QDebug"

void Mesh::setBasicTestMesh(){
  m_rows = 2;
  m_cols = 2;
  points.assign(2, std::vector<ControlPoint>(2));
  ControlPoint P00, P01, P10, P11;
  P00.pos = Vec2(0, 0);
  P01.pos = Vec2(0, 1);
  P10.pos = Vec2(1, 0);
  P11.pos = Vec2(1, 1);

  P00.col = ColorRGB(1, 0, 0);
  P01.col = ColorRGB(0, 1, 0);
  P10.col = ColorRGB(0, 0, 1);
  P11.col = ColorRGB(1, 1, 1);

  Vec2 Su = Vec2(1.0, 0.0);
  Vec2 Sv = Vec2(0.0, 1.0);

  P00.uPosTangent = Su;  P00.vPosTangent = Sv;
  P01.uPosTangent = Su;  P01.vPosTangent = Sv;
  P10.uPosTangent = Su;  P10.vPosTangent = Sv;
  P11.uPosTangent = Su;  P11.vPosTangent = Sv;

  // for now, set colour tangents to something simple, e.g. zero:
  P00.uColTangent = P00.vColTangent = ColorRGB(0,0,0);
  P01.uColTangent = P01.vColTangent = ColorRGB(0,0,0);
  P10.uColTangent = P10.vColTangent = ColorRGB(0,0,0);
  P11.uColTangent = P11.vColTangent = ColorRGB(0,0,0);

  qDebug() << "P00: " << P00.print();
  qDebug() << "P01: " << P01.print();
  qDebug() << "P10: " << P10.print();
  qDebug() << "P11: " << P11.print();

  points[0][0] = P00;
  points[0][1] = P01;
  points[1][0] = P10;
  points[1][1] = P11;


  updateDerivatives();
}

void Mesh::setPaperTestMesh()
{
  m_rows = 3;
  m_cols = 3;
  points.assign(m_rows, std::vector<ControlPoint>(m_cols));
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      double u = (m_cols == 1) ? 0.0 : static_cast<double>(c) / (m_cols - 1);
      double v = (m_rows == 1) ? 0.0 : static_cast<double>(r) / (m_rows - 1);

      ControlPoint cp;
      cp.pos = Vec2(u, v);
      cp.col = ColorRGB(0, 0, 0);
      if (r == 1 && c == 0)
        cp.col = ColorRGB(1, 1, 1);
      if(r == 0 && c == 2)
        cp.col = ColorRGB(1, 0, 0);
      if(r == 1 && c == 2)
        cp.col = ColorRGB(0, 1, 0);
      if(r == 2 && c == 2)
        cp.col = ColorRGB(0, 0, 1);
      points[r][c] = cp;
    }
  }
  updateDerivatives();
}

void Mesh::updateDerivatives()
{
  if (m_rows == 0 || m_cols == 0)
    return;

  const double tangentScale = 1.0;
  const double colourTangentScale = 1.0;

  // Horizontal (v direction)
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      Vec2 dv(0.0, 0.0);

      if (m_cols > 1)
      {
        if (c == 0)
          dv = points[r][1].pos - points[r][0].pos;
        else if (c == m_cols - 1)
          dv = points[r][m_cols - 1].pos - points[r][m_cols - 2].pos;
        else
          dv = 0.5 * (points[r][c + 1].pos - points[r][c - 1].pos);
      }
      points[r][c].vPosTangent = tangentScale * dv;
    }
  }

  // Horizontal (u direction)
  for (int c = 0; c < m_cols; ++c)
  {
    for (int r = 0; r < m_rows; ++r)
    {
      Vec2 du(0.0, 0.0);

      if (m_rows > 1)
      {
        if (r == 0)
          du = points[1][c].pos - points[0][c].pos;
        else if (r == m_rows - 1)
          du = points[m_rows - 1][c].pos - points[m_rows - 2][c].pos;
        else
          du = 0.5 * (points[r + 1][c].pos - points[r - 1][c].pos);
      }
      points[r][c].uPosTangent = tangentScale * du;
    }
  }

  // Vertical colour derivatives
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      ColorRGB dvC(0.0, 0.0, 0.0);

      if (m_cols > 1)
      {
        if (c == 0)
          dvC = points[r][1].col - points[r][0].col;
        else if (c == m_cols - 1)
          dvC = points[r][m_cols - 1].col - points[r][m_cols - 2].col;
        else
          dvC = 0.5 * (points[r][c + 1].col - points[r][c - 1].col);
      }
      points[r][c].vColTangent = colourTangentScale * dvC;
    }
  }

  // Horizontal colour derivatives
  for (int c = 0; c < m_cols; ++c)
  {
    for (int r = 0; r < m_rows; ++r)
    {
      ColorRGB duC(0.0, 0.0, 0.0);

      if (m_rows > 1)
      {
        if (r == 0)
          duC = points[1][c].col - points[0][c].col;
        else if (r == m_rows - 1)
          duC = points[m_rows - 1][c].col - points[m_rows - 2][c].col;
        else
          duC = 0.5 * (points[r + 1][c].col - points[r - 1][c].col);
      }
      points[r][c].uColTangent = colourTangentScale * duC;
    }
  }
}

std::vector<GradientPatch> Mesh::calculatePatches() const
{
  std::vector<GradientPatch> patches;
  if (m_rows < 2 || m_cols < 2)
    return patches;

  for (int r = 0; r < m_rows - 1; ++r)
  {
    for (int c = 0; c < m_cols - 1; ++c)
    {
      GradientPatch gp;
      gp.controlPoints[0][0] = points[r][c];
      gp.controlPoints[0][1] = points[r][c+1];
      gp.controlPoints[1][0] = points[r+1][c];
      gp.controlPoints[1][1] = points[r+1][c+1];
      patches.push_back(gp);
    }
  }

  return patches;
}
