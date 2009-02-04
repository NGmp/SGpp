/*****************************************************************************/
/* This file is part of sg++, a program package making use of spatially      */
/* adaptive sparse grids to solve numerical problems                         */
/*                                                                           */
/* Copyright (C) 2007 Jörg Blank (blankj@in.tum.de)                          */
/* Copyright (C) 2009 Alexander Heinecke (Alexander.Heinecke@mytum.de)       */
/*                                                                           */
/* sg++ is free software; you can redistribute it and/or modify              */
/* it under the terms of the GNU General Public License as published by      */
/* the Free Software Foundation; either version 3 of the License, or         */
/* (at your option) any later version.                                       */
/*                                                                           */
/* sg++ is distributed in the hope that it will be useful,                   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with Foobar; if not, write to the Free Software                     */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA */
/* or see <http://www.gnu.org/licenses/>.                                    */
/*****************************************************************************/

#ifndef GENERATION_EXCEPTION_HPP
#define GENERATION_EXCEPTION_HPP

#include <exception>

namespace sg
{

class generation_exception : public std::exception
{
public:
	generation_exception(const char* msg) throw() : msg(msg)
	{
	}

	generation_exception() throw() : msg(NULL) { }

    virtual ~generation_exception() throw() { }

	virtual const char* what() const throw()
	{
		if(msg)
		{
			return msg;
		}
		else
		{
			return "generation_exception: failure generating grid";
		}
	}
protected:
	const char* msg;

};

}

#endif /* GENERATION_EXCEPTION_HPP */
