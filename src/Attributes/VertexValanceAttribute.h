/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VertexValanceAttribute : public MeshAttributeI {
    public:
        VertexValanceAttribute(const std::string& name) : MeshAttributeI(name) {}
        virtual ~VertexValanceAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);

    private:
        void compute_from_surface_mesh(Mesh& mesh);
        void compute_from_tet_mesh(Mesh& mesh);
        void compute_from_hex_mesh(Mesh& mesh);
};

}
