#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "linbox/matrix/dense-matrix.h"
#include <linbox/linbox-config.h>
#include "linbox/algorithms/gauss.h"
#include <givaro/zring.h>
#include <givaro/givrational.h>
#include <linbox/solutions/rank.h>


using namespace LinBox;


int main (int argc, char **argv)
{
    
    // We expect a matrix a matrix file in sms-format as input
	if ( argc !=  2 ) {
		std::cerr << "Usage to get a random null space basis over Z:  <matrix-file-in-SMS-format>" << std::endl;
		return -1;
	}
    
    // (0) Introduction
    std::cerr << "" << std::endl;
    std::cerr << "This is Linbox..." << std::endl;
    
    // (1) Initialize the field of integers
    typedef Givaro::ZRing<Integer> Field; // Givaro::QField<Givaro::Rational> Field; for rationals
    Field F;
    
    // (2) Read matrix
	std::ifstream input (argv[1]);
	if (!input) { std::cerr << "Error opening matrix file " << argv[1] << std::endl; return -1; }
	SparseMatrix<Field, SparseMatrixFormat::SparseSeq > B (F);
	B.read (input);
    std::cerr << "(*) Matrix read..." << std::endl;
    
    // (3) Determine the rank of the kernel
    size_t r;
    rank (r, B );
    std::cerr << "(*) Kernel has dimension " << B.coldim() - r << std::endl;
    
    // (4) Compute kernel as dense matrix
    // Unfortunately, this will allocate quite a bit of memory. We use the information on the rank to reduce the consumed memory as much as possible
    DenseMatrix<Field> NullSpace(F,B.coldim(),B.coldim() - r);
    GaussDomain<Field> GD(F);
    GD.nullspacebasisin(NullSpace, B );
    std::cerr << "(*) Exact (dense) kernel computed..." << std::endl;
    
    // (5) Process this kernel to turn it (a) into a sparse matrix and (b) see how many columns are actually non-trivial
    Field::Element zero;
    F.init(zero, 0 );
    std::string data_string;
    data_string = "[ ";
    for (int i = 0; (unsigned)i < NullSpace.rowdim(); i++ ){
        for (int j = 0; (unsigned)j < NullSpace.coldim(); j++){
            // Check if the entry is non-zero
            if (NullSpace.getEntry( i, j ) != zero ){
                data_string.append("[");
                data_string.append(std::to_string(i+1));
                data_string.append(",");
                data_string.append(std::to_string(j+1));
                data_string.append(",");
                std::string str(NullSpace.getEntry( i,j ));
                data_string.append(str);
                data_string.append("],");
            }
        }
    }
    data_string.pop_back();
    data_string.append("]");
    std::cerr << "(*) Converted into sparse format..." << std::endl;
    
    // (6) Form output_string
    std::string output_string;
    output_string = "[";
    output_string.append( std::to_string( NullSpace.rowdim() ) );
    output_string.append(",");
    output_string.append( std::to_string( NullSpace.coldim() ) );
    output_string.append(",");
    output_string.append(data_string);
    output_string.append("];");
    std::cout << output_string << std::endl;
    std::cerr << "(*) Output string created and returned..." << std::endl;
    
    // (7) Signal success and terminate
    std::cerr << "(*) Finished" << std::endl;
    std::cerr << "" << std::endl;
    return 0;
    
}
