#include <vector>
#include "hermite.h"

class Mesh
{
public:
  void setPaperTestMesh();
  void setBasicTestMesh();
  void setAppleMesh();
  void setSharpMesh();
  void updateDerivatives();
  std::vector<GradientPatch> calculatePatches() const;
  std::vector<std::vector<ControlPoint>> points;
  int rows() const { return m_rows; }
  int cols() const { return m_cols; }
private:
  int m_rows = 0;
  int m_cols = 0;
  std::array<std::array<ColorRGB, 4>, 4> oneCol(ColorRGB col);
  std::array<std::array<ColorRGB, 4>, 4> difCol(ColorRGB c0, ColorRGB c1, ColorRGB c2, ColorRGB c3);

};

