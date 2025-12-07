struct Vec2
{
  double x = 0.0;
  double y = 0.0;

  Vec2() = default;
  Vec2(double x_, double y_) : x(x_), y(y_) {}
};
inline Vec2 operator+(const Vec2& a, const Vec2& b) { return {a.x + b.x, a.y + b.y}; }
inline Vec2 operator-(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }
inline Vec2 operator*(double s, const Vec2& v)      { return {s * v.x, s * v.y}; }
inline Vec2 operator*(const Vec2& v, double s)      { return {s * v.x, s * v.y}; }

struct Vec3
{
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;

  Vec3() = default;
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_){}
};
inline Vec3 operator+(const Vec3& a, const Vec3& b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }
inline Vec3 operator-(const Vec3& a, const Vec3& b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }
inline Vec3 operator*(double s, const Vec3& v)      { return {s * v.x, s * v.y, s * v.z}; }
inline Vec3 operator*(const Vec3& v, double s)      { return {s * v.x, s * v.y, s * v.z}; }

struct ColorRGB
{
  double r = 0.0;  // in [0,1]
  double g = 0.0;
  double b = 0.0;

  ColorRGB() = default;
  ColorRGB(double r_, double g_, double b_) : r(r_), g(g_), b(b_) {}
};

inline ColorRGB operator+(const ColorRGB& a, const ColorRGB& b) { return {a.r + b.r, a.g + b.g, a.b + b.b}; }
inline ColorRGB operator-(const ColorRGB& a, const ColorRGB& b) { return {a.r - b.r, a.g - b.g, a.b - b.b}; }
inline ColorRGB operator*(const double& a, const ColorRGB& b) { return {a * b.r, a * b.g, a * b.b}; }
inline ColorRGB operator*(const ColorRGB& b, const double& a) { return {a * b.r, a * b.g, a * b.b}; }
