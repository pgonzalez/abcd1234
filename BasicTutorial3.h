#ifndef BASICTUTORIAL3_H_INCLUDED
#define BASICTUTORIAL3_H_INCLUDED

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>
#include "BaseApplication.h"
#include "btBulletDynamicsCommon.h"


class BasicTutorial3 : public BaseApplication
{
private:
    Ogre::TerrainGlobalOptions* mTerrainGlobals;
    Ogre::TerrainGroup* mTerrainGroup;
    bool mTerrainsImported;

    void defineTerrain(long x, long y);
    void initBlendMaps(Ogre::Terrain* terrain);
    void configureTerrainDefaults(Ogre::Light* light);

    // bullet dynamics
    int i;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* overlappingPairCache;
    btSequentialImpulseConstraintSolver* solver;
    btDiscreteDynamicsWorld* dynamicsWorld;
    btCollisionShape* groundShape;
    btAlignedObjectArray<btCollisionShape*> collisionShapes;
public:
    BasicTutorial3(void);
    virtual ~BasicTutorial3(void);

protected:
    virtual void createScene(void);
    virtual void createFrameListener(void);
    virtual void destroyScene(void);
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
    virtual void createBulletSim(void);
    virtual void addDynamicRigidBody(Ogre::SceneNode *bodyNode, btVector3 position);

};

#endif // BASICTUTORIAL3_H_INCLUDED
