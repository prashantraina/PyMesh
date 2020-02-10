/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once

#include <string>

#include "MeshAttribute.h"

namespace PyMesh {

class VertexMeanCurvatureAttribute : public MeshAttributeF {
    public:
        virtual ~VertexMeanCurvatureAttribute() = default;

    public:
        virtual void compute_from_mesh(Mesh& mesh) override;

    private:
        VectorF compute_laplacian_vectors(Mesh& mesh);
        VectorF compute_vertex_normals(Mesh& mesh);
};

}
