/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include <string>

#include "MeshAttribute.h"

namespace PyMesh {

class VertexGaussianCurvatureAttribute : public MeshAttributeF {
    public:
        virtual ~VertexGaussianCurvatureAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh) override;

    private:
        VectorF compute_face_angles(const Mesh& mesh, size_t face_idx);
};
}
