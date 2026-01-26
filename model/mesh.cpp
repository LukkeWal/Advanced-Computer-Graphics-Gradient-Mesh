#include "mesh.h"
#include "QDebug"

std::array<std::array<ColorRGB, 4>, 4> Mesh::oneCol(ColorRGB col){
  return {{
      {{ col, col }},
      {{ col, col }}
  }};
}

std::array<std::array<ColorRGB, 4>, 4> Mesh::difCol(ColorRGB c0, ColorRGB c1, ColorRGB c2, ColorRGB c3){
  return {{
      {{ c0, c1 }},
      {{ c2, c3 }}
  }};
}

void Mesh::setBasicTestMesh(){
  m_rows = 2;
  m_cols = 2;
  points.assign(2, std::vector<ControlPoint>(2));
  ControlPoint P00, P01, P10, P11;
  P00.pos = Vec2(0, 0);
  P01.pos = Vec2(0, 1);
  P10.pos = Vec2(1, 0);
  P11.pos = Vec2(1, 1);

  P00.cols = oneCol(ColorRGB(1, 0, 0));
  P01.cols = oneCol(ColorRGB(0, 1, 0));
  P10.cols = oneCol(ColorRGB(0, 0, 1));
  P11.cols = oneCol(ColorRGB(1, 1, 1));

  Vec2 Su = Vec2(1.0, 0.0);
  Vec2 Sv = Vec2(0.0, 1.0);

  P00.uPosTangent = Su;  P00.vPosTangent = Sv;
  P01.uPosTangent = Su;  P01.vPosTangent = Sv;
  P10.uPosTangent = Su;  P10.vPosTangent = Sv;
  P11.uPosTangent = Su;  P11.vPosTangent = Sv;

  // qDebug() << "P00: " << P00.print();
  // qDebug() << "P01: " << P01.print();
  // qDebug() << "P10: " << P10.print();
  // qDebug() << "P11: " << P11.print();

  points[0][0] = P00;
  points[0][1] = P01;
  points[1][0] = P10;
  points[1][1] = P11;


  updateDerivatives();
}

void Mesh::setPaperTestMesh()
{
  m_rows = 3;
  m_cols = 4;
  points.assign(m_rows, std::vector<ControlPoint>(m_cols));
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      double u = (m_cols == 1) ? 0.0 : static_cast<double>(c) / (m_cols - 1);
      double v = (m_rows == 1) ? 0.0 : static_cast<double>(r) / (m_rows - 1);

      ControlPoint cp;
      cp.pos = Vec2(u, v);
      cp.cols = oneCol(ColorRGB(0, 0, 0));
      if (r == 1 && c == 1)
        cp.cols = oneCol(ColorRGB(1, 1, 1));
      if(r == 0 && c == 3)
        cp.cols = oneCol(ColorRGB(1, 0, 0));
      if(r == 1 && c == 3)
        cp.cols = oneCol(ColorRGB(0, 1, 0));
      if(r == 2 && c == 3)
        cp.cols = oneCol(ColorRGB(0, 0, 1));
      points[r][c] = cp;
    }
  }
  updateDerivatives();
}

void Mesh::setSharpMesh()
{
  m_rows = 3;
  m_cols = 4;
  points.assign(m_rows, std::vector<ControlPoint>(m_cols));
  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      double u = (m_cols == 1) ? 0.0 : static_cast<double>(c) / (m_cols - 1);
      double v = (m_rows == 1) ? 0.0 : static_cast<double>(r) / (m_rows - 1);

      ControlPoint cp;
      cp.pos = Vec2(u, v);

      ColorRGB green = ColorRGB(0, 1, 0);
      ColorRGB gray = ColorRGB(0.5, 0.5, 0.5);

      if (r == 1 && c == 1)
        cp.cols = difCol(gray, gray, green, green);
      else if (r == 1 && c == 2)
        cp.cols = difCol(gray, gray, green, gray);
      else
        cp.cols = oneCol(gray);
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
  const double colourTangentScale = 0;

  // (v direction)
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

  // (u direction)
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
}

void Mesh::setAppleMesh()
{
  // 7x7 regular grid in normalized [0..1]x[0..1]
  m_rows = 7;
  m_cols = 6;
  points.assign(m_rows, std::vector<ControlPoint>(m_cols));

  auto U = [&](int c) -> double { return (m_cols == 1) ? 0.0 : static_cast<double>(c) / (m_cols - 1); };
  auto V = [&](int r) -> double { return (m_rows == 1) ? 0.0 : static_cast<double>(r) / (m_rows - 1); };

  // Colors as ColorRGB(r,g,b) in [0..1] converted from the hex palette I suggested.
  // Layout is row-major: r = y (v) from top (0) to bottom (1), c = x (u) from left (0) to right (1).
  const ColorRGB C[7][6] = {
      { ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 0.5, 0.5), ColorRGB(1, 0, 0), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 0, 0), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(0.5, 0.3, 0), ColorRGB(0.5, 0.3, 0), ColorRGB(0.5, 0.3, 0), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(0.5, 0.3, 0), ColorRGB(0.5, 0.3, 0), ColorRGB(0.5, 0.3, 0), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1) },
      { ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1), ColorRGB(1, 1, 1) },
      };

  const Vec2 p[7][6] = {
      {Vec2(0.411348, 0.248677), Vec2(0.421099, 0.228836), Vec2(0.481383, 0.14418), Vec2(0.614362, 0.140212), Vec2(0.722518, 0.161376), Vec2(0.743794, 0.190476)},
      {Vec2(0.406915, 0.279101), Vec2(0.43883, 0.257937), Vec2(0.501773, 0.187831), Vec2(0.6, 0.166667), Vec2(0.719858, 0.203704), Vec2(0.74734, 0.232804)},
      {Vec2(0.399823, 0.342593), Vec2(0.432624, 0.339947), Vec2(0.578901, 0.34127), Vec2(0.611702, 0.34127), Vec2(0.744681, 0.322751), Vec2(0.771277, 0.324074)},
      {Vec2(0.410461, 0.439153), Vec2(0.435284, 0.431217), Vec2(0.496454, 0.482804), Vec2(0.592199, 0.501323), Vec2(0.702128, 0.452381), Vec2(0.734929, 0.46164)},
      {Vec2(0.47695, 0.518519), Vec2(0.498227, 0.519841), Vec2(0.515957, 0.527778), Vec2(0.535461, 0.519841), Vec2(0.549645, 0.543651), Vec2(0.585106, 0.548942)},
      {Vec2(0.442376, 0.583333), Vec2(0.464539, 0.584656), Vec2(0.457447, 0.634921), Vec2(0.495567, 0.593915), Vec2(0.513298, 0.60582), Vec2(0.545213, 0.585979)},
      {Vec2(0.419326, 0.630952), Vec2(0.431738, 0.653439), Vec2(0.462766, 0.666667), Vec2(0.484043, 0.648148), Vec2(0.507092, 0.634921), Vec2(0.536348, 0.64418)},
  };

  for (int r = 0; r < m_rows; ++r)
  {
    for (int c = 0; c < m_cols; ++c)
    {
      ControlPoint cp;
      cp.pos = p[r][c];
      cp.cols = oneCol(C[r][c]);

      cp.uPosTangent = Vec2(0.0, 0.0);
      cp.vPosTangent = Vec2(0.0, 0.0);

      points[r][c] = cp;
    }
  }

  updateDerivatives();
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
