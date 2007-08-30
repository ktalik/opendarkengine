/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/
 
#include "OIS.h"
#include "GamePlayState.h"
#include "logger.h"
#include "integers.h"

#include <OgreRenderWindow.h>
#include <OgreOverlayElement.h>
#include <OgreStringConverter.h>

using namespace Ogre;
using namespace OIS;

namespace Opde {
	
	template<> GamePlayState* Singleton<GamePlayState>::ms_Singleton = 0;
	
	GamePlayState::GamePlayState() : mSceneMgr(NULL), mDebugOverlay(NULL) {
		mRotateSpeed = 36;
		mMoveSpeed = 50;
		
		mTranslateVector = Vector3::ZERO;
		mRotX = 0;
		mRotY = 0;
		
		mForward = false;
		mBackward = false;
		mLeft = false;
		mRight = false;
		mScreenShot = false;
		
		mSceneDetailIndex = 0;
	}
	
	void GamePlayState::start() {
		mRoot = Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();
		mSceneMgr = mRoot->getSceneManager( "DarkSceneManager" );
		mCamera	= mSceneMgr->createCamera( "MainCamera" );
		
		mWindow = mRoot->getAutoCreatedWindow();
		
		mViewport = mWindow->addViewport( mCamera );
		
		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);
		
		
		mCamera->setNearClipDistance(0.5);
		mCamera->setFarClipDistance(4000);
	
		// Also change position, and set Quake-type orientation
		ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
		mCamera->setPosition(vp.position);
		mCamera->pitch(Degree(90)); 
		mCamera->rotate(vp.orientation);
		
		// Don't yaw along variable axis, causes leaning
		mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);
		
				
		// Thiefy FOV
		mCamera->setFOVy(Degree(70));
		
		// debug overlay
		mDebugOverlay = OverlayManager::getSingleton().getByName("Opde/DebugOverlay");
		mDebugOverlay->show();
		
		// Portal stats overlay
		mPortalOverlay = OverlayManager::getSingleton().getByName("Opde/OpdeDebugOverlay");
		mPortalOverlay->show();
		
		mConsole = new ConsoleFrontend();
		
		mWindow->resetStatistics();
		
		LOG_INFO("GamePlayState: Started");
	}
	
	void GamePlayState::exit() {
		// clear the scene
		mSceneMgr->clearScene();
		
		// Destroy cameras
		mSceneMgr->destroyAllCameras();
		
		// remove all viewports
		mRoot->getAutoCreatedWindow()->removeAllViewports();
		
		mPortalOverlay->hide();
		mDebugOverlay->hide();
		
		LOG_INFO("GamePlayState: Exited");
	}
	
	void GamePlayState::suspend() {
	}
	
	void GamePlayState::resume() {
	}
	
	void GamePlayState::update(unsigned long timePassed) {
		if (timePassed == 0) {
			mMoveScale = 0.1f;
			mRotScale = 0.1f;
		} else {
			mMoveScale = mMoveSpeed * timePassed / 1000;
			mRotScale = mRotateSpeed * timePassed / 1000;
		}
		
		// Quick hack. Let the camera move:
		if (mForward)
			mTranslateVector.z = -mMoveScale;
		
		if (mBackward)
			mTranslateVector.z =  mMoveScale;
		
		if (mLeft)
			mTranslateVector.x = -mMoveScale;

		if (mRight)
			mTranslateVector.x =  mMoveScale;

		mCamera->yaw(mRotX * mRotScale);
		mCamera->pitch(mRotY * mRotScale);
		mCamera->moveRelative(mTranslateVector);
		
		mTranslateVector = Vector3::ZERO;
		mRotX = 0;
		mRotY = 0;
		
		if (mSceneDisplay) {
			mSceneDetailIndex = (mSceneDetailIndex+1)%2 ; // I Do not need points for now
			switch(mSceneDetailIndex) {
				case 0 : mCamera->setPolygonMode(PM_SOLID) ; break ;
				case 1 : mCamera->setPolygonMode(PM_WIREFRAME) ; break ;
				//case 2 : mCamera->setPolygonMode(PM_POINTS) ; break ;
			}
			mSceneDisplay = false;
		}
		
		if (mPortalDisplay) {
			// reuse
			mSceneMgr->getOption("ShowPortals", &mPortalDisplay);
			mPortalDisplay = !mPortalDisplay;
			mSceneMgr->setOption("ShowPortals", &mPortalDisplay);
			
			mPortalDisplay = false;
		}
		
		mConsole->update(timePassed);
		
		// Temporary: Debug Overlay 
		static String currFps = "Current FPS: ";
		static String avgFps = "Average FPS: ";
		static String bestFps = "Best FPS: ";
		static String worstFps = "Worst FPS: ";
		static String tris = "Triangle Count: ";

		// update stats when necessary
		try {
		    OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Opde/AverageFps");
		    OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Opde/CurrFps");
		    OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Opde/BestFps");
		    OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Opde/WorstFps");

		    const RenderTarget::FrameStats& stats = mWindow->getStatistics();

		    guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
		    guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
		    guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
			+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
		    guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
			+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

		    OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Core/NumTris");
		    guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

		    // OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
		}
		catch(...)
		{
		    // ignore
		}
		
		// update the portal statistics
		try {
			OverlayElement* guibc = OverlayManager::getSingleton().getOverlayElement("Opde/BackCulls");
			OverlayElement* guiep = OverlayManager::getSingleton().getOverlayElement("Opde/EvalPorts");
			OverlayElement* guirc = OverlayManager::getSingleton().getOverlayElement("Opde/RendCells");
			
			// Temporary: Debug Overlay 
			static String sbc = "Backface culls: ";
			static String sep = "Evaluated portals: ";
			static String src = "Rendered cells: ";
			
			uint bculls, eports, rendc;
			
			mSceneMgr->getOption("BackfaceCulls", &bculls);
			mSceneMgr->getOption("CellsRendered", &rendc);
			mSceneMgr->getOption("EvaluatedPortals", &eports);
			
			
			guibc->setCaption(sbc + StringConverter::toString(bculls));
			guiep->setCaption(sep + StringConverter::toString(eports));
			guirc->setCaption(src + StringConverter::toString(rendc));
		}
		catch(...)
		{
		    // ignore
		}
	
	}

	bool GamePlayState::keyPressed( const OIS::KeyEvent &e ) {
		if( e.key == KC_F12 ) {
        		mConsole->setActive(!mConsole->isActive());
			return true;
    		}
		
		if (!mConsole->injectKeyPress(e)) {
			if(e.key == KC_W) { 
				mForward = true;
				return true;
			} else if(e.key == KC_S) { 
				mBackward = true;
				return true;
			} else if(e.key == KC_A) { 
				mLeft = true;
				return true;
			} else if(e.key == KC_D) { 
				mRight = true;
				return true;
			} else if (e.key == KC_SYSRQ) {
				mScreenShot = true;
				return true;
			} else if (e.key == KC_O) {
				mSceneDisplay = true;
				return true;
			} else if (e.key == KC_P) {
				mPortalDisplay = true;
				return true;
			}
		} else {
			return true;
		}
	}
	
	bool GamePlayState::keyReleased( const OIS::KeyEvent &e ) {
		if (!mConsole->isActive()) {
			if(e.key == KC_W) { 
				mForward = false;
				return true;
			} else if(e.key == KC_S) { 
				mBackward = false;
				return true;
			} else if(e.key == KC_A) { 
				mLeft = false;
				return true;
			} else if(e.key == KC_D) { 
				mRight = false;
				return true;
			} else	if( e.key == KC_ESCAPE ) {
        			requestTermination();
				return true;
			}
    		}
		
	}
	
	bool GamePlayState::mouseMoved( const OIS::MouseEvent &e ) {
		mRotX -= Degree( e.state.X.rel * 20.00);
		mRotY -= Degree( e.state.Y.rel * 20.00);
		return false;
	}
	
	bool GamePlayState::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		return false;
	}
	
	bool GamePlayState::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		return false;
	}
	
}

