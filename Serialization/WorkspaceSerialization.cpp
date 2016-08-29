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

#include "WorkspaceSerialization.h"

GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF
#include <yaml-cpp/yaml.h>
GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON

#include "Serialization/KnobSerialization.h"

SERIALIZATION_NAMESPACE_ENTER

void
ViewportData::encode(YAML::Emitter& em) const
{
    em << YAML::Flow << YAML::BeginSeq;
    em << left << bottom << zoomFactor << par << zoomOrPanSinceLastFit;
    em << YAML::EndSeq;
}

void
ViewportData::decode(const YAML::Node& node)
{
    if (!node.IsSequence() || node.size() != 5) {
        throw YAML::InvalidNode();
    }
    left = node[0].as<double>();
    bottom = node[1].as<double>();
    zoomFactor = node[2].as<double>();
    par = node[3].as<double>();
    zoomOrPanSinceLastFit = node[4].as<bool>();
}

void
PythonPanelSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::BeginMap;
    em << YAML::Key << "ScriptName" << YAML::Value << name;
    em << YAML::Key << "Func" << YAML::Value << pythonFunction;
    em << YAML::Key << "Data" << YAML::Value << userData;
    if (!knobs.empty()) {
        em << YAML::Key << "Params" << YAML::Value << YAML::BeginSeq;
        for (KnobSerializationList::const_iterator it = knobs.begin(); it!=knobs.end(); ++it) {
            (*it)->encode(em);
        }
        em << YAML::EndSeq;
    }
    em << YAML::EndMap;
}

void
PythonPanelSerialization::decode(const YAML::Node& node)
{
    if (!node.IsMap()) {
        throw YAML::InvalidNode();
    }
    name = node["ScriptName"].as<std::string>();
    pythonFunction = node["Func"].as<std::string>();
    userData = node["Data"].as<std::string>();
    if (node["Params"]) {
        YAML::Node paramsNode = node["Params"];
        for (YAML::const_iterator it = paramsNode.begin(); it!=paramsNode.end(); ++it) {
            KnobSerializationPtr s(new KnobSerialization);
            s->decode(it->second);
            knobs.push_back(s);
        }
    }

}

void
TabWidgetSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::Flow << YAML::BeginMap;
    em << YAML::Key << "ScriptName" << YAML::Value << scriptName;
    em << YAML::Key << "Current" << YAML::Value << currentIndex;
    if (isAnchor) {
        em << YAML::Key << "Anchor" << YAML::Value << isAnchor;
    }
    if (!tabs.empty()) {
        em << YAML::Key << "Tabs" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (std::list<std::string>::const_iterator it = tabs.begin(); it!=tabs.end(); ++it) {
            em << *it;
        }
        em << YAML::EndSeq;
    }
    em << YAML::EndMap;
}

void
TabWidgetSerialization::decode(const YAML::Node& node)
{
    if (!node.IsMap()) {
        throw YAML::InvalidNode();
    }
    scriptName = node["ScriptName"].as<std::string>();
    currentIndex = node["Current"].as<int>();
    if (node["Anchor"]) {
        isAnchor = node["Anchor"].as<bool>();
    } else {
        isAnchor = false;
    }
    if (node["Tabs"]) {
        YAML::Node tabs = node["Tabs"];
        for (YAML::const_iterator it = tabs.begin(); it!=tabs.end(); ++it) {
            tabs.push_back(it->second.as<std::string>());
        }
    }
}

void
WidgetSplitterSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::BeginMap;
    em << YAML::Key << "Layout" << YAML::Value << YAML::Flow << YAML::BeginSeq << orientation << leftChildSize << rightChildSize << YAML::EndSeq;
    em << YAML::Key << "LeftType" << YAML::Value << leftChild.type;
    em << YAML::Key << "LeftChild" << YAML::Value;
    if (leftChild.childIsSplitter) {
        leftChild.childIsSplitter->encode(em);
    } else if (leftChild.childIsTabWidget) {
        leftChild.childIsTabWidget->encode(em);
    }
    em << YAML::Key << "RightType" << YAML::Value << rightChild.type;
    em << YAML::Key << "RightChild" << YAML::Value;
    if (rightChild.childIsSplitter) {
        rightChild.childIsSplitter->encode(em);
    } else if (rightChild.childIsTabWidget) {
        rightChild.childIsTabWidget->encode(em);
    }
    em << YAML::EndMap;
}

void
WidgetSplitterSerialization::decode(const YAML::Node& node)
{
    if (!node.IsMap()) {
        throw YAML::InvalidNode();
    }
    {
        YAML::Node n = node["Layout"];
        if (n.size() != 3) {
            throw YAML::InvalidNode();
        }
        orientation = n[0].as<std::string>();
        leftChildSize = n[1].as<int>();
        rightChildSize = n[2].as<int>();
    }
    leftChild.type = node["LeftType"].as<std::string>();
    if (leftChild.type == kSplitterChildTypeSplitter) {
        leftChild.childIsSplitter.reset(new WidgetSplitterSerialization);
        leftChild.childIsSplitter->decode(node["LeftChild"]);
    } else if (leftChild.type == kSplitterChildTypeTabWidget) {
        leftChild.childIsTabWidget.reset(new TabWidgetSerialization);
        leftChild.childIsTabWidget->decode(node["LeftChild"]);
    }
    rightChild.type = node["RightType"].as<std::string>();
    if (rightChild.type == kSplitterChildTypeSplitter) {
        rightChild.childIsSplitter.reset(new WidgetSplitterSerialization);
        rightChild.childIsSplitter->decode(node["RightChild"]);
    } else if (rightChild.type == kSplitterChildTypeTabWidget) {
        rightChild.childIsTabWidget.reset(new TabWidgetSerialization);
        rightChild.childIsTabWidget->decode(node["RigthChild"]);
    }

}

void
WindowSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::BeginMap;
    em << YAML::Key << "Pos" << YAML::Value << YAML::Flow << YAML::BeginSeq << windowPosition[0] << windowPosition[1] << YAML::EndSeq;
    em << YAML::Key << "Size" << YAML::Value << YAML::Flow << YAML::BeginSeq << windowSize[0] << windowSize[1] << YAML::EndSeq;
    em << YAML::Key << "ChildType" << YAML::Value << childType;
    em << YAML::Key << "Child" << YAML::Value;
    if (isChildSplitter) {
        isChildSplitter->encode(em);
    } else if (isChildTabWidget) {
        isChildTabWidget->encode(em);
    } else if (!isChildSettingsPanel.empty()) {
        em << isChildSettingsPanel;
    }
    em << YAML::EndMap;
}

void
WindowSerialization::decode(const YAML::Node& node)
{
    {
        YAML::Node n = node["Pos"];
        if (!n.IsSequence() || n.size() != 2) {
            throw YAML::InvalidNode();
        }
        windowPosition[0] = n[0].as<int>();
        windowPosition[1] = n[1].as<int>();

    }
    {
        YAML::Node n = node["Size"];
        if (!n.IsSequence() || n.size() != 2) {
            throw YAML::InvalidNode();
        }
        windowSize[0] = n[0].as<int>();
        windowSize[1] = n[1].as<int>();

    }
    childType = node["ChildType"].as<std::string>();
    YAML::Node childNode = node["Child"];
    if (childType == kSplitterChildTypeSplitter) {
        isChildSplitter.reset(new WidgetSplitterSerialization);
        isChildSplitter->decode(childNode);
    } else if (childType == kSplitterChildTypeTabWidget) {
        isChildTabWidget.reset(new TabWidgetSerialization);
        isChildTabWidget->decode(childNode);
    } else if (childType == kSplitterChildTypeSettingsPanel) {
        isChildSettingsPanel = childNode.as<std::string>();
    }
}

void
WorkspaceSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::BeginMap;
    if (!_histograms.empty()) {
        em << YAML::Key << "Histograms" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (std::list<std::string>::const_iterator it = _histograms.begin(); it!=_histograms.end(); ++it) {
            em << *it;
        }
        em << YAML::EndSeq;
    }

    if (!_pythonPanels.empty()) {
        em << YAML::Key << "PyPanels" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (std::list<PythonPanelSerialization>::const_iterator it = _pythonPanels.begin(); it!=_pythonPanels.end(); ++it) {
            it->encode(em);
        }
        em << YAML::EndSeq;
    }
    if (_mainWindowSerialization) {
        em << YAML::Key << "MainWindow" << YAML::Value;
        _mainWindowSerialization->encode(em);
    }
    if (!_floatingWindowsSerialization.empty()) {
        em << YAML::Key << "FloatingWindows" << YAML::Value << YAML::BeginSeq;
        for (std::list<boost::shared_ptr<WindowSerialization> >::const_iterator it = _floatingWindowsSerialization.begin(); it!=_floatingWindowsSerialization.end(); ++it) {
            (*it)->encode(em);
        }
        em << YAML::EndSeq;
    }

    em << YAML::EndMap;
}

void
WorkspaceSerialization::decode(const YAML::Node& node)
{
    if (!node.IsMap()) {
        throw YAML::InvalidNode();
    }
    if (node["Histograms"]) {
        YAML::Node n = node["Histograms"];
        for (YAML::const_iterator it = n.begin(); it!=n.end(); ++it) {
            _histograms.push_back(it->second.as<std::string>());
        }
    }
    if (node["PyPanels"]) {
        YAML::Node n = node["PyPanels"];
        for (YAML::const_iterator it = n.begin(); it!=n.end(); ++it) {
            PythonPanelSerialization p;
            p.decode(it->second);
            _pythonPanels.push_back(p);
        }
    }
    if (node["MainWindow"]) {
        _mainWindowSerialization.reset(new WindowSerialization);
        _mainWindowSerialization->decode(node["MainWindow"]);
    }
    if (node["FloatingWindows"]) {
        YAML::Node n = node["FloatingWindows"];
        for (YAML::const_iterator it = n.begin(); it!=n.end(); ++it) {
            boost::shared_ptr<WindowSerialization> p(new WindowSerialization);
            p->decode(it->second);
            _floatingWindowsSerialization.push_back(p);
        }
    }
}

SERIALIZATION_NAMESPACE_EXIT



