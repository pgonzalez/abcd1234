#ifndef NITEDEVICE_H
#define NITEDEVICE_H

// Headers for OpenNI
#include <XnOpenNI.h>
#include <XnCppWrapper.h>
#include <XnHash.h>
#include <XnLog.h>

// Header for NITE
#include "XnVNite.h"


void initialize(){

    nite::HandTracker* pHandTracker;
    nite::HandTrackerFrameRef handFrame;
    nite::NiTE::initialize();
    pHandTracker = new nite::HandTracker;
    pHandTracker->create();

    pHandTracker->startGestureDetection(nite::GESTURE_CLICK);

    nite::Status rc = pHandTracker->readFrame(&handFrame);
    const nite::Array<nite::GestureData>& gestures = handFrame.getGestures();
    for (int i = 0; i < gestures.getSize(); ++i)
    {
        if (gestures[i].isComplete())
        {
            nite::HandId id;
            pHandTracker->startHandTracking(gestures[i].getCurrentPosition(), &id);
        }
    }

    const nite::Array<nite::HandData>& hands= handFrame.getHands();
    g_nHandsCount = hands.getSize();

    for (int i = 0; i < g_nHandsCount; ++i)
    {
        const nite::HandData& handData = hands[i];
        if (handData.isTracking())
        {
                float x, y;
                pHandTracker->convertHandCoordinatesToDepth(
                    handData.getPosition().x, handData.getPosition().y, handData.getPosition().z,
                    &x, &y);
        }
    }
}
#endif // NITEDEVICE_H
