/*
The Evolving Distribution Objects framework (EDO) is a template-based,
ANSI-C++ evolutionary computation library which helps you to write your
own estimation of distribution algorithms.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

Copyright (C) 2010 Thales group
*/
/*
Authors:
    Johann Dréo <johann.dreo@thalesgroup.com>
    Caner Candan <caner.candan@thalesgroup.com>
*/

#ifndef _edoSamplerNormalMulti_h
#define _edoSamplerNormalMulti_h

#include <cmath>
#include <limits>

#include <edoSampler.h>

/** Sample points in a multi-normal law defined by a mean vector and a covariance matrix.
 *
 * Given M the mean vector and V the covariance matrix, of order n:
 *   - draw a vector T in N(0,I) (i.e. each value is drawn in a normal law with mean=0 an stddev=1)
 *   - compute the Cholesky decomposition L of V (i.e. such as V=LL*)
 *   - return X = M + LT
 */

#ifdef WITH_BOOST

#include <utils/edoCholesky.h>
#include <boost/numeric/ublas/lu.hpp>
#include <boost/numeric/ublas/symmetric.hpp>

template< class EOT, typename EOD = edoNormalMulti< EOT > >
class edoSamplerNormalMulti : public edoSampler< EOD >
{
public:
    typedef typename EOT::AtomType AtomType;

    edoSamplerNormalMulti( edoRepairer<EOT> & repairer ) 
        : edoSampler< EOD >( repairer)
    {}


    EOT sample( EOD& distrib )
    {
        unsigned int size = distrib.size();
        assert(size > 0);

        // L = cholesky decomposition of varcovar
        const typename cholesky::CholeskyBase<AtomType>::FactorMat& L = _cholesky( distrib.varcovar() );

        // T = vector of size elements drawn in N(0,1)
        ublas::vector< AtomType > T( size );
        for ( unsigned int i = 0; i < size; ++i ) {
            T( i ) = rng.normal();
        }

        // LT = L * T
        ublas::vector< AtomType > LT = ublas::prod( L, T );

        // solution = means + LT
        ublas::vector< AtomType > mean = distrib.mean();
        ublas::vector< AtomType > ublas_solution = mean + LT;
        EOT solution( size );
        std::copy( ublas_solution.begin(), ublas_solution.end(), solution.begin() );

        return solution;
    }

protected:
    cholesky::CholeskyLLT<AtomType> _cholesky;
};

#else
#ifdef WITH_EIGEN

template< class EOT, typename EOD = edoNormalMulti< EOT > >
class edoSamplerNormalMulti : public edoSampler< EOD >
{
public:
    typedef typename EOT::AtomType AtomType;
    typedef Eigen::Matrix< AtomType, Eigen::Dynamic, 1> Vector;
    typedef Eigen::Matrix< AtomType, Eigen::Dynamic, Eigen::Dynamic> Matrix;

    edoSamplerNormalMulti( edoRepairer<EOT> & repairer ) 
        : edoSampler< EOD >( repairer)
    {}


    EOT sample( EOD& distrib )
    {
        unsigned int size = distrib.size();
        assert(size > 0);

        // L = cholesky decomposition of varcovar

        // Computes L and D such as V = L D L^T
        Eigen::LDLT<Matrix> cholesky( distrib.varcovar() );
        Matrix L0 = cholesky.matrixL();
        Eigen::Diagonal<const Matrix> D = cholesky.vectorD();

        // now compute the final symetric matrix: this->_L = L D^1/2
        // remember that V = ( L D^1/2) ( L D^1/2)^T
        // fortunately, the square root of a diagonal matrix is the square 
        // root of all its elements
        Eigen::Diagonal<const Matrix> sqrtD = D.cwiseSqrt();

        Matrix L = L0 * D;


        // T = vector of size elements drawn in N(0,1)
        Vector T( size );
        for ( unsigned int i = 0; i < size; ++i ) {
            T( i ) = rng.normal();
        }

        // LT = L * T
        Vector LT = L * T;

        // solution = means + LT
        Vector mean = distrib.mean();
        Vector typed_solution = mean + LT;
        EOT solution( size );
        for( unsigned int i = 0; i < mean.innerSize(); i++ ) {
            solution.push_back( typed_solution(i) );
        }

        return solution;
    }
};
#endif // WITH_EIGEN
#endif // WITH_BOOST


#endif // !_edoSamplerNormalMulti_h
