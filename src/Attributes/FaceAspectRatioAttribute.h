/* This file is part of PyMesh. Copyright (c) 2016 by Qingnan Zhou */
#pragma once

#include "MeshAttribute.h"

namespace PyMesh {

class FaceAspectRatioAttribute : public MeshAttributeF {
    public:
        virtual ~FaceAspectRatioAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh) override;
};

}
