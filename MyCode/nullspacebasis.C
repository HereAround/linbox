#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "linbox/matrix/dense-matrix.h"
#include <linbox/linbox-config.h>
#include "linbox/algorithms/gauss.h"
#include <givaro/zring.h>
//#include <linbox/integer.h>
#include <linbox/solutions/rank.h>

using namespace LinBox;


// remove trailing white spaces
std::string trim(const std::string& str)
{
    const auto strBegin = str.find_first_not_of(" ");
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(" ");
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}


// Evaluate strings used as input
std::vector<int> SplitString (const std::string& input_string)
{
    // Remove sequence of white spaces at beginning of input_string
    std::string input = trim( input_string );
    
    // Split string by empty spaces
    std::istringstream iss(input);
    std::string s;
    std::vector<int> data;
    while ( std::getline( iss, s, ' ' ) ) {
        data.push_back( std::atoi(s.c_str()) );
    }
    
    // And return the result
    return data;
}


int main (int argc, char **argv)
{
    
    // We expect that the entries of the matrix are provided as individual arguments - one by one.
    // "2 2 M" "1 1 1" "1 2 1" "2 1 2" "0 0 0" means that we are dealing with a 2x2 matrix.
    // Its entriy (1,1) is 1, entry (1,2) is 1, entry (2,1) is 2 and all others vanish.
    
    // (0) Introduction
    std::cerr << "" << std::endl;
    std::cerr << "This is Linbox..." << std::endl;
    
    // (1) Initialize the field of integers
    //typedef Givaro::QField<Givaro::Rational> Field;
    //Field F;
    typedef Givaro::ZRing<Integer> Field;
    Field F;
    
    // (2) Read first argument, extract dimensions of the sparse matrix and initialize it
    std::vector<int> split;
    split = SplitString( argv[ 1 ] );
    if ( !( split[ 0 ] > 0 && split[ 1 ] > 0 ) ){
        std::cerr << "Error -- specified dimensions of matrix must be positive, but obtained " << argv[1] << std::endl; 
        return -1; 
    }
    SparseMatrix<Field, SparseMatrixFormat::SparseSeq > B (F,split[0],split[1]);
    
    // (3) Fill sparse matrix with entries provided as input
    std::cerr << "(*) Setting entries..." << std::endl;
    std::cerr << argc -1 << std::endl;
    for (int i = 2; i < argc - 1; i++) {
        split = SplitString( argv[ i ] );
        Field::Element v;
        F.init(v, split[ 2 ] );
        B.setEntry( split[ 0 ] - 1, split[ 1 ] - 1, v);
        //std::cerr << "(" << split[ 0 ] << ", " << split[ 1 ] << "): " << split[ 2 ] << std::endl;
        std::cerr << "(" << split[ 0 ] << ", " << split[ 1 ] << "): " << B.getEntry( split[ 0 ] -1 , split[ 1 ] - 1 ) << std::endl;
        
        //<< " = " << str( B.getEntry( split[ 0 ] -1 , split[ 1 ] - 1 ) ) << std::endl;
    }
    std::cerr << "" << std::endl;
    std::cerr << "(*) Matrix read..." << std::endl;
    
    // (4) Determine the rank of the kernel
    size_t r;
    rank (r, B);
    std::cerr << "(*) Kernel has dimension " << B.coldim() - r << std::endl;

    //typename Field::Element Det;
    //size_t Rank;
    //size_t Ni(B.rowdim()),Nj(B.coldim());
    //Permutation<Field> P(F,(int)Nj);
    //Permutation<Field> P(F);
    //GD.InPlaceLinearPivoting(Rank, Det, B, P, Ni, Nj );
    
    /*for(size_t i=0; i< Ni; ++i) {
        if (B[i].size() == 0) {
            size_t j(i);
            if (nextnonzero(j,Ni,B)) {
                B[i] = B[j];
                B[j].resize(0);
            }
            else {
                break;
            }
        }
    }*/
    //std::cerr << B << std::endl;
    
    //size_t nullity = B.coldim()-Rank;
    
    // (5) Compute kernel as dense matrix - unfortunately, this will allocate quite a bit of memory...
    //DenseMatrix<Field> NullSpace(F,B.coldim(),B.coldim());
    DenseMatrix<Field> NullSpace(F,B.coldim(),B.coldim()-r);
    GaussDomain<Field> GD(F);
    GD.nullspacebasisin(NullSpace, B );
    //GD.nullspacebasisin(NullSpace, Rank, B );
    //GD.nullspacebasisin(NullSpace, Rank, B, P);
    std::cerr << "(*) Exact (dense) kernel computed..." << std::endl;
    
    // (6) Process this kernel to turn it (a) into a sparse matrix and (b) see how many columns are actually non-trivial
    Field::Element zero;
    F.init(zero, 0 );
    std::string data_string;
    data_string = "[ ";
    for (int i = 0; (unsigned)i < NullSpace.rowdim(); i++ ){
        for (int j = 0; (unsigned)j < NullSpace.coldim(); j++){
            // Check if the entry is non-zero
            if (NullSpace.getEntry( i, j ) != zero ){
                // add entry to output string
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
    std::cerr << "(*) Stripped of empty columns..." << std::endl;
    
    // (6) Form output_string
    std::string output_string;
    output_string = "[";
    output_string.append( std::to_string( NullSpace.rowdim() ) );
    output_string.append(",");
    output_string.append( std::to_string( NullSpace.coldim() ) );
    output_string.append(",");
    output_string.append(data_string);
    output_string.append("];");
    
    // (7) Output the result to gap
    std::cout << output_string << std::endl;
    
    // and signal success
    std::cerr << "(*) Finished" << std::endl;
    std::cerr << "" << std::endl;
    return 0;
    
}

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
