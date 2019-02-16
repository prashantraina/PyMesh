/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class FaceIndexAttribute : public MeshAttributeI {
    public:
        FaceIndexAttribute(const std::string& name) : MeshAttributeI(name) {}
        virtual ~FaceIndexAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
