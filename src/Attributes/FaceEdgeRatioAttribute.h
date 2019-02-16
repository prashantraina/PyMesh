/* This file is part of PyMesh. Copyright (c) 2016 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class FaceEdgeRatioAttribute : public MeshAttributeF {
    public:
        FaceEdgeRatioAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~FaceEdgeRatioAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
