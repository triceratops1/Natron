//  Natron
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
/*
 * Created by Alexandre GAUTHIER-FOICHAT on 6/1/2012.
 * contact: immarespond at gmail dot com
 *
 */

#include "RotoPaint.h"

#include "Engine/Image.h"
#include "Engine/Node.h"
#include "Engine/RotoContext.h"

using namespace Natron;

struct RotoPaintPrivate
{
    RotoPaintPrivate()
    {
        
    }
};

std::string
RotoPaint::getDescription() const
{
    return "RotoPaint is a vector based free-hand drawing node that helps for tasks such as rotoscoping, matting, etc...\n";
}

RotoPaint::RotoPaint(boost::shared_ptr<Natron::Node> node)
: EffectInstance(node)
{
}


RotoPaint::~RotoPaint()
{
    
}

std::string
RotoPaint::getInputLabel (int inputNb) const
{
    if (inputNb == 0) {
        return "Bg";
    } else if (inputNb == 1) {
        return "Bg1";
    } else if (inputNb == 2) {
        return "Bg2";
    } else if (inputNb == 3) {
        return "Bg3";
    }
    assert(false);
    return "";
}

void
RotoPaint::addAcceptedComponents(int /*inputNb*/,std::list<Natron::ImageComponents>* comps)
{
    comps->push_back(ImageComponents::getRGBAComponents());
    comps->push_back(ImageComponents::getRGBComponents());
    comps->push_back(ImageComponents::getXYComponents());
    comps->push_back(ImageComponents::getAlphaComponents());
}

void
RotoPaint::addSupportedBitDepth(std::list<Natron::ImageBitDepthEnum>* depths) const
{
    depths->push_back(Natron::eImageBitDepthFloat);
}

void
RotoPaint::initializeKnobs()
{
    
}


void
RotoPaint::getPreferredDepthAndComponents(int /*inputNb*/,std::list<Natron::ImageComponents>* comp,Natron::ImageBitDepthEnum* depth) const
{
    EffectInstance* input = getInput(0);
    if (input) {
        return input->getPreferredDepthAndComponents(-1, comp, depth);
    } else {
        comp->push_back(ImageComponents::getRGBAComponents());
        *depth = eImageBitDepthFloat;
    }
}


Natron::ImagePremultiplicationEnum
RotoPaint::getOutputPremultiplication() const
{
    EffectInstance* input = getInput(0);
    if (input) {
        return input->getOutputPremultiplication();
    } else {
        return eImagePremultiplicationPremultiplied;
    }
    
}

double
RotoPaint::getPreferredAspectRatio() const
{
    EffectInstance* input = getInput(0);
    if (input) {
        return input->getPreferredAspectRatio();
    } else {
        return 1.;
    }
    
}



Natron::StatusEnum
RotoPaint::render(const RenderActionArgs& args)
{
    boost::shared_ptr<RotoContext> roto = getNode()->getRotoContext();
    std::list<boost::shared_ptr<RotoDrawableItem> > items = roto->getCurvesByRenderOrder();
    
    std::list<ImageComponents> bgComps;
    Natron::ImageBitDepthEnum bgDepth;
    getPreferredDepthAndComponents(0, &bgComps, &bgDepth);
    assert(!bgComps.empty());
    RectI bgImgRoI;
    ImagePtr bgImg = getImage(0, args.time, args.mappedScale, args.view, 0, bgComps.front(), bgDepth, getPreferredAspectRatio(), false, &bgImgRoI);
    
    std::list<ImageComponents> neededComps;
    for (std::list<std::pair<Natron::ImageComponents,boost::shared_ptr<Natron::Image> > >::const_iterator plane = args.outputPlanes.begin();
         plane != args.outputPlanes.end(); ++plane) {
        neededComps.push_back(plane->first);
    }
    if (items.empty()) {
        for (std::list<std::pair<Natron::ImageComponents,boost::shared_ptr<Natron::Image> > >::const_iterator plane = args.outputPlanes.begin();
             plane != args.outputPlanes.end(); ++plane) {
            
            if (bgImg) {
                plane->second->pasteFrom(*bgImg, args.roi);
            } else {
                plane->second->fill(args.roi, 0., 0., 0., 0.);
            }
            
        }
    } else {
        
        const boost::shared_ptr<RotoDrawableItem>& firstStrokeItem = items.front();
        RotoStrokeItem* firstStroke = dynamic_cast<RotoStrokeItem*>(firstStrokeItem.get());
        assert(firstStroke);
        boost::shared_ptr<Node> bottomMerge =  firstStroke->getMergeNode();
        
        
        unsigned int mipMapLevel = Image::getLevelFromScale(args.mappedScale.x);
        RenderRoIArgs rotoPaintArgs(args.time,
                                    args.mappedScale,
                                    mipMapLevel,
                                    args.view,
                                    false,
                                    args.roi,
                                    RectD(),
                                    neededComps,
                                    bgDepth);
        ImageList rotoPaintImages;
        RenderRoIRetCode code = bottomMerge->getLiveInstance()->renderRoI(rotoPaintArgs, &rotoPaintImages);
        if (code == eRenderRoIRetCodeFailed) {
            return Natron::eStatusFailed;
        } else if (code == eRenderRoIRetCodeAborted) {
            return Natron::eStatusOK;
        }
        
        assert(rotoPaintImages.size() == args.outputPlanes.size());
        
        ImageList::iterator rotoImagesIt = rotoPaintImages.begin();
        for (std::list<std::pair<Natron::ImageComponents,boost::shared_ptr<Natron::Image> > >::const_iterator plane = args.outputPlanes.begin();
             plane != args.outputPlanes.end(); ++plane, ++rotoImagesIt) {
            plane->second->pasteFrom(**rotoImagesIt, args.roi);
        }
    }
    
    return Natron::eStatusOK;
}