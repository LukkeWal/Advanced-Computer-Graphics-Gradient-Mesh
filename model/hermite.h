#include <array>
#include <QDebug>

#include "basicdatatypes.h"

std::array<std::array<ColorRGB, 4>, 4> oneCol(ColorRGB col);

struct ControlPoint{
  Vec2 pos;
  Vec2 uPosTangent;
  Vec2 vPosTangent;
  std::array<std::array<ColorRGB, 4>, 4> cols;
  //ColorRGB col;
};

struct DrawPoint{
  Vec2 pos;
  ColorRGB col;
};

struct CMatrices{
  std::array<std::array<Vec3, 4>, 4> C_red;
  std::array<std::array<Vec3, 4>, 4> C_green;
  std::array<std::array<Vec3, 4>, 4> C_blue;
};

struct GradientPatch
{
public:
  std::array<std::array<ControlPoint, 2>, 2> controlPoints;
  DrawPoint evaluate(double u, double v, CMatrices Cs) const;
  CMatrices calculateC() const;
private:
  Vec3 evaluate_for_color(std::array<double,4> Hu, std::array<double,4>Hv, std::array<std::array<Vec3, 4>, 4> C) const;
  std::array<double,4> hermite(double t) const;
};
