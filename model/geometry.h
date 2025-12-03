#ifndef GEOMETRY_H
#define GEOMETRY_H

#endif // GEOMETRY_H
#pragma once

#include <array>
#include <cmath>

// ---------- basic vector types ----------
struct Vec2
{
  double x = 0.0;
  double y = 0.0;

  Vec2() = default;
  Vec2(double x_, double y_) : x(x_), y(y_) {}
};

struct Vec3
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  Vec3() = default;
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
};

// simple operators (inline to keep it header-only)
inline Vec2 operator+(const Vec2& a, const Vec2& b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(double s, const Vec2& v)      { return {s * v.x, s * v.y}; }
inline Vec2 operator*(const Vec2& v, double s)      { return {s * v.x, s * v.y}; }

inline Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(double s, const Vec3& v)      { return {s * v.x, s * v.y, s * v.z}; }
inline Vec3 operator*(const Vec3& v, double s)      { return {s * v.x, s * v.y, s * v.z}; }

// ---------- colour ----------

struct ColorRGB
{
  double r = 0.0;  // in [0,1]
  double g = 0.0;
  double b = 0.0;

  ColorRGB() = default;
  ColorRGB(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}
};

// Control point = 2D position + colour at that point
struct ControlPoint
{
  Vec2     pos;   // geometry in xy-plane
  ColorRGB col;   // RGB colour
};

// ---------- cubic basis functions ----------

namespace cubic
{
// Bernstein basis (Bézier) for degree 3
std::array<double,4> bernstein(double t);

// Hermite basis for degree 3 (as in the paper) :contentReference[oaicite:0]{index=0}
std::array<double,4> hermite(double t);

// Derivatives of the bases (for tangents, etc.)
std::array<double,4> bernsteinDeriv(double t);
std::array<double,4> hermiteDeriv(double t);
}

// ---------- curve representations ----------

struct CubicBezier2D
{
  std::array<Vec2,4> P;  // P0..P3

  Vec2 evaluate(double t) const;
  Vec2 derivative(double t) const;
};

struct CubicHermite2D
{
  // Q0, Q3: points
  // Q1, Q2: (scaled) tangent vectors, following Eq. (3) mapping from Bézier :contentReference[oaicite:1]{index=1}
  std::array<Vec2,4> Q;

  Vec2 evaluate(double t) const;
  Vec2 derivative(double t) const;
};

// Conversion between Bézier <-> Hermite for geometry curves (Eq. (1)–(3) in the paper) :contentReference[oaicite:2]{index=2}
CubicHermite2D bezierToHermite(const CubicBezier2D& bezier);
CubicBezier2D  hermiteToBezier(const CubicHermite2D& hermite);
