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

#ifndef NATRON_ENGINE_CURVESERIALIZATION_H
#define NATRON_ENGINE_CURVESERIALIZATION_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"

#include <list>
#include <string>

#ifdef NATRON_BOOST_SERIALIZATION_COMPAT
#include "Engine/Curve.h"
#include "Engine/CurvePrivate.h"
#endif // NATRON_BOOST_SERIALIZATION_COMPAT

#include "Serialization/SerializationBase.h"

SERIALIZATION_NAMESPACE_ENTER;

#define kKeyframeSerializationTypeConstant "K"
#define kKeyframeSerializationTypeLinear "L"
#define kKeyframeSerializationTypeSmooth "S"
#define kKeyframeSerializationTypeCatmullRom "R"
#define kKeyframeSerializationTypeCubic "C"
#define kKeyframeSerializationTypeHorizontal "H"
#define kKeyframeSerializationTypeFree "F"
#define kKeyframeSerializationTypeBroken "X"

/**
 * @brief Basically just the same as a Keyframe but without all member functions and extracted to remove any dependency to Natron.
 * This class i contained into CurveSerialization
 **/
struct KeyFrameSerialization
{

    // The frame in the timeline. This is not necessarily an integer if the user applied retiming operations
    double time;

    // The value of the keyframe
    double value;

    // Letter that corresponds to one value of KeyframeTypeEnum
    // eKeyframeTypeConstant = kKeyframeSerializationTypeConstant,
    // eKeyframeTypeLinear = kKeyframeSerializationTypeLinear
    // eKeyframeTypeSmooth = kKeyframeSerializationTypeSmooth
    // eKeyframeTypeCatmullRom = kKeyframeSerializationTypeCatmullRom
    // eKeyframeTypeCubic = kKeyframeSerializationTypeCubic
    // eKeyframeTypeHorizontal = kKeyframeSerializationTypeHorizontal
    // eKeyframeTypeFree = kKeyframeSerializationTypeFree
    // eKeyframeTypeBroken = kKeyframeSerializationTypeBroken
    // eKeyframeTypeNone = empty string, meaning we do not serialize the value
    //
    // This value is only serialized if this is the first keyframe in the curve OR if the interpolation type changes
    // at this keyframe.
    std::string interpolation;

    // This is only needed in eKeyframeTypeFree mode where the user controls the interpolation of both tangents at once
    // or in eKeyframeTypeBroken mode
    double rightDerivative;

    // This is only needed in eKeyframeTypeBroken mode when user can have set different right/left tangents
    double leftDerivative;

};

#ifdef NATRON_BOOST_SERIALIZATION_COMPAT
SERIALIZATION_NAMESPACE_EXIT
/**
 * @brief Deprecated, used for backward compatibility before Natron 2.2
 **/
template<class Archive>
void
NATRON_NAMESPACE::KeyFrame::serialize(Archive & ar,
                    const unsigned int /*version*/)
{
    ar & ::boost::serialization::make_nvp("Time", _time);
    ar & ::boost::serialization::make_nvp("Value", _value);
    ar & ::boost::serialization::make_nvp("InterpolationMethod", _interpolation);
    ar & ::boost::serialization::make_nvp("LeftDerivative", _leftDerivative);
    ar & ::boost::serialization::make_nvp("RightDerivative", _rightDerivative);
}
/**
 * @brief Deprecated, used for backward compatibility before Natron 2.2
 **/
template<class Archive>
void
NATRON_NAMESPACE::Curve::serialize(Archive & ar,
                                   const unsigned int /*version*/)
{
    QMutexLocker l(&_imp->_lock);
    ar & ::boost::serialization::make_nvp("KeyFrameSet", _imp->keyFrames);
}
SERIALIZATION_NAMESPACE_ENTER
#endif // #ifdef NATRON_BOOST_SERIALIZATION_COMPAT


class CurveSerialization
: public SerializationObjectBase
{
public:

    // We don't need a set complicated data structure here because we trust that the Curve itself
    // gave us a list of keyframes with a correct ordering
    std::list<KeyFrameSerialization> keys;

    virtual void encode(YAML::Emitter& em) const OVERRIDE FINAL;

    virtual void decode(const YAML::Node& node) OVERRIDE FINAL;

};

SERIALIZATION_NAMESPACE_EXIT;

#endif // NATRON_ENGINE_CURVESERIALIZATION_H