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

#define WIND
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5



// this pattern updates the scenenode position when it changes within the bullet simulation
// taken from BulletMotionState docs page24
class MyMotionState : public btMotionState {
public:
    MyMotionState(const btTransform &initialpos, Ogre::SceneNode *node) {
        mVisibleobj = node;
        mPos1 = initialpos;
    }
    virtual ~MyMotionState() {    }
    void setNode(Ogre::SceneNode *node) {
        mVisibleobj = node;
    }
    virtual void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = mPos1;
    }
    virtual void setWorldTransform(const btTransform &worldTrans) {
        if(NULL == mVisibleobj) return; // silently return before we set a node
        btQuaternion rot = worldTrans.getRotation();
        mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        btVector3 pos = worldTrans.getOrigin();
        // TODO **** XXX need to fix this up such that it renders properly since this doesnt know the scale of the node
        // also the getCube function returns a cube that isnt centered on Z
        mVisibleobj->setPosition(pos.x(), pos.y()+5, pos.z()-5);
    }
protected:
    Ogre::SceneNode *mVisibleobj;
    btTransform mPos1;
};

class MyCollisionObject : public btCollisionObject {
public:
    MyCollisionObject(const btTransform &initialpos, Ogre::SceneNode *node){
        mVisibleobj = node;
        mPos1 = initialpos;
        setWorldTransform(initialpos);
    }
    virtual ~MyCollisionObject() {    }
    void setNode(Ogre::SceneNode *node) {
        mVisibleobj = node;
    }
    virtual void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = mPos1;
    }
    virtual void setWorldTransform(const btTransform &worldTrans) {
        if(NULL == mVisibleobj) return; // silently return before we set a node
        btQuaternion rot = worldTrans.getRotation();
        mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        btVector3 pos = worldTrans.getOrigin();
        // TODO **** XXX need to fix this up such that it renders properly since this doesnt know the scale of the node
        // also the getCube function returns a cube that isnt centered on Z
        mVisibleobj->setPosition(pos.x(), pos.y()+5, pos.z()-5);
    }
protected:
    Ogre::SceneNode *mVisibleobj;
    btTransform mPos1;
};

class MyKinematicMotionState : public btMotionState {
public:
    MyKinematicMotionState(const btTransform &initialpos, Ogre::SceneNode *node) {
        mVisibleobj = node;
        mPos1 = initialpos;
    }
    virtual ~MyKinematicMotionState() {    }
    void setNode(Ogre::SceneNode *node) {
        mVisibleobj = node;
    }
    virtual void getWorldTransform(btTransform &worldTrans) const {
        worldTrans = mPos1;
    }
    void setKinematicPos(btTransform &currentPos) {
        mPos1 = currentPos;
        btQuaternion rot = currentPos.getRotation();
        mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        btVector3 pos = currentPos.getOrigin();
        mVisibleobj->setPosition(pos.x(), pos.y()+5, pos.z()-5);

    }
    virtual void setWorldTransform(const btTransform &worldTrans) {
        if(NULL == mVisibleobj) return; // silently return before we set a node
        btQuaternion rot = worldTrans.getRotation();
        mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
        btVector3 pos = worldTrans.getOrigin();
        // TODO **** XXX need to fix this up such that it renders properly since this doesnt know the scale of the node
        // also the getCube function returns a cube that isnt centered on Z
        mVisibleobj->setPosition(pos.x(), pos.y()+5, pos.z()-5);
    }
protected:
    Ogre::SceneNode *mVisibleobj;
    btTransform mPos1;
};


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
Ogre::Vector3 leftHandPos[6];
Ogre::Vector3 leftHandPosFirst;
Ogre::Vector3 leftHandPosBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 leftHandPosOriginal;
Ogre::Vector3 leftWristPos[6];
Ogre::Vector3 leftWristPosFirst;
Ogre::Vector3 leftElbowPos[6];
Ogre::Vector3 leftElbowPosFirst;
Ogre::Vector3 leftWristPosBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 leftWristPosOriginal;

Ogre::Vector3 ninjaPosOriginal;
Ogre::Vector3 distanceVectorTotalBefore = Ogre::Vector3(0,0,0);
Ogre::Vector3 distanceLeftVectorTotalBefore = Ogre::Vector3(0,0,0);
Ogre::Real dotProductMax = 0;
Ogre::Real dotProductMin = 9999999999999999;
Forests::PagedGeometry *trees;
Ogre::SceneNode *node;
Ogre::SceneNode *cubeNodes[16];
btRigidBody* armbRight;
btRigidBody* armbLeft;



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
    //fflush(stdout);


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
            handPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_HAND].pointZ));
            wristPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_WRIST].pointZ));
            elbowPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointX), trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointY), trunc(bodyCoords[1][XN_SKEL_RIGHT_ELBOW].pointZ));
            leftHandPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_LEFT_HAND].pointX), trunc(bodyCoords[1][XN_SKEL_LEFT_HAND].pointY), trunc(bodyCoords[1][XN_SKEL_LEFT_HAND].pointZ));
            leftWristPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_LEFT_WRIST].pointX), trunc(bodyCoords[1][XN_SKEL_LEFT_WRIST].pointY), trunc(bodyCoords[1][XN_SKEL_LEFT_WRIST].pointZ));
            leftElbowPos[1] = Ogre::Vector3(trunc(bodyCoords[1][XN_SKEL_LEFT_ELBOW].pointX), trunc(bodyCoords[1][XN_SKEL_LEFT_ELBOW].pointY), trunc(bodyCoords[1][XN_SKEL_LEFT_ELBOW].pointZ));
        }
    }


    depth_el.Release();
    scriptNode.Release();
    context_elbow.Release();

    return 0;
}

Ogre::ManualObject* createCubeMesh(Ogre::String name, Ogre::String matName) {

   Ogre::ManualObject* cube = new Ogre::ManualObject(name);

   cube->begin(matName);

   cube->position(0.5,-0.5,1.0);cube->normal(0.408248,-0.816497,0.408248);cube->textureCoord(1,0);
   cube->position(-0.5,-0.5,0.0);cube->normal(-0.408248,-0.816497,-0.408248);cube->textureCoord(0,1);
   cube->position(0.5,-0.5,0.0);cube->normal(0.666667,-0.333333,-0.666667);cube->textureCoord(1,1);
   cube->position(-0.5,-0.5,1.0);cube->normal(-0.666667,-0.333333,0.666667);cube->textureCoord(0,0);
   cube->position(0.5,0.5,1.0);cube->normal(0.666667,0.333333,0.666667);cube->textureCoord(1,0);
   cube->position(-0.5,-0.5,1.0);cube->normal(-0.666667,-0.333333,0.666667);cube->textureCoord(0,1);
   cube->position(0.5,-0.5,1.0);cube->normal(0.408248,-0.816497,0.408248);cube->textureCoord(1,1);
   cube->position(-0.5,0.5,1.0);cube->normal(-0.408248,0.816497,0.408248);cube->textureCoord(0,0);
   cube->position(-0.5,0.5,0.0);cube->normal(-0.666667,0.333333,-0.666667);cube->textureCoord(0,1);
   cube->position(-0.5,-0.5,0.0);cube->normal(-0.408248,-0.816497,-0.408248);cube->textureCoord(1,1);
   cube->position(-0.5,-0.5,1.0);cube->normal(-0.666667,-0.333333,0.666667);cube->textureCoord(1,0);
   cube->position(0.5,-0.5,0.0);cube->normal(0.666667,-0.333333,-0.666667);cube->textureCoord(0,1);
   cube->position(0.5,0.5,0.0);cube->normal(0.408248,0.816497,-0.408248);cube->textureCoord(1,1);
   cube->position(0.5,-0.5,1.0);cube->normal(0.408248,-0.816497,0.408248);cube->textureCoord(0,0);
   cube->position(0.5,-0.5,0.0);cube->normal(0.666667,-0.333333,-0.666667);cube->textureCoord(1,0);
   cube->position(-0.5,-0.5,0.0);cube->normal(-0.408248,-0.816497,-0.408248);cube->textureCoord(0,0);
   cube->position(-0.5,0.5,1.0);cube->normal(-0.408248,0.816497,0.408248);cube->textureCoord(1,0);
   cube->position(0.5,0.5,0.0);cube->normal(0.408248,0.816497,-0.408248);cube->textureCoord(0,1);
   cube->position(-0.5,0.5,0.0);cube->normal(-0.666667,0.333333,-0.666667);cube->textureCoord(1,1);
   cube->position(0.5,0.5,1.0);cube->normal(0.666667,0.333333,0.666667);cube->textureCoord(0,0);

   cube->triangle(0,1,2);      cube->triangle(3,1,0);
   cube->triangle(4,5,6);      cube->triangle(4,7,5);
   cube->triangle(8,9,10);      cube->triangle(10,7,8);
   cube->triangle(4,11,12);   cube->triangle(4,13,11);
   cube->triangle(14,8,12);   cube->triangle(14,15,8);
   cube->triangle(16,17,18);   cube->triangle(16,19,17);
   cube->end();

   return cube;
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

    mCamera->setPosition(Ogre::Vector3(1683, 300, 3000));
    mCamera->lookAt(Ogre::Vector3(1963, 50, 1660));
    mCamera->setNearClipDistance(0.1);
    mCamera->setFarClipDistance(50000);

    if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
    {
        mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
    }

    if (mRoot->getRenderSystem()->getCapabilities()->hasCapability(Ogre::RSC_VERTEX_PROGRAM)){
        printf("Vertex Shaders Available\n");
    }
    else{
        printf("Vertex Shaders Unavailable\n");
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
    //Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
    //Ogre::MeshManager::getSingleton().createPlane("ground",Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,1500,1500,20,20,true,1,5,5,Ogre::Vector3::UNIT_Z);


    //mSceneMgr->setSkyBox(true, "Examples/SpaceSkyBox", 5000, false);
    mSceneMgr->setSkyDome(true, "Examples/CloudySky", 5, 8);

    //// WILLOW

//    Ogre::Entity *ent = mSceneMgr->createEntity("Ninja", "stm_37.mesh");
//    node = mSceneMgr->getRootSceneNode()->createChildSceneNode("NinjaNode");
//    ent->setCastShadows(true);
//    node->attachObject(ent);

//    //node->setPosition(1683, 300, 1660);
//    //node->setPosition(0, 0, 0);
//    //node->pitch(Ogre::Degree(90));

//    node->setScale(100,100,100);

//    ninjaPosOriginal = node->getPosition();

    //Ogre::VertexData vertexData = ent->getVertexDataForBinding();

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

    //// PAGEDGEOMETRY TEST

//    trees = new Forests::PagedGeometry();
//    trees->setCamera(mCamera);   //Set the camera so PagedGeometry knows how to calculate LODs
//    trees->setPageSize(50);   //Set the size of each page of geometry
//    trees->setInfinite();      //Use infinite paging mode
//    trees->addDetailLevel<Forests::BatchPage>(90, 30);      //Use batches up to 150 units away, and fade for 30 more units
//    trees->addDetailLevel<Forests::ImpostorPage>(700, 50);   //Use impostors up to 400 units, and for for 50 more units

//    //Create a new TreeLoader2D object
//    Forests::TreeLoader2D *treeLoader = new Forests::TreeLoader2D(trees, Forests::TBounds(0, 0, 1500, 1500));
//    treeLoader->addTree(ent , node->getPosition()); // pos, yaw, scale of myEntity
//    trees->setCustomParam(ent->getName(),"windFactorX", 10000 / 1);
//    trees->setCustomParam(ent->getName(),"windFactorY", 0.0005 / 1);

    //// LINE

//    Ogre::ManualObject* myManualObject =  mSceneMgr->createManualObject("manual1");
//    Ogre::SceneNode* myManualObjectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("manual1_node");

//    // NOTE: The second parameter to the create method is the resource group the material will be added to.
//    // If the group you name does not exist (in your resources.cfg file) the library will assert() and your program will crash
//    Ogre::MaterialPtr myManualObjectMaterial = Ogre::MaterialManager::getSingleton().create("manual1Material","General");
//    myManualObjectMaterial->setReceiveShadows(false);
//    myManualObjectMaterial->getTechnique(0)->setLightingEnabled(true);
//    myManualObjectMaterial->getTechnique(0)->getPass(0)->setDiffuse(0,0,0,0);
//    myManualObjectMaterial->getTechnique(0)->getPass(0)->setAmbient(0,0,0);
//    myManualObjectMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(0,0,0);


//    myManualObject->begin("manual1Material", Ogre::RenderOperation::OT_LINE_LIST);
//    myManualObject->position(1683, 0, 1660);

//    //myManualObject->position(1683, 1, 2116);
//    myManualObject->position(1683, 1000, 1660);

//    // etc
//    myManualObject->end();

//    myManualObjectNode->attachObject(myManualObject);

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

    //// CUBES

    Ogre::ManualObject *cmo = createCubeMesh("manual", "");
    cmo->convertToMesh("cube");

    for (int i = 0; i < 16; i++){

        std::stringstream ss;
        ss << "Cube " << i;
        //std::string s = ss.str();
        Ogre::Entity *cubeEntity = mSceneMgr->createEntity(ss.str(), "cube.mesh");
        cubeEntity->setCastShadows(true);
        Ogre::SceneNode *cubeNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        cubeNode->attachObject(cubeEntity);
        cubeNode->setScale(Ogre::Vector3(1,1,1)); // for some reason converttomesh multiplied dimensions by 10
        cubeNode->pitch(Ogre::Degree(90));
        cubeNodes[i] = cubeNode;
    }


    //// RIGHT HAND

    //    Ogre::Real distanceNinjaCamera = node->getPosition().distance(Ogre::Vector3(1683,1,2116));
    //    printf("Distance Ninja-Camera: %.0f\n", distanceNinjaCamera);

    Ogre::Entity *handEnt = mSceneMgr->createEntity("RightHand", "hand.mesh");

    Ogre::SceneNode *handNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("RightHandNode");
    handEnt->setCastShadows(true);
    handNode->attachObject(handEnt);
    handNode->setScale(40.2, 40.2, 40.2);

    //node2->translate(1800, 50, 1660);
    //leftNode->translate(2050,10,2300);
    handNode->translate(1752, 6, 2018);
    handNode->yaw(Ogre::Degree(90));
    //handNode->pitch(Ogre::Degree(90));
    handNode->roll(Ogre::Degree(-90));


    Ogre::Entity *armEnt = mSceneMgr->createEntity("Arm", "column.mesh");
    Ogre::SceneNode *armNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("ArmNode");

    armEnt->setCastShadows(true);
    armNode->attachObject(armEnt);
    //rightNode->setScale(40.2, 40.2, 40.2);

    armNode->setPosition(1750, 100, 2542);
    //armNode->pitch(Ogre::Degree(133));
    //armNode->roll(Ogre::Degree(20));
    armNode->setScale(1,1,1);

    // pEntNode is the node the Entity is attached to
    // entPos is the current position you're using to place the entity

    Ogre::SceneNode* parent = mSceneMgr->getRootSceneNode()->createChildSceneNode("ParentArmNode");
    // place this node 20 units above where the current entity centre is
    parent->setPosition(armNode->getPosition() + Ogre::Vector3(0,-100,0));
    // re-parent the entity node
    if (armNode->getParent()){
        armNode->getParent()->removeChild(armNode);
        handNode->getParent()->removeChild(handNode);
    }
    parent->addChild(armNode);
    parent->addChild(handNode);
    // now offset the entity node relative to this parent to put it back in its original place
    armNode->setPosition(Ogre::Vector3(0,100,0));
    handNode->setPosition(2,524,11);
    parent->pitch(Ogre::Degree(-90));

    //parent->pitch(Ogre::Degree(-90));
    //parent->roll(Ogre::Degree(20));

    handPosOriginal = handNode->getPosition();
    wristPosOriginal = parent->getPosition();
//    wristPosOriginal = armNode->getPosition();
    //elbowPosOriginal = parent->getPosition();

    //// LEFT HAND

    Ogre::Entity *leftHandEnt = mSceneMgr->createEntity("LeftHand", "left_hand.mesh");

    Ogre::SceneNode *leftHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LeftHandNode");
    leftHandEnt->setCastShadows(true);
    leftHandNode->attachObject(leftHandEnt);
    leftHandNode->setScale(0.5,0.5, 0.5);

    //node2->translate(1800, 50, 1660);
    //leftNode->translate(2050,10,2300);
    //leftHandNode->translate(1752, 6, 2138);
    //leftHandNode->yaw(Ogre::Degree(90));
    leftHandNode->pitch(Ogre::Degree(-70));
    leftHandNode->roll(Ogre::Degree(70));


    Ogre::Entity *leftArmEnt = mSceneMgr->createEntity("LeftArm", "column.mesh");
    Ogre::SceneNode *leftArmNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("LeftArmNode");

    leftArmEnt->setCastShadows(true);
    leftArmNode->attachObject(leftArmEnt);
    //rightNode->setScale(40.2, 40.2, 40.2);

    leftArmNode->setPosition(1650, 100, 2542);
    //armNode->pitch(Ogre::Degree(133));
    //armNode->roll(Ogre::Degree(20));
    leftArmNode->setScale(1,1,1);

    // pEntNode is the node the Entity is attached to
    // entPos is the current position you're using to place the entity

    Ogre::SceneNode* leftParent = mSceneMgr->getRootSceneNode()->createChildSceneNode("ParentLeftArmNode");
    // place this node 20 units above where the current entity centre is
    leftParent->setPosition(leftArmNode->getPosition() + Ogre::Vector3(0,-100,0));
    // re-parent the entity node
    if (leftArmNode->getParent()){
        leftArmNode->getParent()->removeChild(leftArmNode);
        leftHandNode->getParent()->removeChild(leftHandNode);
    }
    leftParent->addChild(leftArmNode);
    leftParent->addChild(leftHandNode);
    // now offset the entity node relative to this parent to put it back in its original place
    leftArmNode->setPosition(Ogre::Vector3(0,100,0));
    leftHandNode->setPosition(-7,460,30);
    leftParent->pitch(Ogre::Degree(-90));

    leftHandPosOriginal = leftHandNode->getPosition();
    leftWristPosOriginal = leftParent->getPosition();


    Ogre::SceneNode *kinectElbowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectElbow");
    Ogre::SceneNode *kinectWristNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectWrist");
    Ogre::SceneNode *kinectHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectHand");

    Ogre::SceneNode *kinectLeftElbowNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectLeftElbow");
    Ogre::SceneNode *kinectLeftWristNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectLeftWrist");
    Ogre::SceneNode *kinectLeftHandNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("KinectLeftHand");

    createBulletSim();

}
//-------------------------------------------------------------------------------------
void BasicTutorial3::createFrameListener(void)
{
    BaseApplication::createFrameListener();
    mTrayMgr->hideAll();
    printf("createFrameListener\n");

    fflush(stdout);

    //mInfoLabel = mTrayMgr->createLabel(OgreBites::TL_TOP, "TInfo", "", 350);
}

void BasicTutorial3::createBulletSim(void) {
    //collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    collisionConfiguration = new btDefaultCollisionConfiguration();

    //use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    dispatcher = new   btCollisionDispatcher(collisionConfiguration);

    //btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    overlappingPairCache = new btDbvtBroadphase();

    //the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,overlappingPairCache,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-100,0));
    //dynamicsWorld->setGravity(btVector3(0,0,0));

    //create a few basic rigid bodies
    // start with ground plane, 1500, 1500
    btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(5000.),btScalar(1.),btScalar(5000.)));

    collisionShapes.push_back(groundShape);

    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0,0,0));

    {
        btScalar mass(0.);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0,0,0);
        if (isDynamic)
        groundShape->calculateLocalInertia(mass,localInertia);

        // lathe - this plane isnt going to be moving so i dont care about setting the motion state
        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);
    }

    {

        btVector3 x(-ARRAY_SIZE_X, 8.0f,-20.f);
        btVector3 y;
        btVector3 deltaX(50,100, 0.f);
        btVector3 deltaY(100, 0.0f,0.f);
        int z = 0;

        for (int i = 0; i < ARRAY_SIZE_X; ++i)
        {
            y = x;

            for (int  j = i; j < ARRAY_SIZE_Y; ++j)
            {
                addDynamicRigidBody(cubeNodes[z], btVector3(1683, 0, 1660) + y);
                y += deltaY;
                z++;
            }

            x += deltaX;
        }

    }

    addDynamicRigidBody(cubeNodes[15], btVector3(1720, 300, 2000) );
    //addDinamicRigidBody(mSceneMgr->getSceneNode("ParentArmNode"), btVector3(1683, 0, 1660) );
    //dynamicsWorld->addCollisionObject();

    {
        Ogre::Node *rightArmNode =  mSceneMgr->getRootSceneNode()->getChild("ParentArmNode");
        Ogre::Node *rightHandNode = rightArmNode->getChild("RightHandNode");
        Ogre::Vector3 handPos = rightArmNode->getPosition();

        printf("Hand Node Derived Position (%d, %d, %d)\n", handPos.x,  handPos.y,  handPos.z);

        //btCollisionShape *mPlayerBox = new btBoxShape(btVector3(25,1,600));
        btCollisionShape *mPlayerBox = new btCylinderShape(btVector3(35,600,35));
        collisionShapes.push_back(mPlayerBox);
        btTransform playerWorld;
        playerWorld.setIdentity();
        //playerPos is a D3DXVECTOR3 that holds the camera position.
        //playerWorld.setOrigin(btVector3(handPos.x, handPos.y, handPos.z));
        playerWorld.setOrigin(btVector3(1750, 100, 2542));
        playerWorld.setRotation(btQuaternion(0.0,-1.6,0.0));
//        btCollisionObject *mPlayerObject = new MyCollisionObject(playerWorld, mSceneMgr->getSceneNode("ParentArmNode"));
//        //btCollisionObject *mPlayerObject = new btCollisionObject();
//        btCollisionObject::
//        mPlayerObject->setWorldTransform(playerWorld);
//        mPlayerObject->setCollisionShape(mPlayerBox);
//        mPlayerObject->forceActivationState(DISABLE_DEACTIVATION);//maybe not needed
//        dynamicsWorld->addCollisionObject(mPlayerObject);
        btScalar   mass(1.f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0,0,-1.0);
        if (isDynamic)
        mPlayerBox->calculateLocalInertia(mass,localInertia);

        MyKinematicMotionState* motionState = new MyKinematicMotionState(playerWorld, mSceneMgr->getSceneNode("ParentArmNode"));
        //motionState->setNode();
        motionState->setWorldTransform(playerWorld);
        //btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,mPlayerBox,localInertia);
        armbRight = new btRigidBody(rbInfo);
        //body->setGravity(btVector3(0,-100,0));

        armbRight->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
        //arm->setActivationState( DISABLE_DEACTIVATION );

        printf("isKinematicObject?: %s\n", armbRight->isKinematicObject()? "True": "False");
        dynamicsWorld->addRigidBody(armbRight);
    }

    {

        Ogre::Node *leftArmNode =  mSceneMgr->getRootSceneNode()->getChild("ParentArmNode");
        Ogre::Node *leftHandNode = leftArmNode->getChild("RightHandNode");
        Ogre::Vector3 leftHandPos = leftArmNode->getPosition();

        printf("Hand Node Derived Position (%d, %d, %d)\n", leftHandPos.x,  leftHandPos.y,  leftHandPos.z);

        //btCollisionShape *mPlayerBox = new btBoxShape(btVector3(50,50,50));
        btCollisionShape *mPlayerBox = new btCylinderShape(btVector3(35,700,100));
        collisionShapes.push_back(mPlayerBox);
        btTransform playerWorld;
        playerWorld.setIdentity();
        //playerPos is a D3DXVECTOR3 that holds the camera position.
        //playerWorld.setOrigin(btVector3(handPos.x, handPos.y, handPos.z));
        playerWorld.setOrigin(btVector3(1650, 100, 2542));
        playerWorld.setRotation(btQuaternion(0.0,-1.6,0.0));
//        btCollisionObject *mPlayerObject = new MyCollisionObject(playerWorld, mSceneMgr->getSceneNode("ParentArmNode"));
//        //btCollisionObject *mPlayerObject = new btCollisionObject();
//        btCollisionObject::
//        mPlayerObject->setWorldTransform(playerWorld);
//        mPlayerObject->setCollisionShape(mPlayerBox);
//        mPlayerObject->forceActivationState(DISABLE_DEACTIVATION);//maybe not needed
//        dynamicsWorld->addCollisionObject(mPlayerObject);
        btScalar   mass(1.f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0,0,-1.0);
        if (isDynamic)
        mPlayerBox->calculateLocalInertia(mass,localInertia);

        MyKinematicMotionState* motionState = new MyKinematicMotionState(playerWorld, mSceneMgr->getSceneNode("ParentLeftArmNode"));
        //motionState->setNode();
        motionState->setWorldTransform(playerWorld);
        //btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,mPlayerBox,localInertia);
        armbLeft = new btRigidBody(rbInfo);
        //body->setGravity(btVector3(0,-100,0));

        armbLeft->setCollisionFlags(btCollisionObject::CF_KINEMATIC_OBJECT);
        //arm->setActivationState( DISABLE_DEACTIVATION );

        printf("isKinematicObject?: %s\n", armbLeft->isKinematicObject()? "True": "False");
        dynamicsWorld->addRigidBody(armbLeft);


    }
}

void BasicTutorial3::addDynamicRigidBody(Ogre::SceneNode *bodyNode, btVector3 position){
    //create a dynamic rigidbody

    btCollisionShape* colShape = new btBoxShape(btVector3(50,50,50));
    //btCollisionShape* colShape = new btConeShape(100, 0);
    //btCollisionShape* colShape = new btSphereShape(btScalar(1.));
    collisionShapes.push_back(colShape);

    // Create Dynamic Objects
    btTransform startTransform;
    startTransform.setIdentity();

    btScalar   mass(1.f);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);


    btVector3 localInertia(0,0,-1.0);
    if (isDynamic)
    colShape->calculateLocalInertia(mass,localInertia);

    //startTransform.setOrigin(btVector3(0,200,0));
    startTransform.setOrigin(position);
    // *** give it a slight twist so it bouncees more interesting
    //startTransform.setRotation(btQuaternion(btVector3(1.0, 1.0, 0.0), 0.6));

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    //btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
    //Ogre::SceneNode *cubeNode = mSceneMgr->getSceneNode("CubeNode");

    MyMotionState* motionState = new MyMotionState(startTransform, bodyNode);
    //btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,motionState,colShape,localInertia);
    btRigidBody* body = new btRigidBody(rbInfo);

    dynamicsWorld->addRigidBody(body);
}

//void BasicTutorial3::createPGDemo(void)
//{

//    //-------------------------------------- LOAD TREES --------------------------------------
//    //Create and configure a new PagedGeometry instance
//    trees = new Forests::PagedGeometry();
//    trees->setCamera(mCamera);	//Set the camera so PagedGeometry knows how to calculate LODs
//    trees->setPageSize(50);	//Set the size of each page of geometry
//    trees->setInfinite();		//Use infinite paging mode
//    trees->setShadersEnabled(true);

//#ifdef WIND
//    //WindBatchPage is a variation of BatchPage which includes a wind animation shader
//    trees->addDetailLevel<Forests::WindBatchPage>(70, 30);		//Use batches up to 70 units away, and fade for 30 more units
//#else
//    trees->addDetailLevel<Forests::BatchPage>(70, 30);		//Use batches up to 70 units away, and fade for 30 more units
//#endif
//    trees->addDetailLevel<Forests::ImpostorPage>(5000, 50);	//Use impostors up to 400 units, and for for 50 more units

//    //Create a new TreeLoader2D object
//    Forests::TreeLoader2D *treeLoader = new Forests::TreeLoader2D(trees, Forests::TBounds(0, 0, 1500, 1500));
//    trees->setPageLoader(treeLoader);	//Assign the "treeLoader" to be used to load geometry for the PagedGeometry instance

//    //Supply a height function to TreeLoader2D so it can calculate tree Y values
////    HeightFunction::initialize(mSceneMgr);
////    treeLoader->setHeightFunction(&HeightFunction::getTerrainHeight);

//    //[NOTE] This sets the color map, or lightmap to be used for trees. All trees will be colored according
//    //to this texture. In this case, the shading of the terrain is used so trees will be shadowed
//    //just as the terrain is (this should appear like the terrain is casting shadows on the trees).
//    //You may notice that TreeLoader2D / TreeLoader3D doesn't have a setMapBounds() function as GrassLoader
//    //does. This is because the bounds you specify in the TreeLoader2D constructor are used to apply
//    //the color map.
//    treeLoader->setColorMap("terrain_lightmap.jpg");

//    //Load a tree entity
//    Ogre::Entity *tree1 = mSceneMgr->createEntity("Tree1", "fir06_30.mesh");
//    //Ogre::Entity *tree1 = mSceneMgr->createEntity("Tree1", "leaves2.mesh");


//    Ogre::Entity *tree2 = mSceneMgr->createEntity("Tree2", "fir14_25.mesh");
//    Ogre::Entity *tree3 = mSceneMgr->getEntity("Ninja");
//    Ogre::Node *ninjaNode = mSceneMgr->getRootSceneNode()->getChild("NinjaNode");



//#ifdef WIND
//    trees->setCustomParam(tree1->getName(), "windFactorX", 15);
//    trees->setCustomParam(tree1->getName(), "windFactorY", 0.01);
//    trees->setCustomParam(tree2->getName(), "windFactorX", 22);
//    trees->setCustomParam(tree2->getName(), "windFactorY", 0.013);

//    trees->setCustomParam("Ninja", "windFactorX", 100);
//    trees->setCustomParam("Ninja", "windFactorY", 0.100);
//#endif

//    //Randomly place 10000 copies of the tree on the terrain
//    Ogre::Vector3 position = Ogre::Vector3::ZERO;
//    Ogre::Radian yaw;
//    Ogre::Real scale;
//    for (int i = 0; i < 60000; i++){
//        yaw = Ogre::Degree(Ogre::Math::RangeRandom(0, 360));

//        position.x = Ogre::Math::RangeRandom(0, 1500);
//        position.z = Ogre::Math::RangeRandom(0, 1500);

//        scale = Ogre::Math::RangeRandom(0.07f, 0.12f);

//        //[NOTE] Unlike TreeLoader3D, TreeLoader2D's addTree() function accepts a Vector2D position (x/z)
//        //The Y value is calculated during runtime (to save memory) from the height function supplied (above)
//        if (Ogre::Math::UnitRandom() < 0.5f)
//            treeLoader->addTree(tree1, position, yaw, scale);
//        else
//            treeLoader->addTree(tree2, position, yaw, scale);
//    }
//    //treeLoader->addTree(tree3, ninjaNode->getPosition(), ninjaNode->getOrientation().getYaw(), 200);
//    //addPG(trees);

//}
//-------------------------------------------------------------------------------------
bool BasicTutorial3::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    bool ret = BaseApplication::frameRenderingQueued(evt);
    //trees->update();
//    btTransform playerWorld;
//    playerWorld = arm->getWorldTransform();
//    //playerWorld.setIdentity();
////    //playerPos is a D3DXVECTOR3 that holds the camera position.
////    //playerWorld.setOrigin(btVector3(handPos.x, handPos.y, handPos.z));
////    playerWorld.setOrigin(btVector3(1750, 100, 2542));
////    playerWorld.setRotation(btQuaternion(0.0,-1.6,0.0));
//    MyKinematicMotionState *motionState = new MyKinematicMotionState(playerWorld, mSceneMgr->getSceneNode("ParentArmNode"));
//    motionState->setKinematicPos(playerWorld);
//    //arm->setMotionState(motionState);
    dynamicsWorld->stepSimulation(evt.timeSinceLastFrame,50);


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
            leftHandPosFirst = leftHandPos[1];
            leftWristPosFirst = leftWristPos[1];
            leftElbowPosFirst = leftElbowPos[1];
        }


        //Ogre::Node *ninjaNode = mSceneMgr->getRootSceneNode()->getChild("NinjaNode");
        //Ogre::Node *handNode = mSceneMgr->getRootSceneNode()->getChild("RightHandNode");
        //Ogre::Node *armNode =  mSceneMgr->getRootSceneNode()->getChild("Arm");
        Ogre::Node *armNode =  mSceneMgr->getRootSceneNode()->getChild("ParentArmNode");
        Ogre::Node *handNode = armNode->getChild("RightHandNode");
        Ogre::Node *leftArmNode =  mSceneMgr->getRootSceneNode()->getChild("ParentLeftArmNode");
        Ogre::Node *leftHandNode = leftArmNode->getChild("LeftHandNode");

        Ogre::Node *kinectElbowNode = mSceneMgr->getRootSceneNode()->getChild("KinectElbow");
        Ogre::Node *kinectWristNode = mSceneMgr->getRootSceneNode()->getChild("KinectWrist");
        Ogre::Node *kinectHandNode = mSceneMgr->getRootSceneNode()->getChild("KinectHand");

        Ogre::Node *kinectLeftElbowNode = mSceneMgr->getRootSceneNode()->getChild("KinectLeftElbow");
        Ogre::Node *kinectLeftWristNode = mSceneMgr->getRootSceneNode()->getChild("KinectLeftWrist");
        Ogre::Node *kinectLeftHandNode = mSceneMgr->getRootSceneNode()->getChild("KinectLeftHand");

//        Ogre::Vector3 wristPitchAngle = Ogre::Vector3(0,wristPos[1].y,wristPos[1].z);
//        Ogre::Vector3 wristPitchAngleFirst = Ogre::Vector3(0,wristPosFirst.y,wristPosFirst.z);


        Ogre::Vector3 handScenePos = handPos[1] - handPosFirst;
        Ogre::Vector3 armScenePos = wristPos[1] - wristPosFirst;
        //Ogre::Vector3 armScenePos = elbowPos[1] - elbowPosFirst;

        Ogre::Vector3 leftHandScenePos = leftHandPos[1] - leftHandPosFirst;
        Ogre::Vector3 leftArmScenePos = leftWristPos[1] - leftWristPosFirst;

        //kinectPos[1] = Ogre::Quaternion(Ogre::Degree(200), Ogre::Vector3::UNIT_Y) * kinectPos[1];
        //handScenePos = handPosOriginal + handScenePos;
        handScenePos = wristPosOriginal + handScenePos;
        armScenePos = wristPosOriginal + armScenePos;

        leftHandScenePos = leftWristPosOriginal + leftHandScenePos;
        leftArmScenePos = leftWristPosOriginal + leftArmScenePos;

        //mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->setPosition(kinectPos[1]);
        //ninjaNode->setPosition(kinectPos[1]);

        kinectElbowNode->setPosition(elbowPos[1]);
        kinectWristNode->setPosition(wristPos[1]);
        kinectHandNode->setPosition(handPos[1]);

        kinectLeftElbowNode->setPosition(leftElbowPos[1]);
        kinectLeftWristNode->setPosition(leftWristPos[1]);
        kinectLeftHandNode->setPosition(leftHandPos[1]);

        Ogre::Vector3 distanceVectorArm = kinectWristNode->getPosition() - kinectElbowNode->getPosition();
        Ogre::Vector3 distanceVectorHand = kinectHandNode->getPosition() - kinectWristNode->getPosition();
        Ogre::Vector3 distanceVectorTotal = kinectHandNode->getPosition() - kinectElbowNode->getPosition();
        Ogre::Real distanceWristElbow = kinectWristNode->getPosition().distance(kinectElbowNode->getPosition());

        Ogre::Vector3 distanceLeftVectorTotal = kinectLeftHandNode->getPosition() - kinectLeftElbowNode->getPosition();

        //handNode->setPosition(handScenePos);
        //armNode->setPosition(armScenePos);

//        Ogre::Real dotProduct = distanceVectorHand.dotProduct(distanceVectorArm);
//        if(dotProduct > dotProductMax) dotProductMax = dotProduct;
//        if(dotProduct < dotProductMin) dotProductMin = dotProduct;
//        /*leftNode->setOrientation(leftNode->convertWorldToLocalOrientation(kinectPosBefore.getRotationTo(kinectPos[1])));*/

//        printf("Lectura %d\n", lectura);
//        printf("Arm to Scene %d Point X: %f, Point Y: %f, Point Z: %f\n",1, handScenePos.x, handScenePos.y, handScenePos.z);
//        printf("Hand to Scene %d, Point X: %f, Point Y: %f, Point Z: %f\n", 1, handNode->_getDerivedPosition().x, handNode->_getDerivedPosition().y, handNode->_getDerivedPosition().z);
//        printf("Arm (Wrist to Scene) %d Point X: %f, Point Y: %f, Point Z: %f\n",1, armScenePos.x, armScenePos.y, armScenePos.z);
//        printf("Elbow %d Point X: %f, Point Y: %f, Point Z: %f\n",1, elbowPos[1].x, elbowPos[1].y, elbowPos[1].z);
//        printf("Wrist %d Point X: %f, Point Y: %f, Point Z: %f\n",1, wristPos[1].x, wristPos[1].y, wristPos[1].z);
//        printf("Hand %d Point X: %f, Point Y: %f, Point Z: %f\n",1, handPos[1].x, handPos[1].y, handPos[1].z);
        //printf("Dot Product X: %e\n",dotProduct );
//        printf("Distance Arm %d Point X: %f, Point Y: %f, Point Z: %f\n",1, distanceVectorArm.x, distanceVectorArm.y, distanceVectorArm.z);
//        printf("Distance Hand %d Point X: %f, Point Y: %f, Point Z: %f\n",1, distanceVectorHand.x, distanceVectorHand.y, distanceVectorHand.z);
//        printf("Distance Wrist-Elbow: %.2f\n", distanceWristElbow);
//        printf("Magnitude Arm: %.2f\n", distanceVectorArm.length());
        //printf("Wrist First %d Point X: %f, Point Y: %f, Point Z: %f\n",1, wristPosFirst.x, wristPosFirst.y, wristPosFirst.z);
        //printf("NinjaNode Original X: %f, Y: %f, Z: %f\n", mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().x, mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().y, mSceneMgr->getRootSceneNode()->getChild("NinjaNode")->getPosition().z);
        //printf("NinjaNode Copia X: %f, Y: %f, Z: %f\n", ninjaPosOriginal.x, ninjaPosOriginal.y, ninjaPosOriginal.z);
        //printf("Wrist Angle: %f\n", wristPitchAngleFirst.angleBetween(wristPitchAngle));
        //armNode->setOrientation(armPos[1].getRotationTo(distanceVector)*armNode->getOrientation());

//        if (lectura > 200){
//            Ogre::Real normal = dotProduct/(dotProductMax+dotProductMin);
//            printf("Normal: %.2f\n", normal);
//        }

        //// MOVER NINJA
//        if (((round(handNode->_getDerivedPosition().x/100)*100) == (round(ninjaPosOriginal.x/100)*100)) && ((round(handNode->_getDerivedPosition().z/100)*100) == (round(ninjaPosOriginal.z/100)*100))){
//            ninjaPosOriginal = ninjaPosOriginal + (handNode->_getDerivedPosition() - handPosBefore);
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
        Ogre::Vector3 distanceLeftVectorArmBefore = kinectLeftWristNode->getPosition() - leftElbowPosFirst;

//        armNode->pitch(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getPitch());
//        armNode->roll(-distanceVectorArmBefore.getRotationTo(distanceVectorArm).getRoll());

        //// ORIGINAL
//        armNode->setPosition(handScenePos);
//        armNode->pitch(distanceVectorTotalBefore.getRotationTo(distanceVectorTotal).getPitch());
        //armNode->roll(distanceVectorTotalBefore.getRotationTo(distanceVectorTotal).getRoll());

        //leftArmNode->setPosition(leftHandScenePos);
        //leftArmNode->pitch(distanceLeftVectorTotalBefore.getRotationTo(distanceLeftVectorTotal).getPitch());
        ////FIN ORIGINAL


        //// PRUEBA BULLET
        Ogre::Quaternion rotRight = distanceVectorTotalBefore.getRotationTo(distanceVectorTotal);
        Ogre::Quaternion rotLeft = distanceLeftVectorTotalBefore.getRotationTo(distanceLeftVectorTotal);

        btTransform playerWorldRight;
        playerWorldRight = armbRight->getWorldTransform();

        btTransform playerWorldLeft;
        playerWorldLeft = armbLeft->getWorldTransform();

        btQuaternion quatBeforeRight = playerWorldRight.getRotation();
        printf("PlayerWorld Quaternion: %.2f, %.2f, %.2f, %.2f \n", quatBeforeRight.getX(), quatBeforeRight.getY(), quatBeforeRight.getZ(), quatBeforeRight.getW());

        btQuaternion quatBeforeLeft = playerWorldLeft.getRotation();
        printf("PlayerWorld Quaternion: %.2f, %.2f, %.2f, %.2f \n", quatBeforeLeft.getX(), quatBeforeLeft.getY(), quatBeforeLeft.getZ(), quatBeforeLeft.getW());

        playerWorldRight.setOrigin(btVector3(handScenePos.x, handScenePos.y, handScenePos.z));
        playerWorldLeft.setOrigin(btVector3(leftHandScenePos.x, leftHandScenePos.y, leftHandScenePos.z));

        Ogre::Quaternion quatPRight;
        quatPRight.FromAngleAxis(rotRight.getPitch(), Ogre::Vector3::UNIT_X);
//        printf("Pitch Angle Degrees: %.2f /", quatPRight.getPitch().valueDegrees());
//        printf("Roll Angle Degrees: %.2f /", quatPRight.getRoll().valueDegrees());
//        printf("Yaw Angle Degrees: %.2f \n", quatPRight.getYaw().valueDegrees());

//        printf("Ogre Right Quaternion: %.2f, %.2f, %.2f, %.2f \n", quatPRight.x, quatPRight.y, quatPRight.z, quatPRight.w);

        Ogre::Quaternion quatPLeft;
        quatPLeft.FromAngleAxis(rotLeft.getPitch(), Ogre::Vector3::UNIT_X);
        printf("Pitch Left Angle Degrees: %.2f /", quatPLeft.getPitch().valueDegrees());
        printf("Roll Left Angle Degrees: %.2f /", quatPLeft.getRoll().valueDegrees());
        printf("Yaw Left Angle Degrees: %.2f \n", quatPLeft.getYaw().valueDegrees());

        printf("Ogre Left Quaternion: %.2f, %.2f, %.2f, %.2f \n", quatPLeft.x, quatPLeft.y, quatPLeft.z, quatPLeft.w);

        btVector3 unitX = btVector3(Ogre::Vector3::UNIT_X.x, Ogre::Vector3::UNIT_X.y, Ogre::Vector3::UNIT_X.z);

        btQuaternion bulletPitchRight = btQuaternion(btVector3(1,0,0), rotRight.getPitch().valueRadians());
        btQuaternion bulletPitchLeft = btQuaternion(btVector3(1,0,0), rotLeft.getPitch().valueRadians());

        printf("Bullet Right Quaternion: %.2f, %.2f, %.2f, %.2f \n", bulletPitchRight.getX(), bulletPitchRight.getY(), bulletPitchRight.getZ(), bulletPitchRight.getW());
        printf("Bullet Left Quaternion: %.2f, %.2f, %.2f, %.2f \n", bulletPitchLeft.getX(), bulletPitchLeft.getY(), bulletPitchLeft.getZ(), bulletPitchLeft.getW());

        btQuaternion bulletPitchRightWBefore = quatBeforeRight*bulletPitchRight;
        btQuaternion bulletPitchLeftWBefore = quatBeforeLeft*bulletPitchLeft;

        printf("Bullet Right W/Before Quaternion: %.2f, %.2f, %.2f, %.2f \n", bulletPitchRightWBefore.getX(), bulletPitchRightWBefore.getY(), bulletPitchRightWBefore.getZ(), bulletPitchRightWBefore.getW());
        printf("Bullet Left W/Before Quaternion: %.2f, %.2f, %.2f, %.2f \n", bulletPitchLeftWBefore.getX(), bulletPitchLeftWBefore.getY(), bulletPitchLeftWBefore.getZ(), bulletPitchLeftWBefore.getW());


  //    //playerPos is a D3DXVECTOR3 that holds the camera position.
        playerWorldRight.setRotation(bulletPitchRightWBefore);
        playerWorldLeft.setRotation(bulletPitchLeftWBefore);

        MyKinematicMotionState *motionStateRight = new MyKinematicMotionState(playerWorldRight, mSceneMgr->getSceneNode("ParentArmNode"));
        motionStateRight->setKinematicPos(playerWorldRight);
        armbRight->setMotionState(motionStateRight);

        MyKinematicMotionState *motionStateLeft = new MyKinematicMotionState(playerWorldLeft, mSceneMgr->getSceneNode("ParentLeftArmNode"));
        motionStateLeft->setKinematicPos(playerWorldLeft);
        armbLeft->setMotionState(motionStateLeft);

        //// FIN PRUEBA BULLET

        printf("Arm Degrees Pitch %.2f\n", distanceVectorTotalBefore.getRotationTo(distanceVectorTotal).getPitch().valueDegrees());
        //printf("Arm Degrees Roll %e\n", distanceVectorTotalBefore.getRotationTo(distanceVectorTotal).getRoll().valueDegrees());

        distanceVectorTotalBefore = distanceVectorTotal;
        distanceLeftVectorTotalBefore = distanceLeftVectorTotal;



        handPosBefore = handNode->_getDerivedPosition();
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

        std::thread t2(elbowTask);

        try {
            isOpen = true;
            app.go();
        } catch( Ogre::Exception& e ) {
            std::cerr << "An exception has occured: " <<
                e.getFullDescription().c_str() << std::endl;
        }
        return 0;

    }

#ifdef __cplusplus
}
#endif
