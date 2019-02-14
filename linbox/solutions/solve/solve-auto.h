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

#include <linbox/solutions/methods-wip.h>
#include <linbox/matrix/dense-matrix.h>

namespace {
// @fixme This should be configured with autotune or something.
#ifndef LINBOX_USE_BLACKBOX_THRESHOLD
#define LINBOX_USE_BLACKBOX_THRESHOLD 1000
#endif

    template <class Matrix>
    bool useBlackboxMethod(const Matrix& A)
    {
        return (A.coldim() > LINBOX_USE_BLACKBOX_THRESHOLD) && (A.rowdim() > LINBOX_USE_BLACKBOX_THRESHOLD);
    }

    template <class Field>
    bool useBlackboxMethod(const LinBox::DenseMatrix<Field>& A)
    {
        return false;
    }
}

namespace LinBox {
    /**
     * \brief Solve specialisation for Auto.
     */
    template <class ResultVector, class Matrix, class Vector, class CategoryTag>
    ResultVector& solve(ResultVector& x, const Matrix& A, const Vector& b, const CategoryTag& tag, const MethodWIP::Auto& m)
    {
        if (useBlackboxMethod(A)) {
            return solve(x, A, b, tag, reinterpret_cast<const MethodWIP::Blackbox&>(m));
        }
        else {
            return solve(x, A, b, tag, reinterpret_cast<const MethodWIP::Elimination&>(m));
        }
    }

    /**
     * \brief Solve specialisation for Auto and IntegerTag.
     */
    template <class ResultVector, class Matrix, class Vector>
    ResultVector& solve(ResultVector& x, const Matrix& A, const Vector& b, const RingCategories::IntegerTag& tag,
                        const MethodWIP::Auto& m)
    {
        MethodWIP::DixonAuto method(m);
        method.iterationMethod = m;
        return solve(x, A, b, tag, method);
    }

    //
    // Rational API.
    //

    /**
     * \brief Solve specialization for Auto and IntegerTag.
     */
    template <class Matrix, class Vector>
    inline void solve(Vector& xNum, typename Vector::Field::Element& xDen, const Matrix& A, const Vector& b,
                      const RingCategories::IntegerTag& tag, const MethodWIP::Auto& m)
    {
        MethodWIP::DixonAuto method(m);
        method.iterationMethod = m;
        solve(xNum, xDen, A, b, tag, method);
    }
}