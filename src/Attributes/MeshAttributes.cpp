/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#include "MeshAttributes.h"

#include <sstream>

#include <Core/EigenTypedef.h>
#include <Core/Exception.h>

#include "MeshAttribute.h"
#include "MeshAttributeFactory.h"

using namespace PyMesh;

bool PyMesh::MeshAttributes::has_attribute(const std::string& name) {
    AttributeMapF::iterator itr = m_attributesF.find(name);
    AttributeMapI::iterator itrI = m_attributesI.find(name);
    return (itr != m_attributesF.end() || itrI != m_attributesI.end());
}

bool PyMesh::MeshAttributes::has_float_attribute(const std::string& name) {
    AttributeMapF::iterator itr = m_attributesF.find(name);
    return (itr != m_attributesF.end());
}

bool PyMesh::MeshAttributes::has_int_attribute(const std::string& name) {
    AttributeMapI::iterator itrI = m_attributesI.find(name);
    return (itrI != m_attributesI.end());
}

void PyMesh::MeshAttributes::add_empty_float_attribute(const std::string& name) {
    MeshAttributeF::Ptr attr = MeshAttributeFactory::create_float(name);
    m_attributesF.insert(AttributeMapEntryF(name, attr));
}

void PyMesh::MeshAttributes::add_empty_int_attribute(const std::string& name) {
    MeshAttributeI::Ptr attr = MeshAttributeFactory::create_int(name);
    m_attributesI.insert(AttributeMapEntryI(name, attr));
}

void PyMesh::MeshAttributes::add_float_attribute(const std::string& name, Mesh& mesh) {
    MeshAttributeF::Ptr attr = MeshAttributeFactory::create_float(name);
    attr->compute_from_mesh(mesh);
    m_attributesF.insert(AttributeMapEntryF(name, attr));
}

void PyMesh::MeshAttributes::add_int_attribute(const std::string& name, Mesh& mesh) {
    MeshAttributeI::Ptr attr = MeshAttributeFactory::create_int(name);
    attr->compute_from_mesh(mesh);
    m_attributesI.insert(AttributeMapEntryI(name, attr));
}

void PyMesh::MeshAttributes::remove_attribute(const std::string& name) {
    AttributeMapF::iterator itrF = m_attributesF.find(name);
    AttributeMapI::iterator itrI = m_attributesI.find(name);

    if(itrF != m_attributesF.end())
        m_attributesF.erase(itrF);
    if(itrI != m_attributesI.end())
        m_attributesI.erase(itrI);
    if (itrF == m_attributesF.end() && itrI == m_attributesI.end())
    {
        std::stringstream err_msg;
        err_msg << "Attribute \"" << name << "\" does not exist.";
        throw RuntimeError(err_msg.str());
    }
}

VectorF& PyMesh::MeshAttributes::get_float_attribute(const std::string& name) {
    AttributeMapF::iterator itr = m_attributesF.find(name);
    if (itr == m_attributesF.end()) {
        std::stringstream err_msg;
        err_msg << "Float attribute \"" << name << "\" does not exist.";
        throw RuntimeError(err_msg.str());
    }
    return itr->second->get_values();
}

VectorI& PyMesh::MeshAttributes::get_int_attribute(const std::string& name) {
    AttributeMapI::iterator itr = m_attributesI.find(name);
    if (itr == m_attributesI.end()) {
        std::stringstream err_msg;
        err_msg << "Int attribute \"" << name << "\" does not exist.";
        throw RuntimeError(err_msg.str());
    }
    return itr->second->get_values();
}

void PyMesh::MeshAttributes::set_attribute(const std::string& name, VectorF& value) {
    MeshAttributeF::Ptr attr;
    AttributeMapF::iterator itr = m_attributesF.find(name);
    if (itr == m_attributesF.end()) {
        std::stringstream err_msg;
        err_msg << "Attribute \"" << name << "\" does not exist.";
        throw RuntimeError(err_msg.str());
    } else {
        attr = itr->second;
    }
    attr->set_values(value);
}

void PyMesh::MeshAttributes::set_attribute(const std::string& name, VectorI& value) {
    MeshAttributeI::Ptr attr;
    AttributeMapI::iterator itr = m_attributesI.find(name);
    if (itr == m_attributesI.end()) {
        std::stringstream err_msg;
        err_msg << "Attribute \"" << name << "\" does not exist.";
        throw RuntimeError(err_msg.str());
    } else {
        attr = itr->second;
    }
    attr->set_values(value);
}

MeshAttributes::AttributeNames PyMesh::MeshAttributes::get_attribute_names() const {
    AttributeNames names;
    for (AttributeMapF::const_iterator itr = m_attributesF.begin();
            itr != m_attributesF.end(); itr++) {
        names.push_back(itr->first);
    }
    for (AttributeMapI::const_iterator itr = m_attributesI.begin();
         itr != m_attributesI.end(); itr++) {
        names.push_back(itr->first);
    }
    return names;
}

MeshAttributes::AttributeNames PyMesh::MeshAttributes::get_float_attribute_names() const {
    AttributeNames names;
    for (AttributeMapF::const_iterator itr = m_attributesF.begin();
         itr != m_attributesF.end(); itr++) {
        names.push_back(itr->first);
    }
    return names;
}


MeshAttributes::AttributeNames PyMesh::MeshAttributes::get_int_attribute_names() const {
    AttributeNames names;
    for (AttributeMapI::const_iterator itr = m_attributesI.begin();
         itr != m_attributesI.end(); itr++) {
        names.push_back(itr->first);
    }
    return names;
}