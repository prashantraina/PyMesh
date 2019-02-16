/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include <string>

#include "MeshAttribute.h"

namespace PyMesh {

class VertexGaussianCurvatureAttribute : public MeshAttributeF {
    public:
        VertexGaussianCurvatureAttribute(const std::string& name) : MeshAttributeF(name) {}
        virtual ~VertexGaussianCurvatureAttribute() {}

    public:
        virtual void compute_from_mesh(Mesh& mesh);

    private:
        VectorF compute_face_angles(const Mesh& mesh, size_t face_idx);
};
}
