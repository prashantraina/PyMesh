/* This file is part of PyMesh. Copyright (c) 2018 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class VoxelFaceIndexAttribute : public MeshAttributeI {
    public:
        VoxelFaceIndexAttribute(const std::string& name) : MeshAttributeI(name) {}
        virtual ~VoxelFaceIndexAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh);
};

}
