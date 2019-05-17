/*
 * Copyright(C) LinBox
 *
 * ========LICENCE========
 * This file is part of the library LinBox.
 *
 * LinBox is free software: you can redistribute it and/or modify
 * it under the terms of the  GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * ========LICENCE========
 */

#pragma once

#include <linbox/solutions/methods.h>

namespace LinBox {
    /**
     * The algorithm solves Ax = b over the integers.
     * It is based on Chen/Storjohann RNS-based p-adic lifting.
     * Based on https://cs.uwaterloo.ca/~astorjoh/p92-chen.pdf
     * A BLAS Based C Library for Exact Linear Algebra on Integer Matrices (ISSAC 2009)
     * But it has been slightly modified in order to use BLAS3 multiplication within the main loop.
     *
     *  RNS Dixon algorithm goes this way:
     *      (i)     Use (p1, ..., pl) primes with an arbitrary l.
     *      (ii)    Algorithm goes:
     *                  for i = 1 .. l:
     *                  |   Bi = A^{-1} mod pi                      < Pre-computing
     *                  [r1|...|rl] = [b|...|b]
     *                  [y1|...|yl] = [0|...|0]
     *                  for j = 1 .. k:
     *                  |   for i = 1 .. l:
     *                  |   |   (Qi, Ri) = such that ri = pi Qi + Ri with |Ri| < pi
     *                  |   |   ci = Bi ri mod pi                   < Matrix-vector in Z/pZ
     *                  |   |   yi = yi + ci * pi^(i-1)             < Done over ZZ
     *                  |   V = [R1|...|Rl] - A [c1|...|cl]         < Matrix-matrix in ZZ
     *                  |   for i = 1 .. l:
     *                  |   |   ri = Qi + (Vi / pi)
     *              @note The computation of V can be done in a RNS system such that each RNS
     * base-prime is bigger than each (p1, ..., pl). This way, [R1|...|Rl] and [c1|...|cl] are
     * zero-cost to get in the RNS system.
     *      (iii)   y = CRT_Reconstruct(y1, ..., yl)
     *      (iv)    x = Rational_Reconstruct(y)
     *
     * One can configure how many primes are used with `Method::DixonRNS.primeBaseLength`.
     * According to the paper, a value of lp = 2 (ln(n) + log2(||A||)) or without the factor 2
     * can be used, but it depends on the problem, really.
     */
    template <class _Field, class _Ring, class _PrimeGenerator>
    class MultiModLiftingContainer final : public LiftingContainerBase<_Ring, DenseMatrix<_Ring>> {
        using BaseClass = LiftingContainerBase<_Ring, DenseMatrix<_Ring>>;

    public:
        using typename BaseClass::IMatrix;
        using typename BaseClass::IVector;
        using typename BaseClass::Ring;

        using Field = _Field;
        using PrimeGenerator = _PrimeGenerator;

    public:
        // @fixme Have dynamic random ones
        const std::vector<Integer> p = {97, 101};

        // @fixme Split to inline file
        MultiModLiftingContainer(const Ring& ring, PrimeGenerator primeGenerator, const IMatrix& A, const IVector& b,
                                 const Method::DixonRNS& m)
            // @fixme Am forces to set the prime here? Why?
            : BaseClass(ring, A, b, 97 * 101)
            , _ring(ring)
        {
            // @note From baseClass, we have _length = log2(2 * N * D)

            // @fixme Have l = log(||A||) + log(n) or so
            uint32_t l = p.size();

            // Ap[0] = A mod p[0]
            // Ap[1] = A mod p[1]

            // B[0] = inv(Ap[0]) mod p[0] @fixme How?
            // B[1] = inv(Ap[1]) mod p[1]

            // @note As _r is row major, we store each ri on each row.
            // So that r[i] = current residue for p[i].
            _r = std::make_unique<DenseMatrix<Ring>>(_ring, l, b.size());
            for (auto i = 0u; i < l; ++i) {
                // @fixme Is there a vector domain to copy to a matrix?
                for (auto j = 0u; j < b.size(); ++j) {
                    _ring.assign(_r[i][j], b[j]);
                }
            }
        }

        IVector& nextdigit(IVector&, const IVector&) const final
        {
            // @fixme With this design, are we forces to CRT_Reconstruct each ci?
            // Is this bad?
        }

    private:
        Ring& _ring;

        // @note r is a big matrix in ZZ holding all residues
        std::unique_ptr<DenseMatrix<Ring>> _r;
    };
}
