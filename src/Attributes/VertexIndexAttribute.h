/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VertexIndexAttribute : public MeshAttributeI {
    public:
        VertexIndexAttribute(const std::string& name) : MeshAttributeI(name) {}
        virtual ~VertexIndexAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
