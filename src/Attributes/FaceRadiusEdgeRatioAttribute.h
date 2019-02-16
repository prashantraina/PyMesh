/* This file is part of PyMesh. Copyright (c) 2018 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class Mesh;

class FaceRadiusEdgeRatioAttribute : public MeshAttributeF {
    public:
        FaceRadiusEdgeRatioAttribute(const std::string& name)
            : MeshAttributeF(name) {}
        virtual ~FaceRadiusEdgeRatioAttribute()=default;

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
