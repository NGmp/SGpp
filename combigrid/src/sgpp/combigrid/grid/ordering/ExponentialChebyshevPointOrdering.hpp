// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <sgpp/combigrid/grid/ordering/AbstractPointOrdering.hpp>

#include <vector>

namespace sgpp {
namespace combigrid {

/**
 * PointOrdering class with the slowest growth (n = 3^l) possible in order to make Chebyshev points
 * nested.
 */
class ExponentialChebyshevPointOrdering : public AbstractPointOrdering {
 public:
  virtual ~ExponentialChebyshevPointOrdering();

  virtual size_t convertIndex(size_t level, size_t numPoints, size_t index);

  virtual size_t numPoints(size_t level);

  virtual std::shared_ptr<AbstractPermutationIterator> getSortedPermutationIterator(
      size_t level, std::vector<double> const &points, size_t numPoints);
};

} /* namespace combigrid */
} /* namespace sgpp*/
