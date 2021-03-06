/*
    Copyright (c) 2018 Contributors as noted in the AUTHORS file

    This file is part of libzmq, the ZeroMQ core engine in C++.

    libzmq is free software; you can redistribute it and/or modify it under
    the terms of the GNU Lesser General Public License (LGPL) as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    As a special exception, the Contributors give you permission to link
    this library with independent modules to produce an executable,
    regardless of the license terms of these independent modules, and to
    copy and distribute the resulting executable under terms of your choice,
    provided that you also meet, for each linked independent module, the
    terms and conditions of the license of that module. An independent
    module is a module which is not derived from or based on this library.
    If you modify this library, you must extend this exception to your
    version of the library.

    libzmq is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
    License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include <stddef.h>

#include "stdint.hpp"

// Wrapper type for a node's data layout.
//
// There are 3 32-bit unsigned integers that act as a header. These
// integers represent the following values in this order:
//
// (1) The reference count of the key held by the node. This is 0 if
// the node doesn't hold a key.
//
// (2) The number of characters in the node's prefix. The prefix is a
// part of one or more keys in the tree, e.g. the prefix of each node
// in a trie consists of a single character.
//
// (3) The number of outgoing edges from this node.
//
// The rest of the layout consists of 3 chunks in this order:
//
// (1) The node's prefix as a sequence of one or more bytes. The root
// node always has an empty prefix, unlike other nodes in the tree.
//
// (2) The first byte of the prefix of each of this node's children.
//
// (3) The pointer to each child node.
//
// The link to each child is looked up using its index, e.g. the child
// with index 0 will have its first byte and node pointer at the start
// of the chunk of first bytes and node pointers respectively.
struct node
{
    unsigned char *data_;

    explicit node (unsigned char *data);

    bool operator== (node other) const;
    bool operator!= (node other) const;

    inline uint32_t refcount ();
    inline uint32_t prefix_length ();
    inline uint32_t edgecount ();
    inline unsigned char *prefix ();
    inline unsigned char *first_bytes ();
    inline unsigned char first_byte_at (size_t i);
    inline unsigned char *node_ptrs ();
    inline node node_at (size_t i);
    inline void set_refcount (uint32_t value);
    inline void set_prefix_length (uint32_t value);
    inline void set_edgecount (uint32_t value);
    inline void set_prefix (const unsigned char *prefix);
    inline void set_first_bytes (const unsigned char *bytes);
    inline void set_first_byte_at (size_t i, unsigned char byte);
    inline void set_node_ptrs (unsigned char const *ptrs);
    inline void set_node_at (size_t i, node n);
    inline void set_edge_at (size_t i, unsigned char byte, node n);
    void resize (size_t prefix_length, size_t edgecount);
};

node make_node (size_t refcount, size_t prefix_length, size_t nedges);

struct match_result
{
    size_t nkey;
    size_t nprefix;
    size_t edge_index;
    size_t gp_edge_index;
    node current_node;
    node parent_node;
    node grandparent_node;

    match_result (size_t i,
                  size_t j,
                  size_t edge_index,
                  size_t gp_edge_index,
                  node current,
                  node parent,
                  node grandparent);
};

namespace zmq
{
class radix_tree
{
  public:
    radix_tree ();
    ~radix_tree ();

    //  Add key to the tree. Returns true if this was a new key rather
    //  than a duplicate.
    bool add (const unsigned char *prefix_, size_t size_);

    //  Remove key from the tree. Returns true if he item is acually
    //  removed from the tree.
    bool rm (const unsigned char *prefix_, size_t size_);

    //  Check whether particular key is in the tree.
    bool check (const unsigned char *prefix, size_t size_);

    //  Apply the function supplied to each key in the tree.
    void apply (void (*func) (unsigned char *data_, size_t size_, void *arg_),
                void *arg);

    size_t size () const;

  private:
    inline match_result
    match (const unsigned char *key, size_t size, bool check) const;

    node root_;
    size_t size_;
};
}

#endif
