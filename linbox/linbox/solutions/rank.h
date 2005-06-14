/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* linbox/solutions/rank.h
 * Copyright (C) 2001, 2002 Bradford Hovinen
 *
 * Written by Bradford Hovinen <hovinen@cis.udel.edu>
 *
 * ------------------------------------
 * See COPYING for license information.
 */

#ifndef __RANK_H
#define __RANK_H

//#include "linbox-config.h"
#include "linbox/blackbox/diagonal.h"
#include "linbox/blackbox/compose.h"
#include "linbox/blackbox/permutation.h"
#include "linbox/blackbox/transpose.h"
#include "linbox/blackbox/blas-blackbox.h"
#include "linbox/algorithms/blackbox-container-symmetrize.h"
#include "linbox/algorithms/blackbox-container-symmetric.h"
#include "linbox/algorithms/blackbox-container.h"
#include "linbox/algorithms/massey-domain.h"
#include "linbox/algorithms/gauss.h"
#include "linbox/algorithms/blas-domain.h"
#include "linbox/matrix/blas-matrix.h"


#include "linbox/vector/vector-traits.h"
#include "linbox/solutions/trace.h"
#include "linbox/solutions/methods.h"

#include "linbox/util/debug.h"

// Namespace in which all LinBox library code resides
namespace LinBox {

	
    const int BlasBound = 1 << 26;

	/** 
            Compute the rank of a linear transform A over a field by selected method. 

            For very large and/or very sparse matrices the Wiedemann method will be faster
            (and is memory efficient).
            For some sparse matrices SparseElimination may outperform Wiedemann.
            For small or dense matrices BlasElimination will be faster.
            \returns <em>r</em> rank of A.
            \param A linear transform, member of any blackbox class.
            \param M may be a Method::Wiedemann (the default), a Method::BlasElimination, or a Method::SparseElimination..
            \ingroup solutions
	*/
// 	template <class Blackbox, class Method, class DomainCategory>
// 	unsigned long &rank (unsigned long                   &r,
// 			     const Blackbox                  &A,
//                              const DomainCategory                      &tag,
// 			     const Method    &M);

	/** 
            Compute the rank of a linear transform A over a field. 

            The default method is Wiedemann(), using diagonal preconditioning and 
            the minpoly.  For small or dense matrices BlasElimination will be faster.
            \returns <em>r</em> rank of A.
            \param A linear transform, member of any blackbox class.
            \ingroup solutions
	*/
    template <class Blackbox>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A)
    {  return rank(r, A, typename FieldTraits<typename Blackbox::Field>::categoryTag(), Method::Hybrid()); 
    }

    template <class Matrix>
    unsigned long &rankin (unsigned long                   &r,
                         Matrix                  &A)
    {  return rankin(r, A, typename FieldTraits<typename Matrix::Field>::categoryTag(), Method::Elimination()); 
    }

    template <class Blackbox>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A,
                         const RingCategories::ModularTag     &tag,
                         const Method::Hybrid         &m)
    { // this should become a BB/Blas hybrid in the style of Duran/Saunders/Wan.  
        if (useBB(A)) return rank(r, A, tag, Method::Blackbox(m )); 
        else return rank(r, A, tag, Method::Elimination( m ));
    }

    template <class Blackbox>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A,
                         const RingCategories::ModularTag                  &tag,
                         const Method::Elimination    &m)
    {  
        typedef typename Blackbox::Field Field;
        const Field F = A.field();
        integer a, b; F.characteristic(a); F.cardinality(b);
        if (a == b && a < LinBox::BlasBound)
            return rank(r, A, tag, Method::BlasElimination(m));
        else
            return rank(r, A, tag, Method::NonBlasElimination( m ));
    }


    template <class Field, class Vector>
    unsigned long &rank (unsigned long                   &r,
                         const SparseMatrix<Field, Vector>         &A,
                         const RingCategories::ModularTag                  &tag,
                         const Method::Elimination    &m)
    {  
        return rank(r, A, tag, Method::SparseElimination(m));
    }


	// specialization of NonBlas for SparseMatrix
    template <class Blackbox>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A,
                         const   RingCategories::ModularTag           &tag,
                         const Method::NonBlasElimination& m)
    {
        return rank(r, A, tag, Method::SparseElimination(m));
    }


    template <class Blackbox>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A,
                         const  RingCategories::ModularTag                   &tag,
                         const Method::Blackbox& m)
    {  return rank(r, A, tag, Method::Wiedemann()); }


    template<class T, template <class T> class Container>
    std::ostream& operator<< (std::ostream& o, const Container<T>& C) {
        o << "[";
        for(typename Container<T>::const_iterator refs = C.begin();
            refs != C.end() ;
            ++refs )
            o << (*refs) << " " ;
        return o << "]";
    }


	/** 
            Compute the rank of a linear transform A over a field. 

            The default method is Wiedemann(), using diagonal preconditioning and 
            the minpoly.  For small or dense matrices BlasElimination will be faster.
            \returns <em>r</em> rank of A.
            \param A linear transform, member of any blackbox class.
            \ingroup solutions
	*/
    template <class Blackbox, class Method>
    unsigned long &rank (unsigned long                   &r,
                         const Blackbox                  &A,
                         const Method    &M)
    {  return rank(r, A, typename FieldTraits<typename Blackbox::Field>::categoryTag(), M); 
    }

	/// M may be <code>Method::Wiedemann()</code>.
    template <class Blackbox>
    unsigned long &rank (unsigned long                   &res,
                         const Blackbox                  &A,
                         const RingCategories::ModularTag          &tag,
                         const Method::Wiedemann    &M) 
    {
            
        typedef typename Blackbox::Field Field;
        const Field F = A.field();
        typename Field::RandIter iter (F);
            
        if (M.symmetric()) {
            commentator.start ("Symmetric Rank", "rank");
		
                
            std::vector<typename Field::Element> d1;
            size_t i;
                
            VectorWrapper::ensureDim (d1, A.coldim ());

            for (i = 0; i < A.coldim (); i++)
                do iter.random (d1[i]); while (F.isZero (d1[i]));

                
            Diagonal<Field> D1 (F, d1);
                
                
            Compose<Diagonal<Field>,Blackbox > B1 (&D1, &A);
            typedef Compose<Compose<Diagonal<Field>,Blackbox >, Diagonal<Field> > BlackBox1;
            BlackBox1 B (&B1, &D1);
                
            BlackboxContainerSymmetric<Field, BlackBox1> TF (&B, F, iter);
            MasseyDomain<Field, BlackboxContainerSymmetric<Field, BlackBox1> > WD (&TF, M.earlyTermThreshold ());
            std::vector<typename Field::Element> phi;
            WD.pseudo_minpoly (phi, res);
                
            typename Field::Element t, p2; F.init(p2, 0UL);
            trace(t, B);            
            if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);
                
            int nbperm = 0; unsigned long rk;
            bool tryagain = (! F.areEqual( t, p2 ));
            while( tryagain ) {
                for (i = 0; i < A.coldim (); i++)
                    do iter.random (d1[i]); while (F.isZero (d1[i]));
                Diagonal<Field> D1 (F, d1);
                BlackBox1 B (&B1, &D1);
                
                BlackboxContainerSymmetric<Field, BlackBox1> TF (&B, F, iter);
                MasseyDomain<Field, BlackboxContainerSymmetric<Field, BlackBox1> > WD (&TF, M.earlyTermThreshold ());
                
                WD.pseudo_minpoly (phi, rk);
                if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);
                
                trace(t, B);

                tryagain = (! F.areEqual( t, p2 ));
                if (res > rk) 
                    tryagain = true;
                else
                    res = rk;
                ++nbperm;
            }
            commentator.report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "symm permutations : " << nbperm << std::endl;
//                 WD.pseudo_rank (res);
                
            commentator.stop ("done", NULL, "rank");
            
            return res;
        } else {
                
            commentator.start ("Rank", "rank");

            std::vector<typename Field::Element> d1, d2;
            size_t i;
                
            VectorWrapper::ensureDim (d1, A.coldim ());
            VectorWrapper::ensureDim (d2, A.rowdim ());
                
            for (i = 0; i < A.coldim (); i++)
                do iter.random (d1[i]); while (F.isZero (d1[i]));
                
            for (i = 0; i < A.rowdim (); i++)
                do iter.random (d2[i]); while (F.isZero (d2[i]));
                
            Diagonal<Field> D1 (F, d1), D2 (F, d2);
            Transpose<Blackbox> AT (&A);
                
            Compose<Diagonal<Field>,Transpose<Blackbox> > B1 (&D1, &AT);
            Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> > B2 (&B1, &D2);
            Compose<Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> >, Blackbox> B3 (&B2, &A);
                // Here there is an extra diagonal computation
                // The probability of success is also divided by two, as 
                // D2^2 contains only squares and squares are half the total elements                
            typedef Compose<Compose<Compose<Compose<Diagonal<Field>,Transpose<Blackbox> >, Diagonal<Field> >, Blackbox>, Diagonal<Field> > Blackbox1;
            Blackbox1 B (&B3, &D1);

            BlackboxContainerSymmetric<Field, Blackbox1> TF (&B, F, iter);
            MasseyDomain<Field, BlackboxContainerSymmetric<Field, Blackbox1> > WD (&TF, M.earlyTermThreshold ());
                
            std::vector<typename Field::Element> phi;
            WD.pseudo_minpoly (phi, res);

            typename Field::Element t, p2; F.init(p2, 0UL);
            trace(t, B);            
            if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);
                
            int nbperm = 0; unsigned long rk;
            bool tryagain = (! F.areEqual( t, p2 ));
            while( tryagain ) {
                Permutation<> P(A.rowdim());          
                for (i = 0; i < A.rowdim (); ++i)
                    P.permute( rand() % A.rowdim() , rand() % A.rowdim() );
                for (i = 0; i < A.rowdim (); ++i)
                    P.permute( rand() % A.rowdim() , rand() % A.rowdim() );

                typedef Compose< Permutation<>, Blackbox > BlackboxP;
                BlackboxP BP(&P, &A);

                for (i = 0; i < A.coldim (); i++)
                    do iter.random (d1[i]); while (F.isZero (d1[i]));
                
                for (i = 0; i < A.rowdim (); i++)
                    do iter.random (d2[i]); while (F.isZero (d2[i]));
                
                Diagonal<Field> D1 (F, d1), D2 (F, d2);
                Transpose<BlackboxP> AT (&BP);
                
                Compose<Diagonal<Field>,Transpose<BlackboxP> > B1 (&D1, &AT);
                Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> > B2 (&B1, &D2);
                Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP> B3 (&B2, &BP);
                typedef Compose<Compose<Compose<Compose<Diagonal<Field>,Transpose<BlackboxP> >, Diagonal<Field> >, BlackboxP>, Diagonal<Field> > Blackbox1;
                Blackbox1 B (&B3, &D1);

                BlackboxContainerSymmetric<Field, Blackbox1> TF (&B, F, iter);
                MasseyDomain<Field, BlackboxContainerSymmetric<Field, Blackbox1> > MD (&TF, M.earlyTermThreshold ());
                
                MD.pseudo_minpoly (phi, rk);
                if (phi.size() >= 2) F.neg(p2, phi[ phi.size()-2]);
                
                trace(t, B);
                tryagain = (! F.areEqual( t, p2 ));
                if (res > rk) 
                    tryagain = true;
                else
                    res = rk;
                ++nbperm;
            }
//             F.write( F.write( commentator.report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION)
//                               << "end trace: ", t) << ", p2: ", p2) << std::endl;
//             commentator.report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION)
//                 << phi << std::endl;
            commentator.report(Commentator::LEVEL_ALWAYS,INTERNAL_DESCRIPTION) << "permutations : " << nbperm << std::endl;
//                 WD.pseudo_rank (res);
                
            commentator.stop ("done", NULL, "rank");
            
            return res;
        }

    }


	/// M may be <code>Method::SparseElimination()</code>.
    template <class Field>
    unsigned long &rank (unsigned long                       &r,
                         const SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq>  &A,
                         const RingCategories::ModularTag    &tag,
                         const Method::SparseElimination     &M) 
    {
         // We make a copy as these data will be destroyed
        SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq> A1 (A);  
        return rankin(r, A1, tag, M);
    }
    
	/// M may be <code>Method::SparseElimination()</code>.
    template <class Field>
    unsigned long &rankin (unsigned long                       &r,
                           SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq>  &A,
                           const RingCategories::ModularTag    &tag,
                           const Method::SparseElimination     &M) 
    {
        commentator.start ("Sparse Elimination Rank", "rank");
        GaussDomain<Field> GD ( A.field() );
        GD.rankin (r, A, M.strategy ());
        commentator.stop ("done", NULL, "rank");
        return r;
    }
    
	/// Change of representation to be able to call the sparse elimination
    template <class Blackbox>
    unsigned long &rank (unsigned long                       &r,
                         const Blackbox  &A,
                         const RingCategories::ModularTag    &tag,
                         const Method::SparseElimination     &M) 
    {
        typedef typename Blackbox::Field Field;
        typedef SparseMatrix<Field, typename LinBox::Vector<Field>::SparseSeq> SparseBB;
        SparseBB  SpA(A.field(), A.rowdim(), A.coldim());
        typename Blackbox::ConstRawIterator        valit = A.rawBegin();
        typename Blackbox::ConstRawIndexedIterator indit = A.rawIndexedBegin();
        for( ; (indit != A.rawIndexedEnd()) && (valit != A.rawEnd()) ; ++indit, ++valit) 
            SpA.setEntry( indit.rowIndex(), indit.colIndex(), *valit);
        return rankin(r, SpA, tag, M);
    }
    
	/// M may be <code>Method::BlasElimination()</code>.
    template <class Blackbox>
    unsigned long &rank (unsigned long                     &r,
                         const Blackbox                      &A,
                         const RingCategories::ModularTag          &tag,
                         const Method::BlasElimination  &M) 
    {
        
        commentator.start ("Blas Rank", "rank");
        typedef typename Blackbox::Field Field;
        const Field F = A.field();
        integer a, b; F.characteristic(a); F.cardinality(b);
        linbox_check( a == b );
        linbox_check( a < LinBox::BlasBound);
        BlasMatrix<typename Field::Element> B(A);
        BlasMatrixDomain<Field> D(F);
        r = D.rank(B);
        commentator.stop ("done", NULL, "rank");
        return r;
    }


	/// A is modified.
    template <class Matrix>
    unsigned long &rankin (unsigned long                      &r,
                           Matrix                               &A,
                           const RingCategories::ModularTag          &tag,
                           const Method::SparseElimination &M) 
    {
        typedef typename Matrix::Field Field;
        const Field F = A.field();
        GaussDomain<Field> GD (F);
        GD.rankin( r, A, M.strategy ());
        return r;
    }

	/// A is modified.
    template <class Field>
    unsigned long &rankin (unsigned long                     &r,
                           BlasBlackbox<Field>                 &A,
                           const RingCategories::ModularTag          &tag,
                           const Method::BlasElimination  &M) 
    {

        commentator.start ("BlasBB Rank", "rank");
        const Field F = A.field();
        BlasMatrixDomain<Field> D(F);
        r = D.rankin(static_cast< BlasMatrix<typename Field::Element>& >(A));
        commentator.stop ("done", NULL, "rank");
        return r;
    }


} // LinBox
#include "linbox/field/modular.h"
#include "linbox/randiter/random-prime.h"
#include "linbox/algorithms/matrix-mod.h"

namespace LinBox {

    template <class Blackbox, class MyMethod>
    unsigned long &rank (unsigned long                     &r,
                         const Blackbox                      &A,
                         const RingCategories::IntegerTag          &tag,
                         const MyMethod                           &M)
    {
        commentator.start ("Integer Rank", "rank");
        typedef Modular<double> Field;
        integer mmodulus; 
        FieldTraits<Field>::maxModulus(mmodulus);
        RandomPrime genprime( (long) floor (log((double)mmodulus) ) );
        genprime.randomPrime( mmodulus );
        typedef typename Blackbox::template rebind< Field >::other FBlackbox;
        FBlackbox * Ap;
        MatrixMod::mod(Ap, A, Field(mmodulus) );
        commentator.report (Commentator::LEVEL_ALWAYS,INTERNAL_WARNING) << "Integer Rank is done modulo " << mmodulus << std::endl;
        rank(r, *Ap, M);
        delete Ap;
        commentator.stop ("done", NULL, "rank");
        return r;
    }
} // LinBox
#endif // __RANK_H
