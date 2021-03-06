// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#ifndef OPERATIONDENSITYSAMPLINGLINEAR_HPP
#define OPERATIONDENSITYSAMPLINGLINEAR_HPP

#include <sgpp/base/grid/Grid.hpp>
#include <sgpp/datadriven/operation/hash/simple/OperationDensitySampling.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace datadriven {

/**
 * keep applying marginalize to function until it's reduced to only 1 dimension
 */

class OperationDensitySamplingLinear : public OperationDensitySampling {
 public:
  explicit OperationDensitySamplingLinear(base::Grid* grid) : grid(grid) {}
  virtual ~OperationDensitySamplingLinear() {}

  /**
   * Sampling with mixed starting dimensions
   *
   * @param alpha Coefficient vector for current grid
   * @param samples Output DataMatrix (rows: # of samples, columns: # of dims)
   * @param num_samples # of samples to draw
   */
  void doSampling(base::DataVector* alpha, base::DataMatrix*& samples, size_t num_samples);

  /**
   * Sampling with specified starting dimension
   *
   * @param alpha Coefficient vector for current grid
   * @param samples Output DataMatrix (rows: # of samples, columns: # of dims)
   * @param num_samples # of samples to draw
   * @param dim_x Starting dimension
   */
  void doSampling(base::DataVector* alpha, base::DataMatrix*& samples, size_t num_samples,
                  size_t dim_x);

 protected:
  base::Grid* grid;
  void doSampling_start_dimX(base::Grid* g_in, base::DataVector* a_in, size_t dim_start,
                             base::DataVector*& sampleVec, unsigned int* seedp);
  void doSampling_in_next_dim(base::Grid* g_in, base::DataVector* a_in, size_t dim_x,
                              base::DataVector*& sampleVec, size_t& curr_dim, unsigned int* seedp);
};
}  // namespace datadriven
}  // namespace sgpp
#endif /* OPERATIONDENSITYSAMPLINGLINEAR_HPP */
