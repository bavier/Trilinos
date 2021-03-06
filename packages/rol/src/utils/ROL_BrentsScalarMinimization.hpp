// @HEADER
// ************************************************************************
//
//               Rapid Optimization Library (ROL) Package
//                 Copyright (2014) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact lead developers:
//              Drew Kouri   (dpkouri@sandia.gov) and
//              Denis Ridzal (dridzal@sandia.gov)
//
// ************************************************************************
// @HEADER

#ifndef ROL_BRENTSSCALARMINIMIZATION_H
#define ROL_BRENTSSCALARMINIMIZATION_H

/** \class ROL::BrentsScalarMinimization
    \brief Implements Brent's method to minimize a scalar function on a
           bounded interval.
*/

#include "ROL_ScalarMinimization.hpp"
#include "ROL_ScalarFunction.hpp"
#include "ROL_Types.hpp"

namespace ROL { 

template<class Real>
class BrentsScalarMinimization : public ScalarMinimization<Real> {
private:
  Real tol_;
  int niter_;

public:
  // Constructor
  BrentsScalarMinimization( Teuchos::ParameterList &parlist ) {
    Teuchos::ParameterList &list = parlist.sublist("Scalar Minimization").sublist("Brent's");
    tol_   = list.get("Tolerance",1.e-10);
    niter_ = list.get("Iteration Limit",1000);
  }

  void run(Real &fx, Real &x, int &nfval, int &ngrad,
           ScalarFunction<Real> &f, const Real A, const Real B,
           ScalarMinimizationStatusTest<Real> &test) const {
    nfval = 0; ngrad = 0;
    // ---> Set algorithmic constants
    const Real c   = 0.5*(3.0 - std::sqrt(5.0));
    const Real eps = std::sqrt(ROL_EPSILON<Real>());
    // ---> Set end points and initial guess
    Real a = A, b = B;
    x = a + c*(b-a);
    // ---> Evaluate function
    fx = f.value(x);
    nfval++;
    // ---> Initialize algorithm storage
    Real v = x, w = v, u = 0.0, fu = 0.0;
    Real p = 0.0, q = 0.0, r = 0.0, d = 0.0, e = 0.0;
    Real fv = fx, fw = fx, tol = 0.0, t2 = 0.0, m = 0.0, gx = ROL_INF<Real>();
    bool deriv = false;
    for (int i = 0; i < niter_; i++) {
      m = 0.5*(a+b);
      tol = eps*std::abs(x) + tol_; t2 = 2.0*tol;
      // Check stopping criterion
      if (std::abs(x-m) <= t2 - 0.5*(b-a) || test.check(x,fx,gx,nfval,ngrad,deriv)) {
        break;
      }
      p = 0.0; q = 0.0; r = 0.0;
      if ( std::abs(e) > tol ) {
        // Fit parabola
        r = (x-w)*(fx-fv);     q = (x-v)*(fx-fw);
        p = (x-v)*q - (x-w)*r; q = 2.0*(q-r);
        if ( q > 0.0 ) {
          p *= -1.0;
        }
        q = std::abs(q);
        r = e; e = d;
      }
      if ( std::abs(p) < std::abs(0.5*q*r) && p > q*(a-x) && p < q*(b-x) ) {
        // A parabolic interpolation step
        d = p/q; u = x + d;
        // f must not be evaluated too close to a or b
        if ( (u - a) < t2 || (b - u) < t2 ) {
          d = (x < m) ? tol : -tol;
        }
      }
      else {
        // A golden section step
        e = ((x < m) ? b : a) - x; d = c*e;
      }
      // f must not be evaluated too close to x
      u  = x + ((std::abs(d) >= tol) ? d : ((d > 0.0) ? tol : -tol));
      fu = f.value(u);
      nfval++;
      // Update a, b, v, w, and x
      if ( fu <= fx ) {
        if ( u < x ) {
          b = x;
        }
        else {
          a = x;
        }
        v = w; fv = fw; w = x; fw = fx; x = u; fx = fu;
      }
      else {
        if ( u < x ) {
          a = u;
        }
        else {
          b = u;
        }
        if ( fu <= fw || w == x ) {
          v = w; fv = fw; w = u; fw = fu;
        }
        else if ( fu <= fv || v == x || v == w ) {
          v = u; fv = fu;
        }
      }
    }
  }
};

}

#endif
