/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class FaceCentroidAttribute : public MeshAttributeF {
    public:
        FaceCentroidAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~FaceCentroidAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
