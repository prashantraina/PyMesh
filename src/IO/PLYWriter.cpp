/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#include "PLYWriter.h"

#include <cassert>
#include <iostream>
#include <sstream>

#include <Core/EigenTypedef.h>
#include <Core/Exception.h>
#include <Mesh.h>
#include <MeshFactory.h>

#include "rply.h"

namespace PLYWriterHelper {
    void assert_success(bool val, std::string message="") {
        if (!val) {
            if (message == "") {
                throw RuntimeError("PLY writing failed");
            } else {
                throw RuntimeError(message);
            }
        }
    }
}

using namespace PLYWriterHelper;

void PLYWriter::with_attribute(const std::string& attr_name) {
    m_attr_names.push_back(attr_name);
}

void PLYWriter::write_mesh(Mesh& mesh) {
    const size_t dim = mesh.get_dim();
    p_ply ply = ply_create(m_filename.c_str(),
            m_in_ascii ? PLY_ASCII : PLY_LITTLE_ENDIAN,
            NULL, 0, NULL);
    assert_success(ply != NULL, "ply_create_failed");

    regroup_attribute_names(mesh);

    add_vertex_elements_header(mesh, ply);
    add_face_elements_header(mesh, ply);
    add_voxel_elements_header(mesh, ply);

    assert_success(ply_add_comment(ply, "Generated by PyMesh"),
            "Adding comment failed");
    assert_success(ply_write_header(ply), "Writting header failed");

    write_vertex_elements(mesh, ply);
    write_face_elements(mesh, ply);
    write_voxel_elements(mesh, ply);

    ply_close(ply);
}

void PLYWriter::write(
        const VectorF& vertices,
        const VectorI& faces,
        const VectorI& voxels,
        size_t dim, size_t vertex_per_face, size_t vertex_per_voxel) {
    if (m_attr_names.size() != 0) {
        std::cerr << "Warning: all attributes are ignored" << std::endl;
        m_attr_names.clear();
    }

    Mesh::Ptr mesh = MeshFactory()
        .load_data(vertices, faces, voxels, dim, vertex_per_face, vertex_per_voxel)
        .create_shared();
    write_mesh(*mesh);
}

void PLYWriter::regroup_attribute_names(Mesh& mesh) {
    const size_t num_vertices = mesh.get_num_vertices();
    const size_t num_faces = mesh.get_num_faces();
    const size_t num_voxels = mesh.get_num_voxels();

    m_vertex_attr_names.clear();
    m_face_attr_names.clear();
    m_voxel_attr_names.clear();

    for (NameArray::const_iterator itr = m_attr_names.begin();
            itr != m_attr_names.end(); itr++) {
        const std::string& name = *itr;
        if (!mesh.has_attribute(name)) {
            std::stringstream err_msg;
            err_msg << "Outputing PLY file failed because attribute \""
                << name << "\" does not exist!" << std::endl;
            throw RuntimeError(err_msg.str());
        }

        const VectorF& attr = mesh.get_attribute(name);
        const size_t attr_size = attr.size();
        if (attr_size % num_vertices == 0) {
            m_vertex_attr_names.push_back(name);
        } else if (attr_size % num_faces == 0) {
            m_face_attr_names.push_back(name);
        } else if (attr_size % num_voxels == 0) {
            m_voxel_attr_names.push_back(name);
        } else {
            throw NotImplementedError("Unknown attribute type");
        }
    }
}

void PLYWriter::add_vertex_elements_header(Mesh& mesh, p_ply& ply) {
    const size_t dim = mesh.get_dim();
    const size_t num_vertices = mesh.get_num_vertices();
    assert_success(ply_add_element(ply, "vertex", num_vertices),
            "Add element failed");
    assert_success(ply_add_scalar_property(ply, "x", m_scalar),
            "Add X coordinate failed");
    assert_success(ply_add_scalar_property(ply, "y", m_scalar),
            "Add Y coordinate failed");
    if (dim == 3) {
        assert_success(ply_add_scalar_property(ply, "z", m_scalar),
                "Add Z coordinate failed");
    }

    for (NameArray::const_iterator itr = m_vertex_attr_names.begin();
            itr != m_vertex_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        const size_t per_vertex_size = attr.size() / num_vertices;
        e_ply_type ply_type = m_scalar;
        if (name == "red" || name == "green" || name == "blue") {
            ply_type = PLY_UCHAR;
        }
        if (per_vertex_size == 1) {
            assert_success(ply_add_scalar_property(ply, name.c_str(), ply_type),
                    "Add scalar property failed");
        } else {
            assert_success(ply_add_list_property(ply, name.c_str(), PLY_UCHAR,
                        ply_type), "Add list proerty failed");
        }
    }
}

void PLYWriter::add_face_elements_header(Mesh& mesh, p_ply& ply) {
    const size_t dim = mesh.get_dim();
    const size_t num_faces = mesh.get_num_faces();
    assert_success(ply_add_element(ply, "face", num_faces), "Add face failed");
    assert_success(ply_add_list_property(ply, "vertex_indices", PLY_UCHAR,
                PLY_INT), "Add face vertex indices failed");

    for (NameArray::const_iterator itr = m_face_attr_names.begin();
            itr != m_face_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        const size_t per_face_size = attr.size() / num_faces;
        e_ply_type ply_type = m_scalar;
        if (name == "red" || name == "green" || name == "blue") {
            ply_type = PLY_UCHAR;
        }
        if (per_face_size == 1) {
            assert_success(ply_add_scalar_property(ply, name.c_str(), ply_type),
                    "Add per face scalar attribute failed");
        } else {
            assert_success(ply_add_list_property(ply, name.c_str(), PLY_UCHAR,
                        ply_type), "Add per face vector attribute failed");
        }
    }
}

void PLYWriter::add_voxel_elements_header(Mesh& mesh, p_ply& ply) {
    const size_t dim = mesh.get_dim();
    const size_t num_voxels = mesh.get_num_voxels();
    if (num_voxels == 0) return;
    assert_success(ply_add_element(ply, "voxel", num_voxels), "Add voxel failed");
    assert_success(ply_add_list_property(ply, "vertex_indices", PLY_UCHAR,
                PLY_INT), "Add voxel vertex indices failed");

    for (NameArray::const_iterator itr = m_voxel_attr_names.begin();
            itr != m_voxel_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        const size_t per_voxel_size = attr.size() / num_voxels;
        e_ply_type ply_type = m_scalar;
        if (name == "red" || name == "green" || name == "blue") {
            ply_type = PLY_UCHAR;
        }
        if (per_voxel_size == 1) {
            assert_success(ply_add_scalar_property(ply, name.c_str(), ply_type),
                    "Add per voxel scalar attribute failed");
        } else {
            assert_success(ply_add_list_property(ply, name.c_str(), PLY_UCHAR,
                        ply_type), "Add per voxel vector attribute failed");
        }
    }
}

void PLYWriter::write_vertex_elements(Mesh& mesh, p_ply& ply) {
    const size_t dim = mesh.get_dim();
    const size_t num_vertices = mesh.get_num_vertices();
    const VectorF& vertices = mesh.get_vertices();

    std::vector<const VectorF*> attributes;
    std::vector<size_t> per_vertex_sizes;
    for (NameArray::const_iterator itr = m_vertex_attr_names.begin();
            itr != m_vertex_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        attributes.push_back(&attr);
        per_vertex_sizes.push_back(attr.size() / num_vertices);
    }

    const size_t num_attributes = attributes.size();

    for (size_t i=0; i<num_vertices; i++) {
        for (size_t j=0; j<dim; j++) {
            ply_write(ply, vertices[i*dim + j]);
        }
        for (size_t j=0; j<num_attributes; j++) {
            const size_t per_vertex_size = per_vertex_sizes[j];
            if (per_vertex_size != 1) {
                ply_write(ply, per_vertex_size);
            }
            for (size_t k=0; k<per_vertex_size; k++) {
                ply_write(ply, attributes[j]->coeff(i*per_vertex_size + k));
            }
        }
    }
}

void PLYWriter::write_face_elements(Mesh& mesh, p_ply& ply) {
    const size_t num_faces = mesh.get_num_faces();
    const size_t num_vertex_per_face = mesh.get_vertex_per_face();
    const VectorI& faces = mesh.get_faces();

    std::vector<const VectorF*> attributes;
    std::vector<size_t> per_face_sizes;
    for (NameArray::const_iterator itr = m_face_attr_names.begin();
            itr != m_face_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        attributes.push_back(&attr);
        per_face_sizes.push_back(attr.size() / num_faces);
    }

    const size_t num_attributes = attributes.size();
    for (size_t i=0; i<num_faces; i++) {
        ply_write(ply, num_vertex_per_face);
        for (size_t j=0; j<num_vertex_per_face; j++) {
            ply_write(ply, faces[i*num_vertex_per_face + j]);
        }
        for (size_t j=0; j<num_attributes; j++) {
            const size_t per_face_size = per_face_sizes[j];
            if (per_face_size != 1) {
                ply_write(ply, per_face_size);
            }
            for (size_t k=0; k<per_face_size; k++) {
                ply_write(ply, attributes[j]->coeff(i*per_face_size + k));
            }
        }
    }
}

void PLYWriter::write_voxel_elements(Mesh& mesh, p_ply& ply) {
    const size_t num_voxels = mesh.get_num_voxels();
    if (num_voxels == 0) return;
    const size_t num_vertex_per_voxel = mesh.get_vertex_per_voxel();
    const VectorI& voxels = mesh.get_voxels();

    std::vector<const VectorF*> attributes;
    std::vector<size_t> per_voxel_sizes;
    for (NameArray::const_iterator itr = m_voxel_attr_names.begin();
            itr != m_voxel_attr_names.end(); itr++) {
        const std::string& name = *itr;
        const VectorF& attr = mesh.get_attribute(name);
        attributes.push_back(&attr);
        assert(attr.size() % num_voxels == 0);
        per_voxel_sizes.push_back(attr.size() / num_voxels);
    }

    const size_t num_attributes = attributes.size();
    for (size_t i=0; i<num_voxels; i++) {
        ply_write(ply, num_vertex_per_voxel);
        for (size_t j=0; j<num_vertex_per_voxel; j++) {
            ply_write(ply, voxels[i*num_vertex_per_voxel + j]);
        }
        for (size_t j=0; j<num_attributes; j++) {
            const size_t per_voxel_size = per_voxel_sizes[j];
            if (per_voxel_size != 1) {
                ply_write(ply, per_voxel_size);
            }
            for (size_t k=0; k<per_voxel_size; k++) {
                ply_write(ply, attributes[j]->coeff(i*per_voxel_size + k));
            }
        }
    }
}

