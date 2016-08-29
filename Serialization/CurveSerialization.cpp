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

#include "CurveSerialization.h"

GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF
#include <yaml-cpp/yaml.h>
GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON

SERIALIZATION_NAMESPACE_ENTER;

void
CurveSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::Flow;
    em << YAML::BeginSeq;

    std::string prevInterpolation; // No valid interpolation set yet
    for (std::list<KeyFrameSerialization>::const_iterator it = keys.begin(); it!=keys.end(); ++it) {

        if (prevInterpolation.empty() ||
            prevInterpolation != it->interpolation) {
            // serialize interpolation
            em << it->interpolation;
        }
        prevInterpolation = it->interpolation;


        em << it->time;
        em << it->value;

        if (it->interpolation == kKeyframeSerializationTypeFree || it->interpolation == kKeyframeSerializationTypeBroken) {
            // When user set tangents manually, we need the right tangents.
            em << it->rightDerivative;
        }

        if (it->interpolation == kKeyframeSerializationTypeBroken) {
            // In broken mode, both tangents can be different so serialize both
            em << it->leftDerivative;
        }

    }
    em << YAML::EndSeq;
}

enum CurveDecodeStateEnum
{
    eCurveDecodeStateExpectTime,
    eCurveDecodeStateExpectValue,
    eCurveDecodeStateMayExpectInterpolation,
    eCurveDecodeStateMayExpectRightDerivative,
    eCurveDecodeStateMayExpectLeftDerivative

};

void
CurveSerialization::decode(const YAML::Node& node)
{
    if (!node.IsSequence()) {
        return;
    }
    if (node.size() == 0) {
        return;
    }
    CurveDecodeStateEnum state = eCurveDecodeStateMayExpectInterpolation;
    std::string interpolation;
    KeyFrameSerialization keyframe;
    bool pushKeyFrame = false;
    for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {

        switch (state) {
            case eCurveDecodeStateExpectTime:
                if (pushKeyFrame) {
                    keys.push_back(keyframe);

                    // Reset keyframe
                    keyframe.interpolation.clear();
                    keyframe.time = keyframe.value = keyframe.leftDerivative = keyframe.rightDerivative = 0.;

                }
                keyframe.time = it->as<double>();
                state  = eCurveDecodeStateExpectValue;
                break;
            case eCurveDecodeStateExpectValue:
                keyframe.value = it->as<double>();
                // Depending on interpolation we may expect derivatives
                if (keyframe.interpolation == kKeyframeSerializationTypeFree || keyframe.interpolation == kKeyframeSerializationTypeBroken) {
                    state = eCurveDecodeStateMayExpectRightDerivative;
                } else {
                    state = eCurveDecodeStateMayExpectInterpolation;
                }
                break;
            case eCurveDecodeStateMayExpectInterpolation:
                try {
                    keyframe.interpolation = it->as<std::string>();
                } catch (const YAML::BadConversion& /*e*/) {
                    // No interpolation, use the interpolation set previously.
                    // If interpolation is not set, set interpolation to linear
                    if (interpolation.empty()) {
                        interpolation = kKeyframeSerializationTypeLinear;
                    }
                    keyframe.interpolation = interpolation;
                }
                state = eCurveDecodeStateExpectTime;
                break;
            case eCurveDecodeStateMayExpectRightDerivative:
                keyframe.rightDerivative = it->as<double>();
                if (keyframe.interpolation == kKeyframeSerializationTypeBroken) {
                    state = eCurveDecodeStateMayExpectLeftDerivative;
                } else {
                    state = eCurveDecodeStateMayExpectInterpolation;
                }
                break;
            case eCurveDecodeStateMayExpectLeftDerivative:
                keyframe.leftDerivative = it->as<double>();
                state = eCurveDecodeStateMayExpectInterpolation;
                break;
        }
        pushKeyFrame = true;
    }
}

#ifdef NATRON_BOOST_SERIALIZATION_COMPAT
// explicit template instantiations
template void Curve::serialize<boost::archive::xml_iarchive>(boost::archive::xml_iarchive & ar,
                                                             const unsigned int file_version);
template void Curve::serialize<boost::archive::xml_oarchive>(boost::archive::xml_oarchive & ar,
                                                             const unsigned int file_version);
#endif // #ifdef NATRON_BOOST_SERIALIZATION_COMPAT

SERIALIZATION_NAMESPACE_EXIT;