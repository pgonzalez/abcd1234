/*****************************************************************************
*                                                                            *
*  OpenNI 1.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
//---------------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------------
#include "NiHandTracker.h"
#include <cassert>


using namespace xn;

xn::UserGenerator       m_UserGenerator;
xn::ScriptNode          m_scriptNode;

XnBool m_bNeedPose = FALSE;
XnChar m_strPose[20] = "";
XnUserID aUsers[MAX_NUM_USERS];
XnUInt16 nUsers;
XnSkeletonJointTransformation torsoJoint;

//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define LENGTHOF(arr)			(sizeof(arr)/sizeof(arr[0]))
#define FOR_ALL(arr, action)	{for(int i = 0; i < LENGTHOF(arr); ++i){action(arr[i])}}

#define ADD_GESTURE(name)		{if(m_GestureGenerator.AddGesture(name, NULL) != XN_STATUS_OK){printf("Unable to add gesture"); exit(1);}}
#define REMOVE_GESTURE(name)	{if(m_GestureGenerator.RemoveGesture(name) != XN_STATUS_OK){printf("Unable to remove gesture"); exit(1);}}

#define ADD_ALL_GESTURES		FOR_ALL(cGestures, ADD_GESTURE)
#define REMOVE_ALL_GESTURES		FOR_ALL(cGestures, REMOVE_GESTURE)


//---------------------------------------------------------------------------
// Consts
//---------------------------------------------------------------------------
// Gestures to track
static const char			cClickStr[] = "Click";
static const char			cWaveStr[] = "Wave";
static const char			cRaiseHandStr[] = "RaiseHand";
static const char			cMovingHandStr[] = "MovingHand";
static const char* const	cGestures[] =
{
    cClickStr,
    cWaveStr,
    //cRaiseHandStr,
    //cMovingHandStr
};

//---------------------------------------------------------------------------
// Statics
//---------------------------------------------------------------------------
XnListT<HandTracker*>	HandTracker::sm_Instances;


//---------------------------------------------------------------------------
// Hooks HandTracker
//---------------------------------------------------------------------------
void XN_CALLBACK_TYPE HandTracker::Gesture_Recognized(	xn::GestureGenerator&	/*generator*/,
                                                        const XnChar*			strGesture,
                                                        const XnPoint3D*		/*pIDPosition*/,
                                                        const XnPoint3D*		pEndPosition,
                                                        void*					pCookie)
{
    printf("Gesture recognized: %s\n", strGesture);

    HandTracker*	pThis = static_cast<HandTracker*>(pCookie);
    if(sm_Instances.Find(pThis) == sm_Instances.End())
    {
        printf("Dead HandTracker: skipped!\n");
        return;
    }

    pThis->m_HandsGenerator.StartTracking(*pEndPosition);
}

void XN_CALLBACK_TYPE HandTracker::Hand_Create(	xn::HandsGenerator& /*generator*/,
                                                XnUserID			nId,
                                                const XnPoint3D*	pPosition,
                                                XnFloat				/*fTime*/,
                                                void*				pCookie)
{
    printf("New Hand: %d @ (%f,%f,%f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);

    HandTracker*	pThis = static_cast<HandTracker*>(pCookie);
    if(sm_Instances.Find(pThis) == sm_Instances.End())
    {
        printf("Dead HandTracker: skipped!\n");
        return;
    }

    pThis->m_History[nId].Push(*pPosition);
}

void XN_CALLBACK_TYPE HandTracker::Hand_Update(	xn::HandsGenerator& /*generator*/,
                                                XnUserID			nId,
                                                const XnPoint3D*	pPosition,
                                                XnFloat				/*fTime*/,
                                                void*				pCookie)
{
    HandTracker*	pThis = static_cast<HandTracker*>(pCookie);
    if(sm_Instances.Find(pThis) == sm_Instances.End())
    {
        printf("Dead HandTracker: skipped!\n");
        return;
    }

    // Add to this user's hands history
    TrailHistory::Iterator it = pThis->m_History.Find(nId);
    if (it == pThis->m_History.End())
    {
        printf("Dead hand update: skipped!\n");
        return;
    }

    it->Value().Push(*pPosition);
    printf("Hand: %d @ (%f,%f,%f)\n", nId, pPosition->X, pPosition->Y, pPosition->Z);

    /*m_UserGenerator.GetUsers(aUsers, nUsers);
    printf("numero de usuarios: %d\n",m_UserGenerator.GetNumberOfUsers());

    if(m_UserGenerator.GetSkeletonCap().IsTracking(aUsers[0])!=FALSE){

        m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[0],XN_SKEL_RIGHT_ELBOW,torsoJoint);
        printf("user %d 0: elbow at (%6.2f,%6.2f,%6.2f)\n",aUsers[0],
                                                        torsoJoint.position.position.X,
                                                        torsoJoint.position.position.Y,
                                                        torsoJoint.position.position.Z);


        // Add to this user's elbows history
        TrailHistory::Iterator ti = m_UserHistory.Find(nId);
        if (ti == m_UserHistory.End())
        {
            printf("Dead user update: skipped!\n");
            return;
        }

        ti->Value().Push(torsoJoint.position.position);
    }*/

}

void XN_CALLBACK_TYPE HandTracker::Hand_Destroy(	xn::HandsGenerator& /*generator*/,
                                                    XnUserID			nId,
                                                    XnFloat				/*fTime*/,
                                                    void*				pCookie)
{
    printf("Lost Hand: %d\n", nId);

    HandTracker*	pThis = static_cast<HandTracker*>(pCookie);
    if(sm_Instances.Find(pThis) == sm_Instances.End())
    {
        printf("Dead HandTracker: skipped!\n");
        return;
    }

    // Remove this user from hands history
    pThis->m_History.Remove(nId);
}

//---------------------------------------------------------------------------
// Hooks UserTracker
//---------------------------------------------------------------------------

// Callback: New user was detected
//void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
/*{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    if (m_bNeedPose)
    {
        m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(m_strPose, nId);
    }
    else
    {
        m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
    m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(nId,XN_SKEL_RIGHT_ELBOW,torsoJoint);
    m_UserHistory[nId].Push(torsoJoint.position.position);


}*/

// Callback: An existing user was lost
//void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* /*pCookie*/)
/*{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Lost user %d\n", epochTime, nId);
    m_UserHistory.Remove(nId);

}*/

// Callback: Detected a pose
//void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* /*pCookie*/)
/*{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    m_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}*/

// Callback: Started calibration
//void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* /*pCookie*/)
/*{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);
}*/

//void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* /*pCookie*/)
/*{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
        m_UserGenerator.GetSkeletonCap().StartTracking(nId);
    }
    else
    {
        // Calibration failed
        printf("%d Calibration failed for user %d\n", epochTime, nId);
        if(eStatus==XN_CALIBRATION_STATUS_MANUAL_ABORT)
        {
            printf("Manual abort occured, stop attempting to calibrate!");
            return;
        }
        if (m_bNeedPose)
        {
            m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(m_strPose, nId);
        }
        else
        {
            m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}*/

//---------------------------------------------------------------------------
// Method Definitions
//---------------------------------------------------------------------------
HandTracker::HandTracker(xn::Context& context) : m_rContext(context)
{
    // Track all living instances (to protect against calling dead pointers in the Gesture/Hand Generator hooks)
    XnStatus rc = sm_Instances.AddLast(this);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to add NiHandTracker instance to the list.");
        exit(1);
    }
}

HandTracker::~HandTracker()
{
    // Remove the current instance from living instances list
    XnListT<HandTracker*>::ConstIterator it = sm_Instances.Find(this);
    assert(it != sm_Instances.End());
    sm_Instances.Remove(it);
}

XnStatus HandTracker::Init()
{
    XnStatus			rc;
    XnCallbackHandle	chandle;

    // Create generators
    rc = m_GestureGenerator.Create(m_rContext);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to create GestureGenerator.");
        return rc;
    }

    rc = m_HandsGenerator.Create(m_rContext);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to create HandsGenerator.");
        return rc;
    }

    // Register callbacks
    // Using this as cookie
    rc = m_GestureGenerator.RegisterGestureCallbacks(Gesture_Recognized, Gesture_Process, this, chandle);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to register gesture callbacks.");
        return rc;
    }

    rc = m_HandsGenerator.RegisterHandCallbacks(Hand_Create, Hand_Update, Hand_Destroy, this, chandle);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to register hand callbacks.");
        return rc;
    }

    rc = m_UserGenerator.Create(m_rContext);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to create UserGenerator.");
        return rc;
    }

    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected, hUpdateDetected;
    if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }

    /*rc = m_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, NULL, hUserCallbacks);
    CHECK_RC(rc, "Register to user callbacks");
    rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, NULL, hCalibrationStart);
    CHECK_RC(rc, "Register to calibration start");
    rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, NULL, hCalibrationComplete);
    CHECK_RC(rc, "Register to calibration complete");

    if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        m_bNeedPose = TRUE;
        if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return 1;
        }
        rc = m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, NULL, hPoseDetected);
        CHECK_RC(rc, "Register to Pose Detected");
        m_UserGenerator.GetSkeletonCap().GetCalibrationPose(m_strPose);
    }

    m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);*/

    return XN_STATUS_OK;
}

XnStatus HandTracker::Run()
{
    //ADD_ALL_GESTURES;

    XnStatus	rc = m_rContext.StartGeneratingAll();
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to start generating.");
        return rc;
    }
    nUsers=MAX_NUM_USERS;


    ADD_ALL_GESTURES;

    return XN_STATUS_OK;
}

