/* -*- mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

/* linbox/blackbox/dense.inl
 * Copyright (C) 2001 B. David Saunders, 
 *               2001-2002 Bradford Hovinen, 
 *               2002 Zhendong Wan
 *
 * Written by B. David Saunders <saunders@cis.udel.edu>, 
 *            Bradford Hovinen <hovinen@cis.udel.edu>, 
 *            Zhendong Wan <wan@mail.eecis.udel.edu>
 *
 * evolved from dense-matrix.h by -bds, Zhendong Wan
 *
 * --------------------------------------------------------
 * 2002-08-09  Bradford Hovinen  <hovinen@cis.udel.edu>
 *
 * Renamed file from dense-matrix1.C to dense.inl
 * --------------------------------------------------------
 *
 * See COPYING for license information
 */

#ifndef __DENSE_INL
#define __DENSE_INL

#include <iostream>
#include <vector>
#include <fstream>

#include "linbox/blackbox/dense.h"
#include "linbox/util/debug.h"

namespace LinBox
{

template <class Field, class Vector>
class DenseMatrix<Field, Vector>::ConstColOfRowsIterator
{
    public:
	ConstColOfRowsIterator (const typename Vector::const_iterator& p, size_t len, size_t d)
		: _row (p, p + len), _dis (d) {}
    
	ConstColOfRowsIterator () {}
    
	ConstColOfRowsIterator (const ConstColOfRowsIterator& colp)
		: _row (colp._row), _dis (colp._dis) {}
    
	ConstColOfRowsIterator& operator = (const ConstColOfRowsIterator& colp)
	{
		_row = colp._row;
		_dis = colp._dis;
		return *this;
	}

	ConstColOfRowsIterator& operator++ ()
	{
		_row = ConstRow (_row.begin () + _dis, _row.end () + _dis);
		return *this;
	}

	ConstColOfRowsIterator  operator++ (int)
	{
		ColOfRowsIterator tmp (*this);
		++_row;
		return tmp;
	}

	ConstColOfRowsIterator& operator+ (int i)
	{
		_row = ConstRow (_row.begin () + _dis * i, _row.end () + _dis * i);
		return *this;
	}

	ConstRow operator[] (int i) const
		{ return ConstRow (_row.begin () + _dis * i, _row.end () + _dis * i); }

	ConstRow* operator-> ()
		{ return &_row; }

	ConstRow& operator* ()
		{ return _row; }
    
	bool operator!= (const ConstColOfRowsIterator& c) const
		{ return (_row.begin () != c._row.begin ()) || (_row.end () != c._row.end ()) || (_dis != c._dis); }

    private:
	ConstRow _row;
	size_t _dis;
};

template <class Field, class Vector>
class DenseMatrix<Field, Vector>::ColOfRowsIterator
{
    public:
	ColOfRowsIterator (const typename Vector::iterator& p, size_t len, size_t d)
		: _row (p, p + len), _dis (d){}

	ColOfRowsIterator () {}

	ColOfRowsIterator (const ColOfRowsIterator& colp)
		: _row (colp._row), _dis (colp._dis) {}
    
	ColOfRowsIterator& operator= (const ColOfRowsIterator& colp)
	{
		_row = colp._row;
		_dis = colp._dis;
		return *this;
	}
    
    
	ColOfRowsIterator& operator++ ()
	{
		_row = Row (_row.begin () + _dis, _row.end () + _dis);
		return *this;
	}
    
	ColOfRowsIterator  operator++ (int)
	{
		ColOfRowsIterator tmp (*this);
		++_row;
		return tmp;
	}
    
	ColOfRowsIterator& operator+ (int i)
	{
		_row = tRow (_row.begin () + _dis * i, _row.end () + _dis * i);
		return *this;
	}

	Row operator[] (int i) const
		{ return Row (const_cast<Row&> (_row).begin () + _dis * i,
			      const_cast<Row&> (_row).end () + _dis * i); }

	Row* operator-> ()
		{ return &_row; }
    
	Row& operator* ()
		{ return _row; }
 
	bool operator!= (const ColOfRowsIterator& c) const
		{ return (_row.begin () != c._row.begin ()) || (_row.end () != c._row.end ()) || (_dis != c._dis); }

	operator ConstColOfRowsIterator ()
		{ return ConstColOfRowsIterator (_row.begin (), _row.size (), _dis); }

    private:
	Row _row;
	size_t _dis;
};

template <class Field, class Vector>
class DenseMatrix<Field, Vector>::ConstRowOfColsIterator
{
    public:
	ConstRowOfColsIterator (typename Vector::const_iterator p, size_t stride, size_t len)
		:_col (Subiterator<typename Vector::const_iterator> (p, stride), Subiterator<typename Vector::const_iterator> (p + len * stride, stride)), _stride (stride) {}
    
	ConstRowOfColsIterator (const ConstCol& col, size_t stride)
		:_col (col), _stride (stride){}

	ConstRowOfColsIterator () {}
    
	ConstRowOfColsIterator (const ConstRowOfColsIterator& rowp)
		:_col (rowp._col){}

	ConstRowOfColsIterator& operator= (const ConstRowOfColsIterator& rowp)
	{
		_col = rowp._col;
		_stride = rowp._stride;
		return *this;
	}

	ConstRowOfColsIterator& operator++ ()
	{
		_col = ConstCol (Subiterator<typename Vector::const_iterator> (_col.begin ().operator-> () + 1, _stride),
				 Subiterator<typename Vector::const_iterator> (_col.end ().operator-> () + 1, _stride));
		return *this;
	}

	ConstRowOfColsIterator  operator++ (int)
	{
		Col tmp (_col);
		this->operator++ ();
		return tmp;
	}

	ConstRowOfColsIterator& operator+ (int i)
	{ 
		_col = ConstCol (Subiterator<typename Vector::const_iterator> (_col.begin ().operator-> () + i, _stride), 
				 Subiterator<typename Vector::const_iterator> (_col.end ().operator-> () + i, _stride));
		return *this;
	}


	ConstCol operator[] (int i) const
		{ return ConstCol (Subiterator<typename Vector::const_iterator> (_col.begin ().operator-> () + i, _stride), 
				   Subiterator<typename Vector::const_iterator> (_col.end ().operator-> () + i, _stride)); }

	ConstCol* operator-> ()
		{ return &_col; }
 
	ConstCol& operator* ()
		{ return _col; }
    
	bool operator!= (const ConstRowOfColsIterator& c) const
		{ return (_col.begin () != c._col.begin ()) || (_col.end () != c._col.end ()); }
    
    private:
	ConstCol _col;
	size_t _stride;
};

template <class Field, class Vector>
class DenseMatrix<Field, Vector>::RowOfColsIterator
{
    public:
	RowOfColsIterator (typename Vector::iterator p, size_t stride, size_t len)
		:_col (Subiterator<typename Vector::iterator> (p, stride), Subiterator<typename Vector::iterator> (p+len*stride, stride)), _stride (stride){}
    
	RowOfColsIterator () {}
    
	RowOfColsIterator (const RowOfColsIterator& rowp)
		:_col (rowp._col){}
    
	RowOfColsIterator& operator= (const RowOfColsIterator& rowp)
	{
		_col = rowp._col;
		_stride = rowp._stride;
		return *this;
	}
    
	const RowOfColsIterator& operator= (const RowOfColsIterator& rowp) const
	{
		const_cast<RowOfColsIterator*> (this)->_col = rowp._col;
		return *this;
	}
    
	RowOfColsIterator& operator++ ()
	{
		_col = Col (Subiterator<typename Vector::iterator> (_col.begin ().operator-> () + 1, _stride),
			    Subiterator<typename Vector::iterator> (_col.end ().operator-> () + 1, _stride));
		return *this;
	}
    
	RowOfColsIterator  operator++ (int)
	{
		Col tmp (_col);
		this->operator++ ();
		return tmp;
	}
        
	RowOfColsIterator& operator+ (int i)
	{ 
		_col = Col (Subiterator<typename Vector::const_iterator> (_col.begin ().operator-> () + i, _stride), 
			    Subiterator<typename Vector::const_iterator> (_col.end ().operator-> () + i, _stride));
		return *this;
	}
    

	Col operator[] (int i) const
		{ return Col (Subiterator<typename Vector::iterator> (const_cast<Col&> (_col).begin ().operator-> () + i, _stride), 
			      Subiterator<typename Vector::iterator> (const_cast<Col&> (_col).end ().operator-> () + i, _stride)); }
    
	Col* operator-> ()
		{ return &_col; }
    
	Col& operator* ()
		{ return _col; }
    
	bool operator!= (const RowOfColsIterator& c) const
		{ return (_col.begin () != c._col.begin ()) || (_col.end () != c._col.end ()); }
    
	operator ConstRowOfColsIterator ()
	{
		ConstCol tmp;
		tmp = _col;
		return ConstRowOfColsIterator (tmp, _stride);
	}
    
    private:
	Col _col;
	size_t _stride;
};

template <class Field, class Vector>
template<class Vect1, class Vect2>
Vect1& DenseMatrix<Field, Vector>::apply (Vect1& y, const Vect2& x) const
{
	ConstColOfRowsIterator            p;
	typename Vect1::iterator          p_y = y.begin ();  

	for (p = colOfRowsBegin (); p != colOfRowsEnd (); ++p, ++p_y)
		_VD.dot (*p_y, *p, x);
    
	return y;
}
 
template <class Field, class Vector>
template<class Iterator1, class Iterator2>
Iterator1& DenseMatrix<Field, Vector>::apply (Iterator1        in,
					      const Iterator2 &outbegin,
					      const Iterator2 &outend) const
{
	linbox_check (coldim () == (outend - outbegin));

	ConstColOfRowsIterator            rowp;
	Iterator2                         p_out;
	typename ConstRow::const_iterator pe;

	for (rowp = colOfRowsBegin (); rowp != colOfRowsEnd (); ++rowp, ++in) {
		_F.init (*in, 0);
		for (pe = rowp->begin (), p_out = outbegin; pe != rowp->end (); ++pe, ++p_out)
			_F.axpyin (*in, *pe, *p_out);
	}
    
	return in;
}
 
template <class Field, class Vector>
template<class Vect1, class Vect2>
Vect1& DenseMatrix<Field, Vector>::applyTranspose (Vect1& y, const Vect2& x) const
{
	ConstRowOfColsIterator            colp;
	typename Vect1::iterator          p_y = y.begin ();  

	for (colp=rowOfColsBegin (); colp!=rowOfColsEnd (); ++colp, ++p_y)
		_VD.dot (*p_y, *colp, x);
    
	return y;
}
  
template <class Field, class Vector>
template<class Iterator1, class Iterator2>
Iterator1& DenseMatrix<Field, Vector>::applyTranspose (Iterator1        in,
						       const Iterator2 &outbegin,
						       const Iterator2 &outend) const
{
	linbox_check (rowdim () == (outend - outbegin));

	ConstRowOfColsIterator            colp;
	Iterator2                         p_out;
	typename ConstCol::const_iterator pe;

	for (colp = rowOfColsBegin (); colp != rowOfColsEnd (); ++colp, ++in) {
		_F.init (*in, 0);
		for (pe = colp->begin (), p_out = outbegin; pe != colp->end (); ++pe, ++p_out)
			_F.axpyin (*in, *pe, *p_out);
	}
    
	return in;
}
  
/// entry access raw view.  Size m*n vector in C (row major) order.
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RawIterator DenseMatrix<Field, Vector>::rawBegin ()
	{ return _rep.begin (); }  

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RawIterator DenseMatrix<Field, Vector>::rawEnd ()
	{ return _rep.end (); }
  
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRawIterator DenseMatrix<Field, Vector>::rawBegin () const
	{ return _rep.begin (); }  

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRawIterator DenseMatrix<Field, Vector>::rawEnd () const
	{ return _rep.end (); }

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ColOfRowsIterator DenseMatrix<Field, Vector>::colOfRowsBegin ()
	{ return ColOfRowsIterator (_rep.begin (), _cols, _cols); }

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ColOfRowsIterator DenseMatrix<Field, Vector>::colOfRowsEnd ()
	{ return ColOfRowsIterator (_rep.end (), _cols, _cols); }
  
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstColOfRowsIterator DenseMatrix<Field, Vector>::colOfRowsBegin () const
	{ return ConstColOfRowsIterator (_rep.begin (), _cols, _cols); }  

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstColOfRowsIterator DenseMatrix<Field, Vector>::colOfRowsEnd () const
	{return ConstColOfRowsIterator (_rep.end (), _cols, _cols); }
  
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RowOfColsIterator DenseMatrix<Field, Vector>::rowOfColsBegin ()
	{ return  DenseMatrix<Field, Vector>::RowOfColsIterator (_rep.begin (), _cols, _rows); }

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RowOfColsIterator DenseMatrix<Field, Vector>::rowOfColsEnd ()
	{ return  DenseMatrix<Field, Vector>::RowOfColsIterator (_rep.begin ()+_cols, _cols, _rows); }
  
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRowOfColsIterator DenseMatrix<Field, Vector>::rowOfColsBegin () const
	{ return  DenseMatrix<Field, Vector>::ConstRowOfColsIterator (_rep.begin (), _cols, _rows); }
  
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRowOfColsIterator DenseMatrix<Field, Vector>::rowOfColsEnd () const
	{ return  DenseMatrix<Field, Vector>::ConstRowOfColsIterator (_rep.begin ()+_cols, _cols, _rows); }
  
template <class Field, class Vector>
void DenseMatrix<Field, Vector>::read (std::istream &file)
{
	RawIterator p;
	for (p = rawBegin (); p != rawEnd (); ++p) {
		file.ignore (1);
		_F.read (file, *p);
	}
}
  
template <class Field, class Vector>
std::ostream& DenseMatrix<Field, Vector>::write (std::ostream &os) const
{
	ConstColOfRowsIterator p;

	for (p = colOfRowsBegin (); p != colOfRowsEnd (); ++p) {
		typename ConstRow::const_iterator pe;

		for (pe = p->begin (); pe != p->end (); ++pe) {
			_F.write (os, *pe);
			os << ' ';
		}

		os << endl;
	}

	os << endl;

	return os;
}

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::Row DenseMatrix<Field, Vector>::operator[] (size_t i)
	{ return Row (_rep.begin ()+i*_cols, _rep.begin ()+i*_cols+_cols); }

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRow DenseMatrix<Field, Vector>::operator[] (size_t i) const
	{ return Row (_rep.begin ()+i*_cols, _rep.begin ()+i*_cols+_cols); }

template <class Field, class Vector>
class DenseMatrix<Field, Vector>::RawIndexedIterator
{
	mutable size_t _r_index;
	mutable size_t _c_index;
	mutable size_t     _dim;
	pointer          _begin;

    public:
	RawIndexedIterator (const size_t  &dim,
			  const size_t  &r_index,
			  const size_t  &c_index,
			  const pointer &begin)
		: _r_index (r_index), _c_index (c_index), _dim (dim), _begin (begin)
	{}
	
	RawIndexedIterator ():_r_index (0), _c_index (0), _dim (1), _begin (0){}

	RawIndexedIterator (const RawIndexedIterator& r)
		: _r_index (r._r_index), _c_index (r._c_index), _dim (r._dim), _begin (r._begin)
	{}

	RawIndexedIterator& operator = (const RawIndexedIterator &iter)
	{
		_r_index = iter._r_index;
		_c_index = iter._c_index;
		_dim = iter._dim;
		_begin = iter._begin;
		return *this;
	}
	
	bool operator == (const RawIndexedIterator &iter) const
		{ return (_r_index == iter._r_index) &&
			  (_c_index == iter._c_index) &&
			  (_dim == iter._dim) &&
			  (_begin==iter._begin); }

	bool operator != (const RawIndexedIterator& iter) const
		{ return (_r_index != iter._r_index) ||
			  (_c_index != iter._c_index) ||
			  (_dim != iter._dim) ||
			  (_begin!=iter._begin); }
	
	RawIndexedIterator &operator ++ ()
	{
		++_c_index;

		if (_c_index == _dim) {
			_c_index = 0;
			++_r_index;
		}

		return *this;
	}

	const RawIndexedIterator &operator ++ () const
	{
		++_c_index;

		if (_c_index==_dim) {
			_c_index = 0;
			++_r_index;
		}

		return *this;
	}
	
	RawIndexedIterator operator ++ (int) const
	{
		RawIndexedIterator tmp = *this;
		++(*this);
		return tmp;
	}

	RawIndexedIterator &operator -- ()
	{ 
		if (_c_index)
			--_c_index;
		else {
			--_r_index;
			_c_index = _dim - 1;
		}

		return *this;
	}

	const RawIndexedIterator &operator -- () const
	{ 
		if (_c_index)
			--_c_index;
		else {
			--_r_index;
			_c_index = _dim - 1;
		}

		return *this;
	}

	RawIndexedIterator operator -- (int) const
	{
		RawIndexedIterator tmp = *this;
		--(*this);
		return tmp;
	}	

	Element &operator * ()
		{ return *(_begin + (_r_index * _dim + _c_index)); }

	const Element &operator * () const
		{ return *(_begin + (_r_index * _dim + _c_index)); }
	
	Element* operator -> ()
		{ return _begin + (_r_index * _dim + _c_index); }
	
	const Element *operator -> () const
		{ return _begin + (_r_index * _dim + _c_index); }

	size_t rowIndex () const
		{ return _r_index; }

	size_t colIndex () const
		{ return _c_index; }
};

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RawIndexedIterator DenseMatrix<Field, Vector>::rawIndexedBegin () 
{
	return RawIndexedIterator (coldim (), 0, 0, _rep.begin ());
}

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::RawIndexedIterator DenseMatrix<Field, Vector>::rawIndexedEnd ()
{
	return RawIndexedIterator (coldim (), rowdim (), 0, _rep.begin ());
}
template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRawIndexedIterator DenseMatrix<Field, Vector>::rawIndexedBegin () const
{
	return RawIndexedIterator (coldim (), 0, 0, _rep.begin ());
}

template <class Field, class Vector>
typename DenseMatrix<Field, Vector>::ConstRawIndexedIterator DenseMatrix<Field, Vector>::rawIndexedEnd () const
{
	return RawIndexedIterator (coldim (), rowdim (), 0, _rep.begin ());
}

} // namespace LinBox

#endif // __DENSE_INL
