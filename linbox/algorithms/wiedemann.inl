/* linbox/algorithms/wiedemann.inl
 * Copyright (C) 2002 Zhendong Wan
 * Copyright (C) 2002 Bradford Hovinen
 *
 * Written by Zhendong Wan <wan@mail.eecis.udel.edu>,
 *            Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * ------------------------------------
 * 2002-10-02  Bradford Hovinen  <bghovinen@math.uwaterloo.ca>
 *
 * Refactoring:
 * Put everything inside a WiedemannSolver class, with the following
 * interface:
 *    solve - Solve a general linear system
 *    solveNonsingular - Solve a nonsingular system
 *    solveSingular - Solve a general singular system
 *    findRandomSolution - Find a random solution to a singular preconditioned
 *                         problem
 *    findNullspaceElement - Find an element of the right nullspace
 *    certifyInconsistency - Find a certificate of inconsistency for a
 *                           linear system
 *    precondition - Form a preconditioner and return it
 * ------------------------------------
 * 2002-08-09  Bradford Hovinen  <hovinen@cis.udel.edu>
 *
 * Move the Wiedemann stuff over to this file
 *
 * Create a singular and nonsingular version that is a bit intelligent about
 * which one to use in different circumstances
 * ------------------------------------
 *
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
 *.
 */

#ifndef __LINBOX_wiedemann_INL
#define __LINBOX_wiedemann_INL

#include <algorithm>
#include <vector>

#include "linbox/algorithms/blackbox-container-symmetric.h"
#include "linbox/algorithms/blackbox-container.h"
#include "linbox/algorithms/wiedemann.h"
#include "linbox/blackbox/butterfly.h"
#include "linbox/blackbox/submatrix.h"
#include "linbox/blackbox/transpose.h"
#include "linbox/solutions/minpoly.h"
//#include "linbox/algorithms/blackbox-container-generic.h"
#include "linbox/algorithms/massey-domain.h"
#include "linbox/solutions/rank.h"
#include "linbox/vector/stream.h"

namespace LinBox {

    template <class Field>
    template <class Blackbox, class Vector>
    typename WiedemannSolver<Field>::ReturnStatus WiedemannSolver<Field>::solve(const Blackbox& A, Vector& x, const Vector& b,
                                                                                Vector& u)
    {
        linbox_check((x.size() == A.coldim()) && (b.size() == A.rowdim()));
        linbox_check(_traits.singularity != Singularity::NonSingular || A.coldim() == A.rowdim());

        commentator().start("Solving linear system (Wiedemann)", "WiedemannSolver::solve");

        Singularity singularity = _traits.singularity;
        if (A.rowdim() != A.coldim()) _traits.singularity = (singularity = Singularity::Singular);
        ReturnStatus status = FAILED;

        auto tries = _traits.trialsBeforeFailure;

        size_t r = (size_t)-1;

        // Rank == 0 means unknown
        if (_traits.rank != 0) r = _traits.rank;

        while (status == FAILED && tries-- > 0) {
            switch (singularity) {
            case Singularity::Unknown: {
                switch (solveNonsingular(A, x, b, true)) {
                case OK: status = OK; break;
                case FAILED: break;
                case SINGULAR:
                    commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                        << "System found to be singular. Reverting from nonsingular solver." << std::endl;
                    tries = _traits.trialsBeforeFailure;
                    singularity = Singularity::Singular;
                    break;
                default: throw LinboxError("Bad return value from solveNonsingular");
                }

                break;
            }

            case Singularity::NonSingular: {
                switch (solveNonsingular(A, x, b, false)) {
                case OK: status = OK; break;
                case FAILED: break;
                case SINGULAR: status = SINGULAR; break;
                default: throw LinboxError("Bad return value from solveNonsingular");
                }

                break;
            }

            case Singularity::Singular: {
                if (r == (size_t)-1) {
                    rank(r, A);
                    commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION) << "Rank of A = " << r << std::endl;
                }

                switch (solveSingular(A, x, b, u, r)) {
                case OK: status = OK; break;
                case FAILED: r = (size_t)-1; break;
                case SINGULAR: throw LinboxError("solveSingular returned SINGULAR");
                case INCONSISTENT: status = INCONSISTENT; break;
                default: break;
                }

                break;
            }
            }
        }

        if (status == FAILED) {
            commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                << "Maximum tries exceeded with no resolution. Giving up." << std::endl;
        }

        commentator().stop("done", NULL, "WiedemannSolver::solve");

        return status;
    }

    template <class Field>
    template <class Blackbox, class Vector>
    typename WiedemannSolver<Field>::ReturnStatus WiedemannSolver<Field>::solveNonsingular(const Blackbox& A, Vector& x,
                                                                                           const Vector& b, bool useRandIter)
    {
        typedef BlasVector<Field> Polynomial;
        typedef typename Polynomial::iterator PolyIterator;

        commentator().start("Solving nonsingular system (Wiedemann)", "WiedemannSolver::solveNonsingular");

        Polynomial m_A(A.field());
        Vector z(A.field());
        bool ret = true;

        {
            // Make it just Blackbox trait and not wiedemann:
            // Might need extension field for minpoly
            // Might also also use better method than Wiedemann ...
            // @fixme Just reinterpret_cast<const Blackbox&>(_traits) when minpoly uses Method
            minpoly(m_A, A, RingCategories::ModularTag(), Method::Blackbox());
        }

        std::ostream& report = commentator().report(Commentator::LEVEL_UNIMPORTANT, INTERNAL_DESCRIPTION);
        report << "Minimal polynomial of degree " << (m_A.size() - 1) << std::endl;
        if (m_A.size() < 50) {
            report << "Minimal polynomial coefficients: ";
            _VD.write(report, m_A) << std::endl;
        }

        if (field().isZero(m_A.front())) {
            commentator().stop("singular", "System found to be singular", "WiedemannSolver::solveNonsingular");
            return SINGULAR;
        }

        {
            commentator().start("Preparing polynomial for application");

            PolyIterator iter = m_A.begin();

            while (++iter != m_A.end()) {
                field().divin(*iter, m_A.front());
                field().negin(*iter);
            }

            commentator().stop("done");
        }

        {
            commentator().start("Applying polynomial via Horner's rule", NULL, m_A.size() - 1);

            _VD.mul(x, b, m_A.back());

            VectorWrapper::ensureDim(z, A.rowdim());

            for (int i = (int)m_A.size() - 1; --i > 0;) {
                if (((long)m_A.size() - i) & (0xff == 0)) commentator().progress((long)m_A.size() - i);

                A.apply(z, x);
                _VD.axpy(x, m_A[(size_t)i], b, z);
            }

            commentator().stop("done");
        }

        if (_traits.checkResult) {
            commentator().start("Checking whether Ax=b");
            A.apply(z, x);

            if (_VD.areEqual(z, b))
                ret = true;
            else {
                std::ostream& Report = commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
                _VD.write(Report << "x is ", x) << std::endl;
                _VD.write(Report << "b is ", b) << std::endl;
                _VD.write(Report << "Ax is ", z) << std::endl;

                ret = false;
            }

            commentator().stop(MSG_STATUS(ret));
        }

        commentator().stop(MSG_STATUS(ret), NULL, "WiedemannSolver::solveNonsingular");

        if (!ret) return FAILED;

        return OK;
    }

    template <class Field>
    template <class Blackbox, class Vector>
    typename WiedemannSolver<Field>::ReturnStatus WiedemannSolver<Field>::solveSingular(const Blackbox& A, Vector& x,
                                                                                        const Vector& b, Vector& u, size_t r)
    {
        commentator().start("Solving singular system (Wiedemann)", "WiedemannSolver::solveSingular");

        Vector Ax(A.field());
        ReturnStatus status = OK, sfrs = OK;

        switch (_traits.preconditioner) {
        case Preconditioner::Butterfly: {
            commentator().start("Constructing butterfly preconditioner");

            typedef Butterfly<Field> ButterflyP;
            ButterflyP P(field(), A.rowdim());
            ButterflyP Q(field(), A.coldim());
            Compose<Blackbox, ButterflyP> AQ(&A, &Q);
            Compose<ButterflyP, Compose<Blackbox, ButterflyP>> PAQ(&P, &AQ);

            commentator().stop("done");

            sfrs = findRandomSolution(PAQ, x, b, r, &P, &Q);
            break;
        }

        case Preconditioner::Sparse: {
            commentator().start("Constructing sparse preconditioner");

            SparseMatrix<Field>*P, *QT;
            P = makeLambdaSparseMatrix(A.rowdim());
            QT = makeLambdaSparseMatrix(A.coldim());

            Transpose<SparseMatrix<Field>> Q(QT);

            Compose<Blackbox, Transpose<SparseMatrix<Field>>> AQ(&A, &Q);
            Compose<SparseMatrix<Field>, Compose<Blackbox, Transpose<SparseMatrix<Field>>>> PAQ(P, &AQ);
            commentator().stop("done");

            sfrs = findRandomSolution(PAQ, x, b, r, P, &Q);

            break;
        }

        case Preconditioner::Toeplitz:
            commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
                << "ERROR: Toeplitz preconditioner not implemented yet. Sorry." << std::endl;
            break;

        case Preconditioner::None: {
            SparseMatrix<Field>* P = NULL;
            sfrs = findRandomSolution(A, x, b, r, P, P);
            delete P;
            break;
        }
        default: throw PreconditionFailed(__func__, __LINE__, "preconditioner should be None, Butterfly, Sparse, or Toeplitz");
        }

        switch (sfrs) {
        case BAD_PRECONDITIONER:
            commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                << "Preconditioned matrix did not have generic rank profile" << std::endl;

            status = BAD_PRECONDITIONER;
            break;

        case FAILED:
            if (_traits.certifyInconsistency) {
                VectorWrapper::ensureDim(u, A.rowdim());

                if (certifyInconsistency(u, A, b))
                    status = INCONSISTENT;
                else
                    status = FAILED;
            }
            else
                status = FAILED;

            break;

        case SINGULAR: throw LinboxError("findRandomSolution returned SINGULAR");

        case INCONSISTENT: throw LinboxError("findRandomSolution returned INCONSISTENT");

        case OK: break;
        }

        if (status == OK && _traits.checkResult) {
            commentator().start("Checking system solution");

            VectorWrapper::ensureDim(Ax, A.rowdim());

            A.apply(Ax, x);

            if (_VD.areEqual(Ax, b))
                commentator().stop("passed");
            else {
                commentator().stop("FAILED");

                if (_traits.certifyInconsistency) {
                    commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                        << "Computed system solution is not correct. "
                        << "Attempting to find certificate of inconsistency." << std::endl;

                    VectorWrapper::ensureDim(u, A.rowdim());

                    if (certifyInconsistency(u, A, b))
                        status = INCONSISTENT;
                    else
                        status = FAILED;
                }
                else
                    status = FAILED;
            }
        }

        commentator().stop("done", NULL, "WiedemannSolver::solveSingular");

        return status;
    }

    template <class Field>
    template <class Blackbox, class Vector, class Prec1, class Prec2>
    typename WiedemannSolver<Field>::ReturnStatus WiedemannSolver<Field>::findRandomSolution(const Blackbox& A, Vector& x,
                                                                                             const Vector& b, size_t r,
                                                                                             const Prec1* P, const Prec2* Q)
    {
        commentator().start("Solving singular system with generic rank profile (Wiedemann)",
                            "WiedemannSolver::findRandomSolution");

        Vector v(A.field()), Avpb(A.field()), PAvpb(A.field()), bp(A.field()), xp(A.field()), Qinvx(A.field());

        RandomDenseStream<Field, Vector> stream(field(), _randiter, A.coldim());

        VectorWrapper::ensureDim(v, A.coldim());
        VectorWrapper::ensureDim(Avpb, A.rowdim());
        VectorWrapper::ensureDim(xp, r);
        VectorWrapper::ensureDim(bp, r);

        {
            commentator().start("Preparing right hand side");

            stream >> v;
            A.apply(Avpb, v);
            _VD.addin(Avpb, b);
            if (P != NULL) {
                VectorWrapper::ensureDim(PAvpb, A.rowdim());
                P->apply(PAvpb, Avpb);
                _VD.copy(bp, PAvpb, 0, r);
            }
            else {
                _VD.copy(bp, Avpb, 0, r);
            }

            commentator().stop("done");
        }

        Submatrix<Blackbox> Ap(&A, 0, 0, r, r);

        switch (solveNonsingular(Ap, xp, bp, false)) {
        case SINGULAR:
            commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                << "Leading principal minor was found to be singular." << std::endl;
            commentator().stop("bad preconditioner", "System was not well-conditioned", "WiedemannSolver::findRandomSolution");
            return BAD_PRECONDITIONER;

        case OK: break;

        default: throw LinboxError("solveNonsingular returned bad value");
        }

        if (Q != NULL) {
            VectorWrapper::ensureDim(Qinvx, A.coldim());
            _VD.copy(Qinvx, xp);
            Q->apply(x, Qinvx);
        }
        else {
            _VD.copy(x, xp);
        }

        _VD.subin(x, v);

        commentator().stop("done", NULL, "WiedemannSolver::findRandomSolution");

        return OK;
    }

    template <class Field>
    template <class Blackbox, class Vector>
    typename WiedemannSolver<Field>::ReturnStatus WiedemannSolver<Field>::findNullspaceElement(Vector& x, const Blackbox& A)
    {
        commentator().start("Finding a nullspace element (Wiedemann)", "WiedemannSolver::findNullspaceElement");

        Vector v(A.field()), Av(A.field()), PAv(A.field()), vp(A.field()), xp(A.field()), Qinvx(A.field());

        RandomDenseStream<Field, Vector> stream(field(), _randiter, A.coldim());

        size_t r = (A.coldim() < A.rowdim()) ? A.coldim() : A.rowdim();

        VectorWrapper::ensureDim(v, A.coldim());
        VectorWrapper::ensureDim(Av, A.rowdim());

        ReturnStatus status;

        {
            commentator().start("Constructing right hand side");

            stream >> v;
            A.apply(Av, v);

            if (A.coldim() < A.rowdim()) {
                VectorWrapper::ensureDim(vp, r);
                _VD.copy(vp, Av, 0, r);
            }

            commentator().stop("done");
        }

        if (A.coldim() < A.rowdim()) {
            Submatrix<Blackbox> Ap(&A, 0, 0, r, r);
            status = solveNonsingular(Ap, x, vp, false);
        }
        else if (A.rowdim() < A.coldim()) {
            Submatrix<Blackbox> Ap(&A, 0, 0, r, r);
            VectorWrapper::ensureDim(xp, r);
            status = solveNonsingular(Ap, xp, Av, false);
            _VD.copy(x, xp);
        }
        else
            status = solveNonsingular(A, x, Av, false);

        if (status == SINGULAR) {
            commentator().report(Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION)
                << "Leading principal minor was found to be singular." << std::endl;
            commentator().stop("bad preconditioner", "System not well-conditioned", "WiedemannSolver::findNullspaceElement");
            return BAD_PRECONDITIONER;
        }

        _VD.subin(x, v);

        commentator().stop("done", NULL, "WiedemannSolver::findNullspaceElement");

        return OK;
    }

    template <class Field>
    template <class Blackbox, class Vector>
    bool WiedemannSolver<Field>::certifyInconsistency(Vector& u, const Blackbox& A, const Vector& b)
    {
        commentator().start("Obtaining certificate of inconsistency (Wiedemann)", "WiedemannSolver::certifyInconsistency");

        // Vector PTinvu(A.field());
        typename Field::Element uTb;

        Method::Wiedemann cert_traits;
        cert_traits.trialsBeforeFailure = 1;
        cert_traits.singularity = Singularity::Singular;

        WiedemannSolver solver(field(), cert_traits, _randiter);

        Transpose<Blackbox> AT(&A);

        solver.findNullspaceElement(u, AT);
        _VD.dot(uTb, u, b);

        bool ret = false;
        if (!field().isZero(uTb)) ret = true;

        commentator().stop(MSG_STATUS(ret), NULL, "WiedemannSolver::certifyInconsistency");

        return ret;
    }

    template <class Field>
    SparseMatrix<Field>* WiedemannSolver<Field>::makeLambdaSparseMatrix(size_t m)
    {
        const double LAMBDA = 3;
        integer card;

        field().cardinality(card);

        double init_p = 1.0 - 1.0 / (double)card;
        double log_m = LAMBDA * log((double)m) / M_LN2;

        SparseMatrix<Field>* P = new SparseMatrix<Field>(field(), m, m);

        RandomSparseStream<Field, typename SparseMatrix<Field>::Row> stream(field(), _randiter, init_p, m, m);

        for (unsigned int i = 0; i < m; ++i) {
            double new_p;
            new_p = log_m / double(m - i + 1);

            if (init_p < new_p)
                stream.setP(init_p);
            else
                stream.setP(new_p);

            stream >> P->getRow(i);
        }

        return P;
    }

}

#endif // __LINBOX_wiedemann_INL

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
