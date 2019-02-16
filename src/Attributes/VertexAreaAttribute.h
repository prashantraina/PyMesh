/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VertexAreaAttribute : public MeshAttributeF {
    public:
        VertexAreaAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~VertexAreaAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);

    private:
        VectorF& get_face_areas(Mesh& mesh);
};

}
