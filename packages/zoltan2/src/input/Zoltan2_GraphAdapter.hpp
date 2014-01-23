// @HEADER
//
// ***********************************************************************
//
//   Zoltan2: A package of combinatorial algorithms for scientific computing
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Karen Devine      (kddevin@sandia.gov)
//                    Erik Boman        (egboman@sandia.gov)
//                    Siva Rajamanickam (srajama@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

/*! \file Zoltan2_GraphAdapter.hpp
    \brief Defines the GraphAdapter interface.
*/


#ifndef _ZOLTAN2_GRAPHADAPTER_HPP_
#define _ZOLTAN2_GRAPHADAPTER_HPP_

#include <Zoltan2_Adapter.hpp>

namespace Zoltan2 {

/*!  \brief Enumerated entity type for graphs:  Vertices or Edges
 */
enum GraphEntityType {
  GRAPH_VERTEX,
  GRAPH_EDGE
};

/*!  \brief GraphAdapter defines the interface for graph-based user data.

    Adapter objects provide access for Zoltan2 to the user's data.
    Many built-in adapters are already defined for common data structures,
    such as Tpetra and Epetra objects and C-language pointers to arrays.

    Data types:
    \li \c scalar_t vertex and edge weights 
    \li \c lno_t    local indices and local counts
    \li \c gno_t    global indices and global counts
    \li \c gid_t    application global Ids
    \li \c node_t is a sub class of KokkosClassic::StandardNodeMemoryModel

    See IdentifierTraits to understand why the user's global ID type (\c gid_t)
    may differ from that used by Zoltan2 (\c gno_t).

    The Kokkos node type can be safely ignored.

    The template parameter \c User is a user-defined data type
    which, through a traits mechanism, provides the actual data types
    with which the Zoltan2 library will be compiled.
    \c User may be the actual class or structure used by application to
    represent a vector, or it may be the helper class BasicUserTypes.
    See InputTraits for more information.

    The \c scalar_t type, representing use data such as matrix values, is
    used by Zoltan2 for weights, coordinates, part sizes and
    quality metrics.
    Some User types (like Tpetra::CrsMatrix) have an inherent scalar type,
    and some
    (like Tpetra::CrsGraph) do not.  For such objects, the scalar type is
    set by Zoltan2 to \c float.  If you wish to change it to double, set
    the second template parameter to \c double.

*/

template <typename User>
  class GraphAdapter : public BaseAdapter<User> {
private:
  enum GraphEntityType primaryEntityType; // Entity (vertex or edge) to
                                          // be partitioned, ordered,
                                          // colored, matched, etc.
  enum GraphEntityType adjacencyEntityType; // Entity (edge or vertex)
                                            // describing adjacencies;
                                            // typically opposite of
                                            // primaryEntityType.

public:

#ifndef DOXYGEN_SHOULD_SKIP_THIS
  typedef typename InputTraits<User>::scalar_t    scalar_t;
  typedef typename InputTraits<User>::lno_t    lno_t;
  typedef typename InputTraits<User>::gno_t    gno_t;
  typedef typename InputTraits<User>::gid_t    gid_t;
  typedef typename InputTraits<User>::node_t   node_t;
  typedef User user_t;
#endif

  enum BaseAdapterType adapterType() const {return GraphAdapterType;}

  /*! \brief Destructor
   */
  virtual ~GraphAdapter() {};

  // Default GraphEntityType is GRAPH_VERTEX.
  GraphAdapter() : primaryEntityType(GRAPH_VERTEX),
                   adjacencyEntityType(GRAPH_EDGE) {};

  ////////////////////////////////////////////////////////////////////////////
  // Methods to be defined in derived classes.

  /*! \brief Returns the number of vertices on this process.
   */
  virtual size_t getLocalNumVertices() const = 0;

  /*! \brief Returns the number of edges on this process.
   */
  virtual size_t getLocalNumEdges() const = 0;

  /*! \brief Sets pointers to this process' graph entries.
      \param vertexIds will on return a pointer to vertex global Ids
   */
  virtual void getVertexIDsView(const gid_t *&vertexIds) const = 0; 

  /*! \brief Gets adjacency lists for all vertices in a compressed
             sparse row (CSR) format.
      \param offsets is an array of size getLocalNumVertices() + 1.  
         The neighboring vertices for vertexId[i] 
         begin at adjIds[offsets[i]].  
          The last element of offsets is the size of the adjIds array.
      \param adjIds on return will point to the array of adjacent vertices for
         for each vertex.
   */
  virtual void getEdgeView(const lno_t *&offsets,
                           const gid_t *&adjIds) const = 0;
       
  /*! \brief Returns the dimension (0 or greater) of vertex weights.
   */
  virtual int getNumWeightsPerVertex() const { return 0; }

  /*! \brief  Provide a pointer to the vertex weights, if any.
      \param weights is the list of weights of the given dimension for
           the vertices returned in getVertexIDsView().  If weights for
           this dimension are to be uniform for all vertices in the
           global problem, the \c weights should be a NULL pointer.
      \param stride The k'th weight is located at weights[stride*k]
      \param idx ranges from zero to one less than getNumWeightsPerVertex().
   */
  virtual void getVertexWeightsView(const scalar_t *&weights, int &stride, 
                                    int idx = 0) const
  {
    weights = NULL;
    stride = 0;
    Z2_THROW_NOT_IMPLEMENTED_ERROR
  }

  /*! \brief Returns the dimension (0 or greater) of vertex weights.
   */
  virtual int getNumWeightsPerEdge() const { return 0; }

  /*! \brief  Provide a pointer to the edge weights, if any.  
      \param weights is the list of weights of the given index for
           the edges returned in getEdgeView().
      \param stride The k'th weight is located at weights[stride*k]
      \param idx ranges from zero to one less than getNumWeightsPerEdge().
   */
  virtual void getEdgeWeightsView(const scalar_t *&weights, int &stride,
                                  int idx = 0) const
  {
    weights = NULL;
    stride = 0;
    Z2_THROW_NOT_IMPLEMENTED_ERROR
  }


  /*! \brief Returns the dimension of the geometry, if any.
   *  Some algorithms can use geometric coordinate information if it is present.
   *  Since coordinate information is optional, we provide a default definition
   *  that does not return coordinate info.  Individual graph adapters can
   *  override this definition.
   */
  virtual int getCoordinateDimension() const { return 0; }

  /*! \brief Provide a pointer to one dimension of vertex coordinates.
   *  \param ent is the entity for which coordinate information is requested
   *         Valid values are GRAPH_VERTEX and GRAPH_EDGE.
   *  \param coordDim  is a value from 0 to one less than
   *     getDimension() specifying which dimension is
   *     being provided in the coords list.
   *  \param coords  points to a list of coordinate values for the dimension.
   *  \param stride  describes the layout of the coordinate values in
   *          the coords list.  If stride is one, then the ith coordinate
   *          value is coords[i], but if stride is two, then the
   *          ith coordinate value is coords[2*i].
   *  Since coordinate information is optional, we provide a default definition
   *  that does not return coordinate info.  Individual graph adapters can
   *  override this definition.
   */

  virtual void getVertexCoordinatesView(const scalar_t *&coords, int &stride,
                                          int idx) const
  {
    coords = NULL;
    stride = 0;
    Z2_THROW_NOT_IMPLEMENTED_ERROR
  }

  ////////////////////////////////////////////////////////////////////////////
  // Implementations of base-class methods

  /*! \brief Returns the entity to be partitioned, ordered, colored, etc.
   *  Valid values are GRAPH_VERTEX or GRAPH_EDGE.
   */
  inline enum GraphEntityType getPrimaryEntityType() const {
    return this->primaryEntityType;
  }

  /*! \brief Sets the primary entity type.  Called by algorithm based on
   *  parameter value in parameter list from application.
   *  Also sets to adjacencyEntityType to something reasonable:  opposite of
   *  primaryEntityType.
   */
  void setPrimaryEntityType(string typestr) {
    if (typestr == "vertex") {
      this->primaryEntityType = GRAPH_VERTEX;
      this->adjacencyEntityType = GRAPH_EDGE;
    }
    else if (typestr == "edge") {
      this->primaryEntityType = GRAPH_EDGE;
      this->adjacencyEntityType = GRAPH_VERTEX;
    }
    else {
      std::ostringstream emsg;
      emsg << __FILE__ << "," << __LINE__
           << " error:  Invalid GraphEntityType " << typestr << std::endl;
      emsg << "Valid values are 'vertex' and 'edge'" << std::endl;
      throw std::runtime_error(emsg.str());
    }
  }

  /*! \brief Returns the entity that describes adjacencies between the
   *  entities to be partitioned, ordered, colored, etc.
   *  Valid values are GRAPH_VERTEX or GRAPH_EDGE.
   */
  inline enum GraphEntityType getAdjacencyEntityType() const {
    return this->adjacencyEntityType;
  }

  /*! \brief Sets the adjacency entity type.  Called by algorithm based on
   *  parameter value in parameter list from application.
   *  Also sets to primaryEntityType to something reasonable:  opposite of
   *  adjacencyEntityType.
   */
  void setAdjacencyEntityType(string typestr) {
    if (typestr == "vertex") {
      this->adjacencyEntityType = GRAPH_VERTEX;
      this->primaryEntityType = GRAPH_EDGE;
    }
    else if (typestr == "edge") {
      this->adjacencyEntityType = GRAPH_EDGE;
      this->primaryEntityType = GRAPH_VERTEX;
    }
    else {
      std::ostringstream emsg;
      emsg << __FILE__ << "," << __LINE__
           << " error:  Invalid GraphEntityType " << typestr << std::endl;
      emsg << "Valid values are 'vertex' and 'edge'" << std::endl;
      throw std::runtime_error(emsg.str());
    }
  }

  // Functions from the BaseAdapter interface
  size_t getLocalNumIDs() const {
    if (getPrimaryEntityType() == GRAPH_VERTEX)
      return getLocalNumVertices();
    else
      return getLocalNumEdges();
   }

  void getIDsView(const gid_t *&Ids) const {
    if (getPrimaryEntityType() == GRAPH_VERTEX)
      getVertexIDsView(Ids);
    else {
      // TODO:  Need getEdgeIDsView?  What is an Edge ID?  
      // TODO:  std::pair<gid_t, gid_t>?
      std::ostringstream emsg;
      emsg << __FILE__ << "," << __LINE__
           << " error:  getIDsView not yet supported for graph edges." 
           << std::endl;
      throw std::runtime_error(emsg.str());
    }
  }

  int getNumWeightsPerID() const {
    if (getPrimaryEntityType() == GRAPH_VERTEX)
      return getNumWeightsPerVertex();
    else
      return getNumWeightsPerEdge();
  }

  void getWeightsView(const scalar_t *&wgt, int &stride, int idx = 0) const {
    if (getPrimaryEntityType() == GRAPH_VERTEX)
      getVertexWeightsView(wgt, stride, idx);
    else {
      // TODO:  Need getEdgeWeightsView that lets Edges be primary object? 
      // TODO:  That is, get edge weights based on some Edge ID.
      std::ostringstream emsg;
      emsg << __FILE__ << "," << __LINE__
           << " error:  getWeightsView not yet supported for graph edges." 
           << std::endl;
      throw std::runtime_error(emsg.str());
    }
  }
};

}  //namespace Zoltan2

#endif