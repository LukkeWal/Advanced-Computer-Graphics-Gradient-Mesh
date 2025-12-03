#pragma once

#include <array>
#include "geometry.h"

// Bicubic Hermite patch for geometry (2D in xy-plane)
// S(u, v) = H^T(u) * C * H(v), where C is 4x4 of Vec2
class BicubicPatch2D
{
public:
  // C[i][j] with i = 0..3 for u-direction Hermite basis
  //                 j = 0..3 for v-direction Hermite basis
  std::array<std::array<Vec2, 4>, 4> C;

  BicubicPatch2D() = default;

  // Evaluate point on the patch at (u,v) in [0,1]×[0,1]
  Vec2 evaluate(double u, double v) const;

  // Partial derivatives for shading / tangents if you ever need them
  Vec2 du(double u, double v) const;
  Vec2 dv(double u, double v) const;
};

// Same structure, but for colour
class BicubicPatchColor
{
public:
  std::array<std::array<ColorRGB, 4>, 4> C;

  BicubicPatchColor() = default;

  ColorRGB evaluate(double u, double v) const;
  ColorRGB du(double u, double v) const;
  ColorRGB dv(double u, double v) const;
};

// Convenience: a full gradient patch = geometry + colour
struct GradientPatch
{
  BicubicPatch2D    geom;
  BicubicPatchColor color;

  // Combined evaluate
  ControlPoint evaluate(double u, double v) const
  {
    ControlPoint cp;
    cp.pos = geom.evaluate(u, v);
    cp.col = color.evaluate(u, v);
    return cp;
  }
};

// Create a simple test patch: geometry in [0,1]x[0,1] and four corner colours.
// Tangents/twists are zero.
GradientPatch makeTestPatch();
