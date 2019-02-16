/* This file is part of PyMesh. Copyright (c) 2016 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class FaceCircumRadiusAttribute : public MeshAttributeF {
    public:
        FaceCircumRadiusAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~FaceCircumRadiusAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
