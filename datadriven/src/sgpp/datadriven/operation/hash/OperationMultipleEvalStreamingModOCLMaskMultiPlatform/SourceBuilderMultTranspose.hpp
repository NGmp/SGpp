// Copyright (C) 2008-today The SG++ project
// This file is part of the SG++ project. For conditions of distribution and
// use, please see the copyright notice provided with SG++ or at
// sgpp.sparsegrids.org

#pragma once

#include <fstream>
#include <memory>
#include <string>

#include "sgpp/base/exception/operation_exception.hpp"
#include "sgpp/base/opencl/OCLOperationConfiguration.hpp"
#include "sgpp/base/opencl/OCLDevice.hpp"
#include "sgpp/base/opencl/KernelSourceBuilderBase.hpp"

namespace SGPP {
namespace datadriven {
namespace StreamingModOCLMaskMultiPlatform {

template <typename T>
class SourceBuilderMultTranspose : public base::KernelSourceBuilderBase<T> {
 private:
  std::shared_ptr<base::OCLDevice> device;

  json::Node &kernelConfiguration;

  size_t dims;

  size_t localWorkgroupSize;
  bool useLocalMemory;
  size_t transGridBlockSize;
  uint64_t maxDimUnroll;

  std::string getLevel(std::string dim, size_t gridBlockingIndex) {
    std::stringstream output;
    if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("array") == 0) {
      output << "level_" << gridBlockingIndex << "[" << dim << "]";
    } else if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("register") == 0) {
      output << "level_" << gridBlockingIndex << "_" << dim;
    } else if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("pointer") == 0) {
      output << "ptrLevel[dimLevelIndex]";
    } else {
      throw new base::operation_exception(
          "OCL error: Illegal value for parameter \"KERNEL_STORE_DATA\"\n");
    }
    return output.str();
  }

  std::string getIndex(std::string dim, size_t gridBlockingIndex) {
    std::stringstream output;
    if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("array") == 0) {
      output << "index_" << gridBlockingIndex << "[" << dim << "]";
    } else if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("register") == 0) {
      output << "index_" << gridBlockingIndex << "_" << dim;
    } else if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("pointer") == 0) {
      output << "ptrIndex[dimLevelIndex]";
    } else {
      throw new base::operation_exception(
          "OCL error: Illegal value for parameter \"KERNEL_STORE_DATA\"\n");
    }
    return output.str();
  }

  std::string getData(std::string dim, size_t dataBlockingIndex) {
    std::stringstream output;
    if (kernelConfiguration["KERNEL_USE_LOCAL_MEMORY"].getBool()) {
      output << "locData[(" << dim << " * " << localWorkgroupSize << ") + k]";
    } else {
      output << "ptrData[(" << dim << " * sourceSize) + k]";
    }
    return output.str();
  }

  std::string unrolledBasisFunctionEvalulation(size_t dims, size_t startDim, size_t endDim,
                                               std::string unrollVariable) {
    std::stringstream output;

    for (size_t d = startDim; d < endDim; d++) {
      std::stringstream dimElement;
      dimElement << "(";
      if (!unrollVariable.compare("") == 0) {
        dimElement << unrollVariable << " + ";
      }
      dimElement << d;
      dimElement << ")";
      std::string pointerAccess = dimElement.str();

      std::string dString;
      if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("register") == 0) {
        std::stringstream stream;
        stream << (d);
        dString = stream.str();
      } else {
        dString = pointerAccess;
      }

      // TODO(pfandedd): replace
      for (size_t gridIndex = 0; gridIndex < transGridBlockSize; gridIndex++) {
        output << this->indent[2] << "curSupport_" << gridIndex << " *= fmax(1.0"
               << this->constSuffix() << " - fabs((";
        output << getLevel(dString, gridIndex) << " * " << getData(dString, 0) << ") - "
               << getIndex(dString, gridIndex) << "), 0.0" << this->constSuffix() << ");"
               << std::endl;
      }
    }
    return output.str();
  }

 public:
  SourceBuilderMultTranspose(std::shared_ptr<base::OCLDevice> device,
                             json::Node &kernelConfiguration, size_t dims)
      : device(device), kernelConfiguration(kernelConfiguration), dims(dims) {
    localWorkgroupSize = kernelConfiguration["LOCAL_SIZE"].getUInt();
    useLocalMemory = kernelConfiguration["KERNEL_USE_LOCAL_MEMORY"].getBool();
    transGridBlockSize = kernelConfiguration["KERNEL_TRANS_GRID_BLOCK_SIZE"].getUInt();
    maxDimUnroll = kernelConfiguration["KERNEL_MAX_DIM_UNROLL"].getUInt();
  }

  std::string generateSource() {
    if (kernelConfiguration["REUSE_SOURCE"].getBool()) {
      return this->reuseSource("streamingModOCLMaskMP_multTranspose.cl");
    }

    std::stringstream sourceStream;

    sourceStream << "// platform: " << device->platformName << " device: " << device->deviceName
                 << std::endl
                 << std::endl;

    if (std::is_same<T, double>::value) {
      sourceStream << "#pragma OPENCL EXTENSION cl_khr_fp64 : enable" << std::endl
                   << std::endl;
    }

    sourceStream << "__kernel" << std::endl;
    sourceStream << "__attribute__((reqd_work_group_size(" << localWorkgroupSize << ", 1, 1)))"
                 << std::endl;
    sourceStream << "void multTransOCLMask(__global const " << this->floatType() << "* ptrLevel,"
                 << std::endl;
    sourceStream << "           __global const " << this->floatType() << "* ptrIndex," << std::endl;
    sourceStream << "           __global const " << this->floatType() << "* ptrMask," << std::endl;
    sourceStream << "           __global const " << this->floatType() << "* ptrOffset,"
                 << std::endl;
    sourceStream << "           __global const " << this->floatType() << "* ptrData," << std::endl;
    sourceStream << "           __global const " << this->floatType() << "* ptrSource,"
                 << std::endl;
    sourceStream << "           __global       " << this->floatType() << "* ptrResult,"
                 << std::endl;
    sourceStream << "           uint start_data," << std::endl;
    sourceStream << "           uint end_data)" << std::endl;
    sourceStream << "{" << std::endl;
    sourceStream << this->indent[0] << "int globalIdx = get_global_id(0);" << std::endl;
    sourceStream << this->indent[0] << "int localIdx = get_local_id(0);" << std::endl;
    sourceStream << this->indent[0] << "int groupSize = get_local_size(0);" << std::endl;
    sourceStream << this->indent[0] << "int globalSize = get_global_size(0);" << std::endl;
    sourceStream << "   uint rangeData = end_data - start_data;" << std::endl;

    sourceStream << std::endl;
    sourceStream << "   " << this->floatType()
                 << " eval, index_calc, abs, last, localSupport, curSupport;" << std::endl
                 << std::endl;

    for (size_t gridIndex = 0; gridIndex < transGridBlockSize; gridIndex++) {
      sourceStream << this->indent[0] << this->floatType() << " myResult_" << gridIndex << " = 0.0;"
                   << std::endl;
    }
    sourceStream << std::endl;

    if (useLocalMemory) {
      sourceStream << this->indent[0] << "__local " << this->floatType() << " locData["
                   << dims * localWorkgroupSize << "];" << std::endl;
      sourceStream << this->indent[0] << "__local " << this->floatType() << " locSource["
                   << localWorkgroupSize << "];" << std::endl
                   << std::endl;
    }

    //    for (size_t d = 0; d < dims; d++) {
    //      sourceStream << " " << this->floatType() << " level_" << d << " = ptrLevel[(globalIdx*"
    //                   << dims << ")+" << d << "];" << std::endl;
    //      sourceStream << " " << this->floatType() << " index_" << d << " = ptrIndex[(globalIdx*"
    //                   << dims << ")+" << d << "];" << std::endl;
    //      sourceStream << " " << this->floatType() << " mask_" << d << " = ptrMask[(globalIdx*" <<
    //      dims
    //                   << ")+" << d << "];" << std::endl;
    //      sourceStream << " " << this->floatType() << " offset_" << d << " =
    //      ptrOffset[(globalIdx*"
    //                   << dims << ")+" << d << "];" << std::endl;
    //    }

    // create a register storage for the level and index of the grid points of
    // the work item
    if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("array") == 0) {
      for (size_t gridIndex = 0; gridIndex < transGridBlockSize; gridIndex++) {
        sourceStream << this->indent[0] << this->floatType() << " level_" << gridIndex << "["
                     << dims << "];" << std::endl;
        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << "level_" << gridIndex << "[" << d
                       << "] = ptrLevel[(((globalSize * " << gridIndex << ") + globalIdx) * "
                       << dims << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        sourceStream << this->indent[0] << this->floatType() << " index_" << gridIndex << "["
                     << dims << "];" << std::endl;
        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << "index_" << gridIndex << "[" << d
                       << "] = ptrIndex[(((globalSize * " << gridIndex << ") + globalIdx) * "
                       << dims << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        sourceStream << this->indent[0] << this->floatType() << " mask_" << gridIndex << "[" << dims
                     << "];" << std::endl;
        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << "mask_" << gridIndex << "[" << d
                       << "] = ptrMask[(((globalSize * " << gridIndex << ") + globalIdx) * " << dims
                       << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        sourceStream << this->indent[0] << this->floatType() << " offset_" << gridIndex << "["
                     << dims << "];" << std::endl;
        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << "mask_" << gridIndex << "[" << d
                       << "] = ptrOffset[(((globalSize * " << gridIndex << ") + globalIdx) * "
                       << dims << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;
      }
    } else if (kernelConfiguration["KERNEL_STORE_DATA"].get().compare("register") == 0) {
      for (size_t gridIndex = 0; gridIndex < transGridBlockSize; gridIndex++) {
        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << this->floatType() << " level_" << gridIndex << "_" << d
                       << " = ptrLevel[(((globalSize * " << gridIndex << ") + globalIdx) * " << dims
                       << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << this->floatType() << " index_" << gridIndex << "_" << d
                       << " = ptrIndex[(((globalSize * " << gridIndex << ") + globalIdx) * " << dims
                       << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << this->floatType() << " mask_" << gridIndex << "_" << d
                       << " = ptrMask[(((globalSize * " << gridIndex << ") + globalIdx) * " << dims
                       << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;

        for (size_t d = 0; d < dims; d++) {
          sourceStream << this->indent[0] << this->floatType() << " offset_" << gridIndex << "_"
                       << d << " = ptrOffset[(((globalSize * " << gridIndex << ") + globalIdx) * "
                       << dims << ") + " << d << "];" << std::endl;
        }
        sourceStream << std::endl;
      }
    }

    sourceStream << std::endl;
    sourceStream << "   // Iterate over all grid points" << std::endl;
    if (useLocalMemory) {
      sourceStream << " for(int i = start_data; i < end_data; i+=" << localWorkgroupSize << ")"
                   << std::endl;
      sourceStream << "   {" << std::endl;

      for (size_t d = 0; d < dims; d++) {
        sourceStream << "     locData[(" << d << "*" << localWorkgroupSize
                     << ")+(localIdx)] = ptrData[(" << d << "*rangeData)+(localIdx+i)];"
                     << std::endl;
      }

      sourceStream << "       locSource[localIdx] = ptrSource[i+localIdx];" << std::endl;
      sourceStream << "       barrier(CLK_LOCAL_MEM_FENCE);" << std::endl
                   << std::endl;
      sourceStream << "       for(int k = 0; k < " << localWorkgroupSize << "; k++)" << std::endl;
      sourceStream << "       {" << std::endl;

      sourceStream << "           curSupport = locSource[k];" << std::endl
                   << std::endl;
    } else {
      sourceStream << "   for(int k = start_data; k < end_data; k++)" << std::endl;
      sourceStream << "   {" << std::endl;
      sourceStream << "     curSupport = ptrSource[k];" << std::endl
                   << std::endl;
    }

    for (size_t d = 0; d < dims; d++) {
      if (useLocalMemory) {
        sourceStream << "         eval = ((level_" << d << ") * (locData[(" << d << "*"
                     << localWorkgroupSize << ")+k]));" << std::endl;
      } else {
        sourceStream << "         eval = ((level_" << d << ") * (ptrData[(" << d
                     << "*rangeData)+k]));" << std::endl;
      }
      sourceStream << "         index_calc = eval - (index_" << d << ");" << std::endl;
      sourceStream << "         abs = as_" << this->floatType() << "(as_" << this->intType()
                   << "(index_calc) | as_" << this->intType() << "(mask_" << d << "));"
                   << std::endl;
      sourceStream << "         last = offset_" << d << " + abs;" << std::endl;
      sourceStream << "         localSupport = fmax(last, 0.0" << this->constSuffix() << ");"
                   << std::endl;
      sourceStream << "         curSupport *= localSupport;" << std::endl;
    }

    sourceStream << std::endl
                 << "      myResult += curSupport;" << std::endl;
    sourceStream << "       }" << std::endl
                 << std::endl;

    if (useLocalMemory) {
      sourceStream << this->indent[1] << "barrier(CLK_LOCAL_MEM_FENCE);" << std::endl;
      sourceStream << this->indent[0] << "}" << std::endl;
    }

    //    sourceStream << "   ptrResult[globalIdx] = myResult;" << std::endl;
    //    sourceStream << "}" << std::endl;

    for (size_t gridIndex = 0; gridIndex < transGridBlockSize; gridIndex++) {
      sourceStream << this->indent[0] << "ptrResult[(globalSize * " << gridIndex
                   << ") + globalIdx] = myResult_" << gridIndex << ";" << std::endl;
    }
    sourceStream << "}" << std::endl;

    if (kernelConfiguration["WRITE_SOURCE"].getBool()) {
      this->writeSource("streamingModOCLMaskMP_multTranspose.cl", sourceStream.str());
    }

    return sourceStream.str();
  }
};

}  // namespace StreamingModOCLMaskMultiPlatform
}  // namespace datadriven
}  // namespace SGPP
