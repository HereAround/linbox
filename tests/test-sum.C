/* tests/test-sum.C
 * Copyright (C) 2002 Bradford Hovinen
 *
 * Written by Bradford Hovinen <hovinen@cis.udel.edu>
 *
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

/*! @file  tests/test-sum.C
 * @ingroup tests
 *
 * @brief no doc
 *
 * @test no doc.
 */


#include "linbox/linbox-config.h"

#include <iostream>
#include <fstream>


#include "linbox/util/commentator.h"
#include "linbox/vector/stream.h"
#include "linbox/field/archetype.h"
#include "linbox/ring/modular.h"
//#include "linbox/field/givaro.h"
#ifdef __LINBOX_HAVE_NTL
#include "linbox/ring/ntl.h"
#endif
#include "linbox/vector/vector-domain.h"
#include "linbox/blackbox/diagonal.h"
#include "linbox/blackbox/scalar-matrix.h"
#include "linbox/blackbox/sum.h"

#include "test-common.h"
#include "test-generic.h"

using namespace LinBox;



template <class Field2, class Blackbox>
static bool testBBrebind (const Field2 &F2, const Blackbox& B)
{
    typedef typename Blackbox::template rebind<Field2>::other FBlackbox;

    FBlackbox A(B, F2);

    return testBlackboxNoRW(A);
}



/* Test 1: Application of zero matrix onto random vectors
 *
 * Construct a random diagonal matrix and its opposite, then construct
 * the sum of the two matrices. Apply to random vectors and check that
 * the result is zero.
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 *
 * Return true on success and false on failure
 */
template <class Field1, class Field2, class Vector>
static bool testZeroApply (Field1 &F1, Field2 &F2, VectorStream<Vector> &stream1, VectorStream<Vector> &stream2)
{
	commentator().start ("Testing zero apply", "testZeroApply", stream1.m ());

	bool ret = true;

	Vector d1(F1), d2(F1), v(F1), w(F1)
		// , zero
		;
	VectorDomain<Field1> VD (F1);

	// VectorWrapper::ensureDim (zero, stream1.dim ());
	VectorWrapper::ensureDim (d1, stream1.dim ());
	VectorWrapper::ensureDim (d2, stream1.dim ());
	VectorWrapper::ensureDim (v, stream1.dim ());
	VectorWrapper::ensureDim (w, stream2.dim ());


	while (stream1) {
		commentator().startIteration ((unsigned)stream1.j ());
		bool iter_passed = true;

		stream1.next (d1);
		VD.mul (d2, d1, F1.mOne);

		Diagonal <Field1> D1 (d1), D2 (d2);

		Sum <Diagonal<Field1>,Diagonal <Field1> > A (&D1, &D2);

		ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);
		report << "Diagonal matrix:  ";
		VD.write (report, d1);
		report << endl;

		report << "Negative diagonal matrix:  ";
		VD.write (report, d2);
		report << endl;

		stream2.reset ();

		while (stream2) {
			stream2.next (w);

			report << "Input vector:  ";
			VD.write (report, w);
			report << endl;

			A.apply (v, w);

			report << "Output vector:  ";
			VD.write (report, v);
			report << endl;

			if (!VD.isZero (v))
				ret = iter_passed = false;
		}

		if (!iter_passed)
			commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_ERROR)
				<< "ERROR: Vector is not zero" << endl;

		commentator().stop ("done");
		commentator().progress ();

                ret = ret && testBBrebind(F2, A);

	}

	commentator().stop (MSG_STATUS (ret), (const char *) 0, "testZeroApply");

	return ret;
}

#if 0

/* Test 2: Random transpose
 *
 * Compute a random diagonal matrix and use the transpose test in test-generic.h
 * to check consistency of transpose apply.
 *
 * F - Field over which to perform computations
 * n - Dimension to which to make matrix
 * iterations - Number of random vectors to which to apply matrix
 *
 * Return true on success and false on failure
 */

template <class Field>
static bool testRandomTranspose (Field &F, size_t n, int iterations)
{
	typedef vector <typename Field::Element> Vector;

	commentator().start ("Testing random transpose", "testRandomTranspose", iterations);

	Vector d(n);
	typename Field::RandIter r (F);

	for (int i = 0; i < n; i++)
		r.random (d[i]);

	Diagonal <Field, Vector> D (F, d);

	ostream &report = commentator().report (Commentator::LEVEL_IMPORTANT, INTERNAL_DESCRIPTION);

	report << "Diagonal vector: ";
	printVector<Field> (F, report, d);

	bool ret = testTranspose<Field> (F, D, iterations);

	commentator().stop (MSG_STATUS (ret), (const char *) 0, "testRandomTranspose");

	return ret;
}

#endif

int main (int argc, char **argv)
{
	bool pass = true;

	static size_t n = 10;
	static integer q1 = 101;
	static integer q2 = 1009;
	static int iterations1 = 2;
	static int iterations2 = 1;

	static Argument args[] = {
		{ 'n', "-n N", "Set dimension of test matrices to NxN.", TYPE_INT,     &n },
		{ 'q', "-q Q", "Operate over the \"field\" GF(Q) [1].", TYPE_INTEGER, &q1 },
		{ 'z', "-z Q", "Operate over the \"field\" GF(Q) [1].", TYPE_INTEGER, &q2 },
		{ 'i', "-i I", "Perform each test for I iterations.", TYPE_INT,     &iterations1 },
		{ 'j', "-j J", "Apply test matrix to J vectors.", TYPE_INT,     &iterations2 },
		END_OF_ARGUMENTS
	};

#ifdef __LINBOX_HAVE_NTL
//        typedef Givaro::ZRing<NTL::zz_p> Field;
        typedef NTL_zz_p Field;
// 	NTL::zz_p::init(q1); // Done in the constructor
#else
	typedef Givaro::Modular<double> Field ;
	//typedef Givaro::Modular<int32_t> Field ;
#endif
	Field F1(q1); Field::RandIter gen(F1);
    

        Givaro::Modular<double> F2(q2);
        //Givaro::Modular<int32_t> F2(q2);

	// typedef BlasVector<Field> Vector;

	parseArguments (argc, argv, args);

	commentator().start("Sum black box test suite", "sum");

	// Make sure some more detailed messages get printed
	commentator().getMessageClass (INTERNAL_DESCRIPTION).setMaxDepth (2);

	RandomDenseStream<Field> stream1 (F1, gen, n, iterations1), stream2 (F1, gen, n, iterations2);

	if (!testZeroApply (F1, F2, stream1, stream2)) pass = false;

	n = 10;
	RandomDenseStream<Field> stream3 (F1, gen, n, iterations1), stream4 (F1, gen, n, iterations2);

	// Vector d1(n), d2(n);
	// stream3.next (d1);
	// stream4.next (d2);

//	Diagonal <Field, Vector> D1 (F, d1), D2 (F, d2);

	Field::Element d; F1.init(d, 5);
	ScalarMatrix<Field> D1(F1, 10, 10, d), D2(F1, 10, 10, d);
	typedef ScalarMatrix<Field> Blackbox;

	Sum <Blackbox, Blackbox> A (D1, D2);
	pass = pass && testBlackboxNoRW(A) && testBBrebind(F2, A);


        Sum <Blackbox, Blackbox> Aref (&D1, &D2);
	pass = pass && testBlackboxNoRW(Aref) && testBBrebind(F2, A);

	commentator().stop("Sum black box test suite");
	return pass ? 0 : -1;
}

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
