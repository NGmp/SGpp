/* Copyright (C) 2008-today The SG++ project
 *
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * HPOScorer.hpp
 *
 *  Created on:	10.12.2017
 *      Author: Eric Koepke
 */

#pragma once

#include <sgpp/datadriven/datamining/modules/scoring/Scorer.hpp>


namespace sgpp {
namespace datadriven {

/**
 * Implements the main code for optimizing hyperparameters.
 */
class HPOScorer : public Scorer {
 public:
  /**
   * Constructor
   *
   * @param metric  #sgpp::datadriven::Metric to to quantify approximation quality of a trained
   * model. Scorer will take ownership of this object.
   * @param shuffling #sgpp::datadriven::ShufflingFunctor to rearrange samples of a dataset in the
   * desired manner, ready to be split into testing and training sets. Scorer will take ownership of
   * this object.
   * @param seed seed for randomization in #sgpp::datadriven::ShufflingFunctor. Default is -1 which
   * puts a random seed.
   * @param trainPortion value between 0 and 1 to specify the ratio between testing set and
   * training set.
   */
  HPOScorer(Metric* metric, ShufflingFunctor* shuffling, int64_t seed,
            double trainPortion);

  Scorer* clone() const override;

  /**
   * Train and test a model on a dataset and provide a score to quantify the approximation quality.
   * Standard deviation is 0 as we only train and test one model.
   * @param model A model to be fitted on the training part of the dataset.
   * @param dataset Set of samples to use for fitting and testing the model.
   * @param stdDeviation return standard deviation. Will always be 0.
   * @return accuracy of the fit as calculated by the #metric provided.
   */
  double calculateScore(ModelFittingBase& model, Dataset& dataset,
                        double* stdDeviation = nullptr) override;
 
 private:
  /**
   * value between 0 and 1 to specify the ration between testing set and
   * training set.
   */
  double trainPortion;

};

} /* namespace datadriven */
} /* namespace sgpp */
