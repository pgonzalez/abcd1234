/*
-----------------------------------------------------------------------------
This source is part of the Stereoscopy manager for OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/
-----------------------------------------------------------------------------
* Copyright (c) 2008, Mathieu Le Ber, AXYZ-IMAGES
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the AXYZ-IMAGES nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Mathieu Le Ber ''AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Mathieu Le Ber BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

// Last update jan 19 2010

#ifndef _STEREOMANAGER_H_
#define _STEREOMANAGER_H_

#include <Ogre.h>
#include <map>

/** Stereo vision manager class.
	This manager supports two stereo vision mode :
	 - An anaglyph mode for red/blue glasses
	 - Three interlaced modes for autostereoscopic screens
	 - A dual output mode to use with a polarized screen and two projectors or mounted head display

	Note :
	Ogre does not support natively the output in fullscreen on two screens. If you want to use the
	dual output mode with two fullscreen displays, you may want to apply this patch to OGRE :
	http://sourceforge.net/tracker/index.php?func=detail&aid=2555758&group_id=2997&atid=302997
	However, this manager is completely indepedent from this patch, the dual output mode only needs two viewports,
	for example two non-fullscreen windows or a windows with two viewports inside.
*/
	class StereoManager
	{
	public:
		enum StereoMode
		{
			SM_NONE,
			/// Anaglyph red/cyan
			SM_ANAGLYPH_RC,
			/// Anaglyph yellow/blue
			SM_ANAGLYPH_YB,
			/// Dual output off-axis mode : suitable for two projectors with polarized filters or head mounted display
			SM_DUALOUTPUT,
			/// Verticaly interlaced mode
			SM_INTERLACED_V,
			/// Horizontaly interlaced mode
			SM_INTERLACED_H,
			/// Interlaced mode with a checkerboard pattern
			SM_INTERLACED_CB,


			/// max value of the enum
			SM_LIMIT
			//SM_SHUTTERGLASSES
		};

		struct StereoModeDescription
		{
			StereoModeDescription() {}
			StereoModeDescription(const Ogre::String &name) :
				mName(name), mUsesCompositor(false), mMaterialName("") {}
			StereoModeDescription(const Ogre::String &name, const Ogre::String &materialName) :
				mName(name), mUsesCompositor(true), mMaterialName(materialName) {}
			StereoModeDescription &operator=(const StereoModeDescription &desc)
			{
				mName = desc.mName;
				mMaterialName = desc.mMaterialName;
				mUsesCompositor = desc.mUsesCompositor;
				return *this;
			}
			Ogre::String mName;
			Ogre::String mMaterialName;
			bool mUsesCompositor;
		};

		typedef std::map<StereoMode, StereoModeDescription> StereoModeList;

	protected:
		class StereoCameraListener : public Ogre::RenderTargetListener
		{
			Ogre::Vector3 mOldPos;
			Ogre::Vector2 mOldOffset;
			Ogre::uint32 mOldVisibilityMask;
			StereoManager *mStereoMgr;
			Ogre::Camera *mCamera;
			Ogre::Viewport *mViewport;
		public:
			bool mIsLeftEye;
			void init(StereoManager *stereoMgr, Ogre::Viewport *viewport, bool isLeftCamera);
	//		virtual void preRenderTargetUpdate (const RenderTargetEvent& evt);
	//		virtual void postRenderTargetUpdate  (const RenderTargetEvent& evt);
			virtual void preViewportUpdate (const Ogre::RenderTargetViewportEvent &evt);
			virtual void postViewportUpdate (const Ogre::RenderTargetViewportEvent &evt);
		};
		friend class StereoCameraListener;

		class DeviceLostListener : public Ogre::RenderSystem::Listener
		{
			StereoManager *mStereoMgr;
		public:
			void init(StereoManager *stereoMgr);
			virtual void eventOccurred (const Ogre::String &eventName, const Ogre::NameValuePairList *parameters);
		};
		friend class DeviceLostListener;

		Ogre::Camera *mCamera;
		StereoCameraListener mLeftCameraListener, mRightCameraListener;
		DeviceLostListener mDeviceLostListener;
		Ogre::Viewport *mLeftViewport, *mRightViewport;
		Ogre::uint32 mLeftMask, mRightMask;
		Ogre::CompositorInstance *mCompositorInstance;

		/* config */
		StereoModeList mAvailableModes;
		StereoMode mStereoMode;
		Ogre::Real mEyesSpacing;
		Ogre::Real mFocalLength;
		bool mFocalLengthInfinite;
		bool mIsFocalPlaneFixed;
		Ogre::Real mScreenWidth;
		bool mIsInversed;
		bool mIsCustomProjection;
		Ogre::Matrix4 mLeftCustomProjection, mRightCustomProjection;
		//String mConfigFileName;

		/* members for anaglyph only  */
		bool mAreOverlaysEnabled;
		Ogre::Viewport *mCompositorViewport;

		/* dependencies */
		typedef std::map<Ogre::RenderTarget *, bool> RenderTargetList;
		//typedef std::list<RenderTarget *> RenderTargetList;
		RenderTargetList mRenderTargetList;

		/* debug plane */
		Ogre::SceneManager *mSceneMgr;
		Ogre::Entity *mDebugPlane;
		Ogre::SceneNode *mDebugPlaneNode;
		Ogre::String mLeftMaterialName, mRightMaterialName;


		void initCompositor(Ogre::Viewport *viewport, const Ogre::String &materialName, Ogre::Viewport *&out_left, Ogre::Viewport *&out_right);
		void shutdownCompositor(void);
		void initListeners(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport);
		void shutdownListeners(void);

		void updateCamera(Ogre::Real delta);

		void init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport);

		void updateAllDependentRenderTargets(bool isLeftEye);
		void chooseDebugPlaneMaterial(bool isLeftEye);

	public:
		//--------------init--------------
		StereoManager(void);
		~StereoManager(void);
		/** The manager should be initialized with two viewports if you want dual output stereo (SM_DUALOUTPUT).
			If you want red/blue anaglyph stereo (SM_ANAGLYPH) you only need one viewport, just set the rightViewport to NULL.
			The left eye and the right eye will be composited on the leftViewport.
			The camera will be detected from the viewports.

			You can also use a configuration file that will store the stereo mode, the focal length, the eye spacing and
			the screen width. */
		void init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport, const Ogre::String &fileName);
		void init(Ogre::Viewport* leftViewport, Ogre::Viewport* rightViewport, StereoMode mode);

		/**	Shutdown and re-init the stereo manager to change stereo mode */
		void shutdown(void);

		//--------------dependencies--------------
		/**	RenderTargets added with this method will no longer be auto-updated but will be updated
			one time before the right viewport is updated and one time before the left viewport is updated.
			It is useful if you have a render texture whose content depends on the camera position,
			for example a render texture for water reflection */
		void addRenderTargetDependency(Ogre::RenderTarget *renderTarget);
		void removeRenderTargetDependency(Ogre::RenderTarget *renderTarget);

		//--------------debug plane--------------
		/**	Create a representation of the focal plane in the scene. If no material name is provided,
			a default wireframe material is used.

			The position of the debug plane will no longer be consistent with the focal length when it is set to infinite.
			The focal plane will still use the last finite focal length. */
		void createDebugPlane(Ogre::SceneManager *sceneMgr, const Ogre::String &leftMaterialName = "", const Ogre::String &rightMaterialName = "");
		void destroyDebugPlane(void);
		void enableDebugPlane(bool enable);
		void toggleDebugPlane(void);
		void updateDebugPlane(void);

		//--------------accessors--------------
		inline StereoMode getStereoMode(void) const {return mStereoMode;}

		inline Ogre::Camera *getCamera(void) const {return mCamera;}
		inline void setCamera(Ogre::Camera *cam) {mCamera = cam;}

		inline Ogre::Real getEyesSpacing(void) const {return mEyesSpacing;}
		inline void setEyesSpacing(Ogre::Real l) {mEyesSpacing = l;}

		/** Sets the focal length of the camera, i.e. the distance at which a point will be rendered at the same position
			on the screen for each eye. Will disable the infinite focal length */
		void setFocalLength(Ogre::Real l);

		/** Returns the focal length.
			Will return std::numeric_limits<Real>::infinity() if you used setFocalLengthInfinite(true) */
		Ogre::Real getFocalLength(void) const;

		/** Sets the focal length to infinite. Usefull if you use Head Mounted Displays because you need parallel frustums.
			The position of the debug plane will no longer be consistent with the focal length when it is set to infinite.
			The focal plane will still use the last finite focal length. */
		void setFocalLengthInfinite(bool isInfinite = true);
		inline bool isFocalLengthInfinite(void) const { return mFocalLengthInfinite; }

		/**	The focal plane represents the screen in the world space. As the screen
			is not and infinite plane but a rectangle, what I call the focal plane is in fact
			a rectangle.
			When the position of the plane is fixed, its position and size doesn't change if
			you change the focal length. Instead the camera is moved to reflect the focal length.
			The FOV angle of the camera is also adjusted in order to keep the focal rectangle
			covering the whole field view.

			If you set the screen width in the manager you can achieve a 1:1 scale effect if the
			observer's distance to the screen is equal to the focal distance.*/
		inline void fixFocalPlanePos(bool fix) {mIsFocalPlaneFixed = fix;}
		inline void setScreenWidth(Ogre::Real w) {mScreenWidth = w;}
		void useScreenWidth(Ogre::Real w);

		/** Use a custom projections matrix for each eye */
		void setCustomProjectonMatrices(bool enable, const Ogre::Matrix4 &leftMatrix, const Ogre::Matrix4 &rightMatrix);
		void getCustomProjectonMatrices(bool &enabled, Ogre::Matrix4 &leftMatrix, Ogre::Matrix4 &rightMatrix) const;

		/** Inverse the left eye and the right eye viewports */
		void inverseStereo(bool inverse);
		bool isStereoInversed(void) const {return mIsInversed;}

		/** Only objects matching the following flags will be rendered.
			This method sets the visibility mask for the right and the left viewports and
			the according mask for each dependent render target.
			It is usefull if you want to display some objects only for the right eye and
			some objects only for the left eye .*/
		void setVisibilityMask(Ogre::uint32 leftMask, Ogre::uint32 rightMask);
		void getVisibilityMask(Ogre::uint32 &outLeftMask, Ogre::uint32 &outRightMask) const;

		/** retrieve the left and right viewports.
		They will be the same as the ones you passed to the init method in the DUAL_OUTPUT mode.
		They will be different in case of the modes using the compositor. */
		Ogre::Viewport *getLeftViewport(void) const { return mLeftViewport; }
		Ogre::Viewport *getRightViewport(void) const { return mRightViewport; }

		//--------------config--------------
		/**	You can save and load the stereo configuration (mode, eyes spacing, focal length and screen width)
			to a file. Then this file can be used to initialize the manager.  */
		void saveConfig(const Ogre::String &filename) const;
		StereoMode loadConfig(const Ogre::String &filename);
	};

#endif
