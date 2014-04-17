
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
#include "NiElbowTracker.h"
#include <cassert>


using namespace xn;



//---------------------------------------------------------------------------
// Defines
//---------------------------------------------------------------------------
#define LENGTHOF(arr)			(sizeof(arr)/sizeof(arr[0]))
#define FOR_ALL(arr, action)	{for(int i = 0; i < LENGTHOF(arr); ++i){action(arr[i])}}


//---------------------------------------------------------------------------
// Consts
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// Statics
//---------------------------------------------------------------------------
XnListT<ElbowTracker*>	ElbowTracker::sm_Instances;



//---------------------------------------------------------------------------
// Hooks UserTracker
//---------------------------------------------------------------------------

//Callback: New user was detected
void XN_CALLBACK_TYPE ElbowTracker::User_NewUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d New User %d\n", epochTime, nId);
    // New user found
    ElbowTracker* pThis = static_cast<ElbowTracker*>(pCookie);

    if (pThis->m_bNeedPose)
    {
        pThis->m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(pThis->m_strPose, nId);
    }
    else
    {
        pThis->m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
    }
    //pThis->m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(nId,XN_SKEL_RIGHT_ELBOW,pThis->torsoJoint);
    //pThis->m_History[nId].Push(pThis->torsoJoint.position.position);


}

// Callback: An existing user was lost
void XN_CALLBACK_TYPE ElbowTracker::User_LostUser(xn::UserGenerator& /*generator*/, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    ElbowTracker* pThis = static_cast<ElbowTracker*>(pCookie);

    printf("%d Lost user %d\n", epochTime, nId);
    //pThis->m_History.Remove(nId);

}

// Callback: Detected a pose
void XN_CALLBACK_TYPE ElbowTracker::UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/, const XnChar* strPose, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    ElbowTracker* pThis = static_cast<ElbowTracker*>(pCookie);

    printf("%d Pose %s detected for user %d\n", epochTime, strPose, nId);
    pThis->m_UserGenerator.GetPoseDetectionCap().StopPoseDetection(nId);
    pThis->m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
}

// Callback: Started calibration
void XN_CALLBACK_TYPE ElbowTracker::UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/, XnUserID nId, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    printf("%d Calibration started for user %d\n", epochTime, nId);

}

void XN_CALLBACK_TYPE ElbowTracker::UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/, XnUserID nId, XnCalibrationStatus eStatus, void* pCookie)
{
    XnUInt32 epochTime = 0;
    xnOSGetEpochTime(&epochTime);
    ElbowTracker* pThis = static_cast<ElbowTracker*>(pCookie);

    if (eStatus == XN_CALIBRATION_STATUS_OK)
    {
        // Calibration succeeded
        printf("%d Calibration complete, start tracking user %d\n", epochTime, nId);
        pThis->m_UserGenerator.GetSkeletonCap().StartTracking(nId);
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
        if (pThis->m_bNeedPose)
        {
            pThis->m_UserGenerator.GetPoseDetectionCap().StartPoseDetection(pThis->m_strPose, nId);
        }
        else
        {
            pThis->m_UserGenerator.GetSkeletonCap().RequestCalibration(nId, TRUE);
        }
    }
}

//---------------------------------------------------------------------------
// Method Definitions
//---------------------------------------------------------------------------
ElbowTracker::ElbowTracker(xn::Context &context) : m_rContext(context){
    // Track all living instances (to protect against calling dead pointers in the User Generator hooks)
    m_strPose[20] = '\0';
    XnStatus rc = sm_Instances.AddLast(this);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to add NiElbowTracker instance to the list.");
        exit(1);
    }
    printf("ElbowTracker Constructor\n");


}

ElbowTracker::~ElbowTracker()
{
    // Remove the current instance from living instances list
    XnListT<ElbowTracker*>::ConstIterator it = sm_Instances.Find(this);
    assert(it != sm_Instances.End());
    sm_Instances.Remove(it);
    //printf("ElbowTracker Constructor\n");


}

XnStatus ElbowTracker::Init()
{
    printf("Elbow Init Pt 1\n");

    XnStatus			rc;
    //XnCallbackHandle	chandle;

    rc = m_UserGenerator.Create(m_rContext);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to create UserGenerator.");
        return rc;
    }


    // Register callbacks
    // Using this as cookie


    XnCallbackHandle hUserCallbacks, hCalibrationStart, hCalibrationComplete, hPoseDetected;
    if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_SKELETON))
    {
        printf("Supplied user generator doesn't support skeleton\n");
        return 1;
    }

    rc = m_UserGenerator.RegisterUserCallbacks(User_NewUser, User_LostUser, this, hUserCallbacks);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to register user callbacks.");
        return rc;
    }

    rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationStart(UserCalibration_CalibrationStart, this, hCalibrationStart);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to register calibration start.");
        return rc;
    }

    rc = m_UserGenerator.GetSkeletonCap().RegisterToCalibrationComplete(UserCalibration_CalibrationComplete, this, hCalibrationComplete);
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to register calibration complete");
        return rc;
    }

    //if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    if (m_UserGenerator.GetSkeletonCap().NeedPoseForCalibration())
    {
        m_bNeedPose = TRUE;
        printf("Need pose for calibration\n");
        if (!m_UserGenerator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION))
        {
            printf("Pose required, but not supported\n");
            return 1;
        }
        rc = m_UserGenerator.GetPoseDetectionCap().RegisterToPoseDetected(UserPose_PoseDetected, this, hPoseDetected);
        if (rc != XN_STATUS_OK)
        {
            printf("Unable to register pose detected.");
            return rc;
        }
        m_UserGenerator.GetSkeletonCap().GetCalibrationPose(m_strPose);
    }

    m_UserGenerator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_UPPER);

    return XN_STATUS_OK;
}

XnStatus ElbowTracker::Run()
{
    //ADD_ALL_GESTURES;

    XnStatus	rc = m_rContext.StartGeneratingAll();
    if (rc != XN_STATUS_OK)
    {
        printf("Unable to start generating.");
        return rc;
    }
    nUsers=MAX_NUM_USERS;
    printf("ElbowTracker Run\n");

    return XN_STATUS_OK;
}

Points** ElbowTracker::Print(){
    Points** m_Points;
    m_Points = new Points*[MAX_NUM_USERS];
    nUsers=MAX_NUM_USERS;
    m_UserGenerator.GetUsers(aUsers, nUsers);
    for(XnUInt16 i=0; i<nUsers; i++)
    {
        //printf("Pass 02\n");
        m_Points[aUsers[i]] = new Points[24];

        if(m_UserGenerator.GetSkeletonCap().IsTracking(aUsers[i])==FALSE){
            m_isTracking = FALSE;

            continue;
        }
        m_isTracking = TRUE;
        m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_RIGHT_ELBOW,torsoJoint);
//        printf("user %d: left elbow at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
//                                                            torsoJoint.position.position.X,
//                                                            torsoJoint.position.position.Y,
//                                                            torsoJoint.position.position.Z);


        m_Points[aUsers[i]][XN_SKEL_RIGHT_ELBOW].pointX = torsoJoint.position.position.X;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_ELBOW].pointY = torsoJoint.position.position.Y;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_ELBOW].pointZ = torsoJoint.position.position.Z;

        m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_RIGHT_HAND,torsoJoint);
//            printf("user %d: left hand at (%6.2f,%6.2f,%6.2f)\n",aUsers[i],
//                                                            torsoJoint.position.position.X,
//                                                            torsoJoint.position.position.Y,
//                                                            torsoJoint.position.position.Z);
        m_Points[aUsers[i]][XN_SKEL_RIGHT_HAND].pointX = torsoJoint.position.position.X;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_HAND].pointY = torsoJoint.position.position.Y;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_HAND].pointZ = torsoJoint.position.position.Z;

        m_UserGenerator.GetSkeletonCap().GetSkeletonJoint(aUsers[i],XN_SKEL_RIGHT_WRIST,torsoJoint);

        m_Points[aUsers[i]][XN_SKEL_RIGHT_WRIST].pointX = torsoJoint.position.position.X;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_WRIST].pointY = torsoJoint.position.position.Y;
        m_Points[aUsers[i]][XN_SKEL_RIGHT_WRIST].pointZ = torsoJoint.position.position.Z;
    }
    return m_Points;
}

