/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
// vim:sts=8:sw=8:ts=8:noet:sr:cino=>s,f0,{0,g0,(0,\:0,t0,+0,=s
/* linbox/matrix/blas-matrix.h
 * Copyright (C) 2004 Pascal Giorgi, Clément Pernet
 *
 * Written by :
 *               Pascal Giorgi  <pascal.giorgi@ens-lyon.fr>
 *               Clément Pernet <clement.pernet@imag.fr>
 *               Brice Boyer    <bboyer@imag.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street - Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

/*!@internal
 * @file matrix/blas-matrix.inl
 * @ingroup matrix
 * A \c BlasMatrix<\c _Field > represents a matrix as an array of
 * <code>_Field</code>s.
 */

#ifndef __LINBOX_blas_matrix_INL
#define __LINBOX_blas_matrix_INL

/////////////////
//   PRIVATE   //
/////////////////

namespace LinBox
{
	template<class _Field>
	void BlasMatrix<_Field>::createBlasMatrix (const BlasMatrix<_Field>& A)
	{
		linbox_check( areFieldEqual(A.field(),_F));
		createBlasMatrix(A,MatrixContainerCategory::BlasContainer());
	}

	template<class _Field>
	void BlasMatrix<_Field>::createBlasMatrix (const Element * v)
	{
		const_pointer iter_v = v ;
		const_pointer v_end = v+(_col*_row) ;
		Iterator  iter_addr = this->Begin();
		for (; v != v_end ; ++v, ++iter_addr)
			_F.init(*iter_addr,*v);
	}

	template<class _Field>
	void BlasMatrix<_Field>::createBlasMatrix (const std::vector<Element> & v)
	{
		typename std::vector< _Field>::const_iterator iter_value = v.begin();
		Iterator  iter_addr = this->Begin();
		for (;iter_value != v.end(); ++iter_value,++iter_addr)
			_F.init(*iter_addr,*iter_value);
	}


	template<class _Field>
	template <class _Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const _Matrix& A,
						   MatrixContainerCategory::BlasContainer)
	{
		linbox_check( areFieldEqual(A.field(),_F));
		typename _Matrix::ConstIterator         iter_value = A.Begin();
		Iterator  iter_addr = this->Begin();
		for (;iter_value != A.End(); ++iter_value,++iter_addr)
			_F.init(*iter_addr, *iter_value);
	}

	template<class _Field>
	template <class Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const Matrix& A,
						   MatrixContainerCategory::Container)
	{
		linbox_check( areFieldEqual(A.field(),_F));
		// const Field & F = A.field();
		//!@bug With both iterators, it is Segfaulting !!!!
		typename Matrix::ConstIndexedIterator  iter_index = A.IndexedBegin();
		for (;iter_index != A.IndexedEnd(); ++iter_index)
			setEntry( iter_index.rowIndex(),
				  iter_index.colIndex(),
				  A.getEntry(iter_index.rowIndex(),iter_index.colIndex())
				);
	}

	template<class _Field>
	template <class Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const Matrix& A,
						   MatrixContainerCategory::Blackbox)
	{
		linbox_check( areFieldEqual(A.field(),_F) );

		std::vector<Element> e(A.coldim(), _F.zero), tmp(A.rowdim());
		ColIterator col_p;

		typename BlasMatrix< _Field>::Col::iterator elt_p;
		typename std::vector<Element>::iterator e_p, tmp_p;


		for (col_p = colBegin(), e_p = e.begin();
		     e_p != e.end(); ++ col_p, ++ e_p)
		{

			_F.assign(*e_p, _F.one);

			A.apply (tmp, e);

			for (tmp_p = tmp.begin(), elt_p = col_p -> begin();
			     tmp_p != tmp.end(); ++ tmp_p, ++ elt_p)

				_F.assign(*elt_p, *tmp_p);

			_F.assign(*e_p, _F.zero);
		}
	}

	template<class _Field>
	template <class _Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const _Matrix& A,
						   const size_t i0,const size_t j0,
						   const size_t m, const size_t n,
						   MatrixContainerCategory::Container)
	{
		linbox_check( areFieldEqual(A.field(),_F) );

		typename _Matrix::ConstIterator         iter_value = A.Begin();
		typename _Matrix::ConstIndexedIterator  iter_index = A.IndexedBegin();

		for (;iter_value != A.End(); ++iter_value,++iter_index){
			size_t i,j;
			i=iter_index.rowIndex()-i0;
			j=iter_index.colIndex()-j0;
			if (( i >= 0) && (i< m) && (j >= 0) && (j < n))
				setEntry(i, j, *iter_value);
		}
	}

	template<class _Field>
	template <class Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const Matrix& A,
						   const size_t i0,const size_t j0,
						   const size_t m, const size_t n,
						   MatrixContainerCategory::BlasContainer)
	{
		linbox_check( areFieldEqual(A.field(),_F) );

		typename Matrix::ConstIterator         iter_value = A.Begin();
		typename Matrix::ConstIndexedIterator  iter_index = A.IndexedBegin();

		for (;iter_value != A.End(); ++iter_value,++iter_index){
			size_t i,j;
			linbox_check(iter_index.rowIndex()>=i0);
			linbox_check(iter_index.colIndex()>=j0);
			i=iter_index.rowIndex()-i0;
			j=iter_index.colIndex()-j0;
			// if (( i >= 0) && (i< m) && (j >= 0) && (j < n))
			if ( (i< m) && (j < n))
				setEntry(i, j, *iter_value);
		}
	}

	template<class _Field>
	template <class Matrix>
	void BlasMatrix<_Field>::createBlasMatrix (const Matrix& A,
						   const size_t i0,const size_t j0,
						   const size_t m, const size_t n,
						   MatrixContainerCategory::Blackbox)
	{
		linbox_check( areFieldEqual(A.field(),_F) );


		std::vector<Element> e(A.coldim(), _F.zero), tmp(A.rowdim());
		ColIterator col_p;

		typename BlasMatrix< _Field>::Col::iterator elt_p;
		typename std::vector<Element>::iterator e_p, tmp_p;


		for (col_p = colBegin(), e_p = e.begin()+j0;
		     e_p != e.begin()+j0+n; ++ col_p, ++ e_p) {

			_F.assign(*e_p, _F.one);

			A.apply (tmp, e);

			for (tmp_p = tmp.begin()+i0, elt_p = col_p -> begin();
			     elt_p != col_p.end(); ++ tmp_p, ++ elt_p) {
				_F.assign(*elt_p, *tmp_p);
			}

			_F.assign(*e_p, _F.zero);
		}
	}

} // LinBox

//////////////////
// CONSTRUCTORS //
//////////////////

namespace LinBox
{


	template <class _Field>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F) :
		_row(0),_col(0),_rep(0),_ptr(NULL),_F(F),_MD(F),_VD(F),_use_fflas(false)
	{ }



	template <class _Field>
	template<class T>
	BlasMatrix< _Field>::BlasMatrix ( const _Field &F, unsigned int m, T n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(F),_VD(F)
	{
		_use_fflas = Protected::checkBlasApply(_F,_col);
	}

	template <class _Field>
	template<class T>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F, long m, T n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(F),_VD(F)
	{
		linbox_check(n>=0);
		linbox_check(m>=0);
		_use_fflas = Protected::checkBlasApply(_F,_col);
	}

	template <class _Field>
	template<class T>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F, unsigned long m, T  n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(F),_VD(F)
	{
		//!@todo
		// linbox_check_non_neg(n);
		// linbox_check(n>=0);
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
	}

	template <class _Field>
	template<class T>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F, int m, T n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(F),_VD(F)
	{
		linbox_check(n>=0);
		linbox_check(m>=0);
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
	}

	template <class _Field>
	template<class T>
	BlasMatrix< _Field>::BlasMatrix ( const _Field &F, Integer & m, T n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(F),_VD(F)
	{
		//!@todo check m,n not too big ?
		linbox_check(n>=0);
		linbox_check(m>=0);
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
	}



	template <class _Field>
	BlasMatrix< _Field>::BlasMatrix(MatrixStream<_Field>& ms) :
		_row(0),_col(0),_rep(0),_F(ms.getField()),_MD(_F),_VD(_F)
	{
		if( !ms.getArray(_rep) || !ms.getRows(_row) || !ms.getColumns(_col) )
			throw ms.reportError(__FUNCTION__,__LINE__);
		_ptr = &_rep[0];
		_use_fflas = Protected::checkBlasApply(_F, _col);
	}

	template <class _Field>
	template <class Matrix>
	BlasMatrix< _Field>::BlasMatrix (const Matrix &A) :
		_row(A.rowdim()),_col(A.coldim()),_rep(_row*_col),_ptr(&_rep[0]),_F(A.field()),_MD(_F),_VD(_F)
	{
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
		createBlasMatrix(A, typename MatrixContainerTrait<Matrix>::Type());
	}

	template <class _Field>
	template <class Matrix>
	BlasMatrix< _Field>::BlasMatrix (const Matrix& A,
					 const size_t i0, const size_t j0,
					 const size_t m, const size_t n) :
		_row(m),_col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(A.field()),_MD(_F),_VD(_F)
	{
		_use_fflas = Protected::checkBlasApply(_F, _col);
		// makePointer();
		createBlasMatrix(A, i0, j0, m, n,
				 typename MatrixContainerTrait<Matrix>::Type());
	}

	template <class _Field>
	template<class _Matrix>
	BlasMatrix< _Field>::BlasMatrix (const _Matrix &A,  const _Field &F) :
		_row(A.rowdim()), _col(A.coldim()),_rep(_row*_col),_ptr(&_rep[0]),
		_F(F),_MD(_F),_VD(_F)
	{
		_use_fflas = Protected::checkBlasApply(_F, _col);
		typename _Matrix::template rebind<_Field>()(*this,A,F);
	}

	template <class _Field>
	BlasMatrix< _Field>::BlasMatrix (const BlasMatrix< _Field>& A) :
		_row(A.rowdim()), _col(A.coldim()),_rep(_row*_col),_ptr(&_rep[0]),_F(A.field()),_MD(_F),_VD(_F)
	{
		_use_fflas = Protected::checkBlasApply(_F, _col);
		// makePointer();
		createBlasMatrix(A);
	}

	template <class _Field>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F,
					 const std::vector<typename _Field::Element>& v,
					 size_t m, size_t n) :
		_row(m), _col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(_F),_VD(_F)
	{
		linbox_check(v.size() == m*n);
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
		createBlasMatrix(v);
	}

	template <class _Field>
	BlasMatrix< _Field>::BlasMatrix (const _Field &F,
					 const typename _Field::Element * v,
					 size_t m, size_t n) :
		_row(m), _col(n),_rep(_row*_col),_ptr(&_rep[0]),_F(F),_MD(_F),_VD(_F)
	{
		// makePointer();
		_use_fflas = Protected::checkBlasApply(_F, _col);
		createBlasMatrix(_F,v);
	}

	template <class _Field>
	BlasMatrix< _Field>::~BlasMatrix ()
	{
		// if (_ptr)
		// free(_ptr);
	}

} // LinBox

///////////////////
//      I/O      //
///////////////////

namespace LinBox
{

	template <class _Field>
	// template <class Field>
	std::istream &BlasMatrix< _Field>::read (std::istream &file)
	{
#if 0
		Iterator p;
		int m,n;
		char c;
		file>>m>>n>>c;

		if (m*n < _row*_col)
			cerr<<"NOT ENOUGH ELEMENT TO READ\n";
		else {
			for (p = Begin (); p != End (); ++p) {
				integer tmp;
				file>>tmp;cout<<tmp<<endl;
				//file.ignore(1);
				F.read (file, *p);
			}
		}
#endif


		Iterator p;
		int m,n;
		char c;
		file>>m>>n>>c;
		// std::cout << m << 'x' << n << ':' << c << std::endl;
		_row = m; _col = n;

		// _ptr.resize(_row * _col, zero);
		resize(_row,_col);

		if ((c != 'M') && (c != 'm')) {
			for (p = Begin (); p != End (); ++p) {
				//file.ignore(1);
				_F.read (file, *p);
			}

		}
		else { // sparse file format - needs fixing
			int i, j;
			while (true)
			{
				file >> i >> j;
				//file.ignore(1);
				//if (! file) break;
				if (i+j <= 0) break;
				// std::cout << i << ',' << j << ':' ;
				_F.read (file, _ptr[_col*(i-1) + j-1]);
				// std::cout << _ptr[_col*(i-1) + j-1] << std::endl;
			}
		}

		return file;
	}

	template <class _Field>
	std::ostream& BlasMatrix< _Field>::write (std::ostream &os,
						  bool mapleFormat) const
	{

		ConstRowIterator p;

		if (!mapleFormat) {
			integer c;
			int wid;




			_F.cardinality (c);

			if (c >0)
				wid = (int) ceil (log ((double) c) / M_LN10);
			else {
				integer tmp;
				size_t max=0;
				ConstIterator it = Begin();
				for (; it != End(); ++it){
					_F.convert(tmp,*it);
					if (tmp.bitsize() > max)
						max= tmp.bitsize();
				}
				wid= (int) ceil ((double)max / M_LN10)+1;
			}

			for (p = rowBegin (); p != rowEnd ();++p) {
				typename ConstRow::const_iterator pe;

				os << "  [ ";

				for (pe = p->begin (); pe != p->end (); ++pe) {
					os.width (wid);
					/*!  @warning
					 * matrix base does not provide this field(), maybe should?
					 * _M.field ().write (os, *pe);
					 * os << *pe;
					 * fixed by using extra field
					 */

					_F.write (os, *pe);
					os << " ";
				}

				os << "]" << std::endl;
			}
		}
		else {

			os << "Matrix( " << rowdim() << ',' << coldim() << ",[" ;
			for (p = rowBegin (); p != rowEnd (); ) {
				typename ConstRow::const_iterator pe;

				os << " [ ";

				for (pe = p->begin (); pe != p->end (); ) {
					_F.write (os, *pe);
					++pe ;
					if (pe != p->end())
						os << ", ";
				}

				os << "]" ;
				++p ;
				if (p != rowEnd() )
					os << ',' << std::endl;;

			}
			os << "])" ;
		}
		return os;
	}

	template <class _Field>
	BlasMatrix< _Field>& BlasMatrix< _Field>::operator= (const BlasMatrix< _Field>& A)
	{

		_col = A.coldim();
		_row = A.rowdim();
		_rep = Rep(_row*_col);
		_ptr = &_rep[0] ;
		// makePointer();
		createBlasMatrix(A);

		return *this;
	}

#if 0
	template <class _Field>
	template<typename _Tp1>
	struct BlasMatrix< _Field>::rebind {
		typedef BlasMatrix<_Tp1> other;

		void operator() (other & Ap, const Self_t& A, const _Tp1& F) {
			// typedef typename BlasMatrix< _Field>::ConstIndexedIterator ConstIndexedIterator ;
			// typedef typename BlasMatrix< _Field>::ConstIterator ConstIterator ;
			ConstIterator         iter_value = A.Begin();
			ConstIndexedIterator  iter_index = A.IndexedBegin();
			typename _Tp1::Element tmp;
			for (;iter_value != A.End(); ++iter_value,++iter_index){
				Ap.field().init(  tmp, *iter_value );
				Ap.setEntry(iter_index.rowIndex(), iter_index.colIndex(),tmp);
			}
		}
		};
#endif

#if 1 /*  HOM */
	template <class _Field>
	template<typename _Tp1>
	struct BlasMatrix< _Field>::rebind {
		typedef BlasMatrix<_Tp1> other;

		void operator() (other & Ap, const Self_t& A, const _Tp1& F) {
			typedef typename BlasMatrix<_Field>::ConstIterator ConstSelfIterator ;
			typedef typename other::Iterator OtherIterator ;
			OtherIterator    Ap_p;
			ConstSelfIterator A_p;
			Hom<Field, _Tp1> hom(A. field(), F);
			for (A_p = A. Begin(), Ap_p = Ap.Begin();
			     A_p != A. End(); ++ A_p, ++ Ap_p)
				hom.image (*Ap_p, *A_p);
		}
	};
#endif


	} // LinBox

//////////////////
//  DIMENSIONS  //
//////////////////

namespace LinBox
{

	template <class _Field>
	size_t BlasMatrix< _Field>::rowdim() const
	{
		return _row ;
	}

	template <class _Field>
	size_t BlasMatrix< _Field>::coldim() const
	{
		return _col ;
	}

	template <class _Field>
	size_t  BlasMatrix< _Field>::getStride() const
	{
		return _col;
	}

	template <class _Field>
	size_t&  BlasMatrix< _Field>::getWriteStride()
	{
		return _col;
	}

	template <class _Field>
	void BlasMatrix< _Field>::resize (size_t m, size_t n, const Element& val )
	{
#ifndef NDEBUG
		if (_col != n)
			std::cerr << " ***Warning*** you are resizing a matrix, possibly loosing data. " << std::endl;
#endif
		_row = m;
		_col = n;
		_rep.resize (m * n, val);
#if 0
		if (_ptr) {
			if (m && n)
				realloc(_ptr,m*n*sizeof(Element));
			else {
				free(_ptr);
				_ptr=NULL ;
			}
		}
		else
			makePointer();
#endif
	}

} // LinBox

//////////////////
//   ELEMENTS   //
//////////////////

namespace LinBox
{


	template <class _Field>
	typename BlasMatrix< _Field>::pointer
	BlasMatrix< _Field>::getPointer() const
	{
		return _ptr;
	}

	template <class _Field>
	typename BlasMatrix< _Field>::const_pointer &
	BlasMatrix< _Field>::getConstPointer() const
	{
		return (const_pointer)_ptr;
	}


	template <class _Field>
	typename BlasMatrix< _Field>::pointer&
	BlasMatrix< _Field>::getWritePointer()
	{
		return _ptr;
	}

	template <class _Field>
	void BlasMatrix< _Field>::setEntry (size_t i, size_t j, const Element &a_ij)
	{
		_ptr[i * _col + j] = a_ij;
	}

	template <class _Field>
	typename _Field::Element & BlasMatrix< _Field>::refEntry (size_t i, size_t j)
	{
		return _ptr[i * _col + j];
	}

	template <class _Field>
	const typename _Field::Element & BlasMatrix< _Field>::getEntry (size_t i, size_t j) const
	{
		return _ptr[i * _col + j];
	}

	template <class _Field>
	typename _Field::Element & BlasMatrix< _Field>::getEntry (Element &x, size_t i, size_t j) const
	{
		x = _ptr[i * _col + j];
		return x;
	}

} // LinBox

///////////////////
// TRANSPOSE &AL //
///////////////////

namespace LinBox
{
	template <class _Field>
	BlasMatrix< _Field> BlasMatrix< _Field>::transpose(BlasMatrix< _Field> & tM) const
	{
		size_t r = this->rowdim() ;
		size_t c = this->coldim() ;
		linbox_check(tM.coldim() == r );
		linbox_check(tM.rowdim() == c);
		for (size_t i = 0 ; i < r ; ++i)
			for (size_t j = 0 ; j < c ; ++j)
				tM.setEntry(j,i,this->getEntry(i,j));
		return tM;
	}

	template <class _Field>
	void BlasMatrix< _Field>::transpose()
	{
		size_t r = this->rowdim() ;
		size_t c = this->coldim() ;
		if ( r == c) {
			for (size_t i = 0 ; i < r ; ++i)
				for (size_t j = i+1 ; j < c ; ++j)
					std::swap(this->refEntry(i,j),this->refEntry(j,i));
		}
		else {
			// maybe this should be possible on a DenseMatrix sharing its data with
			// a BlasMatrix but with rowdim/coldim integer members.
			// Or this has an integer rowdim/coldim member, mutable.
			throw LinBoxError("you cannot transpose a BlasMatrix in place where m != n...");
		}
	}

	template <class _Field>
	void BlasMatrix< _Field>::reverseRows()
	{
		size_t r = this->rowdim()/2 ;
		for (size_t i = 0 ; i <  r ; ++i) {
			_VD.swap( this->rowBegin()+i,
				 this->rowBegin()+(r-1-i) );
		}

	}

	template <class _Field>
	void BlasMatrix< _Field>::reverseCols()
	{
		size_t r = this->rowdim() ;
		size_t c = this->coldim()/2 ;
		for (size_t j = 0 ; j < c ; ++j) {
			for (size_t i = 0 ; i < r ; ++i) {
				std::swap(this->refEntry(i,j),
					  this->refEntry(i,c-j-1));

			}
		}
	}

	template <class _Field>
	void BlasMatrix< _Field>::reverse()
	{
		size_t r = this->rowdim() ;
		size_t c = this->coldim() ;
		for (size_t j = 0 ; j < c ; ++j) {
			for (size_t i = 0 ; i < r ; ++i) {
				std::swap(this->refEntry(i,j),
					  this->refEntry(r-i-1,c-j-1));

			}
		}
	}
} // LinBox

///////////////////
//   ITERATORS   //
///////////////////

namespace LinBox
{

	template <class _Field>
	class BlasMatrix< _Field>::ConstRowIterator {
	public:
		ConstRowIterator (const typename Rep::const_iterator& p, size_t len, size_t d) :
			_row (p, p + len), _dis (d)
		{}

		ConstRowIterator () {}

		ConstRowIterator (const ConstRowIterator& colp) :
			_row (colp._row), _dis (colp._dis)
		{}

		ConstRowIterator& operator = (const ConstRowIterator& colp)
		{
			_row = colp._row;
			_dis = colp._dis;
			return *this;
		}

		ConstRowIterator& operator --()
		{
			_row = ConstRow (_row.begin () - _dis, _row.end () - _dis);
			return *this;
		}

		ConstRowIterator  operator-- (int)
		{
			ConstRowIterator tmp (*this);
			--*this;
			return tmp;
		}


		ConstRowIterator& operator++ ()
		{
			_row = ConstRow (_row.begin () + _dis, _row.end () + _dis);
			return *this;
		}

		ConstRowIterator  operator++ (int)
		{
			ConstRowIterator tmp (*this);
			++*this;
			return tmp;
		}

		ConstRowIterator operator+ (int i)
		{
			return ConstRowIterator (_row.begin () + _dis * i, _row.size (), _dis);
		}

		ConstRowIterator& operator += (int i)
		{
			_row = ConstRow (_row.begin () + _dis * i, _row.end () + _dis * i);
			return *this;
		}

		ConstRow operator[] (int i) const
		{
			return ConstRow (_row.begin () + _dis * i, _row.end () + _dis * i);
		}

		ConstRow* operator-> ()
		{
			return &_row;
		}

		ConstRow& operator* ()
		{
			return _row;
		}

		bool operator!= (const ConstRowIterator& c) const
		{
			return (_row.begin () != c._row.begin ()) || (_row.end () != c._row.end ()) || (_dis != c._dis);
		}

	private:
		ConstRow _row;
		size_t _dis;
	};

	template <class _Field>
	class BlasMatrix< _Field>::RowIterator {
	public:
		RowIterator (const typename Rep::iterator& p, size_t len, size_t d) :
			_row (p, p + len), _dis (d)
		{}

		RowIterator () {}

		RowIterator (const RowIterator& colp) :
			_row (colp._row), _dis (colp._dis)
		{}

		RowIterator& operator = (const RowIterator& colp)
		{
			_row = colp._row;
			_dis = colp._dis;
			return *this;
		}

		RowIterator& operator ++ ()
		{
			_row = Row (_row.begin () + _dis, _row.end () + _dis);
			return *this;
		}

		RowIterator  operator ++ (int)
		{
			RowIterator tmp (*this);
			++*this;
			return tmp;
		}

		RowIterator& operator -- ()
		{
			_row = Row (_row.begin () - _dis, _row.end () - _dis);
			return *this;
		}

		RowIterator  operator -- (int)
		{
			RowIterator tmp (*this);
			--*this;
			return tmp;
		}

		RowIterator operator + (int i)
		{
			return RowIterator (_row.begin () + _dis * i, _row.size (), _dis);
		}

		RowIterator& operator += (int i)
		{
			_row = Row (_row.begin () + _dis * i, _row.end () + _dis * i);
			return *this;
		}

		Row operator[] (int i) const
		{
			return Row (const_cast<Row&> (_row).begin () + _dis * i,
				    const_cast<Row&> (_row).end () + _dis * i);
		}

		Row* operator-> ()
		{
			return &_row;
		}

		Row& operator* ()
		{
			return _row;
		}

		bool operator!= (const RowIterator& c) const
		{
			return (_row.begin () != c._row.begin ()) || (_row.end () != c._row.end ()) || (_dis != c._dis);
		}

		operator ConstRowIterator ()
		{
			return ConstRowIterator (_row.begin (), _row.size (), _dis);
		}

	private:
		Row _row;
		size_t _dis;
	};

	template <class _Field>
	class BlasMatrix< _Field>::ConstColIterator {
	public:
		ConstColIterator (typename Rep::const_iterator p, size_t stride, size_t len) :
			_col (Subiterator<typename Rep::const_iterator> (p, stride),
			      Subiterator<typename Rep::const_iterator> (p + len * stride, stride)),
			_stride (stride)
		{}

		ConstColIterator (const ConstCol& col, size_t stride) :
			_col (col),
			_stride (stride)
		{}

		ConstColIterator () {}

		ConstColIterator (const ConstColIterator& rowp) :
			_col (rowp._col)
		{}

		ConstColIterator& operator= (const ConstColIterator& rowp)
		{
			_col = rowp._col;
			_stride = rowp._stride;
			return *this;
		}

		ConstColIterator& operator++ ()
		{
			_col = ConstCol (Subiterator<typename Rep::const_iterator> (_col.begin ().operator-> () + 1, _stride),
					 Subiterator<typename Rep::const_iterator> (_col.end ().operator-> () + 1, _stride));
			return *this;
		}

		ConstColIterator  operator++ (int)
		{
			ConstColIterator old(*this);
			this->operator++ ();
			return old;
		}

		ConstColIterator operator + (int i)
		{
			return ConstColIterator (_col.begin ().operator-> () + i, _stride, _col.size ());
		}

		ConstColIterator& operator += (int i)
		{
			_col = ConstCol (Subiterator<typename Rep::const_iterator> (_col.begin ().operator-> () + i, _stride),
					 Subiterator<typename Rep::const_iterator> (_col.end ().operator-> () + i, _stride));
			return *this;
		}

		ConstCol operator[] (int i) const
		{
			return ConstCol (Subiterator<typename Rep::const_iterator> (_col.begin ().operator-> () + i, _stride),
					 Subiterator<typename Rep::const_iterator> (_col.end ().operator-> () + i, _stride));
		}

		ConstCol* operator-> ()
		{
			return &_col;
		}

		ConstCol& operator* ()
		{
			return _col;
		}

		bool operator!= (const ConstColIterator& c) const
		{
			return (_col.begin () != c._col.begin ()) || (_col.end () != c._col.end ());
		}

	private:
		ConstCol _col;
		size_t _stride;
	};

	template <class _Field>
	class BlasMatrix< _Field>::ColIterator {
	public:
		ColIterator (typename Rep::iterator p, size_t stride, size_t len) :
			_col (Subiterator<typename Rep::iterator> (p, stride),
			      Subiterator<typename Rep::iterator> (p + len * stride, stride)), _stride (stride)
		{}

		ColIterator () {}

		ColIterator (const ColIterator& rowp) :
			_col (rowp._col)
		{}

		ColIterator& operator= (const ColIterator& rowp)
		{
			_col = rowp._col;
			_stride = rowp._stride;
			return *this;
		}

		const ColIterator& operator= (const ColIterator& rowp) const
		{
			const_cast<ColIterator*> (this)->_col = rowp._col;
			return *this;
		}

		ColIterator& operator++ ()
		{
			_col = Col (Subiterator<typename Rep::iterator> (_col.begin ().operator-> () + 1, _stride),
				    Subiterator<typename Rep::iterator> (_col.end ().operator-> () + 1, _stride));
			return *this;
		}

		ColIterator  operator++ (int)
		{
			Col tmp (_col);
			this->operator++ ();
			return tmp;
		}

		ColIterator operator + (int i)
		{
			return ColIterator (_col.begin ().operator-> () + i, _stride, _col.size ());
		}

		ColIterator& operator += (int i)
		{
			_col = Col (Subiterator<typename Rep::iterator> (_col.begin ().operator-> () + i, _stride),
				    Subiterator<typename Rep::iterator> (_col.end ().operator-> () + i, _stride));
			return *this;
		}

		Col operator[] (int i) const
		{
			return Col (Subiterator<typename Rep::iterator> (const_cast<Col&> (_col).begin ().operator-> () + i, _stride),
				    Subiterator<typename Rep::iterator> (const_cast<Col&> (_col).end ().operator-> () + i, _stride));
		}

		Col* operator-> ()
		{
			return &_col;
		}

		Col& operator* ()
		{
			return _col;
		}

		bool operator!= (const ColIterator& c) const
		{
			return (_col.begin () != c._col.begin ()) || (_col.end () != c._col.end ());
		}

		operator ConstColIterator ()
		{
			return ConstColIterator (reinterpret_cast<ConstCol&> (_col) , _stride);
		}

	private:

		Col _col;
		size_t _stride;
	};

	/*!   Indexed Iterator.
	 * @ingroup iterators
	 * @brief NO DOC
	 */
	template <class _Field>
	class BlasMatrix< _Field>::IndexedIterator {
		size_t _r_index;
		size_t _c_index;
		size_t _dim;
		typename Rep::iterator _begin;
		typedef typename _Field::Element value_type;

	public:
		IndexedIterator (const size_t  &dim,
				 const size_t  &r_index,
				 const size_t  &c_index,
				 const typename Rep::iterator &begin) :
			_r_index (r_index), _c_index (c_index), _dim (dim), _begin (begin)
		{}

		IndexedIterator () :
			_r_index (0), _c_index (0), _dim (1), _begin (0)
		{}

		IndexedIterator (const IndexedIterator& r) :
			_r_index (r._r_index), _c_index (r._c_index), _dim (r._dim), _begin (r._begin)
		{}

		IndexedIterator& operator = (const IndexedIterator &iter)
		{
			_r_index = iter._r_index;
			_c_index = iter._c_index;
			_dim = iter._dim;
			_begin = iter._begin;
			return *this;
		}

		bool operator == (const IndexedIterator &iter) const
		{
			return (_r_index == iter._r_index) &&
			(_c_index == iter._c_index) &&
			(_dim == iter._dim) &&
			(_begin==iter._begin);
		}

		bool operator != (const IndexedIterator& iter) const
		{
			return (_r_index != iter._r_index) ||
			(_c_index != iter._c_index) ||
			(_dim != iter._dim) ||
			(_begin!=iter._begin);
		}

		IndexedIterator &operator ++ ()
		{
			++_c_index;

			if (_c_index == _dim) {
				_c_index = 0;
				++_r_index;
			}

			return *this;
		}


		IndexedIterator operator ++ (int)
		{
			IndexedIterator tmp = *this;
			++(*this);
			return tmp;
		}

		IndexedIterator &operator -- ()
		{
			if (_c_index)
				--_c_index;
			else {
				--_r_index;
				_c_index = _dim - 1;
			}

			return *this;
		}


		IndexedIterator operator -- (int)
		{
			IndexedIterator tmp = *this;
			--(*this);
			return tmp;
		}

		value_type &operator * () const
		{
			return *(_begin + (_r_index * _dim + _c_index));
		}


		value_type * operator -> () const
		{
			return _begin + (_r_index * _dim + _c_index);
		}


		size_t rowIndex () const
		{
			return _r_index;
		}

		size_t colIndex () const
		{
			return _c_index;
		}

		const value_type &value () const
		{
			return *(_begin + (_r_index * _dim + _c_index));
		}


	};

	template <class _Field>
	class BlasMatrix< _Field>::ConstIndexedIterator {
		size_t _r_index;
		size_t _c_index;
		size_t _dim;
		typedef typename _Field::Element value_type;
		typename Rep::const_iterator _begin;

	public:
		ConstIndexedIterator (const size_t  &my_dim,
				      const size_t  &r_index,
				      const size_t  &c_index,
				      const typename Rep::const_iterator &begin) :
			_r_index (r_index), _c_index (c_index), _dim (my_dim), _begin (begin)
		{}

		ConstIndexedIterator () :
			_r_index (0), _c_index (0), _dim (1), _begin (0)
		{}

		ConstIndexedIterator (const ConstIndexedIterator& r) :
			_r_index (r._r_index), _c_index (r._c_index), _dim (r._dim), _begin (r._begin)
		{}

		ConstIndexedIterator& operator = (const ConstIndexedIterator &iter)
		{
			_r_index = iter._r_index;
			_c_index = iter._c_index;
			_dim = iter._dim;
			_begin = iter._begin;
			return *this;
		}

		bool operator == (const ConstIndexedIterator &iter) const
		{
			return (_r_index == iter._r_index) &&
			(_c_index == iter._c_index) &&
			(_dim == iter._dim) &&
			(_begin==iter._begin);
		}

		bool operator != (const ConstIndexedIterator& iter) const
		{
			return (_r_index != iter._r_index) ||
			(_c_index != iter._c_index) ||
			(_dim != iter._dim) ||
			(_begin!=iter._begin);
		}

		ConstIndexedIterator &operator ++ ()
		{
			++_c_index;

			if (_c_index == _dim) {
				_c_index = 0;
				++_r_index;
			}

			return *this;
		}


		ConstIndexedIterator operator ++ (int)
		{
			ConstIndexedIterator tmp = *this;
			++(*this);
			return tmp;
		}

		ConstIndexedIterator &operator -- ()
		{
			if (_c_index)
				--_c_index;
			else {
				--_r_index;
				_c_index = _dim - 1;
			}

			return *this;
		}

		ConstIndexedIterator operator -- (int)
		{
			ConstIndexedIterator tmp = *this;
			--(*this);
			return tmp;
		}

		const value_type &operator * () const
		{
			return *(_begin + (_r_index * _dim + _c_index));
		}

		const value_type *operator -> () const
		{
			return _begin + (_r_index * _dim + _c_index);
		}

		size_t rowIndex () const
		{
			return _r_index;
		}

		size_t colIndex () const
		{
			return _c_index;
		}

		const value_type &value() const
		{
			return *(_begin + (_r_index * _dim + _c_index));
		}
	};

	/*   */

	// Entry access  view.  Size m*n vector in C (row major) order.
	template <class _Field>
	typename BlasMatrix< _Field>::Iterator BlasMatrix< _Field>::Begin ()
	{
		return _rep.begin ();
	}

	template <class _Field>
	typename BlasMatrix< _Field>::Iterator BlasMatrix< _Field>::End ()
	{
		return _rep.end ();
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstIterator BlasMatrix< _Field>::Begin () const
	{
		return _rep.begin ();
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstIterator BlasMatrix< _Field>::End () const
	{
		return _rep.end ();
	}

	/*   Indexed  */

	template <class _Field>
	typename BlasMatrix< _Field>::IndexedIterator BlasMatrix< _Field>::IndexedBegin ()
	{
		return IndexedIterator (coldim (), 0, 0, _rep.begin ());
	}

	template <class _Field>
	typename BlasMatrix< _Field>::IndexedIterator BlasMatrix< _Field>::IndexedEnd ()
	{
		return IndexedIterator (coldim (), rowdim (), 0, _rep.begin ());
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstIndexedIterator BlasMatrix< _Field>::IndexedBegin () const
	{
		return ConstIndexedIterator (coldim (), 0, 0, _rep.begin ());
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstIndexedIterator BlasMatrix< _Field>::IndexedEnd () const
	{
		return ConstIndexedIterator (coldim (), rowdim (), 0, _rep.begin ());
	}

	/*  Row  */

	template <class _Field>
	typename BlasMatrix< _Field>::RowIterator BlasMatrix< _Field>::rowBegin ()
	{
		return RowIterator (_rep.begin (), _col, _col);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::RowIterator BlasMatrix< _Field>::rowEnd ()
	{
		return RowIterator (_rep.end (), _col, _col);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstRowIterator BlasMatrix< _Field>::rowBegin () const
	{
		return ConstRowIterator (_rep.begin (), _col, _col);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstRowIterator BlasMatrix< _Field>::rowEnd () const
	{
		return ConstRowIterator (_rep.end (), _col, _col);
	}

	/*  Col */

	template <class _Field>
	typename BlasMatrix< _Field>::ColIterator BlasMatrix< _Field>::colBegin ()
	{
		return  typename BlasMatrix< _Field>::ColIterator (_rep.begin (), _col, _row);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ColIterator BlasMatrix< _Field>::colEnd ()
	{
		return  typename BlasMatrix< _Field>::ColIterator (_rep.begin ()+_col, _col, _row);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstColIterator BlasMatrix< _Field>::colBegin () const
	{
		return  typename BlasMatrix< _Field>::ConstColIterator (_rep.begin (), _col, _row);
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstColIterator BlasMatrix< _Field>::colEnd () const
	{
		return  typename BlasMatrix< _Field>::ConstColIterator (_rep.begin ()+_col, _col, _row);
	}

	/*  operators */
	template <class _Field>
	typename BlasMatrix< _Field>::Row BlasMatrix< _Field>::operator[] (size_t i)
	{
		return Row (_rep.begin () + i * _col, _rep.begin () + (i * _col +_col));
	}

	template <class _Field>
	typename BlasMatrix< _Field>::ConstRow BlasMatrix< _Field>::operator[] (size_t i) const
	{
		return Row (_rep.begin () + i * _col, _rep.begin () + ( i * _col + _col));
	}

} // LinBox

//////////////////
//     MISC     //
//////////////////

namespace LinBox
{
	template <class _Field>
	template <class Vector>
	Vector &BlasMatrix< _Field>::columnDensity (Vector &v) const
	{
		std::fill (v.begin (), v.end (), _row);
		return v;
	}

} // LinBox
#endif // __LINBOX_blas_matrix_INL