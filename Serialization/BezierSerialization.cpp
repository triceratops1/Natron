/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#include "BezierSerialization.h"

GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF
#include <yaml-cpp/yaml.h>
GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON
#include "Serialization/BezierCPSerialization.h"

SERIALIZATION_NAMESPACE_ENTER

void
BezierSerialization::encode(YAML::Emitter& em) const
{
    RotoDrawableItemSerialization::encode(em);
    em << YAML::BeginMap;
    em << YAML::Key << "Shape" << YAML::Value;
    em << YAML::BeginSeq;
    for (std::list< BezierCPSerialization >::const_iterator it = _controlPoints.begin(); it != _controlPoints.end(); ++it) {
        it->encode(em);
    }
    em << YAML::EndSeq;
    em << YAML::Key << "Feather" << YAML::Value;
    em << YAML::BeginSeq;
    for (std::list< BezierCPSerialization >::const_iterator it = _featherPoints.begin(); it != _featherPoints.end(); ++it) {
        it->encode(em);
    }
    em << YAML::EndSeq;
    em << YAML::Key << "CanClose" << YAML::Value << !_isOpenBezier;
    if (!_isOpenBezier) {
        em << YAML::Key << "Closed" << YAML::Value << _closed;
    }
    em << YAML::EndMap;
}

void
BezierSerialization::decode(const YAML::Node& node)
{
    RotoDrawableItemSerialization::decode(node);
    for (YAML::const_iterator it = node.begin(); it!=node.end(); ++it) {

        std::string key = it->first.as<std::string>();
        if (key == "Shape") {
            for (YAML::const_iterator it2 = it->second.begin(); it2!=it->second.end(); ++it2) {
                BezierCPSerialization s;
                s.decode(it2->second);
                _controlPoints.push_back(s);
            }
        } else if (key == "Feather") {
            for (YAML::const_iterator it2 = it->second.begin(); it2!=it->second.end(); ++it2) {
                BezierCPSerialization s;
                s.decode(it2->second);
                _featherPoints.push_back(s);
            }
        } else if (key == "Closed") {
            _closed = it->second.as<bool>();
        } else if (key == "CanClose") {
            _isOpenBezier = !it->second.as<bool>();
            if (_isOpenBezier) {
                _closed = false;
            }
        }

    }
}

SERIALIZATION_NAMESPACE_EXIT