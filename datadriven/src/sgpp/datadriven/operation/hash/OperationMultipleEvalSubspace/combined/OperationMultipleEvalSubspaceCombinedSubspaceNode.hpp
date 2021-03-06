// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <limits>
#include <vector>

#include <omp.h>

#include <sgpp/base/datatypes/DataVector.hpp>

#include <sgpp/globaldef.hpp>

namespace sgpp {
namespace datadriven {

class SubspaceNodeCombined {
 public:
  enum class SubspaceType { NOT_SET, ARRAY, LIST };

  std::vector<uint32_t> level;
  std::vector<uint32_t> hInverse;
  uint32_t gridPointsOnLevel;
  uint32_t existingGridPointsOnLevel;
  SubspaceType type;
  std::vector<uint32_t> indices;  // for list representation (and future streaming subspaces
  std::vector<std::pair<uint32_t, double> > indexFlatSurplusPairs;
  double* subspaceArray;
  omp_lock_t subspaceLock;

  uint32_t jumpTargetIndex;
  uint32_t flatLevel;

  // every node that reaches this subspace has to calculate this diff
  uint32_t arriveDiff;

  SubspaceNodeCombined(std::vector<uint32_t>& level, uint32_t flatLevel,
                       std::vector<uint32_t>& hInverse, std::vector<uint32_t>& index) {
    size_t dim = level.size();
    this->level = level;
    this->hInverse = hInverse;
    this->indices = index;

    // exactly one gp is added in the loop above
    this->existingGridPointsOnLevel = 1;
    this->flatLevel = flatLevel;
    this->type = SubspaceType::NOT_SET;

    this->gridPointsOnLevel = 1;

    for (size_t j = 0; j < dim; j++) {
      uint32_t dimTemp = hInverse[j];
      dimTemp >>= 1;  // skip even indices
      this->gridPointsOnLevel *= dimTemp;
    }

    this->subspaceArray = nullptr;

    // initalize other member variables with dummies
    this->jumpTargetIndex = 9999;
    this->arriveDiff = 9999;

    // initialize the lock for this subspace
    omp_init_lock(&this->subspaceLock);
  }

  SubspaceNodeCombined(size_t dim, uint32_t index) {
    for (size_t i = 0; i < dim; i++) {
      this->level.push_back(1);
      this->hInverse.push_back(2);
    }

    this->gridPointsOnLevel = 0;
    this->existingGridPointsOnLevel = 0;
    this->flatLevel = 0;
    this->type = SubspaceType::NOT_SET;

    // initalize other member variables with dummies
    this->jumpTargetIndex = index;
    this->arriveDiff = 9999;

    this->subspaceArray = nullptr;
  }

  ~SubspaceNodeCombined() {
    if (this->type == SubspaceType::ARRAY) {
      delete[] this->subspaceArray;
    }
  }

  void lockSubspace() { omp_set_lock(&this->subspaceLock); }

  void unlockSubspace() { omp_unset_lock(&this->subspaceLock); }

  // increases number of grid points on the subspace
  void addGridPoint(std::vector<uint32_t>& index) {
    size_t dim = index.size();

    for (size_t i = 0; i < dim; i++) {
      this->indices.push_back(index[i]);
    }

    this->existingGridPointsOnLevel += 1;
  }

  void printLevel() {
    for (size_t i = 0; i < level.size(); i++) {
      if (i > 0) {
        std::cout << ", ";
      }

      std::cout << level[i];
    }

    std::cout << std::endl;
  }

  // unpack has to be called when the subspace is set up (except for surplus valus)
  // this method will decide how to best represent the subspace (list or array type)
  // and prepare the subspace for its representation
  void unpack() {
    double usageRatio = (double)this->existingGridPointsOnLevel / (double)this->gridPointsOnLevel;

    if (usageRatio < X86COMBINED_LIST_RATIO &&
        this->existingGridPointsOnLevel < X86COMBINED_STREAMING_THRESHOLD) {
      this->type = SubspaceType::LIST;
    } else {
      this->type = SubspaceType::ARRAY;

      if (this->subspaceArray == nullptr) {
        this->subspaceArray = new double[this->gridPointsOnLevel];

        for (size_t i = 0; i < this->gridPointsOnLevel; i++) {
          this->subspaceArray[i] = std::numeric_limits<double>::quiet_NaN();
        }
      }
    }
  }

  // the first call initializes the array for ARRAY type subspaces
  //
  void setSurplus(size_t indexFlat, double surplus) {
    if (this->type == SubspaceType::ARRAY) {
      this->subspaceArray[indexFlat] = surplus;
    } else if (this->type == SubspaceType::LIST) {
      bool found = false;

      for (std::pair<uint32_t, double>& tuple : this->indexFlatSurplusPairs) {
        if (tuple.first == indexFlat) {
          tuple.second = surplus;
          found = true;
          break;
        }
      }

      if (!found) {
        this->indexFlatSurplusPairs.emplace_back(std::make_pair(indexFlat, surplus));
      }
    }
  }

  // the first call initializes the array for ARRAY type subspaces
  double getSurplus(size_t indexFlat) {
    if (this->type == SubspaceType::ARRAY) {
      return this->subspaceArray[indexFlat];
    } else if (this->type == SubspaceType::LIST) {
      for (std::pair<uint32_t, double> tuple : this->indexFlatSurplusPairs) {
        if (tuple.first == indexFlat) {
          return tuple.second;
        }
      }
    }

    throw;
  }

  static uint32_t compareLexicographically(SubspaceNodeCombined& current,
                                           SubspaceNodeCombined& last) {
    for (uint32_t i = 0; i < current.level.size(); i++) {
      if (current.level[i] != last.level[i]) {
        return i;
      }
    }

    throw "illegal input";
  }

  static bool subspaceCompare(SubspaceNodeCombined left, SubspaceNodeCombined right) {
    for (size_t i = 0; i < left.level.size(); i++) {
      if (left.level[i] >= right.level[i]) {
        if (left.level[i] > right.level[i]) {
          return 0;
        }
      } else {
        return 1;
      }
    }

    return 1;
  }
};
}
}
