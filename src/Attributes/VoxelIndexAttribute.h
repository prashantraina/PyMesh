/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VoxelIndexAttribute : public MeshAttributeI {
    public:
        virtual ~VoxelIndexAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh) override;
};

}
