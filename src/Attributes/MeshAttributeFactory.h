/* This file is part of PyMesh. Copyright (c) 2015 by Qingnan Zhou */
#pragma once
#include <string>

#include "MeshAttribute.h"

namespace PyMesh {

class MeshAttributeFactory {
    public:

        /**
         * create function creates the attribute of the specified name and type. If the
         * name specifies a predefined attribute, e.g. vertex_normal, the
         * attribute is created, and its run(mesh) function will fill in the
         * corresponding values. Otherwise, it creates a generic attribute, and
         * the user is responsible for resizing and filling in values.
         *
         * See MeshAttributeFactory.cpp for a list of predefined attributes.
         */
        static MeshAttributeF::Ptr create_float(const std::string& name);
        static MeshAttributeI::Ptr create_int(const std::string& name);
};

}
