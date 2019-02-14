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

#include <linbox/matrix/dense-matrix.h>
#include <linbox/matrix/sparse-matrix.h>
#include <linbox/solutions/methods-wip.h>

namespace LinBox {
    /**
     * \brief Solve specialisation for DenseElimination.
     */
    template <class ResultVector, class Matrix, class Vector, class CategoryTag>
    ResultVector& solve(ResultVector& x, const Matrix& A, const Vector& b, const CategoryTag& tag,
                        const MethodWIP::DenseElimination& m)
    {
        // @fixme Original code would copy the sparse or other to a DenseMatrix
        throw NotImplementedYet("Dense eliminating.");
    }

    /**
     * \brief Solve specialisation for DenseElimination on dense matrices with ModularTag.
     */
    template <class Field, class Vector>
    Vector& solve(Vector& x, const DenseMatrix<Field>& A, const Vector& b, const RingCategories::ModularTag& tag,
                        const MethodWIP::DenseElimination& m)
    {
        linbox_check((A.coldim() != x.size()) || (A.rowdim() != b.size()));

        commentator().start("solve.dense-elimination.modular.dense");

        LQUPMatrix<Field> LQUP(A);
        LQUP.left_solve(x, b);

        commentator().stop("solve.dense-elimination.modular.dense");

        return x;
    }

    /**
     * \brief Solve specialisation for DenseElimination on dense matrices with IntegerTag.
     */
    template <class ResultVector, class Field, class Vector>
    ResultVector& solve(ResultVector& x, const DenseMatrix<Field>& A, const Vector& b, const RingCategories::IntegerTag& tag,
                        const MethodWIP::DenseElimination& m)
    {
        MethodWIP::Dixon<MethodWIP::DenseElimination> method(m);
        method.iterationMethod = m;
        return solve(x, A, b, tag, method);
    }
}