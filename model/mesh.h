#include <vector>
#include "hermite.h"

class Mesh
{
public:
  void setPaperTestMesh();
  void setBasicTestMesh();
  void setAppleMesh();
  void updateDerivatives();
  std::vector<GradientPatch> calculatePatches() const;
  std::vector<std::vector<ControlPoint>> points;
  int rows() const { return m_rows; }
  int cols() const { return m_cols; }
private:
  int m_rows = 0;
  int m_cols = 0;
};

