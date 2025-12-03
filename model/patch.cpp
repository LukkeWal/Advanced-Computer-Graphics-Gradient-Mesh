#include <cstring> // for memset

#include "patch.h"

// ---------- BicubicPatch2D ----------

Vec2 BicubicPatch2D::evaluate(double u, double v) const
{
  auto Hu = cubic::hermite(u);
  auto Hv = cubic::hermite(v);

  Vec2 res(0.0, 0.0);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = Hu[i] * Hv[j];
      res = res + C[i][j] * w;
    }
  }
  return res;
}

Vec2 BicubicPatch2D::du(double u, double v) const
{
  auto dHu = cubic::hermiteDeriv(u);
  auto Hv  = cubic::hermite(v);

  Vec2 res(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = dHu[i] * Hv[j];
      res = res + C[i][j] * w;
    }
  }
  return res;
}

Vec2 BicubicPatch2D::dv(double u, double v) const
{
  auto Hu  = cubic::hermite(u);
  auto dHv = cubic::hermiteDeriv(v);

  Vec2 res(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = Hu[i] * dHv[j];
      res = res + C[i][j] * w;
    }
  }
  return res;
}

// ---------- BicubicPatchColor ----------

ColorRGB BicubicPatchColor::evaluate(double u, double v) const
{
  auto Hu = cubic::hermite(u);
  auto Hv = cubic::hermite(v);

  ColorRGB res(0.0, 0.0, 0.0);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = Hu[i] * Hv[j];
      const ColorRGB& c = C[i][j];
      res.r += w * c.r;
      res.g += w * c.g;
      res.b += w * c.b;
    }
  }
  return res;
}

ColorRGB BicubicPatchColor::du(double u, double v) const
{
  auto dHu = cubic::hermiteDeriv(u);
  auto Hv  = cubic::hermite(v);

  ColorRGB res(0.0, 0.0, 0.0);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = dHu[i] * Hv[j];
      const ColorRGB& c = C[i][j];
      res.r += w * c.r;
      res.g += w * c.g;
      res.b += w * c.b;
    }
  }
  return res;
}

ColorRGB BicubicPatchColor::dv(double u, double v) const
{
  auto Hu  = cubic::hermite(u);
  auto dHv = cubic::hermiteDeriv(v);

  ColorRGB res(0.0, 0.0, 0.0);

  for (int i = 0; i < 4; ++i)
  {
    for (int j = 0; j < 4; ++j)
    {
      double w = Hu[i] * dHv[j];
      const ColorRGB& c = C[i][j];
      res.r += w * c.r;
      res.g += w * c.g;
      res.b += w * c.b;
    }
  }
  return res;
}


GradientPatch makeTestPatch()
{
  GradientPatch gp;

  // Zero everything first
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
    {
      gp.geom.C[i][j]  = Vec2(0.0, 0.0);
      gp.color.C[i][j] = ColorRGB(0.0, 0.0, 0.0);
    }

  // Corner positions in xy-plane
  // (0,0) bottom-left
  gp.geom.C[0][0] = Vec2(0.0, 0.0);
  // (1,0) bottom-right
  gp.geom.C[2][0] = Vec2(1.0, 0.0);
  // (0,1) top-left
  gp.geom.C[0][2] = Vec2(0.0, 1.0);
  // (1,1) top-right
  gp.geom.C[2][2] = Vec2(1.0, 1.0);

  // Corner colours (just something obvious)
  gp.color.C[0][0] = ColorRGB(1.0, 0.0, 0.0); // red
  gp.color.C[2][0] = ColorRGB(0.0, 1.0, 0.0); // green
  gp.color.C[0][2] = ColorRGB(0.0, 0.0, 1.0); // blue
  gp.color.C[2][2] = ColorRGB(0.0, 0.0, 0.0); // white

  // All tangents/twists are implicitly zero (kept at default),
  // so the patch interpolates the four corner positions/colours
  // with Hermite basis.

  return gp;
}

