/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#include "VertexIndexAttribute.h"

#include <Mesh.h>

using namespace PyMesh;

void VertexIndexAttribute::compute_from_mesh(Mesh& mesh) {
    const size_t num_vertices = mesh.get_num_vertices();
    VectorI& indices = m_values;
    indices = VectorI::LinSpaced(num_vertices, 0, num_vertices-1);
}
