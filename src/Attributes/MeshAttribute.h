/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once
#include <string>
#include <memory>

#include <Core/EigenTypedef.h>

namespace PyMesh {
class Mesh;

/**
 * MeshAttribute provides functionality to compute and store information
 * associated with each of the mesh internal structure.  For example, vertex
 * normal, face area, voxel volumes, etc.
 */
template <typename TVector>
class MeshAttribute {
    public:
        typedef std::shared_ptr<MeshAttribute<TVector>> Ptr;

    public:
        virtual ~MeshAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh) {}
        virtual TVector& get_values() { return m_values; }
        virtual void set_values(TVector& values) { m_values = values; }

    protected:
        TVector m_values;
};

typedef MeshAttribute<VectorF> MeshAttributeF;
typedef MeshAttribute<VectorI> MeshAttributeI;
}
