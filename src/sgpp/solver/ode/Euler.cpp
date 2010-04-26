/*****************************************************************************/
/* This file is part of sgpp, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2009-2010 Alexander Heinecke (Alexander.Heinecke@mytum.de)  */
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

#include "grid/common/DirichletUpdateVector.hpp"
#include "solver/ode/Euler.hpp"
#include "operation/common/OperationEval.hpp"
#include "tools/common/GridPrinter.hpp"
#include "exception/solver_exception.hpp"

#include <iostream>
#include <string>
#include <sstream>

namespace sg
{

Euler::Euler(std::string Mode, size_t imax, double timestepSize, bool generateAnimation, size_t numEvalsAnimation, ScreenOutput* screen) : ODESolver(imax, timestepSize), bAnimation(generateAnimation), evalsAnimation(numEvalsAnimation), ExMode(Mode), myScreen(screen)
{
	this->residuum = 0.0;

	if (Mode != "ExEul" && Mode != "ImEul")
	{
		throw new solver_exception("Euler::Euler : An unknown Euler-Mode was specified!");
	}
}

Euler::~Euler()
{
}

void Euler::solve(SLESolver& LinearSystemSolver, OperationODESolverSystem& System, bool verbose)
{
	size_t allIter = 0;
    DataVector* rhs;

    // Do some animation creation exception handling
    size_t animationStep = this->nMaxIterations/1500;
    if (animationStep == 0)
    {
    	animationStep = 1;
    }

	for (size_t i = 0; i < this->nMaxIterations; i++)
	{
		// generate right hand side
		rhs = System.generateRHS();

		// solve the system of the current timestep
		LinearSystemSolver.solve(System, *System.getGridCoefficientsForCG(), *rhs, true, false, -1.0);
	    allIter += LinearSystemSolver.getNumberIterations();
	    if (verbose == true)
	    {
	    	if (myScreen == NULL)
	    	{
	    		std::cout << "Final residuum " << LinearSystemSolver.residuum << "; with " << LinearSystemSolver.getNumberIterations() << " Iterations (Total Iter.: " << allIter << ")" << std::endl;
	    	}
	    }
	    if (myScreen != NULL)
    	{
    		std::stringstream soutput;
    		soutput << "Final residuum " << LinearSystemSolver.residuum << "; with " << LinearSystemSolver.getNumberIterations() << " Iterations (Total Iter.: " << allIter << ")";

    		if (i < this->nMaxIterations-1)
    		{
    			myScreen->update((size_t)(((double)(i+1)*100.0)/((double)this->nMaxIterations)), soutput.str());
    		}
    		else
    		{
    			myScreen->update(100, soutput.str());
    		}
    	}
	    System.finishTimestep();

		// Create pictures of the animation, if specified
	    if ((this->bAnimation == true) && (i%animationStep == 0))
		{
			// Build filename
			std::string tFilename = "00000000000000000000000000000000";
			std::stringstream number;
			number << i;
			tFilename.append(number.str());
			tFilename = tFilename.substr(tFilename.length()-14,14);
			tFilename.append(".gnuplot");

			// Print grid to file
			GridPrinter myPrinter(*System.getGrid());
			myPrinter.printGrid(*System.getGridCoefficients(), tFilename, this->evalsAnimation);
		}
	}

	// write some empty lines to console
    if (myScreen != NULL)
	{
    	myScreen->writeEmptyLines(2);
	}
}

}
