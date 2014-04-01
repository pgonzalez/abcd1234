#ifndef NIELBOWTRACKER_H
#define NIELBOWTRACKER_H

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

#include <XnCppWrapper.h>
#include <XnCyclicStackT.h>
#include <XnHashT.h>

// Hand position history length (positions)
#define MAX_HAND_TRAIL_LENGTH	10
#define MAX_NUM_USERS 15

struct Points{
    XnFloat pointX;
    XnFloat pointY;
    XnFloat pointZ;
};

typedef XnCyclicStackT<XnPoint3D, MAX_HAND_TRAIL_LENGTH> Trail;
typedef XnHashT<XnUserID, Trail> TrailHistory;

//TrailHistory m_UserHistory;

class ElbowTracker
{
public:
    ElbowTracker(xn::Context& context);
    ~ElbowTracker();

    XnStatus Init();
    XnStatus Run();
    Points **Print();

    const TrailHistory&	GetHistory()	const	{return m_History;}
    XnBool m_bNeedPose = FALSE;
    XnChar m_strPose[20];
    XnUserID aUsers[MAX_NUM_USERS];
    XnUInt16 nUsers;
    XnSkeletonJointTransformation torsoJoint;
    xn::UserGenerator       m_UserGenerator;
    xn::ScriptNode          m_scriptNode;



private:
    // OpenNI User Generator callbacks
    static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& /*generator*/,
                                              XnUserID nId,
                                              void* pCookie);

    static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& /*generator*/,
                                               XnUserID nId,
                                               void* pCookie);

    static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& /*capability*/,
                                                       const XnChar* strPose,
                                                       XnUserID nId,
                                                       void* pCookie);

    static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& /*capability*/,
                                                                  XnUserID nId,
                                                                  void* pCookie);

    static void XN_CALLBACK_TYPE UserCalibration_CalibrationComplete(xn::SkeletonCapability& /*capability*/,
                                                                     XnUserID nId,
                                                                     XnCalibrationStatus eStatus,
                                                                     void* pCookie);


    xn::Context&			m_rContext;
    TrailHistory			m_History;

    static XnListT<ElbowTracker*>	sm_Instances;	// Living instances of the class

private:
    XN_DISABLE_COPY_AND_ASSIGN(ElbowTracker);
};

#define CHECK_RC(rc, what)											\
    if (rc != XN_STATUS_OK)											\
    {																\
        printf("%s failed: %s\n", what, xnGetStatusString(rc));		\
        return rc; \
    }


#endif // NIELBOWTRACKER_H
