#include <iostream>
#include "linbox/matrix/dense-matrix.h"
#include <linbox/linbox-config.h>
#include "linbox/algorithms/gauss.h"
#include <givaro/zring.h>
#include <linbox/solutions/rank.h>

using namespace LinBox;


int main (int argc, char **argv)
{
    
    // (0) We expect a matrix a matrix file in sms-format as input
    
    // (1) Initialize the field of integers
    typedef Givaro::ZRing<Integer> Field;
    Field F;
    
    // (2) Read matrix
	std::ifstream input (argv[1]);
	if (!input) { std::cerr << "Error opening matrix file " << argv[1] << std::endl; return -1; }
	SparseMatrix<Field, SparseMatrixFormat::SparseSeq > B (F);
	B.read (input);
    std::cerr << "(*) Matrix read..." << std::endl;
    
    // (3) Determine the dimension of the kernel
    size_t r;
    rank (r, B);
    std::cerr << "(*) Kernel has dimension " << B.coldim() - r << std::endl;
    
    // (4) Compute kernel as dense matrix
    // Unfortunately, this will allocate quite a bit of memory. We use the information on the rank to reduce the consumed memory as much as possible
    DenseMatrix<Field> NullSpace(F,B.coldim(),B.coldim() - r);
    GaussDomain<Field> GD(F);
    GD.nullspacebasisin(NullSpace, B );
    std::cerr << "(*) Exact (dense) kernel computed..." << std::endl;
    
    // (5) Signal success and terminate
    std::cerr << "(*) Finished" << std::endl;
    std::cerr << "" << std::endl;
    return 0;
    
}
