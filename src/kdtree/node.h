// =========================================================================
// Last updated on 7/31/05
// =========================================================================

/** \file
 * Defines interfaces for nodes as used by the KDTree class.
 *
 * \author Martin F. Krafft <krafft@ailab.ch>
 * \date $Date: 2004/11/15 17:40:32 $
 * \version $Revision: 1.6 $
 */

#ifndef INCLUDE_KDTREE_NODE_HPP
#define INCLUDE_KDTREE_NODE_HPP

#include <iostream>
#include <cstddef>

namespace KDTree
{
   struct _Node_base
   {
         typedef _Node_base* _Base_ptr;

         int ID;
         _Base_ptr _M_parent;
         _Base_ptr _M_left;
         _Base_ptr _M_right;

         _Node_base(_Base_ptr const __PARENT = NULL,
                    _Base_ptr const __LEFT = NULL,
                    _Base_ptr const __RIGHT = NULL) throw ()
            : _M_parent(__PARENT), _M_left(__LEFT), _M_right(__RIGHT) {}

         static _Base_ptr
         _S_minimum(_Base_ptr __x) throw ()
         {
            while (__x->_M_left) __x = __x->_M_left;
            return __x;
         }
    
         static _Base_ptr
         _S_maximum(_Base_ptr __x) throw ()
         {
            while (__x->_M_right) __x = __x->_M_right;
            return __x;
         }
   };

   template <typename _Val>
      struct __Node : public _Node_base
      {
            using _Node_base::_Base_ptr;
            typedef __Node* _Link_type;

            _Val _M_value;

            __Node(_Val const& __VALUE = _Val(),
                   _Base_ptr const __PARENT = NULL,
                   _Base_ptr const __LEFT = NULL,
                   _Base_ptr const __RIGHT = NULL) throw ()
               : _Node_base(__PARENT, __LEFT, __RIGHT), _M_value(__VALUE) {}

            _Val* get_value_ptr() 
            {
               return &_M_value;
            }

            const _Val* get_value_ptr() const
            {
               return &_M_value;
            }

// Method find_ancestors generates an STL vector containing _Node_base
// pointers to the current node and all its ancestors up to the top of
// the KDtree:

            std::vector<KDTree::_Node_base*> find_ancestors()
            {
               ancestors.clear();
               get_ancestors(this);
//               for (unsigned int a=0; a<ancestors.size(); a++)
//               {
//                  std::cout << "a = " << a
//                            << " ID = " << ancestors[a]->ID 
//                            << " node = " << ancestors[a]
//                            << std::endl;
//               }
               return ancestors;
            }
            
// Method get_ancestors recursively pushes back _Node_base pointer
// information onto member STL vecdtor ancestors.

            void get_ancestors(KDTree::_Node_base* _N_ptr)
            {
               ancestors.push_back(_N_ptr);
               if (_N_ptr->_M_parent->ID != -1)
               {
                  get_ancestors(_N_ptr->_M_parent);
               }
            } // get_ancestor method

            private:

            std::vector<KDTree::_Node_base*> ancestors;

      }; // __Node structure

   template <typename _Val, typename _Acc, typename _Cmp>
      class _Node_compare
      {
        public:
         typedef __Node<_Val>* _Link_type;

         _Node_compare(size_t const __DIM) : _M_DIM(__DIM) {} 

         bool operator()(_Link_type const& __A, _Link_type const& __B) const
            {
               return _M_cmp(_M_acc(__A->_M_value, _M_DIM),
                             _M_acc(__B->_M_value, _M_DIM));
            }

         bool operator()(_Link_type const& __A, _Val const& __B) const
            {
               return _M_cmp(_M_acc(__A->_M_value, _M_DIM), 
                             _M_acc(__B, _M_DIM));
            }

         bool operator()(_Val const& __A, _Link_type const& __B) const
            {
               return _M_cmp(_M_acc(__A, _M_DIM), 
                             _M_acc(__B->_M_value, _M_DIM));
            }

         bool operator()(_Val const& __A, _Val const& __B) const
            {
               return _M_cmp(_M_acc(__A, _M_DIM), _M_acc(__B, _M_DIM));
            }

        private:

         size_t const _M_DIM;
         _Acc _M_acc;
         _Cmp _M_cmp;
      };



   template <typename _Char, typename _Traits, typename _Val>
      std::basic_ostream<_Char, _Traits>&
      operator<<(std::basic_ostream<_Char, _Traits>& __out,
                 typename KDTree::__Node<_Val> const& __N) throw ()
      {
         __out << "-----------------------------------------------------"
               << std::endl;
         __out << "inside node << ";
//         __out << &__N << " " << __N._M_value;
//         __out << "ID = " << __N.ID;
         __out << "node# = " << __N.ID ;
         if (__N.ID >=0)
         {
            __out << " value = " << __N._M_value;
         }

         if (__N._M_left != NULL)
         {
            __out << "; left node# = " << (__N._M_left)->ID;
         }
         if (__N._M_right != NULL)
         {
            __out << "; right node# = " << (__N._M_right)->ID;
         }

         if (__N._M_parent==NULL)
         {
            __out << "Parent = NULL";
         }
         else
         {
            __out << "; parent node# = " << (__N._M_parent)->ID;
         }
               
/*         
         if (__N._M_parent != NULL)
         {
            __out << "; parent: " 
//                  << __N._M_parent << " " 
                  << static_cast<KDTree::__Node<_Val> *const > 
               (__N._M_parent)->_M_value;
         }
         else
         {
            __out << "; parent = meta node";
         }
      
         if (__N._M_left != NULL)
         {
            __out << "; left: " 
//                  << __N._M_left << " " 
                  << static_cast<KDTree::__Node<_Val> *const > 
               (__N._M_left)->_M_value;
         }
         else
         {
            __out << "; left = NULL";
         }
      
         if (__N._M_right != NULL)
         {
            __out << "; right: " 
//                  << __N._M_right << " " 
                  << static_cast<KDTree::__Node<_Val> *const > 
               (__N._M_right)->_M_value;
         }
         else
         {
            __out << "; right = NULL";
         }
*/

         return __out;

//      __out << &__N;
//      __out << ' ' << __N._M_value;
//      __out << "; parent: " << __N._M_parent;
//      __out << "; left: " << __N._M_left;
//      __out << "; right: " << __N._M_right;
//      return __out;
      }

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
