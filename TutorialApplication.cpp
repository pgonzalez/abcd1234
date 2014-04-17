/*
-----------------------------------------------------------------------------
Filename:    TutorialApplication.cpp
-----------------------------------------------------------------------------

This source file is part of the
   ___                 __    __ _ _    _
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/


#include "BasicTutorial3.h"
#include "NiHandTracker.h"
#include "NiElbowTracker.h"

#include <thread>
#include <iostream>

xn::Context context_hand;
xn::Context context_elbow;
xn::ScriptNode scriptNode;
xn::ScriptNode scriptNode_elbow;
xn::EnumerationErrors errors;
XnStatus nRetVal = XN_STATUS_OK;
XnStatus nRetVal_Elbow = XN_STATUS_OK;
bool isOpen = false;
Ogre::Vector3 handPos[6];
Ogre::Vector3 handPosFirst;
Ogre::Vector3 handPosBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 handPosOriginal;
Ogre::Vector3 wristPos[6];
Ogre::Vector3 wristPosFirst;
Ogre::Vector3 wristPosBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 wristPosOriginal;
Ogre::Vector3 elbowPos[6];
Ogre::Vector3 elbowPosFirst;
Ogre::Vector3 elbowPosBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 elbowPosOriginal;
Ogre::Vector3 ninjaPosOriginal;
Ogre::Real dotProductMax = 0;
Ogre::Real dotProductMin = 9999999999999999;
//XnSkeletonJointTransformation torsoJoint;

int lectura = 0;


typedef TrailHistory			History;
typedef History::ConstIterator	HistoryIterator;
typedef Trail::ConstIterator	TrailIterator;

XnBool fileExists(const char *fn)
{
    XnBool exists;
    xnOSDoesFileExist(fn, &exists);
    return exists;
}

int elbowTask(){
    printf("ElbowTask\n");
    Points** bodyCoords;
    const char *fn = NULL;
    if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
    else {
        printf("Could not find %s'. Aborting.\n" , SAMPLE_XML_PATH_LOCAL);
        //return XN_STATUS_ERROR;
    }
    printf("Reading config from: '%s'\n", fn);
    nRetVal = context_elbow.InitFromXmlFile(fn, scriptNode_elbow, &errors);

    if (nRetVal_Elbow == XN_STATUS_NO_NODE_PRESENT)
    {
        XnChar strError[1024];
        errors.ToString(strError, 1024);
        printf("%s\n", strError);
        return (nRetVal_Elbow);
    }
    else if (nRetVal_Elbow != XN_STATUS_OK)
    {
        printf("Open failed: %s\n", xnGetStatusString(nRetVal_Elbow));
        return (nRetVal_Elbow);
    }

    xn::DepthGenerator depth_el;
    nRetVal_Elbow = context_elbow.FindExistingNode(XN_NODE_TYPE_DEPTH, depth_el);
    CHECK_RC(nRetVal_Elbow, "Find depth generator");
    if(nRetVal_Elbow != XN_STATUS_OK){
        printf("Kinect Error");
        exit(1);
    }

    XnFPSData xnFPS_el;
    nRetVal_Elbow = xnFPSInit(&xnFPS_el, 180);
    CHECK_RC(nRetVal_Elbow, "FPS Init");

    ElbowTracker mElbowTracker(context_elbow);

    XnStatus rc_elbow= mElbowTracker.Init();
    rc_elbow = mElbowTracker.Run();
    while (isOpen)
    {
        //if(mElbowTracker.m_UserGenerator.GetSkeletonCap().IsTracking(mElbowTracker.aUsers[0])==FALSE)
          //  continue;

        nRetVal_Elbow = context_elbow.WaitOneUpdateAll(depth_el);
        bodyCoords = mElbowTracker.Print();
        if (mElbowTracker.m_isTracking){
            //printf("Elbow PointX: %6.2f\n", bodyCoords[1][XN_SKEL_LEFT_ELBOW].pointY);
            handPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointZ));
            //armPos[1] = Ogre::Vector3(trunc((bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointX + bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointX)/2), trunc((bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointY + bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointY)/2), trunc((bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointZ + bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointZ)/2));
            wristPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointZ));
            elbowPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointZ));
        }
    }


    depth_el.Release();
    scriptNode.Release();
    context_elbow.Release();

    return 0;
}

int kinectTask(){
    printf("kinectTask\n");

//    const char *fn = NULL;
//    if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
//    else {
//        printf("Could not find '%s'. Aborting.\n" , SAMPLE_XML_PATH_LOCAL);
//        //return XN_STATUS_ERROR;
//    }
//    printf("Reading config from: '%s'\n", fn);
//    nRetVal = context_hand.InitFromXmlFile(fn, scriptNode, &errors);


//    if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
//    {
//        XnChar strError[1024];
//        errors.ToString(strError, 1024);
//        printf("%s\n", strError);
//        return (nRetVal);
//    }
//    else if (nRetVal != XN_STATUS_OK)
//    {
//        printf("Open failed: %s\n", xnGetStatusString(nRetVal));
//        return (nRetVal);
//    }

    xn::DepthGenerator depth;
    nRetVal = context_hand.FindExistingNode(XN_NODE_TYPE_DEPTH, depth);
    CHECK_RC(nRetVal, "Find depth generator");

    XnFPSData xnFPS;
    nRetVal = xnFPSInit(&xnFPS, 180);
    CHECK_RC(nRetVal, "FPS Init");

    HandTracker mHandTracker(context_hand);
    XnStatus rc_hand= mHandTracker.Init();
    //std::cout << "Status: " << rc << std::endl;

    rc_hand = mHandTracker.Run();

    while (isOpen)
    {

        nRetVal = context_hand.WaitOneUpdateAll(depth);
        if (nRetVal != XN_STATUS_OK)
        {
            printf("UpdateData Hand failed: %s\n", xnGetStatusString(nRetVal));
            continue;
        }

        const TrailHistory&	history_hand = mHandTracker.GetHistory();
        const HistoryIterator	hend_hand = history_hand.End();
        for(HistoryIterator		hit_hand = history_hand.Begin(); hit_hand != hend_hand; ++hit_hand)
        {

            // Dump the history to local buffer
            int				numpoints_hand = 0;
            const Trail&	trail_hand = hit_hand->Value();

            const TrailIterator	tit_hand = trail_hand.Begin();

                XnPoint3D	point = *tit_hand;
                handPos[hit_hand->Key()] = Ogre::Vector3(point.X, point.Y, point.Z);
                //printf("Kinect %d Punto X: %f, Punto Y: %f, Punto Z: %f\n",hit->Key(), kinectPos[hit->Key()].x, kinectPos[hit->Key()].y, kinectPos[hit->Key()].z);

                ++numpoints_hand;

            assert(numpoints_hand <= MAX_HAND_TRAIL_LENGTH);
        }


    }

    depth.Release();
    scriptNode.Release();
    context_hand.Release();
    context_elbow.Release();

    return 0;
}

//-------------------------------------------------------------------------------------
BasicTutorial3::BasicTutorial3(void)
{
}
//-------------------------------------------------------------------------------------
BasicTutorial3::~BasicTutorial3(void)
{
}
//-------------------------------------------------------------------------------------
void BasicTutorial3::destroyScene(void)
{
    OGRE_DELETE mTerrainGroup;
    OGRE_DELETE mTerrainGlobals;
    isOpen = false;
}
//-------------------------------------------------------------------------------------
void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
{
    img.load("terrain3.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    if (flipX)
        img.flipAroundY();
    if (flipY)
        img.flipAroundX();
}
//-------------------------------------------------------------------------------------
void BasicTutorial3::defineTerrain(long x, long y)
{
    Ogre::String filename = mTerrainGroup->generateFilename(x, y);
    if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
    {
        mTerrainGroup->defineTerrain(x, y);
    }
    else
    {
        Ogre::Image img;
        getTerrainImage(x % 2 != 0, y % 2 != 0, img);
        mTerrainGroup->defineTerrain(x, y, &img);
        mTerrainsImported = true;
    }
}
//-------------------------------------------------------------------------------------
void BasicTutorial3::initBlendMaps(Ogre::Terrain* terrain)
{
    Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
    Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
    Ogre::Real minHeight0 = 70;
    Ogre::Real fadeDist0 = 40;
    Ogre::Real minHeight1 = 70;
    Ogre::Real fadeDist1 = 15;
    float* pBlend0 = blendMap0->getBlendPointer();
    float* pBlend1 = blendMap1->getBlendPointer();
    for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
    {
        for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
        {
            Ogre::Real tx, ty;

            blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
            Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
            Ogre::Real val = (height - minHeight0) / fadeDist0;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend0++ = val;

            val = (height - minHeight1) / fadeDist1;
            val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
            *pBlend1++ = val;
        }
    }
    blendMap0->dirty();
    blendMap1->dirty();
    blendMap0->update();
    blendMap1->update();
}
//-------------------------------------------------------------------------------------
void BasicTutorial3::configureTerrainDefaults(Ogre::Light* light)
{
    // Configure global
    mTerrainGlobals->setMaxPixelError(8);
    // testing composite map
    mTerrainGlobals->setCompositeMapDistance(3000);

    // Important to set these so that the terrain knows what to use for derived (non-realtime) data
    mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
    mTerrainGlobals->setCompositeMapAmbient(mSceneMgr->getAmbientLight());
    mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

    // Configure default import settings for if we use imported image
    Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
    defaultimp.terrainSize = 513;
    defaultimp.worldSize = 12000.0f;
    defaultimp.inputScale = 600;
    defaultimp.minBatchSize = 33;
    defaultimp.maxBatchSize = 65;
    // textures
    defaultimp.layerList.resize(3);
    defaultimp.layerList[0].worldSize = 100;
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
    defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
    defaultimp.layerList[1].worldSize = 30;
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
    defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
    defaultimp.layerList[2].worldSize = 200;
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
    defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
}
//-------------------------------------------------------------------------------------
void BasicTutorial3::createScene(void)
{
//    mCamera->setPosition(Ogre::Vector3(1683, 1000, 2116));
//    mCamera->lookAt(Ogre::Vector3(1683, 1000, 1660));
    mCamera->setPosition(Ogre::Vector3(1683, 300, 2700));
    mCamera->lookAt(Ogre::Vector3(1963, 100, 1660));
    mCamera->setNearClipDistance(0.1);
    mCamera->setFarClipDistance(50000);

    if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
    {
        mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
    }

    Ogre::MaterialManager::getSingleton().setDefaultTextureFiltering(Ogre::TFO_ANISOTROPIC);
    Ogre::MaterialManager::getSingleton().setDefaultAnisotropy(7);

    Ogre::Vector3 lightdir(0.55, -0.3, 0.75);
    lightdir.normalise();

    Ogre::Light* light = mSceneMgr->createLight("tstLight");
    light->setType(Ogre::Light::LT_DIRECTIONAL);
    light->setDirection(lightdir);
    light->setDiffuseColour(Ogre::ColourValue::White);
    light->setSpecularColour(Ogre::ColourValue(0.4, 0.4, 0.4));

    mSceneMgr->setAmbientLight(Ogre::ColourValue(0.2, 0.2, 0.2));

    mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

    mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneMgr, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);
    mTerrainGroup->setFilenameConvention(Ogre::String("BasicTutorial3Terrain"), Ogre::String("dat"));
    mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

    configureTerrainDefaults(light);

    for (long x = 0; x <= 0; ++x)
        for (long y = 0; y <= 0; ++y)
            defineTerrain(x, y);

    // sync load since we want everything in place when we start
    mTerrainGroup->loadAllTerrains(true);

    if (mTerrainsImported)
    {
        Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
        while(ti.hasMoreElements())
        {
            Ogre::Terrain* t = ti.getNext()->instance;
            initBlendMaps(t);
        }
    }

    mTerrainGroup->freeTemporaryResources();

    //mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000, false);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    // NINJA

    Ogre::Entity *ent = mSceneMgr->createEntity("Ninja", "column.mesh");
    Ogre::SceneNode *node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
    ent->setCastShadows(true);
    node->attachObject(ent);

    node->setPosition(1683, 1000, 1660);
    node->yaw(Ogre::Degree(180));

    //node->setScale(1,2,1);

    ninjaPosOriginal = node->getPosition();

//    // pEntNode is the node the Entity is attached to
//    // entPos is the current position you're using to place the entity

//    Ogre::SceneNode* parent = mSceneMgr->getRootSceneNode()->createChildSceneNode("ParentNinjaNode");
//    // place this node 20 units above where the current entity centre is
//    parent->setPosition(ninjaPosOriginal + Ogre::Vector3(0,-100,0));
//    // re-parent the entity node
//    if (node->getParent())
//        node->getParent()->removeChild(node);
//    parent->addChild(node);
//    // now offset the entity node relative to this parent to put it back in its original place
//    node->setPosition(Ogre::Vector3(0,100,0));
//    //parent->pitch(Ogre::Degree(90));

    // LINE

    Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1");
    Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node");

    // NOTE: The second parameter to the create method is the resource group the material will be added to.
    // If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
    Ogre::MaterialPtr myManualObjectMaterial = Ogre::MaterialManager::getSingleton().create("manual1Material","General");
    myManualObjectMaterial->setReceiveShadows(false);
    myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true);
    myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,1,0);
    myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,1);
    myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,1);


    myManualObject->begin("manual1Material", Ogre::RenderOperation::OT_LINE_LIST);
    myManualObject->position(1750, 0, 2142);
    //myManualObject->position(1683, 1, 2116);
    myManualObject->position(1750,450, 2142);

    // etc
    myManualObject->end();

    myManualObjectNode->attachObject(myManualObject);

    /*Ogre::ManualObject* myManualObject2 =  mSceneMgr->createManualObject("manual2");
    Ogre::SceneNode* myManualObjectNode2 = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual2_node");

    // NOTE: The second parameter to the create method is the resource group the material will be added to.
    // If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
    Ogre::MaterialPtr myManualObjectMaterial2 = Ogre::MaterialManager::getSingleton().create("manual2Material","General");
    myManualObjectMaterial2->setReceiveShadows(false);
    myManualObjectMaterial2->getTechnique(0)->setLightingEnabled(true);
    myManualObjectMaterial2->getTechnique(0)->getPass(0)->setDiffuse(0,1,0,0);
    myManualObjectMaterial2->getTechnique(0)->getPass(0)->setAmbient(0,1,0);
    myManualObjectMaterial2->getTechnique(0)->getPass(0)->setSelfIllumination(0,1,0);


    myManualObject2->begin("manual2Material", Ogre::RenderOperation::OT_LINE_LIST);
    myManualObject2->position(1683, 1, 1760);
    myManualObject2->position(1683, 1, 1860);
    // etc
    myManualObject2->end();

    myManualObjectNode2->attachObject(myManualObject2);

    Ogre::ManualObject* myManualObject3 =  mSceneMgr->createManualObject("manual3");
    Ogre::SceneNode* myManualObjectNode3 = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual3_node");

    // NOTE: The second parameter to the create method is the resource group the material will be added to.
    // If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
    Ogre::MaterialPtr myManualObjectMaterial3 = Ogre::MaterialManager::getSingleton().create("manual3Material","General");
    myManualObjectMaterial3->setReceiveShadows(false);
    myManualObjectMaterial3->getTechnique(0)->setLightingEnabled(true);
    myManualObjectMaterial3->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,0);
    myManualObjectMaterial3->getTechnique(0)->getPass(0)->setAmbient(1,0,0);
    myManualObjectMaterial3->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);


    myManualObject3->begin("manual3Material", Ogre::RenderOperation::OT_LINE_LIST);
    myManualObject3->position(1683, 1, 1860);
    myManualObject3->position(1683, 1, 1960);
    // etc
    myManualObject3->end();

    myManualObjectNode3->attachObject(myManualObject3);

    Ogre::ManualObject* myManualObject4 =  mSceneMgr->createManualObject("manual4");
    Ogre::SceneNode* myManualObjectNode4 = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual4_node");

    // NOTE: The second parameter to the create method is the resource group the material will be added to.
    // If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
    Ogre::MaterialPtr myManualObjectMaterial4 = Ogre::MaterialManager::getSingleton().create("manual4Material","General");
    myManualObjectMaterial4->setReceiveShadows(false);
    myManualObjectMaterial4->getTechnique(0)->setLightingEnabled(true);
    myManualObjectMaterial4->getTechnique(0)->getPass(0)->setDiffuse(1,1,0,0);
    myManualObjectMaterial4->getTechnique(0)->getPass(0)->setAmbient(1,1,0);
    myManualObjectMaterial4->getTechnique(0)->getPass(0)->setSelfIllumination(1,1,0);


    myManualObject4->begin("manual4Material", Ogre::RenderOperation::OT_LINE_LIST);
    myManualObject4->position(1683, 1, 1960);
    myManualObject4->position(1683, 1, 2060);
    // etc
    myManualObject4->end();

    myManualObjectNode4->attachObject(myManualObject4);*/


    //Ogre::Vector3 distanceNinjaCamera = node->getPosition().distance(mCamera->getPosition());
    //printf("Distance Ninja-Camera: %d, %d, %d", distanceNinjaCamera.x, distanceNinjaCamera.y, distanceNinjaCamera.z);

    Ogre::Real distanceNinjaCamera = node->getPosition().distance(Ogre::Vector3(1683,1,2116));
    printf("Distance Ninja-Camera: %.0f\n", distanceNinjaCamera);


    // HAND

    Ogre::Entity *leftEnt = mSceneMgr->createEntity("RightHand", "hand.mesh");

    Ogre::SceneNode *leftNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("RightHandNode");
    leftEnt->setCastShadows(true);
    leftNode->attachObject(leftEnt);
    leftNode->setScale(40.2, 40.2, 40.2);

    //node2->translate(1800, 50, 1660);
    //leftNode->translate(2050,10,2300);
    leftNode->translate(1830, 70, 2000);
    leftNode->yaw(Ogre::Degree(-120));
    leftNode->pitch(Ogre::Degree(180));
    leftNode->roll(Ogre::Degree(30));


    Ogre::Entity *armEnt = mSceneMgr->createEntity("Arm", "column.mesh");
    Ogre::SceneNode *armNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Arm");

    armEnt->setCastShadows(true);
    armNode->attachObject(armEnt);
    //rightNode->setScale(40.2, 40.2, 40.2);

    armNode->translate(1750, 100, 2542);
    //armNode->pitch(Ogre::Degree(133));
    //armNode->roll(Ogre::Degree(20));
    armNode->setScale(1,1,1);

    // pEntNode is the node the Entity is attached to
    // entPos is the current position you're using to place the entity

    Ogre::SceneNode* parent = mSceneMgr->getRootSceneNode()->createChildSceneNode("ParentArmNode");
    // place this node 20 units above where the current entity centre is
    parent->setPosition(armNode->getPosition() + Ogre::Vector3(0,-100,0));
    // re-parent the entity node
    if (armNode->getParent())
        armNode->getParent()->removeChild(armNode);
    parent->addChild(armNode);
    // now offset the entity node relative to this parent to put it back in its original place
    armNode->setPosition(Ogre::Vector3(0,100,0));
    parent->pitch(Ogre::Degree(-90));

    //parent->pitch(Ogre::Degree(-90));
    //parent->roll(Ogre::Degree(20));

    handPosOriginal = leftNode->getPosition();
    wristPosOriginal = parent->getPosition();
//    wristPosOriginal = armNode->getPosition();
    //elbowPosOriginal = parent->getPosition();


    Ogre::SceneNode *kinectElbowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectElbow");
    Ogre::SceneNode *kinectWristNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectWrist");
    Ogre::SceneNode *kinectHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectHand");


}
//-------------------------------------------------------------------------------------
void BasicTutorial3::createFrameListener(void)
{
    BaseApplication::createFrameListener();
    mTrayMgr->hideAll();

    //mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);
}
//-------------------------------------------------------------------------------------
bool BasicTutorial3::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);

    /*if (mTerrainGroup->isDerivedDataUpdateInProgress())
    {
        mTrayMgr->moveWidgetToTray(mInfoLabel, OgreBites::TL_TOP, 0);
        mInfoLabel->show();
        if (mTerrainsImported)
        {
            mInfoLabel->setCaption("Building terrain, please wait...");
        }
        else
        {
            mInfoLabel->setCaption("Updating textures, patience...");
        }
    }
    else
    {
        mTrayMgr->removeWidgetFromTray(mInfoLabel);
        mInfoLabel->hide();
        if (mTerrainsImported)
        {
            mTerrainGroup->saveAllTerrains(true);
            mTerrainsImported = false;
        }
    }*/

    if (handPos[1] != Ogre::Vector3(0,0,0)){
        lectura++;

        if (lectura == 1){
            handPosFirst = handPos[1];
            wristPosFirst = wristPos[1];
            elbowPosFirst = elbowPos[1];
        }


        Ogre::Node *ninjaNode = mSceneMgr->getRootSceneNode()->getChild("NinjaNode");
        Ogre::Node *handNode = mSceneMgr->getRootSceneNode()->getChild("RightHandNode");
        //Ogre::Node *armNode =  mSceneMgr->getRootSceneNode()->getChild("Arm");
        Ogre::Node *armNode =  mSceneMgr->getRootSceneNode()->getChild("ParentArmNode");
        Ogre::Node *kinectElbowNode = mSceneMgr->getRootSceneNode()->getChild("KinectElbow");
        Ogre::Node *kinectWristNode = mSceneMgr->getRootSceneNode()->getChild("KinectWrist");
        Ogre::Node *kinectHandNode = mSceneMgr->getRootSceneNode()->getChild("KinectHand");



//        Ogre::Vector3 wristPitchAngle = Ogre::Vector3(0,wristPos[1].y,wristPos[1].z);
//        Ogre::Vector3 wristPitchAngleFirst = Ogre::Vector3(0,wristPosFirst.y,wristPosFirst.z);


        Ogre::Vector3 handScenePos = handPos[1] - handPosFirst;
        Ogre::Vector3 armScenePos = wristPos[1] - wristPosFirst;
        //Ogre::Vector3 armScenePos = elbowPos[1] - elbowPosFirst;

        //kinectPos[1] = Ogre::Quaternion(Ogre::Degree(200), Ogre::Vector3::UNIT_Y) * kinectPos[1];
        handScenePos = handPosOriginal + handScenePos;
        armScenePos = wristPosOriginal + armScenePos;

        //mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->setPosition(kinectPos[1]);
        //ninjaNode->setPosition(kinectPos[1]);

        kinectElbowNode->setPosition(elbowPos[1]);
        kinectWristNode->setPosition(wristPos[1]);
        kinectHandNode->setPosition(handPos[1]);
        //wristNode->setPosition(wristPos);
        //armNode->pitch(wristPitchAngleFirst.angleBetween(wristPitchAngle));
//        Ogre::Quaternion qI = armNode->getInitialOrientation();
//        Ogre::Vector3 vP = wristPos[1] - wristPosFirst;
//        vP.normalise();
//        Ogre::Vector3 vd =  qI * -Ogre::Vector3::UNIT_Y;
//        Ogre::Quaternion newQ = vd.getRotationTo(vP);
//        armNode->resetOrientation();
//        newQ = armNode->convertWorldToLocalOrientation(newQ);
//        armNode->setOrientation(newQ * qI);

        Ogre::Vector3 distanceVectorArm = kinectWristNode->getPosition() - kinectElbowNode->getPosition();
        Ogre::Vector3 distanceVectorHand = kinectHandNode->getPosition() - kinectWristNode->getPosition();
        Ogre::Real distanceWristElbow = kinectWristNode->getPosition().distance(kinectElbowNode->getPosition());

        handNode->setPosition(handScenePos);
        //armNode->setPosition(armScenePos);

//        Ogre::Real dotProduct = distanceVectorHand.dotProduct(distanceVectorArm);
//        if(dotProduct > dotProductMax) dotProductMax = dotProduct;
//        if(dotProduct < dotProductMin) dotProductMin = dotProduct;
//        /*leftNode->setOrientation(leftNode->convertWorldToLocalOrientation(kinectPosBefore.getRotationTo(kinectPos[1])));*/

        printf("Lectura %d\n", lectura);
        printf("Hand to Scene %d Point X: %f, Point Y: %f, Point Z: %f\n",1, handScenePos.x, handScenePos.y, handScenePos.z);
        printf("Arm (Wrist to Scene) %d Point X: %f, Point Y: %f, Point Z: %f\n",1, armScenePos.x, armScenePos.y, armScenePos.z);
        printf("Elbow %d Point X: %f, Point Y: %f, Point Z: %f\n",1, elbowPos[1].x, elbowPos[1].y, elbowPos[1].z);
        printf("Wrist %d Point X: %f, Point Y: %f, Point Z: %f\n",1, wristPos[1].x, wristPos[1].y, wristPos[1].z);
        printf("Hand %d Point X: %f, Point Y: %f, Point Z: %f\n",1, handPos[1].x, handPos[1].y, handPos[1].z);
        //printf("Dot Product X: %e\n",dotProduct );
        printf("Distance Arm %d Point X: %f, Point Y: %f, Point Z: %f\n",1, distanceVectorArm.x, distanceVectorArm.y, distanceVectorArm.z);
        printf("Distance Hand %d Point X: %f, Point Y: %f, Point Z: %f\n",1, distanceVectorHand.x, distanceVectorHand.y, distanceVectorHand.z);
        printf("Distance Wrist-Elbow: %.2f\n", distanceWristElbow);
        printf("Magnitude Arm: %.2f\n", distanceVectorArm.length());
        //printf("Wrist First %d Point X: %f, Point Y: %f, Point Z: %f\n",1, wristPosFirst.x, wristPosFirst.y, wristPosFirst.z);
        //printf("NinjaNode Original X: %f, Y: %f, Z: %f\n", mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().x, mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().y, mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().z);
        //printf("NinjaNode Copia X: %f, Y: %f, Z: %f\n", ninjaPosOriginal.x, ninjaPosOriginal.y, ninjaPosOriginal.z);
        //printf("Wrist Angle: %f\n", wristPitchAngleFirst.angleBetween(wristPitchAngle));
        //armNode->setOrientation(armPos[1].getRotationTo(distanceVector)*armNode->getOrientation());

//        if (lectura > 200){
//            Ogre::Real normal = dotProduct/(dotProductMax+dotProductMin);
//            printf("Normal: %.2f\n", normal);
//        }

        // MOVER NINJA
//        if (((round(handNode->getPosition().x/100)*100) == (round(ninjaPosOriginal.x/100)*100)) && ((round(handNode->getPosition().z/100)*100) == (round(ninjaPosOriginal.z/100)*100))){
//            ninjaPosOriginal = ninjaPosOriginal + (handScenePos - handPosBefore);
//            ninjaNode->setPosition(ninjaPosOriginal);
//            printf("Toque!\n");
//        }

        // FIN MOVER NINJA

        // PRUEBA MOVER COLUMNA ORIENTACION BRAZO

//        Ogre::Vector3 distanceVectorArmBefore = kinectWristNode->getPosition() - elbowPosFirst;
//        ninjaNode->pitch(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getPitch());
//        ninjaNode->roll(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getRoll());
//        //ninjaNode->setPosition(armScenePos);

        // FIN PRUEBA

        Ogre::Vector3 distanceVectorArmBefore = kinectWristNode->getPosition() - elbowPosFirst;
        armNode->pitch(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getPitch());
        armNode->roll(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getRoll());

        handPosBefore = handScenePos;
        handScenePos = 0;
        wristPosBefore = armScenePos;
        armScenePos = 0;
        elbowPosFirst = elbowPos[1];

    }

    return ret;
}

#ifdef __cplusplus
extern "C" {
#endif

    int main(int argc, char *argv[])
    {
        // Create application object*/
        BasicTutorial3 app;

//        const char *fn = NULL;
//        if (fileExists(SAMPLE_XML_PATH_LOCAL)) fn = SAMPLE_XML_PATH_LOCAL;
//        else {
//            printf("Could not find '%s'. Aborting.\n" , SAMPLE_XML_PATH_LOCAL);
//            //return XN_STATUS_ERROR;
//        }
//        printf("Reading config from: '%s'\n", fn);
//        nRetVal = context_hand.InitFromXmlFile(fn, scriptNode, &errors);


//        if (nRetVal == XN_STATUS_NO_NODE_PRESENT)
//        {
//            XnChar strError[1024];
//            errors.ToString(strError, 1024);
//            printf("%s\n", strError);
//            return (nRetVal);
//        }
//        else if (nRetVal != XN_STATUS_OK)
//        {
//            printf("Open failed: %s\n", xnGetStatusString(nRetVal));
//            return (nRetVal);
//        }

        //std::thread t1(kinectTask);
        std::thread t2(elbowTask);

        try {
            isOpen = true;
            app.go();
            //t1.join();
            //t2.join();
        } catch( Ogre::Exception& e ) {
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
        }
        return 0;

    }

#ifdef __cplusplus
}
#endif
