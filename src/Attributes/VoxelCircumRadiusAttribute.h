/* This file is part of PyMesh. Copyright (c) 2017 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VoxelCircumRadiusAttribute : public MeshAttributeF {
    public:
        virtual ~VoxelCircumRadiusAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh) override;
};

}
