/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2009-2010 Alexander Heinecke (Alexander.Heinecke@mytum.de)  */
/*               2010      Stefanie Schraufstetter (schraufs@in.tum.de)      */
/*                                                                           */
/* sgpp is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU Lesser General Public License as published  */
/* by the Free Software Foundation; either version 3 of the License, or      */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sgpp is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with sgpp; if not, write to the Free Software                       */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#include "basis/linear/boundary/operation/pde/finance/OperationGammaLogLinearBoundary.hpp"

#include "basis/linear/boundary/algorithm_sweep/PhiPhiDownBBLinearBoundary.hpp"
#include "basis/linear/boundary/algorithm_sweep/PhiPhiUpBBLinearBoundary.hpp"

#include "basis/linear/boundary/algorithm_sweep/PhidPhiDownBBLinearBoundary.hpp"
#include "basis/linear/boundary/algorithm_sweep/PhidPhiUpBBLinearBoundary.hpp"

#include "basis/linear/boundary/algorithm_sweep/DPhiPhiDownBBLinearBoundary.hpp"
#include "basis/linear/boundary/algorithm_sweep/DPhiPhiUpBBLinearBoundary.hpp"

#include "basis/linear/boundary/algorithm_sweep/DPhidPhiDownBBLinearBoundary.hpp"
#include "basis/linear/boundary/algorithm_sweep/DPhidPhiUpBBLinearBoundary.hpp"

#include "algorithm/common/sweep.hpp"

#include <iostream>

namespace sg
{

OperationGammaLogLinearBoundary::OperationGammaLogLinearBoundary(GridStorage* storage, DataVector& coef) : UpDownTwoOpDims(storage, coef)
{
}

OperationGammaLogLinearBoundary::~OperationGammaLogLinearBoundary()
{
}

void OperationGammaLogLinearBoundary::up(DataVector& alpha, DataVector& result, size_t dim)
{
	// phi * phi
	detail::PhiPhiUpBBLinearBoundary func(this->storage);
	sweep<detail::PhiPhiUpBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::down(DataVector& alpha, DataVector& result, size_t dim)
{
	// phi * phi
	detail::PhiPhiDownBBLinearBoundary func(this->storage);
	sweep<detail::PhiPhiDownBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::upOpDimOne(DataVector& alpha, DataVector& result, size_t dim)
{
	// phi * dphi
	detail::PhidPhiUpBBLinearBoundary func(this->storage);
	sweep<detail::PhidPhiUpBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::downOpDimOne(DataVector& alpha, DataVector& result, size_t dim)
{
	// phi * dphi
	detail::PhidPhiDownBBLinearBoundary func(this->storage);
	sweep<detail::PhidPhiDownBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::upOpDimTwo(DataVector& alpha, DataVector& result, size_t dim)
{
	// dphi * phi
	detail::DPhiPhiUpBBLinearBoundary func(this->storage);
	sweep<detail::DPhiPhiUpBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::downOpDimTwo(DataVector& alpha, DataVector& result, size_t dim)
{
	// dphi * phi
	detail::DPhiPhiDownBBLinearBoundary func(this->storage);
	sweep<detail::DPhiPhiDownBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::upOpDimOneAndOpDimTwo(DataVector& alpha, DataVector& result, size_t dim)
{
	// dphi * dphi
	detail::DPhidPhiUpBBLinearBoundary func(this->storage);
	sweep<detail::DPhidPhiUpBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

void OperationGammaLogLinearBoundary::downOpDimOneAndOpDimTwo(DataVector& alpha, DataVector& result, size_t dim)
{
	// dphi * dphi
	detail::DPhidPhiDownBBLinearBoundary func(this->storage);
	sweep<detail::DPhidPhiDownBBLinearBoundary> s(func, this->storage);

	s.sweep1D_Boundary(alpha, result, dim);
}

}
