// The libMesh Finite Element Library.
// Copyright (C) 2002-2022 Benjamin S. Kirk, John W. Peterson, Roy H. Stogner

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA



#ifndef LIBMESH_CELL_HEX27_H
#define LIBMESH_CELL_HEX27_H

// Local includes
#include "libmesh/cell_hex.h"

namespace libMesh
{

/**
 * The \p Hex27 is an element in 3D composed of 27 nodes.  It is
 * numbered like this:
 *
 * \verbatim
 *   HEX27:     7              18             6
 *              o--------------o--------------o
 *             /:             /              /|
 *            / :            /              / |
 *           /  :           /              /  |
 *        19/   :        25/            17/   |
 *         o--------------o--------------o    |
 *        /     :        /              /|    |
 *       /    15o       /    23o       / |  14o
 *      /       :      /              /  |   /|           zeta
 *    4/        :   16/             5/   |  / |            ^   eta (into page)
 *    o--------------o--------------o    | /  |            | /
 *    |         :    |   26         |    |/   |            |/
 *    |  24o    :    |    o         |  22o    |            o---> xi
 *    |         :    |       10     |   /|    |
 *    |        3o....|.........o....|../.|....o
 *    |        .     |              | /  |   / 2
 *    |       .    21|            13|/   |  /
 * 12 o--------------o--------------o    | /
 *    |     .        |              |    |/
 *    |  11o         | 20o          |    o
 *    |   .          |              |   / 9
 *    |  .           |              |  /
 *    | .            |              | /
 *    |.             |              |/
 *    o--------------o--------------o
 *    0              8              1
 * \endverbatim
 *
 * (xi, eta, zeta) in [-1,1]^3 are the reference element coordinates
 * associated with the given numbering.
 *
 * \author Benjamin S. Kirk
 * \date 2002
 * \brief A 3D hexahedral element with 27 nodes.
 */
class Hex27 final : public Hex
{
public:

  /**
   * Constructor.  By default this element has no parent.
   */
  explicit
  Hex27 (Elem * p=nullptr) :
    Hex(Hex27::n_nodes(), p, _nodelinks_data)
  {}

  Hex27 (Hex27 &&) = delete;
  Hex27 (const Hex27 &) = delete;
  Hex27 & operator= (const Hex27 &) = delete;
  Hex27 & operator= (Hex27 &&) = delete;
  virtual ~Hex27() = default;

  /**
   * \returns \p HEX27.
   */
  virtual ElemType type () const override { return HEX27; }

  /**
   * \returns 27.
   */
  virtual unsigned int n_nodes() const override { return num_nodes; }

  /**
   * \returns 8.
   */
  virtual unsigned int n_sub_elem() const override { return 8; }

  /**
   * \returns \p true if the specified (local) node number is a vertex.
   */
  virtual bool is_vertex(const unsigned int i) const override;

  /**
   * \returns \p true if the specified (local) node number is an edge.
   */
  virtual bool is_edge(const unsigned int i) const override;

  /**
   * \returns \p true if the specified (local) node number is a face.
   */
  virtual bool is_face(const unsigned int i) const override;

  /**
   * \returns \p true if the specified (local) node number is on the
   * specified side.
   */
  virtual bool is_node_on_side(const unsigned int n,
                               const unsigned int s) const override;

  virtual std::vector<unsigned int> nodes_on_side(const unsigned int s) const override;

  virtual std::vector<unsigned int> nodes_on_edge(const unsigned int e) const override;

  /**
   * \returns \p true if the specified (local) node number is on the
   * specified edge.
   */
  virtual bool is_node_on_edge(const unsigned int n,
                               const unsigned int e) const override;

  /**
   * \returns \p true if the element map is definitely affine within
   * numerical tolerances.
   */
  virtual bool has_affine_map () const override;

  /**
   * \returns SECOND.
   */
  virtual Order default_order() const override;

  /**
   * Don't hide Elem::key() defined in the base class.
   */
  using Elem::key;

  /**
   * \returns An id associated with the \p s side of this element.
   * The id is not necessarily unique, but should be close.  This is
   * particularly useful in the \p MeshBase::find_neighbors() routine.
   *
   * We reimplement this method here for the \p Hex27 since we can
   * use the center node of each face to provide a perfect (unique)
   * key.
   */
  virtual dof_id_type key (const unsigned int s) const override;

  /**
   * \returns \p Hex27::side_nodes_map[side][side_node] after doing some range checking.
   */
  virtual unsigned int local_side_node(unsigned int side,
                                       unsigned int side_node) const override;

  /**
   * \returns \p Hex27::edge_nodes_map[edge][edge_node] after doing some range checking.
   */
  virtual unsigned int local_edge_node(unsigned int edge,
                                       unsigned int edge_node) const override;

  /**
   * Builds a \p QUAD9 built coincident with face i.
   * The \p std::unique_ptr<Elem> handles the memory aspect.
   */
  virtual std::unique_ptr<Elem> build_side_ptr (const unsigned int i,
                                                bool proxy=false) override;

  /**
   * Rebuilds a \p QUAD9 built coincident with face i.
   */
  virtual void build_side_ptr (std::unique_ptr<Elem> & elem,
                               const unsigned int i) override;

  /**
   * Builds a \p EDGE3 built coincident with edge i.
   * The \p std::unique_ptr<Elem> handles the memory aspect.
   */
  virtual std::unique_ptr<Elem> build_edge_ptr (const unsigned int i) override;

  /**
   * Rebuilds a \p EDGE3 built coincident with edge i.
   */
  virtual void build_edge_ptr (std::unique_ptr<Elem> & edge, const unsigned int i) override;

  virtual void connectivity(const unsigned int sc,
                            const IOPackage iop,
                            std::vector<dof_id_type> & conn) const override;

  /**
   * \returns 2 for all edge nodes, 4 for all face nodes, and
   * 8 for the bubble node.
   */
  virtual unsigned int n_second_order_adjacent_vertices (const unsigned int) const override;

  /**
   * \returns The element-local number of the \f$ v^{th} \f$ vertex
   * that defines the \f$ n^{th} \f$ second-order node.
   *
   * \note \p n is counted as depicted above, \f$ 8 \le n < 27 \f$.
   */
  virtual unsigned short int second_order_adjacent_vertex (const unsigned int n,
                                                           const unsigned int v) const override;

  /**
   * \returns The child number \p c and element-local index \p v of the
   * \f$ n^{th} \f$ second-order node on the parent element.  See
   * elem.h for further details.
   */
  virtual std::pair<unsigned short int, unsigned short int>
  second_order_child_vertex (const unsigned int n) const override;

  /**
   * Geometric constants for Hex27.
   */
  static const int num_nodes = 27;
  static const int num_sides = 6;
  static const int num_edges = 12;
  static const int num_children = 8;
  static const int nodes_per_side = 9;
  static const int nodes_per_edge = 3;

  /**
   * This maps the \f$ j^{th} \f$ node of the \f$ i^{th} \f$ side to
   * element node numbers.
   */
  static const unsigned int side_nodes_map[num_sides][nodes_per_side];

  /**
   * This maps the \f$ j^{th} \f$ node of the \f$ i^{th} \f$ edge to
   * element node numbers.
   */
  static const unsigned int edge_nodes_map[num_edges][nodes_per_edge];

  /**
   * A specialization for computing the volume of a Hex27.
   */
  virtual Real volume () const override;

  virtual void permute(unsigned int perm_num) override final;

  unsigned int center_node_on_side(const unsigned short side) const override final;

  ElemType side_type (const unsigned int s) const override final;

protected:


  /**
   * Data for links to nodes.
   */
  Node * _nodelinks_data[num_nodes];


#ifdef LIBMESH_ENABLE_AMR

  /**
   * Matrix used to create the elements children.
   */
  virtual Real embedding_matrix (const unsigned int i,
                                 const unsigned int j,
                                 const unsigned int k) const override
  { return _embedding_matrix[i][j][k]; }

  /**
   * Matrix that computes new nodal locations/solution values
   * from current nodes/solution.
   */
  static const Real _embedding_matrix[num_children][num_nodes][num_nodes];

  LIBMESH_ENABLE_TOPOLOGY_CACHES;

#endif // LIBMESH_ENABLE_AMR


private:

  /**
   * Matrix that tells which vertices define the location
   * of mid-side (or second-order) nodes.  This matrix only
   * covers the nodes that are unique to \p Hex27, while the
   * second-order-nodes that are identical with \p Hex20 are covered
   * through the \p _second_order_adjacent_vertices matrix in
   * \p cell_hex.C.
   *
   * \note This matrix also does not cover the bubble node.  The
   * interpolation is trivial and would only blow up the size of this
   * matrix.
   */
  static const unsigned short int _remaining_second_order_adjacent_vertices[6][4];

};

} // namespace libMesh

#endif // LIBMESH_CELL_HEX27_H
