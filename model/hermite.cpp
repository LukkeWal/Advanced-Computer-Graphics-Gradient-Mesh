#include "hermite.h"

DrawPoint GradientPatch::evaluate(double u, double v) const{
  CMatrices Cs = calculateC();
  // get the curve values
  auto Hu = hermite(u);
  auto Hv = hermite(v);
  // evaluate for each color channel with S = (x, y, z) where z is a single color channel
  Vec3 R = evaluate_for_color(Hu, Hv, Cs.C_red);
  Vec3 G = evaluate_for_color(Hu, Hv, Cs.C_green);
  Vec3 B = evaluate_for_color(Hu, Hv, Cs.C_blue);
  // format result (x, y are the same for R G and B)
  DrawPoint result;
  result.col = {R.z, G.z, B.z};
  result.pos = {R.x, R.y};
  return result;
}

Vec3 GradientPatch::evaluate_for_color(std::array<double,4> Hu,
                                       std::array<double,4> Hv,
                                       std::array<std::array<Vec3, 4>, 4> C) const{
  // HT(u) * C:
  std::array<Vec3,4> HTuC;
  for(int c=0; c<4; c++){
    HTuC[c] = {0, 0, 0};
    for(int r=0; r<4; r++){
      HTuC[c] = HTuC[c] + Hu[r] * C[r][c];
    }
  }
  // HT(u) * C * H(v)
  Vec3 HTuCHv = {0, 0, 0};
  for(int c=0; c<4; c++){
    HTuCHv = HTuCHv + Hv[c] * HTuC[c];
  }
  return HTuCHv;
}

std::array<double,4> GradientPatch::hermite(double t) const
{
  double t2 = t * t;
  double t3 = t2 * t;

  double h00 = 2.0 * t3 - 3.0 * t2 + 1.0;
  double h10 = t3 - 2.0 * t2 + t;
  double h01 = -2.0 * t3 + 3.0 * t2;
  double h11 = t3 - t2;
  // order: [pos0, tan0, tan1, pos1]
  return {h00, h10, h11, h01};
}


CMatrices GradientPatch::calculateC() const{
  auto buildChannel = [&](auto getCol, auto getCol_u, auto getCol_v) {
    ControlPoint P00 = controlPoints[0][0];
    ControlPoint P01 = controlPoints[0][1];
    ControlPoint P10 = controlPoints[1][0];
    ControlPoint P11 = controlPoints[1][1];

    Vec3 S00 = {P00.pos.x, P00.pos.y, getCol(P00)};
    Vec3 S01 = {P01.pos.x, P01.pos.y, getCol(P01)};
    Vec3 S10 = {P10.pos.x, P10.pos.y, getCol(P10)};
    Vec3 S11 = {P11.pos.x, P11.pos.y, getCol(P11)};

    Vec3 Sv00 = {P00.vPosTangent.x, P00.vPosTangent.y, getCol_v(P00)};
    Vec3 Sv01 = {P01.vPosTangent.x, P01.vPosTangent.y, getCol_v(P01)};
    Vec3 Sv10 = {P10.vPosTangent.x, P10.vPosTangent.y, getCol_v(P10)};
    Vec3 Sv11 = {P11.vPosTangent.x, P11.vPosTangent.y, getCol_v(P11)};

    Vec3 Su00 = {P00.uPosTangent.x, P00.uPosTangent.y, getCol_u(P00)};
    Vec3 Su01 = {P01.uPosTangent.x, P01.uPosTangent.y, getCol_u(P01)};
    Vec3 Su10 = {P10.uPosTangent.x, P10.uPosTangent.y, getCol_u(P10)};
    Vec3 Su11 = {P11.uPosTangent.x, P11.uPosTangent.y, getCol_u(P11)};

    Vec3 zero = {0, 0, 0};

    std::array<std::array<Vec3,4>,4> C;
    C[0] = {S00, Sv00, Sv01, S01};
    C[1] = {Su00, zero, zero, Su01};
    C[2] = {Su10, zero, zero, Su11};
    C[3] = {S10, Sv10, Sv11, S11};

    return C;
  };

  CMatrices result;

  result.C_red   = buildChannel(
      [](auto& p){ return p.col.r; },
      [](auto& p){ return p.uColTangent.r; },
      [](auto& p){ return p.vColTangent.r; }
      );

  result.C_green = buildChannel(
      [](auto& p){ return p.col.g; },
      [](auto& p){ return p.uColTangent.g; },
      [](auto& p){ return p.vColTangent.g; }
      );

  result.C_blue  = buildChannel(
      [](auto& p){ return p.col.b; },
      [](auto& p){ return p.uColTangent.b; },
      [](auto& p){ return p.vColTangent.b; }
      );
  return result;
}


