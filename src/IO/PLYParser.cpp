/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#include "PLYParser.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <Core/EigenTypedef.h>
#include <Core/Exception.h>

#include "rply.h"

using namespace PyMesh;

namespace PLYParserHelper {
    void throw_attribute_not_found_exception(
            const std::string& name) {
        std::stringstream err_msg;
        err_msg << "Attribute " << name << " does not exist";
        throw RuntimeError(err_msg.str());
    }

    void assert_success(bool val) {
        if (!val) {
            throw RuntimeError("PLY parsing failed");
        }
    }

    std::string form_attribute_name(const std::string& elem_name,
            const std::string& prop_name) {
        return elem_name + "_" + prop_name;
    }

    int ply_parser_call_back(p_ply_argument argument) {
        p_ply_element elem;
        p_ply_property prop;
        e_ply_type type;
        e_ply_type value_type;
        const char* elem_name;
        const char* prop_name;
        PLYParser* parser;
        long prop_len;
        long value_idx;

        assert_success(ply_get_argument_element(argument, &elem, NULL));
        assert_success(ply_get_argument_property(argument, &prop, &prop_len, &value_idx));
        assert_success(ply_get_element_info(elem, &elem_name, NULL));
        assert_success(ply_get_property_info(prop, &prop_name, &type, NULL, &value_type));
        assert_success(ply_get_argument_user_data(argument, (void**)&parser, NULL));

        double value = ply_get_argument_value(argument);

        if (value_idx >= 0)
            if(type == PLY_FLOAT || type == PLY_DOUBLE ||
                    value_type == PLY_FLOAT || value_type == PLY_DOUBLE)
                parser->add_property_value(elem_name, prop_name, value);
            else
                parser->add_property_value(elem_name, prop_name, static_cast<int>(value));
        return 1;
    }

    void parse_ply(const std::string& filename, PLYParser* parser) {
        p_ply ply = ply_open(filename.c_str(), NULL, 0, NULL);
        assert_success(ply != NULL);
        assert_success(ply_read_header(ply));

        const char* elem_name;
        const char* prop_name;
        e_ply_type type;
        e_ply_type value_type;
        long num_elements;
        p_ply_element element = ply_get_next_element(ply, NULL);
        while (element != NULL) {
            assert_success(ply_get_element_info(element, &elem_name, &num_elements));

            p_ply_property property = ply_get_next_property(element, NULL);
            while (property != NULL) {
                assert_success(ply_get_property_info(property, &prop_name, &type, NULL, &value_type));

                ply_set_read_cb(ply, elem_name, prop_name, ply_parser_call_back, parser, 0);
                if(type == PLY_FLOAT || type == PLY_DOUBLE ||
                value_type == PLY_FLOAT || value_type == PLY_DOUBLE)
                    parser->add_float_property(elem_name, prop_name, num_elements);
                else
                    parser->add_int_property(elem_name, prop_name, num_elements);

                property = ply_get_next_property(element, property);
            }
            element = ply_get_next_element(ply, element);
        }
        assert_success(ply_read(ply));
        ply_close(ply);
    }
}

using namespace PLYParserHelper;

bool PLYParser::parse(const std::string& filename) {
    parse_ply(filename, this);
    init_vertices();
    init_faces();
    init_voxels();
    return true;
}

size_t PLYParser::num_vertices() const {
    return m_num_vertices;
}

size_t PLYParser::num_faces() const {
    return m_num_faces;
}

size_t PLYParser::num_voxels() const {
    return m_num_voxels;
}

size_t PLYParser::num_attributes() const {
    return m_attributesF.size() + m_attributesI.size();
}

PLYParser::AttrNames PLYParser::get_float_attribute_names() const {
    AttrNames names;
    for (AttributeMapF::const_iterator itr = m_attributesF.begin();
            itr != m_attributesF.end(); itr++) {
        names.push_back(itr->first);
    }
    return names;
}
PLYParser::AttrNames PLYParser::get_int_attribute_names() const {
    AttrNames names;
    for (AttributeMapI::const_iterator itr = m_attributesI.begin();
         itr != m_attributesI.end(); itr++) {
        names.push_back(itr->first);
    }
    return names;
}

size_t PLYParser::get_attribute_size(const std::string& name) const {
    AttributeMapF::const_iterator itrF = m_attributesF.find(name);
    AttributeMapI::const_iterator itrI = m_attributesI.find(name);
    if(itrF != m_attributesF.end())
        return itrF->second.size();
    else if(itrI != m_attributesI.end())
        return itrI->second.size();
    else
        throw_attribute_not_found_exception(name);
}

void PLYParser::export_vertices(Float* buffer) {
    std::copy(m_vertices.data(), m_vertices.data() + m_vertices.size(), buffer);
}

void PLYParser::export_faces(int* buffer) {
    std::copy(m_faces.data(), m_faces.data() + m_faces.size(), buffer);
}

void PLYParser::export_voxels(int* buffer) {
    std::copy(m_voxels.data(), m_voxels.data() + m_voxels.size(), buffer);
}

void PLYParser::export_float_attribute(const std::string& name, Float* buffer) {
    AttributeMapF::const_iterator itr = m_attributesF.find(name);
    if (itr == m_attributesF.end()) {
        throw_attribute_not_found_exception(name);
    }
    const std::vector<Float>& attr = itr->second;
    std::copy(attr.begin(), attr.end(), buffer);
}
void PLYParser::export_int_attribute(const std::string& name, int* buffer) {
    AttributeMapI::const_iterator itr = m_attributesI.find(name);
    if (itr == m_attributesI.end()) {
        throw_attribute_not_found_exception(name);
    }
    const std::vector<int>& attr = itr->second;
    std::copy(attr.begin(), attr.end(), buffer);
}

void PLYParser::add_float_property(const std::string& elem_name,
        const std::string& prop_name, size_t size) {
    if (elem_name == "vertex") {
        m_num_vertices = size;
    } else if (elem_name == "face") {
        m_num_faces = size;
    } else if (elem_name == "voxel") {
        m_num_voxels = size;
    }

    std::string attr_name = form_attribute_name(elem_name, prop_name);
    AttributeMapF::const_iterator itr = m_attributesF.find(attr_name);
    if (itr == m_attributesF.end()) {
        m_attributesF[attr_name] = std::vector<Float>();
    } else {
        std::stringstream err_msg;
        err_msg << "Duplicated property name: " << prop_name << std::endl;
        err_msg << "PyMesh requires unique custom property names";
        throw IOError(err_msg.str());
    }
}
void PLYParser::add_int_property(const std::string& elem_name,
                                   const std::string& prop_name, size_t size) {
    if (elem_name == "vertex") {
        m_num_vertices = size;
    } else if (elem_name == "face") {
        m_num_faces = size;
    } else if (elem_name == "voxel") {
        m_num_voxels = size;
    }

    std::string attr_name = form_attribute_name(elem_name, prop_name);
    AttributeMapI::const_iterator itr = m_attributesI.find(attr_name);
    if (itr == m_attributesI.end()) {
        m_attributesI[attr_name] = std::vector<int>();
    } else {
        std::stringstream err_msg;
        err_msg << "Duplicated property name: " << prop_name << std::endl;
        err_msg << "PyMesh requires unique custom property names";
        throw IOError(err_msg.str());
    }
}

void PLYParser::add_property_value(const std::string& elem_name,
        const std::string& prop_name, Float value) {
    std::string attr_name = form_attribute_name(elem_name, prop_name);
    AttributeMapF::iterator itr = m_attributesF.find(attr_name);
    if (itr == m_attributesF.end()) {
        throw_attribute_not_found_exception(attr_name);
    }
    std::vector<Float>& attr = itr->second;
    attr.push_back(value);
}
void PLYParser::add_property_value(const std::string& elem_name,
                                   const std::string& prop_name, int value) {
    std::string attr_name = form_attribute_name(elem_name, prop_name);
    AttributeMapI::iterator itr = m_attributesI.find(attr_name);
    if (itr == m_attributesI.end()) {
        throw_attribute_not_found_exception(attr_name);
    }
    std::vector<int>& attr = itr->second;
    attr.push_back(value);
}

void PLYParser::init_vertices() {
    std::string field_names[] = {
        std::string("vertex_x"),
        std::string("vertex_y"),
        std::string("vertex_z") };

    AttributeMapF::const_iterator iterators[3];
    m_dim = 0;
    for (size_t i=0; i<3; i++) {
        iterators[i] = m_attributesF.find(field_names[i]);
        if (iterators[i] != m_attributesF.end()) {
            m_dim++;
        }
    }

    m_vertices = VectorF(m_dim * m_num_vertices);
    for (size_t i=0; i<m_dim; i++) {
        const std::vector<Float>& coord = iterators[i]->second;
        assert(coord.size() == m_num_vertices);
        for (size_t j=0; j<m_num_vertices; j++) {
            m_vertices[j*m_dim + i] = coord[j];
        }
    }
    if (!m_vertices.allFinite()) {
        throw IOError("NaN or Inf detected in input file.");
    }
}

void PLYParser::init_faces() {
    AttributeMapI::const_iterator face_attr_itr = m_attributesI.find("face_vertex_indices");
    if (face_attr_itr == m_attributesI.end()) {
        face_attr_itr = m_attributesI.find("face_vertex_index");
        if (face_attr_itr == m_attributesI.end()) {
            m_vertex_per_face = 3; // default to triangle.
            m_num_faces = 0;
            return;
        }
    }
    const std::vector<int>& faces = face_attr_itr->second;
    assert(faces.size() == 0 || faces.size() % m_num_faces == 0);
    m_vertex_per_face = m_num_faces == 0 ? 3:faces.size() / m_num_faces;
    m_faces = VectorI(faces.size());
    std::copy(faces.begin(), faces.end(), m_faces.data());
}

void PLYParser::init_voxels() {
    AttributeMapI::const_iterator voxel_attr_itr = m_attributesI.find("voxel_vertex_indices");
    if (voxel_attr_itr == m_attributesI.end()) {
        voxel_attr_itr = m_attributesI.find("voxel_vertex_index");
        if (voxel_attr_itr == m_attributesI.end()) {
            m_vertex_per_voxel = 0;
            return;
        }
    }
    const std::vector<int>& voxels = voxel_attr_itr->second;
    assert(voxels.size() % m_num_voxels == 0);
    m_vertex_per_voxel = voxels.size() / m_num_voxels;
    m_voxels = VectorI(voxels.size());
    std::copy(voxels.begin(), voxels.end(), m_voxels.data());
}

