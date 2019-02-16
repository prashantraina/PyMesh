/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class EdgeDihedralAngleAttribute : public MeshAttributeF {
    public:
        EdgeDihedralAngleAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~EdgeDihedralAngleAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
