// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONSECONDMOMENTMODLINEAR_HPP
#define OPERATIONSECONDMOMENTMODLINEAR_HPP

#include <sgpp/base/operation/hash/OperationSecondMoment.hpp>
#include <sgpp/base/grid/Grid.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace base {

/**
 * SecondMomemnt of sparse grid function, linear grid without boundaries
 */
class OperationSecondMomentModLinear : public OperationSecondMoment {
 public:
  /**
   * Constructor of OperationSecondMomentModLinear
   *
   * @param storage Pointer to the grid's GridStorage object
   */
  explicit OperationSecondMomentModLinear(GridStorage& storage) : storage(storage) {}

  ~OperationSecondMomentModLinear() override {}

  /**
   * Compute first moment of the function
   * @f[ \int_{\Omega} x\cdot f(x) dx. @f]
   *
   * @param alpha Coefficient vector for current grid
   * @param bounds describes the boundaries of the hypercube of the original function
   */
  double doQuadrature(DataVector& alpha, DataMatrix* bounds = nullptr) override;

 protected:
  // Pointer to the grid's GridStorage object
  GridStorage& storage;
};

}  // namespace base
}  // namespace sgpp

#endif /* OPERATIONSECONDMOMENTMODLINEAR_HPP */
