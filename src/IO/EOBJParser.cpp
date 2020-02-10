/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#include "EOBJParser.h"
#include <cstdio>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <list>
#include <sstream>

#include <Core/EigenTypedef.h>
#include <Core/Exception.h>

using namespace PyMesh;

EOBJParser::EOBJParser() :
    m_dim(0),
    m_vertex_per_face(0),
    m_texture_dim(0),
    m_parameter_dim(0){ }

bool EOBJParser::parse(const std::string& filename) {
    const size_t LINE_SIZE = 4096;
    char line[LINE_SIZE];

    std::ifstream fin(filename.c_str());
    if (!fin.is_open()) {
        std::stringstream err_msg;
        err_msg << "failed to open file \"" << filename << "\"";
        throw IOError(err_msg.str());
    }

    bool success;
    while (!fin.eof()) {
        fin.getline(line, LINE_SIZE);
        char* continue_char = strrchr(line, '\\');
        while (continue_char != NULL &&
                (continue_char - line + 1) == strlen(line)) {
            fin.getline(continue_char, LINE_SIZE-(continue_char - line + 1));
            continue_char = strchr(line, '\\');
        }

        switch (line[0]) {
            case 'v':
                success = parse_vertex_line(line);
                break;
            case 'f':
                success = parse_face_line(line);
                break;
            case '#':
                success = parse_comment_line(line);
                success = true; //it might be just a regular comment
            default:
                // Ignore other lines by default.
                success = true;
                break;
        }
        if (!success) return false;
    }

    fin.close();
    unify_faces();

    if (m_vertices.size() == 0) {
        m_dim = 3; // default: 3D
    }
    if (m_faces.size() == 0) {
        m_vertex_per_face = 3; // default: triangle
    }
    finalize_textures();
    finalize_normals();
    finalize_parameters();
    return true;
}

size_t EOBJParser::num_attributes() const {
    size_t r = 0;
    if (m_corner_normals.size() > 0) r++;
    if (m_corner_textures.size() > 0) r++;
    if (m_parameters.size() > 0) r++;
    return r;
}

EOBJParser::AttrNames EOBJParser::get_float_attribute_names() const {
    EOBJParser::AttrNames attr_names;
    if (m_corner_normals.size() > 0)
        attr_names.push_back("corner_normal");
    if (m_corner_textures.size() > 0)
        attr_names.push_back("corner_texture");
    if (m_parameters.size() > 0)
        attr_names.push_back("vertex_parameter");
    for (AttributeMapF::const_iterator itr = m_attributesF.begin();
         itr != m_attributesF.end(); itr++) {
        attr_names.push_back(itr->first);
    }
    return attr_names;
}

EOBJParser::AttrNames EOBJParser::get_int_attribute_names() const {
    EOBJParser::AttrNames attr_names;
    for (AttributeMapI::const_iterator itr = m_attributesI.begin();
         itr != m_attributesI.end(); itr++) {
        attr_names.push_back(itr->first);
    }
    return attr_names;
}

size_t EOBJParser::get_attribute_size(const std::string& name) const {
    if (name == "corner_normal")
        return m_corner_normals.size() * m_dim;
    else if (name == "corner_texture")
        return m_corner_textures.size() * m_texture_dim;
    else if (name == "vertex_parameter")
        return m_parameters.size() * m_parameter_dim;
    else
    {
        const auto pairF = m_attributesF.find(name);
        const auto pairI = m_attributesI.find(name);
        if(pairF != m_attributesF.end()) {
            std::cout << "Float attribute '" << name << "' has size " << pairF->second.size() << std::endl;
            return pairF->second.size();
        }
        else if(pairI != m_attributesI.end())
            return pairI->second.size();
        else {
            std::cerr << "Attribute " << name << " does not exist." << std::endl;
            return 0;
        }
    }
}

void EOBJParser::export_vertices(Float* buffer) {
    const size_t dim = m_dim;
    size_t count=0;
    for (VertexList::const_iterator vi=m_vertices.begin();
            vi != m_vertices.end(); vi++) {
        const VectorF& v = *vi;
        for (size_t i=0; i<dim; i++) {
            buffer[dim * count + i] = v[i];
        }
        count++;
    }
}

void EOBJParser::export_faces(int* buffer) {
    size_t count=0;
    for (FaceList::const_iterator fi=m_faces.begin();
            fi != m_faces.end(); fi++) {
        const VectorI& f = *fi;
        assert(f.size() == m_vertex_per_face);
        std::copy(f.data(), f.data() + m_vertex_per_face,
                buffer + count*m_vertex_per_face);
        count++;
    }
}

void EOBJParser::export_voxels(int* buffer) {
    size_t num_vertex_per_voxel = vertex_per_voxel();
    size_t count=0;
    for (VoxelList::const_iterator vi=m_voxels.begin();
            vi != m_voxels.end(); vi++) {
        const VectorI& v = *vi;
        const int* voxel = v.data();
        std::copy(voxel, voxel + num_vertex_per_voxel,
                buffer + num_vertex_per_voxel * count);
        count++;
    }
}

void EOBJParser::export_float_attribute(const std::string& name, Float* buffer) {
    if (name == "corner_normal")
        export_normals(buffer);
    else if (name == "corner_texture")
        export_textures(buffer);
    else if (name == "vertex_parameter")
        export_parameters(buffer);
    else {
        AttributeMapF::const_iterator itr = m_attributesF.find(name);
        if (itr == m_attributesF.end()) {
            std::cerr << "Warning: mesh does not have attribute with name "
                      << name << std::endl;
        } else {
            const std::vector<Float> &attr = itr->second;
            std::copy(attr.begin(), attr.end(), buffer);
        }
    }
}

void EOBJParser::export_int_attribute(const std::string& name, int* buffer) {
   AttributeMapI::const_iterator itr = m_attributesI.find(name);
    if (itr == m_attributesI.end()) {
        std::cerr << "Warning: mesh does not have attribute with name "
                  << name << std::endl;
    } else {
        const std::vector<int> &attr = itr->second;
        std::copy(attr.begin(), attr.end(), buffer);
    }
}

void EOBJParser::export_normals(Float* buffer) const {
    const size_t dim = m_dim;
    size_t count=0;
    for (const auto& n : m_corner_normals) {
        std::copy(n.data(), n.data() + dim, buffer+count*dim);
        count++;
    }
}

void EOBJParser::export_textures(Float* buffer) const {
    const size_t dim = m_texture_dim;
    size_t count=0;
    for (const auto& t : m_corner_textures) {
        std::copy(t.data(), t.data() + dim, buffer+count*dim);
        count++;
    }
}

void EOBJParser::export_parameters(Float* buffer) const {
    const size_t dim = m_parameter_dim;
    size_t count=0;
    for (const auto& p : m_parameters) {
        std::copy(p.data(), p.data() + dim, buffer+count*dim);
        count++;
    }
}

void EOBJParser::add_float_attribute(const std::string& elem_name,
                                   const std::string& prop_name, size_t size) {


    std::string attr_name = elem_name + "_" + prop_name;
    AttributeMapF::const_iterator itr = m_attributesF.find(attr_name);
    if (itr == m_attributesF.end()) {
        m_attributesF[attr_name] = std::vector<Float>();
        if(elem_name == "vertex") {
            m_attributesF[attr_name].reserve(m_vertices.size() * size);
            m_vertexAttributeNames.push_back(attr_name);
            m_vertexAttributeSizes.push_back(size);
        }
        else if(elem_name == "face") {
            m_attributesF[attr_name].reserve(m_faces.size() * size);
            m_facetAttributeNames.push_back(attr_name);
            m_facetAttributeSizes.push_back(size);
        }
    } else {
        std::stringstream err_msg;
        err_msg << "Duplicated property name: " << prop_name << std::endl;
        err_msg << "PyMesh requires unique custom property names";
        throw IOError(err_msg.str());
    }
}

void EOBJParser::add_int_attribute(const std::string& elem_name,
                                     const std::string& prop_name, size_t size) {


    std::string attr_name = elem_name + "_" + prop_name;
    AttributeMapI::const_iterator itr = m_attributesI.find(attr_name);
    if (itr == m_attributesI.end()) {
        m_attributesI[attr_name] = std::vector<int>();
        if(elem_name == "vertex") {
            m_attributesI[attr_name].reserve(m_vertices.size() * size);
            m_vertexAttributeNames.push_back(attr_name);
            m_vertexAttributeSizes.push_back(size);
        }
        else if(elem_name == "face") {
            m_attributesI[attr_name].reserve(m_faces.size() * size);
            m_facetAttributeNames.push_back(attr_name);
            m_facetAttributeSizes.push_back(size);
        }
    } else {
        std::stringstream err_msg;
        err_msg << "Duplicated property name: " << prop_name << std::endl;
        err_msg << "PyMesh requires unique custom property names";
        throw IOError(err_msg.str());
    }
}


bool EOBJParser::parse_vertex_line(char* line) {
    assert(line[0] == 'v');
    switch (line[1]) {
        case ' ':
        case '\t':
            return parse_vertex_coordinate(line);
        case 't':
            return parse_vertex_texture(line);
        case 'n':
            return parse_vertex_normal(line);
        case 'p':
            return parse_vertex_parameter(line);
        case 'c':
            // Unofficial custom line.  Ignore.
            return true;
        default:
            throw IOError("Invalid vertex line: " + std::string(line));
    }
}

bool EOBJParser::parse_vertex_coordinate(char* line) {
    char header[8];
    Float data[4];
    size_t n = sscanf(line, "%s %lf %lf %lf %lf", header,
            &data[0], &data[1], &data[2], &data[3]);
    if (n < 3) return false;
    assert(strcmp(header, "v") == 0);

    // Check to handle homogeneous coordinates.
    if (n == 5) {
        data[0] /= data[3];
        data[1] /= data[3];
        data[2] /= data[3];
        n -= 1;
    }
    if (m_dim == 0) { m_dim = n-1; }
    else if (m_dim != n-1) { return false; }

    Eigen::Map<VectorF> coord(data, m_dim);
    m_vertices.push_back(coord);
    return true;
}

bool EOBJParser::parse_vertex_normal(char* line) {
    char header[8];
    Float data[3];
    size_t n = sscanf(line, "%s %lf %lf %lf", header,
            &data[0], &data[1], &data[2]);
    if (n < 3) return false;
    assert(strcmp(header, "vn") == 0);
    m_corner_normals.emplace_back(Eigen::Map<VectorF>(data, n-1));
    return true;
}

bool EOBJParser::parse_vertex_texture(char* line) {
    char header[8];
    Float data[3];
    size_t n = sscanf(line, "%s %lf %lf %lf", header,
            &data[0], &data[1], &data[2]);
    if (n < 3) return false;
    assert(strcmp(header, "vt") == 0);
    m_corner_textures.emplace_back(Eigen::Map<VectorF>(data, n-1));
    return true;
}

bool EOBJParser::parse_vertex_parameter(char* line) {
    char header[8];
    Float data[3];
    size_t n = sscanf(line, "%s %lf %lf %lf", header,
            &data[0], &data[1], &data[2]);
    if (n < 3) return false;
    assert(strcmp(header, "vp") == 0);
    m_parameters.emplace_back(Eigen::Map<VectorF>(data, n-1));
    return true;
}

bool EOBJParser::parse_face_line(char* line) {
    const char WHITE_SPACE[] = " \t\n\r";
    constexpr int INVALID = std::numeric_limits<int>::max();
    char* field = strtok(line, WHITE_SPACE);
    assert(field != NULL);

    // Ignore header "f"
    field = strtok(NULL, WHITE_SPACE);

    // Extract vertex idx
    std::vector<size_t> idx;
    std::vector<size_t> t_idx;
    std::vector<size_t> n_idx;
    int v_idx, vt_idx, vn_idx;
    while (field != NULL) {
        // Note each vertex field could be in any of the following formats:
        // v_idx  or  v_idx/vt_idx  or  v_idx/vt_idx/vn_idx or v_idx//vn_idx
        v_idx = vt_idx = vn_idx = INVALID;
        v_idx = atoi(field);
        char* loc = strchr(field, '/');
        if (loc != NULL) {
            loc++;
            vt_idx = atoi(loc);
            loc = strchr(loc, '/');
            if (loc != NULL) {
                loc++;
                vn_idx = atoi(loc);
            }
        }

        if (v_idx == INVALID) return false;

        // Negative index means relative index from the vertices read so
        // far.  -1 refers to the last vertex read in.
        if (v_idx < 0) {
            v_idx = m_vertices.size() + v_idx + 1;
        }
        if (vt_idx < 0) {
            vt_idx = m_corner_textures.size() + vt_idx + 1;
        }
        if (vn_idx < 0) {
            vn_idx = m_corner_normals.size() + vn_idx + 1;
        }
        assert(v_idx > 0);
        assert(vt_idx >= 0);
        assert(vn_idx >= 0);
        idx.push_back(v_idx-1); // OBJ has index starting from 1
        if (vt_idx != INVALID) t_idx.push_back(vt_idx-1);
        else t_idx.push_back(INVALID);
        if (vn_idx != INVALID) n_idx.push_back(vn_idx-1);
        else n_idx.push_back(INVALID);

        // Get next token
        field = strtok(NULL, WHITE_SPACE);
    }

    const size_t num_idx_parsed = idx.size();
    if (num_idx_parsed == 3) {
        m_tris.push_back(Vector3I(idx[0], idx[1], idx[2]));
        if (!t_idx.empty())
            m_tri_textures.push_back(Vector3I(t_idx[0], t_idx[1], t_idx[2]));
        if (!n_idx.empty())
            m_tri_normals.push_back(Vector3I(n_idx[0], n_idx[1], n_idx[2]));
    } else if (num_idx_parsed == 4) {
        m_quads.push_back(Vector4I(idx[0], idx[1], idx[2], idx[3]));
        if (!t_idx.empty())
            m_quad_textures.push_back(
                    Vector4I(t_idx[0], t_idx[1], t_idx[2], t_idx[3]));
        if (!n_idx.empty())
            m_quad_normals.push_back(
                    Vector4I(n_idx[0], n_idx[1], n_idx[2], n_idx[3]));
    } else {
        // N-gon detected, assuming it is convex and break it into triangles.
        std::cerr << num_idx_parsed << "-gon detected, converting to triangles"
            << std::endl;
        const auto tris = earclip(idx);
        for (const auto& t : tris) {
            m_tris.emplace_back(Vector3I(idx[t[0]], idx[t[1]], idx[t[2]]));
        }
        if (!t_idx.empty()) {
            assert(t_idx.size() == idx.size());
            for (const auto& t : tris) {
                m_tri_textures.push_back(
                        Vector3I(t_idx[t[0]], t_idx[t[1]], t_idx[t[2]]));
            }
        }
        if (!n_idx.empty()) {
            assert(n_idx.size() == idx.size());
            for (const auto& t : tris) {
                m_tri_normals.push_back(
                        Vector3I(n_idx[t[0]], n_idx[t[1]], n_idx[t[2]]));
            }
        }
    }
    return true;
}

bool EOBJParser::parse_comment_line(char* line) {
    char attr_name[256];
    char localization[8];
    char attr_type[32];
    size_t n = sscanf(line, "# attribute %s %s %s", attr_name, localization, attr_type);
    if (n >= 3)
        return parse_attribute_declaration_line(attr_name, localization, attr_type);

    char localization_short[1];
    unsigned int index;

    n = sscanf(line, "# attrs %s %u", localization_short, &index);

    if (n >= 2)
        return parse_attribute_values_line(line);

    return false;
}

bool EOBJParser::parse_attribute_declaration_line(std::string attr_name, std::string localization, std::string attr_type)
{
    if(localization != "vertex" && localization != "facet")
    {
        std::cerr << "Unsupported localization: " << localization << std::endl;
        return true;
    }

    bool isInt = false;
    size_t attr_size = 1;

    if(attr_type == "real" || attr_type == "float" || attr_type == "double"){}
    else if(attr_type == "integer" || attr_type == "boolean" || attr_type == "int" || attr_type == "bool")
        isInt = true;
    else if(attr_type == "vec2")
        attr_size = 2;
    else if(attr_type == "vec3")
        attr_size = 3;
    else if(attr_type == "vec4" || attr_type == "Color")
        attr_size = 4;
    //TODO: support vecd, mat2, mat3, mat4
    else
    {
        std::cerr << "Unsupported attribute type: " << attr_type << std::endl;
        return false;
    }

    if(localization == "vertex")
    {
        if(isInt)
            add_int_attribute("vertex", attr_name, attr_size);
        else
            add_float_attribute("vertex", attr_name, attr_size);
    }
    else if(localization == "facet")
    {
        if(isInt)
            add_int_attribute("face", attr_name, attr_size);
        else
            add_float_attribute("face", attr_name, attr_size);
    }

    std::cout << "Found attribute '" << attr_name << "' on element '" << localization
     << "' of type '" << attr_type << "' with length " << attr_size << std::endl;

    return true;
}

bool EOBJParser::parse_attribute_values_line(char* line)
{
    std::istringstream strin(line);

    std::string hashChar, attrs, localization_short;
    size_t elem_index;

    strin >> hashChar >> attrs >> localization_short >> elem_index;

    if(strin.fail() || hashChar != "#" || attrs != "attrs" ||
            (localization_short != "v" && localization_short != "f" && localization_short != "h")) {
        std::cerr << "Invalid attribute values line in EOBJ file!" << std::endl;
        std::cerr << "hashChar = '" << hashChar <<  "'\nattrs = '" << attrs
        << "'\nlocalization_short = '" << localization_short << "'\nelem_index = " << elem_index << std::endl;
        return false;
    }
    if(localization_short == "h") //halfedge attribute is not supported
        return true;

    auto& m_attributeNames = localization_short == "v" ? m_vertexAttributeNames : m_facetAttributeNames;
    auto& m_attributeSizes = localization_short == "v" ? m_vertexAttributeSizes : m_facetAttributeSizes;

    for(size_t i = 0; i < m_attributeNames.size(); i++)
    {
        const std::string& attr_name = m_attributeNames[i];
        const size_t& attr_size = m_attributeSizes[i];
        bool isInt = m_attributesF.find(attr_name) == m_attributesF.end();

        for(size_t elem_i = 0; elem_i < attr_size; elem_i++)
        {
            if(!isInt)
            {
                Float value;
                strin >> value;
                if(strin.fail()) {
                    std::cerr << "Invalid attribute values line in EOBJ file!" << std::endl;
                    std::cerr << "Expected " << attr_size << " floats for attribute '" << attr_name
                    << "' but got " << elem_i << " instead" << std::endl;
                    return false;
                }
                m_attributesF[attr_name].push_back(value);
                //std::cout << "Scanned " << (elem_i+1) << " of " << attr_size << " floats for attribute '"
                // << attr_name << "'" << std::endl;
            }
            else
            {
                int value;
                strin >> value;
                if(strin.fail()) {
                    std::cerr << "Invalid attribute values line in EOBJ file!" << std::endl;
                    std::cerr << "Expected " << attr_size << " ints for attribute '" << attr_name
                              << "' but got " << elem_i << " instead" << std::endl;
                    return false;
                }
                m_attributesI[attr_name].push_back(value);
            }
        }
    }

    return true;
}

EOBJParser::FaceList EOBJParser::earclip(const std::vector<size_t>& idx) {
    // This method implements the naive ear clipping algorithm with complexity
    // O(n^2).  It may be slow for large n.
    assert(idx.size() > 3);
    using List = std::list<size_t>;
    using Iterator = List::iterator;
    FaceList tris;
    List active_idx;
    const size_t num_idx = idx.size();
    for (size_t i=0; i<num_idx; i++) {
        active_idx.push_back(i);
    }

    auto cyclic_next = [&active_idx](Iterator itr) {
        itr++;
        if (itr == active_idx.end()) itr = active_idx.begin();
        return itr;
    };
    auto cyclic_prev = [&active_idx](Iterator itr) {
        if (itr == active_idx.begin()) {
            itr = active_idx.end();
        }
        itr--;
        return itr;
    };
    auto estimate_normal = [this, &idx]() {
        const size_t num_idx = idx.size();
        assert(num_idx > 0);
        Vector3F n(0.0, 0.0, 0.0);
        Vector3F seed;
        seed.segment(0, m_dim) = m_vertices[idx[0]];
        for (size_t i=0; i<num_idx-1; i++) {
            Vector3F vi,vj;
            vi.segment(0, m_dim) = m_vertices[idx[i]];
            vj.segment(0, m_dim) = m_vertices[idx[i+1]];
            n += (vi - seed).cross(vj - seed);
        }
        n.normalize();
        return n;
    };
    auto can_clip = [this, &idx, &cyclic_prev, &cyclic_next](const Iterator& itr, const Vector3F& normal) {
        const auto curr = itr;
        const auto next = cyclic_next(itr);
        const auto prev = cyclic_prev(itr);
        const size_t i = idx[*prev];
        const size_t j = idx[*curr];
        const size_t k = idx[*next];
        Vector3F vi,vj,vk;
        vi.segment(0, m_dim) = m_vertices[i];
        vj.segment(0, m_dim) = m_vertices[j];
        vk.segment(0, m_dim) = m_vertices[k];
        const Vector3F nj = (vk-vj).cross(vi-vj);
        if (nj.norm() <= 0.0) return false; // Degenerate ear.
        if (nj.dot(normal) <= 0.0) return false; // Concave face.
        for (Iterator itr = cyclic_next(next); itr != prev; itr=cyclic_next(itr)) {
            const size_t l = idx[*itr];
            const size_t m = idx[*cyclic_next(itr)];
            Vector3F vl,vm;
            vl.segment(0, m_dim) = m_vertices[l];
            vm.segment(0, m_dim) = m_vertices[m];
            if (l == k) {
                const Vector3F n_mki = (vk-vm).cross(vi-vm);
                const Vector3F n_mij = (vi-vm).cross(vj-vm);
                const Vector3F n_mjk = (vj-vm).cross(vk-vm);
                if (n_mki.dot(nj) > 0 && n_mij.dot(nj) > 0 && n_mjk.dot(nj) > 0) {
                    return false;
                }
            } else if (m == i) {
                const Vector3F n_lki = (vk-vl).cross(vi-vl);
                const Vector3F n_lij = (vi-vl).cross(vj-vl);
                const Vector3F n_ljk = (vj-vl).cross(vk-vl);
                if (n_lki.dot(nj) > 0 && n_lij.dot(nj) > 0 && n_ljk.dot(nj) > 0) {
                    return false;
                }
            } else {
                const Vector3F nl = (vk-vl).cross(vi-vl);
                const Vector3F nm = (vk-vm).cross(vi-vm);
                const Vector3F ni = (vl-vi).cross(vm-vi);
                const Vector3F nk = (vl-vk).cross(vm-vk);
                const bool cross_ik = nm.dot(nl) < 0.0;
                const bool cross_lm = ni.dot(nk) < 0.0;

                if (cross_ik && cross_lm) return false;
            }
        }
        return true;
    };

    auto clip = [&tris, &active_idx, &cyclic_next, &cyclic_prev](const Iterator& itr) {
        const auto curr = itr;
        const auto next = cyclic_next(itr);
        const auto prev = cyclic_prev(itr);
        tris.emplace_back(Vector3I(*prev, *curr, *next));
        active_idx.erase(curr);
    };

    const Vector3F normal = estimate_normal();
    while (active_idx.size() > 3) {
        const size_t n = active_idx.size();
        for (Iterator itr=active_idx.begin(); itr!=active_idx.end(); itr++) {
            const auto curr = itr;
            if (can_clip(curr, normal)) {
                clip(curr);
                break;
            }
        }
        if (active_idx.size() == n) {
            // Cannot find an "ear" to clip.  This means the polygon is either
            // degenerate or is nonplanar and complex.  Fall back to just
            // clipping the first ear. :(
            clip(active_idx.begin());
        }
    }
    assert(active_idx.size() == 3);
    clip(active_idx.begin());
    return tris;
}

void EOBJParser::unify_faces() {
    if (m_tris.size() > 0 && m_quads.size() == 0) {
        m_faces = std::move(m_tris);
        m_textures = std::move(m_tri_textures);
        m_normals = std::move(m_tri_normals);
        m_vertex_per_face = 3;
    } else if (m_tris.size() == 0 && m_quads.size() > 0) {
        m_faces = std::move(m_quads);
        m_textures = std::move(m_quad_textures);
        m_normals = std::move(m_quad_normals);
        m_vertex_per_face = 4;
    } else if (m_tris.size() > 0 && m_quads.size() > 0){
        std::cerr << "Mixed triangle and quads in the input file" << std::endl;
        std::cerr << "Converting quads in triangles, face order is not kept!"
            << std::endl;
        m_faces = std::move(m_tris);
        for (auto& quad : m_quads) {
            m_faces.push_back(Vector3I(quad[0], quad[1], quad[2]));
            m_faces.push_back(Vector3I(quad[0], quad[2], quad[3]));
        }
        m_textures = std::move(m_tri_textures);
        for (auto& tex : m_quad_textures) {
            m_textures.push_back(Vector3I(tex[0], tex[1], tex[2]));
            m_textures.push_back(Vector3I(tex[0], tex[2], tex[3]));
        }
        m_normals = std::move(m_tri_normals);
        for (auto& n : m_quad_normals) {
            m_normals.push_back(Vector3I(n[0], n[1], n[2]));
            m_normals.push_back(Vector3I(n[0], n[2], n[3]));
        }
        m_vertex_per_face = 3;
    }
}

void EOBJParser::finalize_textures() {
    const size_t num_faces = m_faces.size();
    if (m_textures.size() != num_faces || m_corner_textures.size() == 0)
        return;

    m_texture_dim = 2;
    bool bad_texture = false;
    const size_t num_corner_textures = m_corner_textures.size();
    for (const auto& t : m_textures) {
        if (t.size() != m_vertex_per_face) {
            std::cerr << "Texture and face type mismatch." << std::endl;
            bad_texture = true;
            break;
        }
    }

    if (bad_texture) {
        m_textures.clear();
        return;
    }

    TextureVector textures;
    const Vector2F INVALID_UV {
        std::numeric_limits<Float>::quiet_NaN(),
        std::numeric_limits<Float>::quiet_NaN()};
    for (const auto& t : m_textures) {
        for (size_t i=0; i<m_vertex_per_face; i++) {
            if (t[i] >= 0 && t[i] < num_corner_textures) {
                textures.emplace_back(m_corner_textures[t[i]]);
            } else {
                textures.emplace_back(INVALID_UV);
            }
        }
    }
    std::swap(textures, m_corner_textures);
    assert(m_corner_textures.size() == num_faces * m_vertex_per_face);
}

void EOBJParser::finalize_normals() {
    if (m_normals.size() != m_faces.size() || m_corner_normals.size() == 0)
        return;

    bool bad_normal = false;
    const size_t num_corner_normals = m_corner_normals.size();
    for (const auto& n : m_normals) {
        if (n.size() != m_vertex_per_face) {
            std::cerr << "Normal and face type mismatch." << std::endl;
            bad_normal = true;
            break;
        }
        if (n.minCoeff() < 0 || n.maxCoeff() >= num_corner_normals) {
            std::cerr << "Normalindex out of bound: <" << n.transpose()
                << "> exceeds " << num_corner_normals << "."
                << std::endl;
            bad_normal = true;
            break;
        }
    }

    if (bad_normal) {
        m_normals.clear();
        return;
    }

    NormalVector normals;
    for (const auto& n : m_normals) {
        for (size_t i=0; i<m_vertex_per_face; i++) {
            normals.emplace_back(m_corner_normals[n[i]]);
        }
    }
    std::swap(normals, m_corner_normals);
}

void EOBJParser::finalize_parameters() {
    if (m_parameters.empty()) return;
    if (m_parameters.size() != m_vertices.size()) {
        std::cerr << "Mismatch between vertex and vertex parameters."
            << std::endl;
        m_parameters.clear();
    }
    m_parameter_dim = m_parameters.front().size();
    for (const auto& n : m_parameters) {
        if (n.size() != m_parameter_dim) {
            std::cerr << "Inconsistent parameter dimension" << std::endl;
            m_parameter_dim = 0;
            break;
        }
    }
    if (m_parameter_dim == 0) {
        m_parameters.clear();
    }
}
