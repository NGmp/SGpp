// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef UPDOWNONEOPDIMENHANCED_HPP
#define UPDOWNONEOPDIMENHANCED_HPP

#include <sgpp/base/grid/GridStorage.hpp>
#include <sgpp/base/operation/hash/OperationMatrix.hpp>
#include <sgpp/base/datatypes/DataVector.hpp>
#include <sgpp/base/datatypes/DataMatrix.hpp>

#ifndef TASKS_PARALLEL_UPDOWN
#define TASKS_PARALLEL_UPDOWN 4
#endif

#include <sgpp/globaldef.hpp>

#include <vector>

namespace sgpp {
namespace pde {

/**
 * Implements a modified Up/Down Schema with one operation dim. All
 * $d$ Up/Downs are executed concurrently and the results are merged
 * afterwards.
 *
 */
class UpDownOneOpDimEnhanced : public sgpp::base::OperationMatrix {
 public:
  /**
   * Constructor
   *
   * @param storage the grid's sgpp::base::GridStorage object
   * @param coef reference to a sgpp::base::DataVector object that contains the bilinear form's
   * constant coefficients; one per dimension
   */
  UpDownOneOpDimEnhanced(sgpp::base::GridStorage* storage, sgpp::base::DataVector& coef);

  /**
   * Constructor
   *
   * @param storage the grid's sgpp::base::GridStorage object
   */
  explicit UpDownOneOpDimEnhanced(sgpp::base::GridStorage* storage);

  /**
   * Destructor
   */
  virtual ~UpDownOneOpDimEnhanced();

  virtual void mult(sgpp::base::DataVector& alpha, sgpp::base::DataVector& result);

  /**
   * this functions provides the same functionality as the normal mult routine.
   * However, it doesn't set up an OpenMP task initialization as the mult routine.
   * This method has to be called within a OpenMP task parallelized region.
   *
   * Using this function is useful in following case: Assuming the solver of a certain
   * requires several operators in the space discretization (e.g. Black Scholes Equations)
   * this method can be used to parallelize their calculation which might results results
   * in a better parallel efficiency on systems with 4 or more cores hence fewer barriers
   * are needed.
   *
   * @param alpha vector of coefficients
   * @param result vector to store the results in
   */
  void multParallelBuildingBlock(sgpp::base::DataVector& alpha, sgpp::base::DataVector& result);

 protected:
  typedef sgpp::base::GridStorage::grid_iterator grid_iterator;

  /// Pointer to the grid's storage object
  sgpp::base::GridStorage* storage;
  /// Pointer to the sgpp::base::DataVector of the coefs
  sgpp::base::DataVector* coefs;
  /// algorithmic dimensions, operator is applied in this dimensions
  const std::vector<size_t> algoDims;
  /// number of algorithmic dimensions
  const size_t numAlgoDims_;
  /// max number of parallel stages (dimension recursive calls)
  static const size_t maxParallelDims_ = TASKS_PARALLEL_UPDOWN;

  /**
   * Recursive procedure for updown
   *
   * @param dim the current dimension
   * @param alpha matrix of coefficients
   * @param result matrix to store the results of all dimensions
   */
  void updown(sgpp::base::DataMatrix& alpha, sgpp::base::DataMatrix& result, size_t dim);

  /**
   * 1D up Operation
   *
   * @param dim dimension in which to apply the up-part
   * @param alpha matrix of coefficients
   * @param result matrix to store the results of all dimensions
   */
  virtual void up(sgpp::base::DataMatrix& alpha, sgpp::base::DataMatrix& result, size_t dim) = 0;

  /**
   * 1D down Operation
   *
   * @param dim dimension in which to apply the down-part
   * @param alpha matrix of coefficients
   * @param result matrix to store the results of all dimensions
   */
  virtual void down(sgpp::base::DataMatrix& alpha, sgpp::base::DataMatrix& result, size_t dim) = 0;
};

}  // namespace pde
}  // namespace sgpp

#endif /* STDUPDOWN_HPP */
