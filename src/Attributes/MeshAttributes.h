/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once
#include <string>
#include <vector>
#include <map>

#include <Core/EigenTypedef.h>

#include "MeshAttribute.h"

namespace PyMesh {
class Mesh;

class MeshAttributes {
    public:
        virtual ~MeshAttributes() {}

    public:
        typedef std::vector<std::string> AttributeNames;

    public:
        // Simple API
        virtual bool has_attribute(const std::string& name);
        virtual bool has_float_attribute(const std::string& name);
        virtual bool has_int_attribute(const std::string& name);
        virtual void add_empty_float_attribute(const std::string& name);
        virtual void add_empty_int_attribute(const std::string& name);
        virtual void add_float_attribute(const std::string& name, Mesh& mesh);
        virtual void add_int_attribute(const std::string& name, Mesh& mesh);
        virtual void remove_attribute(const std::string& name);
        virtual VectorF& get_float_attribute(const std::string& name);
        virtual VectorI& get_int_attribute(const std::string& name);
        virtual void set_attribute(const std::string& name, VectorF& value);
        virtual void set_attribute(const std::string& name, VectorI& value);
        virtual AttributeNames get_attribute_names() const;
        virtual AttributeNames get_float_attribute_names() const;
        virtual AttributeNames get_int_attribute_names() const;

    protected:
        typedef std::map<std::string, MeshAttribute<VectorF>::Ptr> AttributeMapF;
        typedef std::pair<std::string, MeshAttribute<VectorF>::Ptr> AttributeMapEntryF;
        typedef std::map<std::string, MeshAttribute<VectorI>::Ptr> AttributeMapI;
        typedef std::pair<std::string, MeshAttribute<VectorI>::Ptr> AttributeMapEntryI;
        AttributeMapF m_attributesF;
        AttributeMapI m_attributesI;
};
}
