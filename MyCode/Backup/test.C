typedef Givaro::ZRing<Integer> Integers;
                Integers ZZ;

ifstream input (argv[1]);
if (!input)
{ cerr << "Error opening matrix file " << argv[1] << endl;
                        return -1;
}
DenseMatrix<Integers> A(ZZ); A.read(input);
cout << "Matrix is " << A.rowdim() << " by " << A.coldim() << endl;

Integers::Element det_A;
det (det_A, A);

cout << "Determinant is ";
ZZ.write(cout, det_A) << endl;




#ifndef __GIVARO_USE_OPENMP
#define __GIVARO_USE_OPENMP
#endif




//         // Reposition Output with empty rows at the end
//     auto newend = std::remove_if(
//         A.rowBegin(), A.rowEnd(),
//         [](typename SparseMat::ConstRow V)->bool { return V.size()==0; });
//     A.refRep().erase(newend, A.rowEnd());
//     A.refRep().resize(A.rowdim());


LinBox::rank (r, A);
