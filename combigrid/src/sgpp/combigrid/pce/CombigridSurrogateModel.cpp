// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/pce/CombigridSurrogateModel.hpp>

#include <sgpp/base/exception/application_exception.hpp>

namespace sgpp {
namespace combigrid {

void CombigridSurrogateModelConfiguration::loadFromCombigridOperation(
    std::shared_ptr<CombigridOperation> op) {
  storage = op->getStorage();
  pointHierarchies = op->getPointHierarchies();
  levelStructure = op->getLevelManager()->getLevelStructure();
}

void CombigridSurrogateModelConfiguration::loadFromCombigridOperation(
    std::shared_ptr<CombigridMultiOperation> op) {
  storage = op->getStorage();
  pointHierarchies = op->getPointHierarchies();
  levelStructure = op->getLevelManager()->getLevelStructure();
}

void CombigridSurrogateModelConfiguration::loadFromCombigridOperation(
    std::shared_ptr<CombigridTensorOperation> op) {
  storage = op->getStorage();
  pointHierarchies = op->getPointHierarchies();
  levelStructure = op->getLevelManager()->getLevelStructure();
}

CombigridSurrogateModel::CombigridSurrogateModel(
    sgpp::combigrid::CombigridSurrogateModelConfiguration& config)
    : config(config), numDims(0) {
  // initialize number of dimensions
  if (config.pointHierarchies.size() > 0) {
    numDims = config.pointHierarchies.size();
  } else {
    throw sgpp::base::application_exception(
        "CombigridSurrogateModel: number of dimensions is unknown. Setting the point hierarchies "
        "is required.");
  }

  if ((config.basisFunctions.size() > 0 && config.basisFunctions.size() != numDims) ||
      (config.weightFunctions.size() > 0 && config.weightFunctions.size() != numDims) ||
      (config.bounds.size() > 0 && static_cast<size_t>(config.bounds.size() / 2) != numDims)) {
    throw sgpp::base::application_exception(
        "CombigridSurrogateModel: number of dimensions do not match.");
  }
}

CombigridSurrogateModel::~CombigridSurrogateModel() {}

sgpp::combigrid::CombigridSurrogateModelConfiguration& CombigridSurrogateModel::getConfig() {
  return config;
}

} /* namespace combigrid */
} /* namespace sgpp */
