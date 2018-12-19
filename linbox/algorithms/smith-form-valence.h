/*
 * algorithms/smith-form-valence.h
 * Copyright (c) Linbox
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

/**\file algorithms/smith-form-valence.h
 * @example  algorithms/smith-form-valence.h
 \brief Valence of sparse matrix over Z or Zp.
 \ingroup examples
*/
 
#include <string>
#include <givaro/modular.h>
#include <givaro/givintnumtheo.h>

#include <givaro/gf2.h>
#include <linbox/field/field-traits.h>
#include <linbox/blackbox/transpose.h>
#include <linbox/blackbox/compose.h>
#include <linbox/matrix/sparse-matrix.h>
#include <linbox/solutions/rank.h>
#include <linbox/solutions/valence.h>
#include <linbox/algorithms/smith-form-sparseelim-local.h>
#include <linbox/util/matrix-stream.h>
#include <linbox/util/timer.h>
#include <linbox/util/error.h>
#ifdef SILENT
const bool reporting = false;
#else 
const bool reporting = true;
#endif

#ifdef NOT_USING_OMP
#define THREADS 1
#else
#include <omp.h>
#define THREADS omp_get_thread_num()
#endif

#ifndef __VALENCE_FACTOR_LOOPS__
#define __VALENCE_FACTOR_LOOPS__ 50000
#endif

template<class Field>
unsigned long& TempLRank(unsigned long& r, const char * filename, const Field& F)
{
	std::ifstream input(filename);
	LinBox::MatrixStream< Field > msf( F, input );
	LinBox::SparseMatrix<Field,LinBox::SparseMatrixFormat::SparseSeq> FA(msf);
	input.close();
	LinBox::Timer tim; tim.start();
	LinBox::rankin(r, FA);
	tim.stop();
	if (reporting)
	F.write(std::clog << "Rank over ") << " is " << r << ' ' << tim <<  " on T" << THREADS << std::endl;
	return r;
}

unsigned long& TempLRank(unsigned long& r, const char * filename, const LinBox::GF2& F2)
{
	std::ifstream input(filename);
	LinBox::ZeroOne<LinBox::GF2> A;
	A.read(input);
	input.close();

	LinBox::Timer tim; tim.start();
	LinBox::rankin(r, A, LinBox::Method::SparseElimination() );
	tim.stop();
	if (reporting)
	F2.write(std::clog << "Rank over ") << " is " << r << ' ' << tim <<  " on T" << THREADS << std::endl;
	return r;
}

unsigned long& LRank(unsigned long& r, const char * filename,Givaro::Integer p)
{

	Givaro::Integer maxmod16; LinBox::FieldTraits<Givaro::Modular<int16_t> >::maxModulus(maxmod16);
	Givaro::Integer maxmod32; LinBox::FieldTraits<Givaro::Modular<int32_t> >::maxModulus(maxmod32);
	Givaro::Integer maxmod53; LinBox::FieldTraits<Givaro::Modular<double> >::maxModulus(maxmod53);
	Givaro::Integer maxmod64; LinBox::FieldTraits<Givaro::Modular<int64_t> >::maxModulus(maxmod64);
	if (p == 2) {
		LinBox::GF2 F2;
		return TempLRank(r, filename, F2);
	}
	else if (p <= maxmod16) {
		typedef Givaro::Modular<int16_t> Field;
		Field F(p);
		return TempLRank(r, filename, F);
	}
	else if (p <= maxmod32) {
		typedef Givaro::Modular<int32_t> Field;
		Field F(p);
		return TempLRank(r, filename, F);
	}
	else if (p <= maxmod53) {
		typedef Givaro::Modular<double> Field;
		Field F(p);
		return TempLRank(r, filename, F);
	}
	else if (p <= maxmod64) {
		typedef Givaro::Modular<int64_t> Field;
		Field F(p);
		return TempLRank(r, filename, F);
	}
	else {
		typedef Givaro::Modular<Givaro::Integer> Field;
		Field F(p);
		return TempLRank(r, filename, F);
	}
	return r;
}

std::vector<size_t>& PRank(std::vector<size_t>& ranks, size_t& effective_exponent, const char * filename,Givaro::Integer p, size_t e, size_t intr)
{
	effective_exponent = e;
	Givaro::Integer maxmod;
	LinBox::FieldTraits<Givaro::Modular<int64_t> >::maxModulus(maxmod);
	if (p <= maxmod) {
		typedef Givaro::Modular<int64_t> Ring;
		int64_t lp(p);
		Givaro::Integer q = pow(p,uint64_t(e)); int64_t lq(q);
		if (q > Ring::maxCardinality()) {
			if (reporting)
			std::clog << "Power rank might need extra large composite (" << p << '^' << e << ")." << std::endl;
			q = p;
			for(effective_exponent=1; q <= Ring::maxCardinality(); ++effective_exponent) {
				q *= p;
			}
			q/=p; --effective_exponent;
			lq = (int64_t)q;
            if (effective_exponent <= 1) {
                    // Not able to use Modular<int64_t>, 
                    // modulus is ok, but is already too large when squared
                    // Return that no prime power was performed
                if (reporting)
                    std::clog << "Exceeding int64_t ... power rank useless, nothing done." << std::endl;
                effective_exponent=1;
                return ranks;
            }
			if (reporting)
			std::clog << "First trying: " << lq << " (=" << p << '^' << effective_exponent << ", without further warning this will be sufficient)." << std::endl;
		}
		Ring F(lq);
		std::ifstream input(filename);
		LinBox::MatrixStream<Ring> ms( F, input );
		LinBox::SparseMatrix<Ring,LinBox::SparseMatrixFormat::SparseSeq > A (ms);
		input.close();

		LinBox::PowerGaussDomain< Ring > PGD( F );
        LinBox::Permutation<Ring> Q(F,A.coldim());

		LinBox::Timer tim; tim.clear(); tim.start();
		PGD.prime_power_rankin( lq, lp, ranks, A, Q, A.rowdim(), A.coldim(), std::vector<size_t>());
		tim.stop();
		if (reporting) {
			F.write(std::clog << "Ranks over ") << " are " ;
			for(std::vector<size_t>::const_iterator rit=ranks.begin(); rit != ranks.end(); ++rit)
				std::clog << *rit << ' ';
			std::clog << ' ' << tim <<  " on T" << THREADS << std::endl;
		}
	} else {
            // Not able to use Modular<int64_t>, modulus too large
            // Return that no prime power was performed
        if (reporting)
            std::clog << "Exceeding int64_t ... even for the prime, nothing done." << std::endl;
        effective_exponent=1;
	}
	return ranks;
}

#include <linbox/field/gf2.h>
#include <linbox/algorithms/smith-form-sparseelim-poweroftwo.h>


std::vector<size_t>& PRankPowerOfTwo(std::vector<size_t>& ranks, size_t& effective_exponent, const char * filename, size_t e, size_t intr)
{
	effective_exponent = e;
	if (e > 63) {
		if (reporting)
		std::clog << "Power rank power of two might need extra large composite (2^" << e << ")." << std::endl;
		if (reporting)
		std::clog << "First trying: 63, without further warning this will be sufficient)." << std::endl;
		effective_exponent = 63;
	}


	typedef Givaro::ZRing<int64_t> Ring;
	Ring F;
	std::ifstream input(filename);
	LinBox::MatrixStream<Ring> ms( F, input );
	LinBox::SparseMatrix<Ring,LinBox::SparseMatrixFormat::SparseSeq > A (ms);
	input.close();
	LinBox::PowerGaussDomainPowerOfTwo< uint64_t > PGD;
    LinBox::GF2 F2;
    LinBox::Permutation<LinBox::GF2> Q(F2,A.coldim());

	LinBox::Timer tim; tim.clear(); tim.start();
	PGD.prime_power_rankin( effective_exponent, ranks, A, Q, A.rowdim(), A.coldim(), std::vector<size_t>());
	tim.stop();
	if (reporting) {
		F.write(std::clog << "Ranks over ") << " modulo 2^" << effective_exponent << " are " ;
		for(std::vector<size_t>::const_iterator rit=ranks.begin(); rit != ranks.end(); ++rit)
			std::clog << *rit << ' ';
		std::clog << ' ' << tim <<  " on T" << THREADS << std::endl;
	}
	return ranks;
}

std::vector<size_t>& PRankInteger(std::vector<size_t>& ranks, const char * filename,Givaro::Integer p, size_t e, size_t intr)
{
	typedef Givaro::Modular<Givaro::Integer> Ring;
	Givaro::Integer q = pow(p,uint64_t(e));
	Ring F(q);
	std::ifstream input(filename);
	LinBox::MatrixStream<Ring> ms( F, input );
	LinBox::SparseMatrix<Ring,LinBox::SparseMatrixFormat::SparseSeq > A (ms);
	input.close();
	LinBox::PowerGaussDomain< Ring > PGD( F );
    LinBox::Permutation<Ring> Q(F,A.coldim());

	LinBox::Timer tim; tim.clear(); tim.start();
	PGD.prime_power_rankin( q, p, ranks, A, Q, A.rowdim(), A.coldim(), std::vector<size_t>());
	tim.stop();
	if (reporting){
		F.write(std::clog << "Ranks over ") << " are " ;
		for(std::vector<size_t>::const_iterator rit=ranks.begin(); rit != ranks.end(); ++rit)
			std::clog << *rit << ' ';
		std::clog << ' ' << tim << std::endl;
	}
	return ranks;
}

std::vector<size_t>& PRankIntegerPowerOfTwo(std::vector<size_t>& ranks, const char * filename, size_t e, size_t intr)
{
	typedef Givaro::ZRing<Givaro::Integer> Ring;
	Ring ZZ;
	std::ifstream input(filename);
	LinBox::MatrixStream<Ring> ms( ZZ, input );
	LinBox::SparseMatrix<Ring,LinBox::SparseMatrixFormat::SparseSeq > A (ms);
	input.close();
	LinBox::PowerGaussDomainPowerOfTwo< Givaro::Integer > PGD;
    LinBox::Permutation<Ring> Q(ZZ, A.coldim());

	LinBox::Timer tim; tim.clear(); tim.start();
	PGD.prime_power_rankin( e, ranks, A, Q, A.rowdim(), A.coldim(), std::vector<size_t>());
	tim.stop();
	if (reporting) {
		ZZ.write(std::clog << "Ranks over ") << " modulo 2^" << e << " are " ;
		for(std::vector<size_t>::const_iterator rit=ranks.begin(); rit != ranks.end(); ++rit)
			std::clog << *rit << ' ';
		std::clog << ' ' << tim << std::endl;
	}
	return ranks;
}


typedef std::pair<Givaro::Integer,size_t> PairIntRk;


std::vector<size_t>& AllPowersRanks(
    std::vector<size_t>& ranks,
    const Givaro::Integer& squarefreePrime,// smith[j].first
    const size_t& squarefreeRank,// smith[j].second
    const size_t& exponentBound,	// exponents[j]
    const size_t& coprimeRank,		// coprimeR
    const char * filename) {		// argv[1]

    if (squarefreeRank != coprimeRank) {

        ranks.push_back(squarefreeRank);
        size_t effexp;
        if (exponentBound > 1) {
                // See if a not too small, not too large exponent would work
                // Usually, closest to word size
            if (squarefreePrime == 2)
                PRankPowerOfTwo(ranks, effexp, filename, exponentBound, coprimeRank);
            else
                PRank(ranks, effexp, filename, squarefreePrime, exponentBound, coprimeRank);
        } else {
                // Square does not divide valence
                // Try first with the smallest possible exponent: 2
            if (squarefreePrime == 2)
                PRankPowerOfTwo(ranks, effexp, filename, 2, coprimeRank);
            else
                PRank(ranks, effexp, filename, squarefreePrime, 2, coprimeRank);
        }

        if (effexp < exponentBound) {
                // Above report shows that more powers are needed,
                // try successive doublings Over abitrary precision
            for(size_t expo = effexp<<1; ranks.back() < coprimeRank; expo<<=1) {
                if (squarefreePrime == 2)
                    PRankIntegerPowerOfTwo(ranks, filename, expo, coprimeRank);
                else
                    PRankInteger(ranks, filename, squarefreePrime, expo, coprimeRank);
            }
        } else {
                // Larger exponents are needed
                // Try first small precision, then arbitrary
            for(size_t expo = (exponentBound)<<1; ranks.back() < coprimeRank; expo<<=1) {
                if (squarefreePrime == 2)
                    PRankPowerOfTwo(ranks, effexp, filename, expo, coprimeRank);
                else
                    PRank(ranks, effexp, filename, squarefreePrime, expo, coprimeRank);
                if (ranks.size() < expo) {
                    if (reporting)
                        std::clog << "It seems we need a larger prime power, it will take longer ..." << std::endl;
                        // break;
                    if (squarefreePrime == 2)
                        PRankIntegerPowerOfTwo(ranks, filename, expo, coprimeRank);
                    else
                        PRankInteger(ranks, filename, squarefreePrime, expo, coprimeRank);
                }
            }
        }
    }

    return ranks;
}

std::vector<Givaro::Integer>& populateSmithForm(
    std::vector<Givaro::Integer>& SmithDiagonal,
    const std::vector<size_t>& ranks,
    const Givaro::Integer& squarefreePrime,// smith[j].first
    const size_t& squarefreeRank,// smith[j].second
    const size_t& coprimeRank) {	// coprimeR
   

    SmithDiagonal.resize(coprimeRank, Givaro::Integer(1) );

    for(size_t i=squarefreeRank; i < coprimeRank; ++i) {
        SmithDiagonal[i] *= squarefreePrime;
    }
    auto rit=ranks.begin(); for(++rit; rit!= ranks.end(); ++rit) {
        if ((*rit)>= coprimeRank) break;
        for(size_t i=(*rit); i < coprimeRank; ++i)
            SmithDiagonal[i] *= squarefreePrime;
    }
    
    return SmithDiagonal;
}

template<class Blackbox>
std::vector<Givaro::Integer>& smithValence(std::vector<Givaro::Integer>& SmithDiagonal, 
                                           Givaro::Integer& valence,
                                           const Blackbox& A,
                                           const std::string& filename,
                                           Givaro::Integer& coprimeV, 
                                           size_t method=0) {
        // method for valence squarization: 0 for automatic, 1 for aat, 2 for ata
        // Blackbox provides the Integer matrix rereadable from filename
        // if valence != 0, then the valence is not computed and the parameter is used
        // if coprimeV != 0, then this value is supposed to be coprime with the valence

    std::clog << "sV threads: " << NUM_THREADS << std::endl;

    if (valence == 0) {
        squarizeValence(valence, A, method);
    }

	std::clog << "Valence is " << valence << std::endl;

	std::vector<Givaro::Integer> Moduli;
	std::vector<size_t> exponents;

    std::clog << "Some factors (" << __VALENCE_FACTOR_LOOPS__ << " factoring loop bound): ";
	
 	Givaro::IntFactorDom<> FTD;
    FTD.set(Moduli, exponents, valence, __VALENCE_FACTOR_LOOPS__);

	auto eit=exponents.begin();
	for(auto mit: Moduli) std::clog << mit << '^' << *eit << ' ';
	std::clog << std::endl;

	std::vector< size_t > smith(Moduli.size());

    if (coprimeV == 1) {
        coprimeV=2;
        while ( gcd(valence,coprimeV) > 1 ) {
            FTD.nextprimein(coprimeV);
        }
    }

    size_t coprimeR;
    std::vector<std::vector<size_t> > AllRanks(Moduli.size());

    for(size_t j=0; j<Moduli.size(); ++j) {
        { TASK(MODE(CONSTREFERENCE(Moduli,smith,filename) WRITE(smith[j]) ),
        {
            LRank(smith[j], filename.c_str(), Moduli[j]);
        })}
    }

//     { TASK(MODE(CONSTREFERENCE(coprimeV,filename) WRITE(coprimeR) ),
//     {
        LRank(coprimeR, filename.c_str(), coprimeV);
//     })}

    WAIT;

    SYNCH_GROUP(
        for(size_t j=0; j<Moduli.size(); ++j) {
            { TASK(MODE(CONSTREFERENCE(smith,Moduli,AllRanks,filename,coprimeR,exponents)
                        WRITE(AllRanks[j])),
            {
                AllPowersRanks(AllRanks[j], Moduli[j], smith[j], exponents[j],
                               coprimeR, filename.c_str());
            })}
        }
    )
    
    for(size_t j=0; j<Moduli.size(); ++j) {
        if (smith[j] != coprimeR) {
            populateSmithForm(SmithDiagonal, AllRanks[j], Moduli[j], smith[j], coprimeR);
        }
    }

    return SmithDiagonal;
}

std::ostream& compressedSmith(std::ostream& out, 
                              const std::vector<Givaro::Integer>& SmithDiagonal, 
                              const size_t m, const size_t n) {
    // Output is a list of pairs (integral value, number of repetitions)

    out << '(';

    Givaro::Integer si=1;
	size_t num=0;
	for( auto dit : SmithDiagonal ) {
		if (dit == si) ++num;
		else {
			out << '[' << si << ',' << num << "] ";
			num=1;
			si = dit;
		}
	}
	out << '[' << si << ',' << num << "] ";

	num = std::min(m,n) - SmithDiagonal.size();
	typedef Givaro::ZRing<Givaro::Integer> Ring;
	Ring ZZ;
	si = ZZ.zero;
	if (num > 0) out << '[' << si << ',' << num << ']';
	return out << ')';
}



#undef THREADS

// Local Variables:
// mode: C++
// tab-width: 4
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
// vim:sts=4:sw=4:ts=4:et:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s