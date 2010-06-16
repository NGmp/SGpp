/******************************************************************************
* Copyright (C) 2009 Technische Universitaet Muenchen                         *
* This file is part of the SG++ project. For conditions of distribution and   *
* use, please see the copyright notice at http://www5.in.tum.de/SGpp          *
******************************************************************************/
// @author Alexander Heinecke (Alexander.Heinecke@mytum.de)

#include "algorithm/pde/BlackScholesODESolverSystem.hpp"
#include "application/pde/BlackScholesSolver.hpp"
#include "solver/ode/Euler.hpp"
#include "solver/ode/CrankNicolson.hpp"
#include "solver/sle/BiCGStab.hpp"
#include "grid/Grid.hpp"
#include "exception/application_exception.hpp"
#include <cstdlib>
#include <sstream>
#include <cmath>

namespace sg
{

BlackScholesSolver::BlackScholesSolver() : ParabolicPDESolver()
{
	this->bStochasticDataAlloc = false;
	this->bGridConstructed = false;
	this->myScreen = NULL;
}

BlackScholesSolver::~BlackScholesSolver()
{
	if (this->bStochasticDataAlloc)
	{
		delete this->mus;
		delete this->sigmas;
		delete this->rhos;
	}
	if (this->myScreen != NULL)
	{
		delete this->myScreen;
	}
}

void BlackScholesSolver::constructGrid(BoundingBox& BoundingBox, size_t level)
{
	this->dim = BoundingBox.getDimensions();
	this->levels = level;

	this->myGrid = new LinearTrapezoidBoundaryGrid(BoundingBox);

	GridGenerator* myGenerator = this->myGrid->createGridGenerator();
	myGenerator->regular(this->levels);
	delete myGenerator;

	this->myBoundingBox = this->myGrid->getBoundingBox();
	this->myGridStorage = this->myGrid->getStorage();

	//std::string serGrid;
	//myGrid->serialize(serGrid);
	//std::cout << serGrid << std::endl;

	this->bGridConstructed = true;
}

void BlackScholesSolver::refineInitialGridWithPayoff(DataVector& alpha, double strike, std::string payoffType, double dStrikeDistance)
{
	size_t nRefinements = 0;

	if (this->bGridConstructed)
	{

		DataVector refineVector(alpha.getSize());

		if (payoffType == "std_euro_call" || payoffType == "std_euro_put")
		{
			double tmp;
			double* dblFuncValues = new double[dim];
			double dDistance = 0.0;

			for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
			{
				std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*this->myBoundingBox);
				std::stringstream coordsStream(coords);

				for (size_t j = 0; j < this->dim; j++)
				{
					coordsStream >> tmp;

					dblFuncValues[j] = tmp;
				}

				tmp = 0.0;
				for (size_t j = 0; j < this->dim; j++)
				{
					tmp += dblFuncValues[j];
				}

				if (payoffType == "std_euro_call")
				{
					dDistance = fabs(((tmp/static_cast<double>(this->dim))-strike));
				}
				if (payoffType == "std_euro_put")
				{
					dDistance = fabs((strike-(tmp/static_cast<double>(this->dim))));
				}

				if (dDistance <= dStrikeDistance)
				{
					refineVector[i] = dDistance;
					nRefinements++;
				}
				else
				{
					refineVector[i] = 0.0;
				}
			}

			delete[] dblFuncValues;

			SurplusRefinementFunctor* myRefineFunc = new SurplusRefinementFunctor(&refineVector, nRefinements, 0.0);

			this->myGrid->createGridGenerator()->refine(myRefineFunc);

			delete myRefineFunc;

			alpha.resize(this->myGridStorage->size());

			// reinit the grid with the payoff function
			initGridWithPayoff(alpha, strike, payoffType);
		}
		else
		{
			throw new application_exception("BlackScholesSolver::refineGrid : An unsupported payoffType was specified!");
		}
	}
	else
	{
		throw new application_exception("BlackScholesSolver::refineInitialGrid : The grid wasn't initialized before!");
	}
}

void BlackScholesSolver::setStochasticData(DataVector& mus, DataVector& sigmas, DataVector& rhos, double r)
{
	this->mus = new DataVector(mus);
	this->sigmas = new DataVector(sigmas);
	this->rhos = new DataVector(rhos);
	this->r = r;

	bStochasticDataAlloc = true;
}

void BlackScholesSolver::solveExplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		Euler* myEuler = new Euler("ExEul", numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		BiCGStab* myCG = new BiCGStab(maxCGIterations, epsilonCG);
		BlackScholesODESolverSystem* myBSSystem = new BlackScholesODESolverSystem(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "ExEul");
		SGppStopwatch* myStopwatch = new SGppStopwatch();
		double execTime;

		std::cout << "Using Explicit Euler to solve " << numTimesteps << " timesteps:" << std::endl;
		myStopwatch->start();
		myEuler->solve(*myCG, *myBSSystem, verbose);
		execTime = myStopwatch->stop();

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << execTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		delete myBSSystem;
		delete myCG;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveExplicitEuler : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}

void BlackScholesSolver::solveImplicitEuler(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		Euler* myEuler = new Euler("ImEul", numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		BiCGStab* myCG = new BiCGStab(maxCGIterations, epsilonCG);
		BlackScholesODESolverSystem* myBSSystem = new BlackScholesODESolverSystem(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "ImEul");
		SGppStopwatch* myStopwatch = new SGppStopwatch();
		double execTime;

		std::cout << "Using Implicit Euler to solve " << numTimesteps << " timesteps:" << std::endl;
		myStopwatch->start();
		myEuler->solve(*myCG, *myBSSystem, verbose);
		execTime = myStopwatch->stop();

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << execTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		delete myBSSystem;
		delete myCG;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveImplicitEuler : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}

void BlackScholesSolver::solveCrankNicolson(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, size_t NumImEul)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{
		BiCGStab* myCG = new BiCGStab(maxCGIterations, epsilonCG);
		BlackScholesODESolverSystem* myBSSystem = new BlackScholesODESolverSystem(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "CrNic");
		SGppStopwatch* myStopwatch = new SGppStopwatch();
		double execTime;

		size_t numCNSteps;
		size_t numIESteps;

		numCNSteps = numTimesteps;
		if (numTimesteps > NumImEul)
		{
			numCNSteps = numTimesteps - NumImEul;
		}
		numIESteps = NumImEul;

		Euler* myEuler = new Euler("ImEul", numIESteps, timestepsize, false, 0, this->myScreen);
		CrankNicolson* myCN = new CrankNicolson(numCNSteps, timestepsize, this->myScreen);

		myStopwatch->start();
		if (numIESteps > 0)
		{
			std::cout << "Using Implicit Euler to solve " << numIESteps << " timesteps:" << std::endl;
			myBSSystem->setODESolver("ImEul");
			myEuler->solve(*myCG, *myBSSystem, false);
		}
		myBSSystem->setODESolver("CrNic");
		std::cout << "Using Crank Nicolson to solve " << numCNSteps << " timesteps:" << std::endl << std::endl << std::endl << std::endl;
		myCN->solve(*myCG, *myBSSystem, false);
		execTime = myStopwatch->stop();

		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << execTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		delete myBSSystem;
		delete myCG;
		delete myCN;
		delete myEuler;
		delete myStopwatch;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveCrankNicolson : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}



void BlackScholesSolver::solveAdamsBashforth(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{

		AdamsBashforth* myAdamsBashforth = new AdamsBashforth(numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		BiCGStab* myCG = new BiCGStab(maxCGIterations, epsilonCG);
		BlackScholesODESolverSystem* myBSSystem = new BlackScholesODESolverSystem(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "AdBas");
		SGppStopwatch* myStopwatch = new SGppStopwatch();

		double execTime;

		myStopwatch->start();
		myAdamsBashforth->solve(*myCG, *myBSSystem, verbose);
		execTime = myStopwatch->stop();
		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << execTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		delete myBSSystem;
		delete myCG;
		delete myAdamsBashforth;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveAdamsBashforth : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}


void BlackScholesSolver::solveVarTimestep(size_t numTimesteps, double timestepsize, size_t maxCGIterations, double epsilonCG, DataVector& alpha, bool verbose, bool generateAnimation, size_t numEvalsAnimation)
{
	if (this->bGridConstructed && this->bStochasticDataAlloc)
	{

		VarTimestep* myVarTimestep = new VarTimestep(numTimesteps, timestepsize, generateAnimation, numEvalsAnimation, myScreen);
		BiCGStab* myCG = new BiCGStab(maxCGIterations, epsilonCG);
		BlackScholesODESolverSystem* myBSSystem = new BlackScholesODESolverSystem(*this->myGrid, alpha, *this->mus, *this->sigmas, *this->rhos, this->r, timestepsize, "ImEul");
		SGppStopwatch* myStopwatch = new SGppStopwatch();

		double execTime;

		myStopwatch->start();
		myVarTimestep->solve(*myCG, *myBSSystem, verbose);
		execTime = myStopwatch->stop();
		if (this->myScreen != NULL)
		{
			std::cout << "Time to solve: " << execTime << " seconds" << std::endl;
			this->myScreen->writeEmptyLines(2);
		}

		delete myBSSystem;
		delete myCG;
		delete myVarTimestep;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solveVarTimestep : A grid wasn't constructed before or stochastic parameters weren't set!");
	}
}

void BlackScholesSolver::initGridWithPayoff(DataVector& alpha, double strike, std::string payoffType)
{
	double tmp;

	if (this->bGridConstructed)
	{
		for (size_t i = 0; i < this->myGrid->getStorage()->size(); i++)
		{
			std::string coords = this->myGridStorage->get(i)->getCoordsStringBB(*this->myBoundingBox);
			std::stringstream coordsStream(coords);
			double* dblFuncValues = new double[dim];

			for (size_t j = 0; j < this->dim; j++)
			{
				coordsStream >> tmp;

				dblFuncValues[j] = tmp;
			}

			if (payoffType == "std_euro_call")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += dblFuncValues[j];
				}
				alpha[i] = max(((tmp/static_cast<double>(dim))-strike), 0.0);
			}
			else if (payoffType == "std_euro_put")
			{
				tmp = 0.0;
				for (size_t j = 0; j < dim; j++)
				{
					tmp += dblFuncValues[j];
				}
				alpha[i] = max(strike-((tmp/static_cast<double>(dim))), 0.0);
			}
			else
			{
				throw new application_exception("BlackScholesSolver::initGridWithPayoff : An unknown payoff-type was specified!");
			}

			delete[] dblFuncValues;
		}

		OperationHierarchisation* myHierarchisation = this->myGrid->createOperationHierarchisation();
		myHierarchisation->doHierarchisation(alpha);
		delete myHierarchisation;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::initGridWithPayoff : A grid wasn't constructed before!");
	}
}

double BlackScholesSolver::get1DEuroCallPayoffValue(double assetValue, double strike)
{
	if (assetValue <= strike)
	{
		return 0.0;
	}
	else
	{
		return assetValue - strike;
	}
}

void BlackScholesSolver::solve1DAnalytic(std::vector< std::pair<double, double> >& premiums, double maxStock, double StockInc, double strike, double t)
{
	if (bStochasticDataAlloc)
	{
		double stock = 0.0;
		double vola = this->sigmas->get(0);
		StdNormalDistribution* myStdNDis = new StdNormalDistribution();

		for (stock = 0.0; stock <= maxStock; stock += StockInc)
		{
			double dOne = (log((stock/strike)) + ((this->r + (vola*vola*0.5))*(t)))/(vola*sqrt(t));
			double dTwo = dOne - (vola*sqrt(t));
			double prem = (stock*myStdNDis->getCumulativeDensity(dOne)) - (strike*myStdNDis->getCumulativeDensity(dTwo)*(exp((-1.0)*this->r*t)));

			premiums.push_back(std::make_pair(stock, prem));
		}

		delete myStdNDis;
	}
	else
	{
		throw new application_exception("BlackScholesSolver::solve1DAnalytic : Stochastic parameters weren't set!");
	}
}

void BlackScholesSolver::print1DAnalytic(std::vector< std::pair<double, double> >& premiums, std::string tfilename)
{
	typedef std::vector< std::pair<double, double> > printVector;
	std::ofstream fileout;

	fileout.open(tfilename.c_str());
	for(printVector::iterator iter = premiums.begin(); iter != premiums.end(); iter++)
	{
		fileout << iter->first << " " << iter->second << " " << std::endl;
	}
	fileout.close();
}

std::vector<size_t> BlackScholesSolver::getAlgorithmicDimensions()
{
	return this->myGrid->getAlgorithmicDimensions();
}

void BlackScholesSolver::setAlgorithmicDimensions(std::vector<size_t> newAlgoDims)
{
	this->myGrid->setAlgorithmicDimensions(newAlgoDims);
}

void BlackScholesSolver::initScreen()
{
	this->myScreen = new ScreenOutput();
	this->myScreen->writeTitle("SGpp - Black Scholes Solver, 1.2.1", "Alexander Heinecke, (C) 2009-2010");
	this->myScreen->writeStartSolve("Multidimensional Black Scholes Solver");
}

}
