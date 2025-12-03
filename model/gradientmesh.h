#pragma once

#include <vector>
#include "geometry.h"
#include "patch.h"

// Simple regular gradient mesh (rectangular topology, Sect. 3)
class GradientMesh
{
public:
  GradientMesh() = default;
  GradientMesh(int rows, int cols);

  int rows() const { return m_rows; }
  int cols() const { return m_cols; }

  ControlPoint& at(int row, int col);
  const ControlPoint& at(int row, int col) const;

  // Build one bicubic patch per quad cell
  std::vector<GradientPatch> buildPatches() const;

  // Recompute per-vertex derivatives (call after changing positions/colours)
  void updateDerivatives();

  // Helper: create a unit-square mesh with some simple colour pattern
  static GradientMesh makeRegularTestMesh(int rows, int cols);
  static GradientMesh makePaperTestMesh();

private:
  int m_rows = 0;
  int m_cols = 0;
  std::vector<ControlPoint> m_points; // size = rows * cols

  // per-vertex derivatives for geometry
  std::vector<Vec2> m_duGeom; // derivative in "u" / horizontal direction
  std::vector<Vec2> m_dvGeom; // derivative in "v" / vertical direction

  // per-vertex derivatives for colour
  std::vector<ColorRGB> m_duColor;
  std::vector<ColorRGB> m_dvColor;

  int index(int row, int col) const { return row * m_cols + col; }
};
