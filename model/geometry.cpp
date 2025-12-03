#include "geometry.h"

// ---------- basis functions ----------

std::array<double,4> cubic::bernstein(double t)
{
  double u  = 1.0 - t;
  double u2 = u * u;
  double t2 = t * t;

  return {
      u2 * u,          // (1 - t)^3
      3.0 * u2 * t,    // 3 (1 - t)^2 t
      3.0 * u * t2,    // 3 (1 - t) t^2
      t2 * t           // t^3
  };
}

std::array<double,4> cubic::bernsteinDeriv(double t)
{
  // derivative of cubic Bernstein basis
  double u  = 1.0 - t;
  double u2 = u * u;
  double t2 = t * t;

  // d/dt B0 = -3(1 - t)^2
  // d/dt B1 = 3(1 - t)^2 - 6(1 - t)t
  // d/dt B2 = 6(1 - t)t - 3 t^2
  // d/dt B3 = 3 t^2
  return {
      -3.0 * u2,
      3.0 * u2 - 6.0 * u * t,
      6.0 * u * t - 3.0 * t2,
      3.0 * t2
  };
}

// Hermite basis used in the paper: C(t) = Q^T H(t)  :contentReference[oaicite:3]{index=3}
// We use the standard cubic Hermite basis for position + tangents:
//
// h00(t) =  2t^3 - 3t^2 + 1
// h10(t) =      t^3 - 2t^2 + t
// h01(t) = -2t^3 + 3t^2
// h11(t) =      t^3 -    t^2
//
// but note: the paper uses Q = X P with X as in Eq. (3), which scales tangents.
// We'll match that scaling in the conversion functions below.
std::array<double,4> cubic::hermite(double t)
{
  double t2 = t * t;
  double t3 = t2 * t;

  double h00 =  2.0 * t3 - 3.0 * t2 + 1.0;
  double h10 =        t3 - 2.0 * t2 + t;
  double h01 = -2.0 * t3 + 3.0 * t2;
  double h11 =        t3 -        t2;

  return { h00, h10, h01, h11 };
}

std::array<double,4> cubic::hermiteDeriv(double t)
{
  double t2 = t * t;

  double dh00 =  6.0 * t2 - 6.0 * t;
  double dh10 =  3.0 * t2 - 4.0 * t + 1.0;
  double dh01 = -6.0 * t2 + 6.0 * t;
  double dh11 =  3.0 * t2 - 2.0 * t;

  return { dh00, dh10, dh01, dh11 };
}

// ---------- curve evaluation ----------

Vec2 CubicBezier2D::evaluate(double t) const
{
  auto B = cubic::bernstein(t);
  Vec2 r(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
    r = r + P[i] * B[i];
  return r;
}

Vec2 CubicBezier2D::derivative(double t) const
{
  auto dB = cubic::bernsteinDeriv(t);
  Vec2 r(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
    r = r + P[i] * dB[i];
  return r;
}

Vec2 CubicHermite2D::evaluate(double t) const
{
  auto H = cubic::hermite(t);
  Vec2 r(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
    r = r + Q[i] * H[i];
  return r;
}

Vec2 CubicHermite2D::derivative(double t) const
{
  auto dH = cubic::hermiteDeriv(t);
  Vec2 r(0.0, 0.0);
  for (int i = 0; i < 4; ++i)
    r = r + Q[i] * dH[i];
  return r;
}

// ---------- Bézier <-> Hermite conversion ----------
//
// Paper: Q = X P with
// X = [ 1  0  0  0
//      -3  3  0  0
//       0  0 -3  3
//       0  0  0  1 ]   :contentReference[oaicite:4]{index=4}
//
// We interpret:
//   Q0 = P0
//   Q1 = -3 P0 + 3 P1  (tangent at t=0, scaled)
//   Q2 = -3 P2 + 3 P3  (tangent at t=1, scaled, points "backwards")
//   Q3 = P3
//
// For our Hermite basis, we want *actual* tangents M0, M1,
// so we rescale these when converting.

CubicHermite2D bezierToHermite(const CubicBezier2D& bez)
{
  CubicHermite2D h;

  const Vec2& P0 = bez.P[0];
  const Vec2& P1 = bez.P[1];
  const Vec2& P2 = bez.P[2];
  const Vec2& P3 = bez.P[3];

  // positions
  h.Q[0] = P0;
  h.Q[3] = P3;

  // canonical cubic Bézier -> tangent relation:
  // C'(0) = 3 (P1 - P0)
  // C'(1) = 3 (P3 - P2)
  Vec2 M0 = 3.0 * (P1 - P0);
  Vec2 M1 = 3.0 * (P3 - P2);

  // For standard Hermite basis we use Q = {P0, M0, P3, M1}
  // but we keep the layout Q0,Q1,Q2,Q3 = P0, M0, M1, P3 for clarity
  h.Q[1] = M0;
  h.Q[2] = M1;

  return h;
}

CubicBezier2D hermiteToBezier(const CubicHermite2D& h)
{
  CubicBezier2D bez;

  const Vec2& P0 = h.Q[0];   // position at t=0
  const Vec2& M0 = h.Q[1];   // tangent at t=0
  const Vec2& M1 = h.Q[2];   // tangent at t=1
  const Vec2& P3 = h.Q[3];   // position at t=1

  // invert the relations:
  // C'(0) = 3 (P1 - P0) = M0  =>  P1 = P0 + M0 / 3
  // C'(1) = 3 (P3 - P2) = M1  =>  P2 = P3 - M1 / 3
  bez.P[0] = P0;
  bez.P[1] = P0 + (1.0 / 3.0) * M0;
  bez.P[2] = P3 - (1.0 / 3.0) * M1;
  bez.P[3] = P3;

  return bez;
}
