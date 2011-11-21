/*
 * CombiBasisFunction.hpp
 *
 *  Created on: Feb 21, 2011
 *      Author: benk
 */

#ifndef COMBIBASISFUNCTION_HPP_
#define COMBIBASISFUNCTION_HPP_

#include "combigrid/utils/combigrid_ultils.hpp"

namespace combigrid{

   /** The basis function for 1D Cell, the generalization for ND is simply
    * the tensor product. This class contains two methods since in a 1D cell
    * we have two points at the end of the cell, and the two methods returns
    * the component of the first point and the second returns the component
    * of the second point to the evaluation point <br>
    * All the evaluations should be done on the reference cell [0,1], except
    * the extrapolation, that can be [-1,2]. */
   class BasisFunctionBasis {
   public:

	   /** empty Ctror */
	   BasisFunctionBasis() {;}

	   /** first method which returns the contribution of the first point in the 1D cell
	    * @param coord  1D coordonate idealy should be [0,1] but for extrapolation could be different [-1,2]*/
	   virtual double functionEval1(double coord) const = 0;

	   /** second method which returns the contribution of the second point in the 1D cell
	    * @param coord  1D coordonate idealy should be [0,1] but for extrapolation could be different [-1,2]*/
	   virtual double functionEval2(double coord) const = 0;

   };
}

#endif /* COMBIBASISFUNCTION_HPP_ */
