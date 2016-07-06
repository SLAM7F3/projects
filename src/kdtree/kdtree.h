// =========================================================================
// Last updated on 7/29/05; 2/28/06; 9/3/09; 1/23/14
// =========================================================================

/** \file
 * Defines the interface for the KDTree class.
 *
 * \author Martin F. Krafft <krafft@ailab.ch>
 * \date $Date: 2004/11/15 17:40:32 $
 * \version $Revision: 1.9 $
 */

#ifndef INCLUDE_KDTREE_KDTREE_HPP
#define INCLUDE_KDTREE_KDTREE_HPP

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>
#include <stack>
#include "kdtree/accessor.h"
#include "kdtree/allocator.h"
#include "math/constants.h"
#include "kdtree/iterator.h"
#include "kdtree/node.h"
#include "kdtree/region.h"

namespace KDTree
{
   static int sort_count=0;
   static int M_opt_entries=0;
   static int node_ID=0;

// Forward declare KDTree class before overloading operator<< friend
// function:

   template <size_t const __K, typename _Val,typename _Acc, typename _Cmp, 
      typename _Alloc> class KDTree;

   template <size_t const __K, typename _Val,typename _Acc, typename _Cmp, 
      typename _Alloc>
      std::ostream& operator<<(
         std::ostream& __out,
         KDTree<__K, _Val, _Acc, _Cmp, _Alloc> const& __T) throw ()
      {
         __out << "meta node:   " << *__T._M_header << std::endl;
         __out << "dimensions:  " << __K << std::endl;

         if (__T.empty())
            return __out << "[empty " << __K << "d-tree " << &__T << "]";

         __out << "nodes total: " << __T.size() << std::endl;

         typedef KDTree<__K, _Val, _Acc, _Cmp, _Alloc> _Tree;
         typedef typename _Tree::_Link_type _Link_type;

         std::stack<_Link_type> s;
         s.push(__T._M_root());
      
         while (!s.empty())
         {
            _Link_type n = s.top();
            s.pop();
            std::cout << *n << std::endl;
            if (_Tree::_S_left(n)) s.push(_Tree::_S_left(n));
            if (_Tree::_S_right(n)) s.push(_Tree::_S_right(n));
         }

         return __out;
      }

   template <size_t const __K, typename _Val,
      typename _Acc = _Bracket_accessor<_Val>,
      typename _Cmp = std::less<typename _Acc::subvalue_type>,
      typename _Alloc = std::allocator<__Node<_Val> > >
      class KDTree : protected _Alloc_base<_Val, _Alloc>
      {
        protected:
         typedef _Alloc allocator_type;
         typedef _Alloc_base<_Val, _Alloc> _Base;
	 //         typedef typename _Base::allocator_type allocator_type;

         typedef _Node_base* _Base_ptr;
         typedef __Node<_Val>* _Link_type;

         typedef _Node_compare<_Val, _Acc, _Cmp> _Node_Compare;

         typedef __Region<__K, _Val, typename _Acc::subvalue_type, _Acc, _Cmp>
            _Region;

        public:
         typedef _Val value_type;
         typedef value_type* pointer;
         typedef value_type const* const_pointer;
         typedef value_type& reference;
         typedef value_type const& const_reference;
         typedef typename _Acc::subvalue_type subvalue_type;
         typedef size_t size_type;
         typedef ptrdiff_t difference_type;

// Constructor:

         KDTree(const allocator_type& __a = allocator_type()) throw ()
            : _Base(__a), _M_header(_Base::_M_allocate_node()), _M_count(0)
            {
               _M_header->ID=-1;
               _M_empty_initialise();
            }

// Copy constructor:

         KDTree(const KDTree& __x) throw ()
            : _Base(__x.get_allocator()), _M_header(
               _Base::_M_allocate_node()), 
            _M_count(0)
            {
               _M_empty_initialise();
               this->insert(begin(), __x.begin(), __x.end()); 
            }

         template<typename _InputIterator>
            KDTree(_InputIterator __first, _InputIterator __last,
                   const allocator_type& __a = allocator_type()) throw ()
            : _Base(__a), _M_header(_Base::_M_allocate_node()), _M_count(0)
            {
               _M_empty_initialise();
               this->insert(begin(), __first, __last); 
            }

         KDTree& operator=(const KDTree& __x)
            {
               if (this != &__x)
               {
                  this->clear();
                  this->insert(begin(),__x.begin(),__x.end());
               }
               return *this;
            }

         ~KDTree() throw ()
            {
               this->clear();
               this->_M_deallocate_node(_M_header);
            }

         allocator_type get_allocator() const 
            { 
               return _Base::get_allocator(); 
            }
      
         size_type size() const
            {
               return _M_count;
            }
      
         size_type max_size() const
            { 
               return size_type(-1); 
            }

         bool empty() const
            {
               return this->size() == 0;
            }

         void clear()
            {
               _M_erase_subtree(_M_root());
               _M_leftmost() = _M_rightmost() = _M_header;
               _M_root() = NULL;
               _M_count = 0;
            }

//      typedef _Iterator<_Val, reference, pointer> iterator;
         typedef _Iterator<_Val, const_reference, const_pointer> 
            const_iterator;

         // there is no such thing as a mutable iterator,
         // but we need to define one anyway.
         typedef const_iterator iterator;

//      typedef std::reverse_iterator<iterator> reverse_iterator;
         // TODO: const_reverse_iterators are broken
         // see http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=244894
         typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
         typedef const_reverse_iterator reverse_iterator;

         iterator begin() const { return iterator(_M_leftmost()); }
         iterator end() const { return iterator(_M_header); }
         reverse_iterator rbegin() const { return reverse_iterator(end()); }
         reverse_iterator rend() const { return reverse_iterator(begin()); }

         iterator insert(iterator /* ignored */, 
                         const_reference __V) throw (std::bad_alloc)
            {
               return this->insert(__V);
            }

         iterator insert(const_reference __V) throw (std::bad_alloc)
            {
               if (!_M_root()) 
               {

// Instantiate KDtree's root node if it doesn't already exist:

                  _Link_type __n = _M_new_node(__V, _M_header); 
                  ++_M_count;
                  _M_root() = _M_leftmost() = _M_rightmost() = __n;
                  return iterator(__n);
               }
               return _M_insert(_M_root(), __V, 0);
            }

// Member function locate_leaf is a minor variant of the insert
// method.  It takes some input data containing world-space position
// information and determines under which leaf node it would be
// inserted in the KDtree.  A pointer to that leaf node is returned by
// this method.

         _Link_type locate_leaf(const_reference __V) throw (std::bad_alloc)
            {
               return _M_locate_leaf(_M_root(), __V, 0);
            }

         template <class _InputIterator>
            void insert(_InputIterator __first, _InputIterator __last) {
            for (; __first != __last; ++__first)
               this->insert(*__first);
         }
      
         void insert(iterator __pos, size_type __n, const value_type& __x)
            {
               for (; __n > 0; --__n)
                  this->insert(__pos, __x);
            }

         template<typename _InputIterator>
            void insert(iterator __pos, _InputIterator __first, 
                        _InputIterator __last) 
            {
               for (; __first != __last; ++__first)
                  this->insert(__pos, *__first);
            }

         void erase(const_reference __V) throw () 
            {
               this->erase(this->find(__V));
            }

         void erase(iterator const& __IT) throw ()
            {
               if (__IT == this->end()) return;
               _Link_type const __N = (_Link_type const) __IT._M_node;
               _Link_type __n = __N;
               size_t __l = 0;
               while ((__n = _S_parent(__n)) != _M_header) ++__l;
               _M_erase(__N, __l);
               _M_delete_node(__N); --_M_count;
            }

/* this does not work since erasure changes sort order
   void
   erase(iterator __A, iterator const& __B) throw ()
   {
   if (0 && __A == this->begin() && __B == this->end())
   {
   this->clear();
   }
   else
   {
   while (__A != __B)
   this->erase(__A++);
   }
   }
*/

         const_iterator find(const_reference __V) const throw ()
            {
               if (!_M_root()) return this->end();
               return _M_find(_M_root(), __V, 0);
            }

         size_t count_within_range(
            const_reference __V, subvalue_type const __R) const throw ()
            {
               if (!_M_root()) return 0;
               _Region __region;
               _Acc __acc;
               for (size_t __i = 0; __i < __K; ++__i) {
                  __region._M_low_bounds[__i] = __acc(__V, __i) - __R;
                  __region._M_high_bounds[__i] = __acc(__V, __i) + __R;
               }
               return this->count_within_range(__region);
            } 

         size_t count_within_range(_Region const& __REGION) const throw ()
            {
               if (!_M_root()) return 0;

               _Region __bounds(__REGION);
               return _M_count_within_range(_M_root(),
                                            __REGION, __bounds, 0);
            }

// ----------------------------------------------------------------------
// Member function find_within_range returns STL vector __out filled
// with nodes lying within a k-dimensional bounding box centered upon
// input location __V.  The linear size of this bounding box equals
// +/- __R for dimensions 0 through __K-upper_dims_to_ignore-1 and +/-
// POSITIVEINFINITY for dimensions __K-upper_dims_to_ignore through
// __K-1.

         template <typename _OutputIterator>
            _OutputIterator find_within_range(
               const_reference __V, subvalue_type const __R,
               _OutputIterator __out, size_t upper_dims_to_ignore=0) 
            const throw ()
            {
               if (!_M_root()) return __out;
               _Region __region;
               _Acc __acc;
               for (size_t __i = 0; __i < __K-upper_dims_to_ignore; ++__i) 
               {
                  __region._M_low_bounds[__i] = __acc(__V, __i) - __R;
                  __region._M_high_bounds[__i] = __acc(__V, __i) + __R;
               }

               for (size_t __i = __K-upper_dims_to_ignore; __i < __K; ++__i) 
               {
                  __region._M_low_bounds[__i] = __acc(__V,__i)
                     - POSITIVEINFINITY;
                  __region._M_high_bounds[__i] = __acc(__V, __i) 
                     + POSITIVEINFINITY;
               }
               return this->find_within_range(__region, __out);
            } 

         template <typename _OutputIterator>
            _OutputIterator
            find_within_range(_Region const& __REGION,
                              _OutputIterator __out) const throw ()
            {
               if (_M_root())
               {
                  _Region __bounds(__REGION);
                  __out = _M_find_within_range(__out, _M_root(),
                                               __REGION, __bounds, 0);
               }
               return __out;
            }

         void optimise()
            {
               std::vector<value_type> __v(this->begin(),this->end());
               this->clear();
               _M_optimise(__v.begin(), __v.end(), 0);

               static int opt_iter=0;
               opt_iter++;
//               std::cout << "Optimization iteration = " << opt_iter 
//                         << " M_optimize entries = " << M_opt_entries
//                         << " # sorts performed = " << sort_count
//                         << std::endl;
            }

         inline void optimize()
            { // cater for people who cannot spell :)
               optimise();
            }

// ----------------------------------------------------------------------
// Member function fill_nodes_vector traverses over the entire KDTree
// and fills an STL vector with the nodes' pointers.

         std::vector<_Link_type> fill_nodes_vector()
            {
               std::vector<_Link_type> V;
               if (empty())
               {
                  return V;
               }
         
               std::stack<_Link_type> s;
               s.push(_M_root());
               while (!s.empty())
               {
                  _Link_type n = s.top();
                  s.pop();
                  V.push_back(n);
                  if (_S_left(n)) s.push(_S_left(n));
                  if (_S_right(n)) s.push(_S_right(n));
               }
               return V;
            }

// ----------------------------------------------------------------------
// Member function fill_leaf_nodes_vector traverses over the entire
// KDTree and searches for its leaf nodes.  This method fills an STL
// vector with leaf node pointers.

         std::vector<_Link_type> fill_leaf_nodes_vector()
            {
               std::vector<_Link_type> V;
               if (empty())
               {
                  return V;
               }
         
               std::stack<_Link_type> s;
               s.push(_M_root());
               while (!s.empty())
               {
                  _Link_type n = s.top();
                  s.pop();
                  if (n->_M_left==NULL && n->_M_right==NULL)
                  {
                     V.push_back(n);
                  }
                  
                  if (_S_left(n)) s.push(_S_left(n));
                  if (_S_right(n)) s.push(_S_right(n));
               }
               return V;
            }

// ----------------------------------------------------------------------
// Member function get_root_node returns a pointer to the head of the
// KDTree:

         _Link_type get_root_node()
            {
               return _M_root();
            }
         
        protected:

         void _M_empty_initialise()
            {
               _M_leftmost() = _M_rightmost() = _M_header;
               _M_root() = NULL;
            }

         iterator _M_insert_left(_Link_type __N, const_reference __V)
            {
               _S_left(__N) = _M_new_node(__V); 
               ++_M_count;
               _S_left(__N)->_M_parent = __N;
               if (__N == _M_leftmost()) _M_leftmost() = _S_left(__N);
               return iterator(_S_left(__N));
            }

         iterator
            _M_insert_right(_Link_type __N, const_reference __V)
            {
               _S_right(__N) = _M_new_node(__V); 
               ++_M_count;
               _S_right(__N)->_M_parent = __N;
               if (__N == _M_rightmost()) _M_rightmost() = _S_right(__N);
               return iterator(_S_right(__N));
            }

         iterator _M_insert(_Link_type __N, const_reference __V,
                            size_t const __L) throw (std::bad_alloc)
            {
               if (_Node_Compare(__L % __K)(__V, __N))
               {
                  if (!_S_left(__N)) 
                  {
                     return _M_insert_left(__N, __V);
                  }
                  return _M_insert(_S_left(__N), __V, __L+1);
               }
               else
               {
                  if (!_S_right(__N) || __N == _M_rightmost())
                  {
                     return _M_insert_right(__N, __V);
                  }
                  return _M_insert(_S_right(__N), __V, __L+1);
               }
            }

         _Link_type _M_locate_leaf(_Link_type __N, const_reference __V,
                                   size_t const __L) throw (std::bad_alloc)
            {
               if (_Node_Compare(__L % __K)(__V, __N))
               {
                  if (!_S_left(__N)) 
                  {
                     return __N;
                  }
                  return _M_locate_leaf(_S_left(__N), __V, __L+1);
               }
               else
               {
                  if (!_S_right(__N) || __N == _M_rightmost())
                  {
                     return __N;
                  }
                  return _M_locate_leaf(_S_right(__N), __V, __L+1);
               }
            }

         _Link_type _M_erase(_Link_type const __N, size_t const __L) throw ()
            {
               return _M_erase_replace(
                  __N, _M_get_erase_replacement(__N, __L));
            }

         _Link_type
            _M_get_erase_replacement(_Link_type const __N,
                                     size_t const __L) throw ()
            {
               if (_S_is_leaf(__N)) return NULL;
               _Link_type __ret;
               size_t __j = __L;
               if (!_S_left(__N))
                  __ret = _M_get_j_min(_S_right(__N), __j, __L+1);
               else if ((!_S_right(__N)))
                  __ret = _M_get_j_max(_S_left(__N), __j, __L+1);
               else 
               {
                  _Node_Compare __cmp(__L % __K);
                  if (__cmp(_S_right(__N), _S_left(__N)))
                     __ret = _M_get_j_min(_S_right(__N), __j, __L+1);
                  else
                     __ret = _M_get_j_max(_S_left(__N), __j, __L+1);
               }
               _Link_type __p = _S_parent(__ret);
               if (_S_left(__p) == __ret) _S_left(__p) = _M_erase(__ret, __j);
               else _S_right(__p) = _M_erase(__ret, __j);

               return __ret;
            }

         _Link_type
            _M_erase_replace(_Link_type const __N, _Link_type __q) throw ()
            {
               if (__q)
               {
                  _S_parent(__q) = _S_parent(__N);
                  _S_left(__q) = _S_left(__N);
                  if (_S_left(__q)) _S_parent(_S_left(__q)) = __q;
                  _S_right(__q) = _S_right(__N);
                  if (_S_right(__q)) _S_parent(_S_right(__q)) = __q;
               }
               if (__N == _M_root()) _S_parent(_M_header) = __q;
               else if (_S_left(_S_parent(__N)) == __N)
               {
                  _S_left(_S_parent(__N)) = __q;
               }
               else 
               {
                  _S_right(_S_parent(__N)) = __q;
               }
               if (__N == _M_leftmost())
                  _M_leftmost() = __q ? __q : _S_parent(__N);
               if (__N == _M_rightmost())
                  _M_rightmost() = __q ? __q : _S_parent(__N);
               return __q;
            }

         _Link_type
            _M_get_j_min(_Link_type const __N, size_t& __j,
                         size_t const __L) throw ()
            {
               if (_S_is_leaf(__N)) 
               { 
                  __j = __L; 
                  return __N; 
               }
               _Node_Compare __cmp(__j % __K);
               _Link_type __ret = __N;
               if (_S_left(__N))
               {
                  _Link_type __l = _M_get_j_min(_S_left(__N), __j, __L+1);
                  if (__cmp(__l, __ret))
                  {
                     __ret = __l;
                  }
               }
               if (_S_right(__N))
               {
                  _Link_type __r = _M_get_j_min(_S_right(__N), __j, __L+1);
                  if (__cmp(__r, __ret))
                  {
                     __ret = __r;
                  }
               }
               if (__ret == __N) __j = __L;
               return __ret;
            }

         _Link_type
            _M_get_j_max(_Link_type const __N, size_t& __j,
                         size_t const __L) throw ()
            {
               if (_S_is_leaf(__N)) 
               { 
                  __j = __L; 
                  return __N; 
               }
               _Node_Compare __cmp(__j % __K);
               _Link_type __ret = __N;
               if (_S_left(__N))
               {
                  _Link_type __l = _M_get_j_max(_S_left(__N), __j, __L+1);
                  if (__cmp(__ret, __l))
                  {
                     __ret = __l;
                  }
               }
               if (_S_right(__N))
               {
                  _Link_type __r = _M_get_j_max(_S_right(__N), __j, __L+1);
                  if (__cmp(__ret, __r))
                  {
                     __ret = __r;
                  }
               }
               if (__ret == __N) __j = __L;
               return __ret;
            }

         void _M_erase_subtree(_Link_type __n)
            {
               while (__n)
               {
                  _M_erase_subtree(_S_right(__n));
                  _Link_type __t = _S_left(__n);
                  _M_delete_node(__n);
                  __n = __t;
               }
            }

         const_iterator _M_find(_Link_type __N, const_reference __V,
                                size_t const __L) const throw ()
            {
               _Node_Compare __cmp(__L % __K);
               if (__cmp(__V, __N))
               {
                  if (_S_left(__N)) return _M_find(_S_left(__N), __V, __L+1);
               }
               else
               {
                  if (!__cmp(__N, __V) && 
                      _M_matches_node_in_other_ds(__N, __V, __L))
                     return const_iterator(__N);
                  if (_S_right(__N)) 
                     return _M_find(_S_right(__N), __V, __L+1);
               }
               return this->end();
            }

         bool _M_matches_node_in_d(_Link_type __N, const_reference __V,
                                   size_t const __L) const throw ()
            {
               _Node_Compare __cmp(__L % __K);
               return !(__cmp(__N, __V) || __cmp(__V, __N));
            }

         bool _M_matches_node_in_other_ds(_Link_type __N, const_reference __V,
                                          size_t const __L = 0) const throw ()
            {
               size_t __i = __L;
               while ((__i = (__i + 1) % __K) != __L % __K)
                  if (!_M_matches_node_in_d(__N, __V, __i)) return false;
               return true;
            }

         bool _M_matches_node(_Link_type __N, const_reference __V,
                              size_t __L = 0) const throw ()
            {
               return _M_matches_node_in_d(__N, __V, __L)
                  && _M_matches_node_in_other_ds(__N, __V, __L);
            }

         size_t _M_count_within_range(_Link_type __N, _Region const& __REGION,
                                      _Region const& __BOUNDS,
                                      size_t const __L) const throw ()
            {
               size_t count = 0;
               if (__REGION.encloses(_S_value(__N)))
               {
                  ++count;
               }
               if (_S_left(__N))
               {
                  _Region __bounds(__BOUNDS);
                  __bounds.set_high_bound(_S_value(__N), __L);
                  if (__REGION.intersects_with(__bounds))
                     count += _M_count_within_range(
                        _S_left(__N),
                        __REGION, __bounds, __L+1);
               }
               if (_S_right(__N))
               {
                  _Region __bounds(__BOUNDS);
                  __bounds.set_low_bound(_S_value(__N), __L);
                  if (__REGION.intersects_with(__bounds))
                     count += _M_count_within_range(
                        _S_right(__N),__REGION, __bounds, __L+1);
               }
               return count;
            }


         template <typename _OutputIterator>
            _OutputIterator
            _M_find_within_range(_OutputIterator __out,
                                 _Link_type __N, _Region const& __REGION,
                                 _Region const& __BOUNDS,
                                 size_t const __L) const throw ()
            {
               if (__REGION.encloses(_S_value(__N)))
               {
                  *__out++ = _S_value(__N);
               }
               if (_S_left(__N))
               {
                  _Region __bounds(__BOUNDS);
                  __bounds.set_high_bound(_S_value(__N), __L);
                  if (__REGION.intersects_with(__bounds))
                     __out = _M_find_within_range(__out, _S_left(__N),
                                                  __REGION, __bounds, __L+1);
               }
               if (_S_right(__N))
               {
                  _Region __bounds(__BOUNDS);
                  __bounds.set_low_bound(_S_value(__N), __L);
                  if (__REGION.intersects_with(__bounds))
                     __out = _M_find_within_range(__out, _S_right(__N),
                                                  __REGION, __bounds, __L+1);
               }

               return __out;
            }

         template <typename _Iter>
            void _M_optimise(_Iter const& __A, _Iter const& __B,
                             size_t const __L) throw ()
            {
               M_opt_entries++;

               if (__A == __B) return;
               _Node_Compare __cmp(__L % __K);
               std::sort(__A, __B, __cmp);

               sort_count++;

               _Iter __m = __A + (__B - __A) / 2;
               this->insert(*__m);
               if (__m != __A) _M_optimise(__A, __m, __L+1);
               if (++__m != __B) _M_optimise(__m, __B, __L+1);
            }

         _Link_type& _M_root() const 
            { 
               return (_Link_type&) _M_header->_M_parent; 
            }
      
         _Link_type& _M_leftmost() const 
            { 
               return (_Link_type&) _M_header->_M_left; 
            }
      
         _Link_type& _M_rightmost() const 
            { 
               return (_Link_type&) _M_header->_M_right; 
            }

         static _Link_type& _S_parent(_Base_ptr N)
            {
               return (_Link_type&) N->_M_parent;
            }

         static _Link_type& _S_parent(_Link_type N)
            {
               return (_Link_type&) N->_M_parent;
            }

         static _Link_type& _S_left(_Base_ptr N)
            {
               return (_Link_type&) N->_M_left;
            }

         static _Link_type& _S_left(_Link_type N)
            {
               return (_Link_type&) N->_M_left;
            }

         static _Link_type& _S_right(_Base_ptr N)
            {
               return (_Link_type&) N->_M_right;
            }

         static _Link_type& _S_right(_Link_type N)
            {
               return (_Link_type&) N->_M_right;
            }

         static bool _S_is_leaf(_Base_ptr N)
            {
               return !_S_left(N) && !_S_right(N);
            }

         static _Val _S_value(_Link_type N)
            {
               return N->_M_value;
            }

         static _Val _S_value(_Base_ptr N)
            {
               return ((_Link_type) N)->_M_value;
            }

         static _Link_type _S_minimum(_Link_type const __X)
            {
               return (_Link_type) _Node_base::_S_minimum(__X);
            }

         static _Link_type _S_maximum(_Link_type const __X)
            {
               return (_Link_type) _Node_base::_S_maximum(__X);
            }

         _Link_type _M_new_node(const_reference __V, //  = value_type(),
                                _Base_ptr const __PARENT = NULL,
                                _Base_ptr const __LEFT = NULL,
                                _Base_ptr const __RIGHT = NULL)
            {
               _Link_type __ret = _Base::_M_allocate_node();

// Assign unique integer ID to each node as soon as it's instantiated:

               __ret->ID=node_ID;
               node_ID++;

               try 
                  {
                     this->_M_construct_node(
                        __ret, __V, __PARENT, __LEFT, __RIGHT);
                  }
               catch(...)
                  {
                     this->_M_deallocate_node(__ret);
                     __throw_exception_again;
                  }
               return __ret;
            }

         _Link_type _M_clone_node(_Link_type const __X)
            {
               _Link_type __ret = _M_allocate_node(__X->_M_value);
               // TODO
               return __ret;
            }

         void _M_delete_node(_Link_type __p)
            {
               this->_M_destroy_node(__p);
               this->_M_deallocate_node(__p);
            }
      
         _Link_type _M_header;
         size_type _M_count;

         friend std::ostream&
            operator<< <>(std::ostream&,
                          KDTree<__K, _Val, _Acc, _Cmp, _Alloc> const&) 
            throw ();
         
      }; // KDTree class 

} // namespace KDTree

#endif // include guard

/* COPYRIGHT --
 *
 * This file is part of libkdtree++, a C++ template KD-Tree sorting container.
 * libkdtree++ is (c) 2004 Martin F. Krafft <krafft@ailab.ch>
 * and distributed under the terms of the Artistic Licence.
 * See the ./COPYING file in the source tree root for more information.
 *
 * THIS PACKAGE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES
 * OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
