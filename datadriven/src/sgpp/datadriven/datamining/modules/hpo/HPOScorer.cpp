/* Copyright (C) 2008-today The SG++ project
 * This file is part of the SG++ project. For conditions of distribution and
 * use, please see the copyright notice provided with SG++ or at
 * sgpp.sparsegrids.org
 *
 * HPOScorer.cpp
 *
 *  Created on:	10.12.2017
 *      Author: Eric Koepke
 */
#include <sgpp/datadriven/datamining/modules/hpo/HPOScorer.hpp>

namespace sgpp {
namespace datadriven {

using base::DataVector;

HPOScorer::HPOScorer(Metric *metric, ShufflingFunctor *shuffling, int64_t seed,
                     double trainPortion, Dataset *testDataset)
    : Scorer{metric, shuffling, seed}, trainPortion{trainPortion}, testDataset{testDataset} {}

void HPOScorer::resizeTrainData(Dataset &original, Dataset &smaller) {
  // perform randomization of indices
  shuffling->setSeed(shuffling->getSeed());
  std::vector<size_t> randomizedIndices(original.getNumberInstances());
  randomizeIndices(original, randomizedIndices);
  Dataset dummyDataset
      {original.getNumberInstances() - smaller.getNumberInstances(), original.getDimension()};
  splitSet(original, smaller, dummyDataset, randomizedIndices);
}

Dataset *HPOScorer::prepareTestData(Dataset &dataset) {
  // perform randomization of indices
  std::vector<size_t> randomizedIndices(dataset.getNumberInstances());
  randomizeIndices(dataset, randomizedIndices);
  // calculate size of testing and training portions
  size_t trainSize = static_cast<size_t>(std::lround(
      static_cast<double>(dataset.getNumberInstances()) * trainPortion));
  size_t testSize = dataset.getNumberInstances() - trainSize;
  size_t dim = dataset.getDimension();

  testDataset = std::make_unique<Dataset>(testSize, dim);
  Dataset *trainDataset = new Dataset{trainSize, dim};
  splitSet(dataset, *trainDataset, *testDataset, randomizedIndices);

  return trainDataset;
}

double HPOScorer::calculateScore(ModelFittingBase &model, Dataset &trainDataset,
                                 double *stdDeviation) {
  bool resetVerbose = model.verboseSolver;
  model.verboseSolver = false;

  model.fit(trainDataset);
  double score = test(model, *testDataset);
  double best = score + 1;
  while (score < best) {
    best = score;
    model.refine();
    score = test(model, *testDataset);
    // std::cout<<"RefinedScore :"<<score<<std::endl;
  }

  model.verboseSolver = resetVerbose;

  if (stdDeviation) {
    *stdDeviation = 0;
  }

  return best;
}
Scorer *HPOScorer::clone() const {
  return nullptr;
}
} /* namespace datadriven */
} /* namespace sgpp */