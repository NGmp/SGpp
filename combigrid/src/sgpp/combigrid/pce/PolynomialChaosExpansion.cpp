// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#include <sgpp/combigrid/functions/AbstractInfiniteFunctionBasis1D.hpp>
#include <sgpp/combigrid/functions/OrthogonalBasisFunctionsCollection.hpp>
#include <sgpp/combigrid/operation/CombigridMultiOperation.hpp>
#include <sgpp/combigrid/operation/CombigridOperation.hpp>
#include <sgpp/combigrid/operation/CombigridTensorOperation.hpp>

#include <sgpp/combigrid/operation/multidim/AveragingLevelManager.hpp>

#include <sgpp/combigrid/pce/PolynomialChaosExpansion.hpp>
#include <sgpp/combigrid/pce/SGppToDakota.hpp>

#include <sgpp/base/exception/application_exception.hpp>

#ifdef USE_DAKOTA
#include <pecos_data_types.hpp>
#endif

#include <vector>

namespace sgpp {
namespace combigrid {

PolynomialChaosExpansion::PolynomialChaosExpansion(
    sgpp::combigrid::CombigridSurrogateModelConfiguration& config)
    : CombigridSurrogateModel(config), currentNumGridPoints(0), computedSobolIndicesFlag(false) {
  // create vector of function bases
  if (config.basisFunctions.size() == 0) {
    for (size_t idim = 0; idim < numDims; idim++) {
      this->config.basisFunctions.push_back(config.basisFunction);
    }
  } else if (numDims != config.basisFunctions.size()) {
    throw sgpp::base::application_exception(
        "PolynomialChaosExpansion: number of basis function do not match with the number of "
        "dimensions of the operation");
  }

  updateConfig(config);
}

PolynomialChaosExpansion::~PolynomialChaosExpansion() {}

void PolynomialChaosExpansion::initializeTensorOperation(
    std::vector<std::shared_ptr<AbstractPointHierarchy>> pointHierarchies,
    std::shared_ptr<AbstractCombigridStorage> storage) {
  // create tensor operation for pce transformation
  combigridTensorOperation =
      sgpp::combigrid::CombigridTensorOperation::createOperationTensorPolynomialInterpolation(
          pointHierarchies, storage, config.basisFunctions);

  currentNumGridPoints = 0;
}

bool PolynomialChaosExpansion::updateStatus() {
  if (currentNumGridPoints < combigridTensorOperation->numGridPoints()) {
    expansionCoefficients = combigridTensorOperation->getResult();
    currentNumGridPoints = combigridTensorOperation->numGridPoints();
    computedSobolIndicesFlag = false;
    return true;
  } else {
    return false;
  }
}

double PolynomialChaosExpansion::mean() {
  //  if (orthogPoly != nullptr) {
  // #ifdef USE_DAKOTA
  //    return orthogPoly->mean();
  // #endif
  //  }
  updateStatus();
  return expansionCoefficients.get(MultiIndex(numDims, 0)).getValue();
}

double PolynomialChaosExpansion::variance() {
  //  if (orthogPoly != nullptr) {
  // #ifdef USE_DAKOTA
  //    return orthogPoly->variance();
  // #endif
  //  }
  // PCE norm, i.e. variance (if using appropriate normalized orthogonal polynomials)
  updateStatus();
  double var = 0.0;
  auto it = expansionCoefficients.getValues()->getStoredDataIterator();
  if (!it->isValid()) {
    return 0.0;
  }
  it->moveToNext();  // ignore first entry (belonging to mean)

  for (; it->isValid(); it->moveToNext()) {
    double coeff = it->value().value();
    var += coeff * coeff;
  }
  return var;
}

void PolynomialChaosExpansion::computeComponentSobolIndices() {
  // compute the component sobol indices
  if (computedSobolIndicesFlag) {
    return;
  }

  size_t numSobolIndices = static_cast<size_t>(std::pow(2, numDims) - 1);
  sobolIndices.resizeZero(numSobolIndices);

  // load index vectors
  auto it_coeffs = expansionCoefficients.getValues()->getStoredDataIterator();
  std::vector<MultiIndex> indexList;
  for (; it_coeffs->isValid(); it_coeffs->moveToNext()) {
    indexList.push_back(it_coeffs->getMultiIndex());
  }

  for (size_t i = 0; i < numSobolIndices; i++) {
    // loop over all the remaining basis functions
    for (std::vector<MultiIndex>::iterator it_ixlist = indexList.begin();
         it_ixlist != indexList.end();) {
      MultiIndex multiIndex = *it_ixlist;

      // check if all the current dimensions are set
      bool dimsAreSet = true;
      size_t idim = 0;
      while (dimsAreSet && idim < numDims) {
        // get kth bit from permuation mask
        size_t isSet = ((i + 1) & (1 << idim)) >> idim;  // in {0, 1}
        // check if the degree of the basis in the current dimension
        // is > 0 if set and equal to 0 if not set
        if (isSet == 0) {
          dimsAreSet &= multiIndex[idim] == 0;
        } else {
          dimsAreSet &= multiIndex[idim] > 0;
        }
        idim++;
      }

      // add the squared coefficient of the current term to the
      // sobol index matrix if all dimensions are present
      // in the current term
      if (dimsAreSet) {
        // load index and delete it from list since every term
        // can just contribute to one sobol index
        indexList.erase(it_ixlist);
        double coefficient = expansionCoefficients.get(multiIndex).value();
        sobolIndices[i] += coefficient * coefficient;
      } else {
        it_ixlist++;
      }
    }
  }

  // update the computed flag
  computedSobolIndicesFlag = true;
}

void PolynomialChaosExpansion::getComponentSobolIndices(
    sgpp::base::DataVector& componentSobolIndices, bool normalized) {
  updateStatus();
  computeComponentSobolIndices();
  // copy sobol indices to output vector
  componentSobolIndices.resize(sobolIndices.getSize());
  componentSobolIndices.copyFrom(sobolIndices);

  // divide all the entries by the variance to obtain the Sobol indices
  if (normalized) {
    double var = variance();
    if (var > 1e-14) {
      componentSobolIndices.mult(1. / var);
    }
  }
}

void PolynomialChaosExpansion::getTotalSobolIndices(sgpp::base::DataVector& totalSobolIndices,
                                                    bool normalized) {
  updateStatus();
  computeComponentSobolIndices();
  totalSobolIndices.resizeZero(numDims);
  for (size_t idim = 0; idim < numDims; idim++) {
    for (size_t iperm = 0; iperm < sobolIndices.getSize(); iperm++) {
      // check if the current dimension is set in the current key of the sobol index
      size_t isSet = ((iperm + 1) & (1 << idim)) >> idim;  // in {0, 1}
      if (isSet == 1) {
        totalSobolIndices[idim] += sobolIndices[iperm];
      }
    }
  }

  // divide all the entries by the variance to obtain the Sobol indices
  if (normalized) {
    double var = variance();
    if (var > 1e-14) {
      totalSobolIndices.mult(1. / variance());
    }
  }
}

void PolynomialChaosExpansion::updateConfig(
    sgpp::combigrid::CombigridSurrogateModelConfiguration config) {
  // update the tensor operation
  if (config.pointHierarchies.size() == numDims && config.storage != nullptr) {
    initializeTensorOperation(config.pointHierarchies, config.storage);
  }

  if (combigridTensorOperation == nullptr) {
    throw sgpp::base::application_exception(
        "PolynomialChaosExpansion:updateConfig - tensor operation is null -> something is "
        "seriously wrong");
  }

  if (config.levelManager != nullptr) {
    combigridTensorOperation->setLevelManager(config.levelManager);
  }

  if (config.levelStructure != nullptr) {
    combigridTensorOperation->getLevelManager()->addLevelsFromStructure(config.levelStructure);
  }

  if (config.enableLevelManagerStatsCollection) {
    combigridTensorOperation->getLevelManager()->enableStatsCollection();
  }
}

size_t PolynomialChaosExpansion::numGridPoints() { return currentNumGridPoints; }

std::shared_ptr<LevelInfos> PolynomialChaosExpansion::getInfoOnAddedLevels() {
  return combigridTensorOperation->getLevelManager()->getInfoOnAddedLevels();
}

} /* namespace combigrid */
} /* namespace sgpp */
