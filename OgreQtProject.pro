TEMPLATE = app
TARGET = OgreQtProject
DEFINES -= UNICODE

QMAKE_CXXFLAGS += -std=gnu++0x

TEMPLATE = app

unix {
    # You may need to change this include directory
    INCLUDEPATH += /usr/local/lib/OGRE
    #INCLUDEPATH += /usr/local/include/OGRE/Paging
    INCLUDEPATH += /usr/local/include/OGRE/Terrain
    #INCLUDEPATH += /usr/local/include/libfreenect
    INCLUDEPATH += /usr/include/ni
    INCLUDEPATH += /usr/include/nite
    INCLUDEPATH += /usr/include/bullet

    CONFIG += link_pkgconfig
    PKGCONFIG += OGRE

    LIBS += /usr/lib/libXnCore.so
    LIBS += /usr/lib/libXnDDK.so
    LIBS += /usr/lib/libXnDeviceFile.so
    LIBS += /usr/lib/libXnDeviceSensorV2KM.so
    LIBS += /usr/lib/libXnFormats.so
    LIBS += /usr/lib/libXnVCNITE_1_5_2.so
    LIBS += /usr/lib/libXnVFeatures_1_5_2.so
    LIBS += /usr/lib/libXnVNite_1_5_2.so
    LIBS += /usr/lib/libnimCodecs.so
    LIBS += /usr/lib/libnimMockNodes.so
    LIBS += /usr/lib/libnimRecorder.so
    LIBS += /usr/lib/libOpenNI.so
    LIBS += /usr/lib/x86_64-linux-gnu/libBulletDynamics.so
    LIBS += /usr/lib/x86_64-linux-gnu/libBulletCollision.so
    LIBS += /usr/lib/x86_64-linux-gnu/libLinearMath.so
}
debug {
    TARGET = $$join(TARGET,,,d)
    LIBS += -lOgreMain -lOIS -lOgreTerrain -lglut -lboost_system
    #LIBS += -lfreenect
    # If you are using Ogre 1.9 also include -lOgreOverlay_d, like this:
    # LIBS *= -lOgreMain_d -lOIS_d -lOgreOverlay_d
}
release {
    LIBS += -lOgreMain -lOIS -lOgreTerrain -lboost_system
    #LIBS += -lfreenect -lglut
    LIBS += -lglut
    # If you are using Ogre 1.9 also include -lOgreOverlay, like this:
    # LIBS *= -lOgreMain -lOIS -lOgreOverlay
}

HEADERS += \
    StereoManager.h \
    TutorialApplication.h \
    BasicTutorial3.h \
    BaseApplication.h \
    KinectDevice.h \
    OpenNIDevice.h \
    NiHandTracker.h \
    NiElbowTracker.h

SOURCES += \
    StereoManager.cpp \
    TutorialApplication.cpp \
    BaseApplication.cpp \
    NiHandTracker.cpp \
    NiElbowTracker.cpp
