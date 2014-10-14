// Copyright (c) 2013, Sandia Corporation.
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 

#ifndef stk_mesh_MeshUtils_hpp
#define stk_mesh_MeshUtils_hpp

//----------------------------------------------------------------------

#include <stk_mesh/base/Types.hpp>

#include <vector>

//----------------------------------------------------------------------

namespace stk {
namespace mesh {
namespace impl {

//----------------------------------------------------------------------
//These functions are not part of the public API of stk-mesh.
//They are intended for use internally in the implementation of
//stk-mesh capabilities.
//----------------------------------------------------------------------

void find_elements_these_nodes_have_in_common(BulkData& mesh, unsigned numNodes, const Entity* nodes, std::vector<Entity>& elems);

void find_locally_owned_elements_these_nodes_have_in_common(BulkData& mesh, unsigned numNodes, const Entity* nodes, std::vector<Entity>& elems);

bool find_element_edge_ordinal_and_equivalent_nodes(BulkData& mesh, Entity element, unsigned numEdgeNodes, const Entity* edgeNodes, unsigned& elemEdgeOrdinal, Entity* elemEdgeNodes);


template<class DO_THIS_FOR_ENTITY_IN_CLOSURE, class DESIRED_ENTITY>
void VisitClosureGeneral(
        const BulkData & mesh,
        Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    if (desired_entity(entity_of_interest)) {
        do_this(entity_of_interest);
        if (mesh.is_valid(entity_of_interest)) {
            EntityRank entity_of_interest_rank = mesh.entity_rank(entity_of_interest);
            for (EntityRank rank = stk::topology::NODE_RANK ; rank < entity_of_interest_rank ; ++rank) {
                EntityVector entities_of_rank;
                size_t num_entities_of_rank = 0;
                const Entity * entity_it = NULL;
                if (mesh.connectivity_map().valid(mesh.entity_rank(entity_of_interest),rank))
                {
                    num_entities_of_rank = mesh.num_connectivity(entity_of_interest,rank);
                    entity_it = mesh.begin(entity_of_interest,rank);
                }
                else
                {
                    num_entities_of_rank = get_connectivity(mesh,entity_of_interest,rank,entities_of_rank);
                    entity_it = &*entities_of_rank.begin();
                }
                for (size_t i=0 ; i<num_entities_of_rank ; ++i, ++entity_it) {
                    VisitClosureGeneral(mesh,*entity_it,do_this,desired_entity);
                }
            }
        }
    }
}

template<class DO_THIS_FOR_ENTITY_IN_CLOSURE, typename FORWARD_ITERATOR, class DESIRED_ENTITY>
void VisitClosureGeneral(
        const stk::mesh::BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    for (FORWARD_ITERATOR entity_iterator = start ; entity_iterator != finish ; ++entity_iterator)
    {
        VisitClosureGeneral<DO_THIS_FOR_ENTITY_IN_CLOSURE,DESIRED_ENTITY>(mesh,*entity_iterator,do_this,desired_entity);
    }
}

struct StoreInVector {
    void operator()(stk::mesh::Entity entity) {
      ev.push_back(entity);
    }
    stk::mesh::EntityVector ev;
};

struct StoreInSet {
    void operator()(stk::mesh::Entity entity) {
      es.insert(entity);
    }
    std::set<Entity> es;
};

struct AlwaysVisit {
    bool operator()(Entity entity) { return true; }
};

struct OnlyVisitOnce {
    bool operator()(Entity entity) {
        if (already_visited.find(entity) == already_visited.end()) {
            already_visited.insert(entity);
            return true;
        }
        return false;
    }
    std::set<Entity> already_visited;
};

struct OnlyVisitLocallyOwnedOnce {
    OnlyVisitLocallyOwnedOnce(const BulkData & mesh_in) : mesh(mesh_in) {}
    bool operator()(Entity entity)
    {
        return ovo(entity) && mesh.bucket(entity).owned();
    }
    const BulkData& mesh;
    OnlyVisitOnce ovo;
};

struct OnlyVisitSharedOnce {
    OnlyVisitSharedOnce(const BulkData & mesh_in) : mesh(mesh_in) {}
    bool operator()(Entity entity)
    {
        if (ovo(entity) && !mesh.in_shared(mesh.entity_key(entity))) { return true; }
        return false;
    }
    const BulkData & mesh;
    OnlyVisitOnce ovo;
};

struct OnlyVisitGhostsOnce
{
    OnlyVisitGhostsOnce(BulkData & mesh_in) : mesh(mesh_in) {}
    bool operator()(Entity entity) {
        if (ovo(entity) && mesh.in_receive_ghost(mesh.entity_key(entity))) { return true; }
        return false;
    }
   BulkData & mesh;
   OnlyVisitOnce ovo;
};


template<class DO_THIS_FOR_ENTITY_IN_CLOSURE>
void VisitClosure(
        const stk::mesh::BulkData & mesh,
        stk::mesh::Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitClosureGeneral(mesh,entity_of_interest,do_this,ovo);
}


template<class DO_THIS_FOR_ENTITY_IN_CLOSURE, typename FORWARD_ITERATOR>
void VisitClosure(
        const stk::mesh::BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitClosureGeneral(mesh,start,finish,do_this,ovo);
}


// cyclomatic complexity = 6
template<class DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE, class DESIRED_ENTITY>
void VisitUpwardClosureGeneral(
        const BulkData & mesh,
        Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    if (desired_entity(entity_of_interest)) {
        do_this(entity_of_interest);
        if (mesh.is_valid(entity_of_interest)) {
            EntityRank entity_of_interest_rank = mesh.entity_rank(entity_of_interest);
            EntityVector entities_of_rank_up;
            for (EntityRank rank_up = EntityRank(entity_of_interest_rank+1) ; rank_up < stk::topology::END_RANK ; ++rank_up) {
                size_t num_entities_of_rank_up = 0;
                const Entity * entity_up_it = NULL;
                if (mesh.connectivity_map().valid(mesh.entity_rank(entity_of_interest),rank_up))
                {
                    num_entities_of_rank_up = mesh.num_connectivity(entity_of_interest,rank_up);
                    entity_up_it = mesh.begin(entity_of_interest,rank_up);
                }
                else
                {
                    num_entities_of_rank_up = get_connectivity(mesh,entity_of_interest,rank_up,entities_of_rank_up);
                    entity_up_it = &*entities_of_rank_up.begin();
                }
                for (size_t j=0 ; j<num_entities_of_rank_up ; ++j, ++entity_up_it) {
                    VisitUpwardClosureGeneral(mesh,*entity_up_it,do_this,desired_entity);
                }
            }
        }
    }
}

template<class DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE, typename FORWARD_ITERATOR, class DESIRED_ENTITY>
void VisitUpwardClosureGeneral(
        const BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    for (FORWARD_ITERATOR entity_iterator = start ; entity_iterator != finish ; ++entity_iterator)
    {
        VisitUpwardClosureGeneral<DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE,DESIRED_ENTITY>(mesh,*entity_iterator,do_this,desired_entity);
    }
}

template<class DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE>
void VisitUpwardClosure(
        const BulkData & mesh,
        Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitUpwardClosureGeneral(mesh,entity_of_interest,do_this,ovo);
}

template<class DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE, typename FORWARD_ITERATOR>
void VisitUpwardClosure(
        const BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_UPWARD_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitUpwardClosureGeneral(mesh,start,finish,do_this,ovo);
}

template<class DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE, typename FORWARD_ITERATOR, class DESIRED_ENTITY>
void VisitAuraClosureGeneral(
        const BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    StoreInSet ses;
    VisitClosure(mesh,start,finish,ses);
    VisitUpwardClosure(mesh,ses.es.begin(),ses.es.end(),ses);
    VisitClosureGeneral(mesh,ses.es.begin(),ses.es.end(),do_this,desired_entity);
}

template<class DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE, class DESIRED_ENTITY>
void VisitAuraClosureGeneral(
        const BulkData & mesh,
        Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE & do_this,
        DESIRED_ENTITY & desired_entity)
{
    Entity * start = &entity_of_interest;
    Entity * finish = start+1;
    VisitAuraClosureGeneral(mesh,start,finish,do_this,desired_entity);
}

template<class DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE, typename FORWARD_ITERATOR>
void VisitAuraClosure(
        const BulkData & mesh,
        const FORWARD_ITERATOR & start,
        const FORWARD_ITERATOR & finish,
        DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitAuraClosureGeneral(mesh,start,finish,do_this,ovo);
}

template<class DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE>
void VisitAuraClosure(
        const BulkData & mesh,
        Entity entity_of_interest,
        DO_THIS_FOR_ENTITY_IN_AURA_CLOSURE & do_this)
{
    OnlyVisitOnce ovo;
    VisitAuraClosureGeneral(mesh,entity_of_interest,do_this,ovo);
}



} // namespace impl
} // namespace mesh
} // namespace stk

//----------------------------------------------------------------------
//----------------------------------------------------------------------

#endif

