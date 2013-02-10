/*
	Gui3D
	-------
    
	Copyright (c) 2012 Valentin Frechaud
                                                                                  
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:
                                                                                  
	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.
                                                                                  
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/

#include <iostream>
#include <sstream>
#include <vector>
 
#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include "Gui3D.h"
#include "Gui3DPanel.h"
#include "Gui3DScreenPanel.h"

#include "MyPurplePanelColors.h"
 
class Simple2DDemo : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
	// Gui3D main object
	Gui3D::Gui3D* mGui3D;

	// The main panel (display in 3D)
	Gui3D::Panel* mPanel;

	// The screen Panel (display in 2D)
	Gui3D::ScreenPanel* mSPanel;
	Gui3D::ScreenPanel* mSPanel2;

	// Keep track of some captions to modify their contents on callbacks
	Gui3D::Caption* captionButton;
	Gui3D::Caption* captionGlobalTime;
	Gui3D::Caption* captionDisplayTextZone;
	Gui3D::Caption* captionLocalTime;

	// As Gui3D doesn't fully abstract Gorilla, you still have to deal with it. 
	// See http://www.valentinfrechaud.fr/Gui3DWiki/index.php/Gui3D_and_Gorilla for more infos.
	Gorilla::Layer* mMousePointerLayer;
	Gorilla::Rectangle* mMousePointer;
	Ogre::Vector2 mNormalizedMousePosition;

	// For the smooth movment when moving the mouse
	Ogre::Vector3 cameraDirection;

	// Animations for sinbad
	Ogre::AnimationState* animDance;

	// Keep track of number of click on the button
	int mClicksOnButton;
	
	Gui3D::Clock globalClock;
	Gui3D::Clock localClock;

	// Keep track of number of frames
	int nbFramesSinceStart;
	int numFrameLastClicked;

	Simple2DDemo()
	{
		_makeOgre();
		_makeOIS();
		_makeScene();

		mGui3D = new Gui3D::Gui3D(new MyPurplePanelColors());
		mGui3D->createScreen(mViewport, "purple", "mainScreen");

		_createDemoPanel();

		// Create a layer for the mousePointer
		mNormalizedMousePosition = Ogre::Vector2(0.5, 0.5);
		mMousePointerLayer = mGui3D->getScreen("mainScreen")->createLayer(4);
		mMousePointer = mMousePointerLayer->createRectangle(mViewport->getActualWidth()/2, 
			mViewport->getActualHeight()/2, 12, 18);
		mMousePointer->background_image("mousepointer");

		mCameraNode->setPosition(0, 2, -2);
		cameraDirection = Ogre::Vector3(0, 0, -1);
		mCamera->setDirection(cameraDirection);

		numFrameLastClicked = 0;
		nbFramesSinceStart = 0;
		mClicksOnButton = 0;
	}

	void _createDemoPanel()
	{
		// 3D Panel (using Gorilla::ScreenRenderable)
		mPanel = new Gui3D::Panel(
			mGui3D, mSceneMgr, Ogre::Vector2(400, 200), 10, "purple", "test_panel");

		mPanel->makeCaption(5, -30, 390, 30, "Simple 2D Demo Panel");

		mPanel->makeButton(100, 10, 200, 30, "click me!")->setPressedCallback(this, &Simple2DDemo::buttonPressed);
		captionButton = mPanel->makeCaption(10, 60, 380, 30, "The button hasn't been clicked yet");
		captionGlobalTime = mPanel->makeCaption(10, 110, 380, 30, "global time: 0s");
		mPanel->makeCaption(10, 150, 130, 30, "text entered : ");
		captionDisplayTextZone = mPanel->makeCaption(140, 150, 190, 30, "");

		mPanel->mNode->setPosition(0, 2.1, -8);
		
		// 2D Panel (using Gorilla::Screen)
		Gorilla::Screen* myScreen = mGui3D->getScreen("mainScreen");

		mSPanel = new Gui3D::ScreenPanel(mGui3D,
			myScreen,
			Ogre::Vector2(450, 350),
			Ogre::Vector2(300, 200),
			"purple",
			"test_screenPanel");

		captionLocalTime = mSPanel->makeCaption(0, 0, 300, 40, "local time: 0s");
		
		mSPanel->makeButton(25, 40, 250, 50, "Reset local timer")
			->setPressedCallback(this, &Simple2DDemo::resetLocalFrameCount);
		
		mSPanel->makeCaption(0, 110, 300, 40, "Enter some text : ");

		Gui3D::TextZone* t = mSPanel->makeTextZone(10, 150, 280, 30, "");
		t->setValueChangedCallback(this, &Simple2DDemo::textChanged);
		t->setMaxStringLength(15);

		// 2nd test panel
		mSPanel2 = new Gui3D::ScreenPanel(
			mGui3D,
			myScreen,
			Ogre::Vector2(50, 450),
			Ogre::Vector2(250, 100),
			"purple",
			"test_screenPanel2");

		mSPanel2->makeCaption(0, 0, 250, 30, "Health : 60");
		mSPanel2->makeCaption(0, 30, 250, 30, "Power : 50");
		mSPanel2->makeButton(0, 60, 250, 40, "Reset global timer")
			->setPressedCallback(this, &Simple2DDemo::resetGlobalFrameCount);

		// We don't want any panels to display mouse cursor. It is handled
		//  by our Simple2DDemo.
		mPanel->hideInternalMousePointer();
		mSPanel->hideInternalMousePointer();
		mSPanel2->hideInternalMousePointer();
	}
 
	~Simple2DDemo()
	{
		std::ostringstream s;
		s << "\n** Average FPS (with FSAA to 1) is " << mWindow->getAverageFPS() << "\n\n";
		Ogre::LogManager::getSingleton().logMessage(s.str());

		delete mGui3D;
		delete mRoot;
	}

	bool textChanged(Gui3D::PanelElement* e)
	{
		Gui3D::TextZone* t = (Gui3D::TextZone*)e;
		captionDisplayTextZone->text(t->text());
		return true;
	}

	bool resetGlobalFrameCount(Gui3D::PanelElement* e)
	{
		globalClock.reset();
		localClock.reset();
		return true;
	}

	bool resetLocalFrameCount(Gui3D::PanelElement* e)
	{
		localClock.reset();
		return true;
	}

	bool buttonPressed(Gui3D::PanelElement* e)
	{
		mClicksOnButton++;
		std::ostringstream s;
		s << "The button has been pressed " << mClicksOnButton << " times";
		captionButton->text(s.str());
		return true;
	}
 
	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		if (mWindow->isClosed())
			return false;

		// Actualize counters
		globalClock.addTime(evt.timeSinceLastFrame);
		localClock.addTime(evt.timeSinceLastFrame);

		std::ostringstream s;
		s << "global time: " << std::fixed << std::setprecision(1) << globalClock.getTimeSec() << "s";
		captionGlobalTime->text(s.str());

		s.str("");
		s << "local time: " << std::fixed << std::setprecision(1) << localClock.getTimeSec() << "s";
		captionLocalTime->text(s.str());
		
		// Make sinbad move
		animDance->addTime(evt.timeSinceLastFrame);

		// Inject time on all Gui3D panels
		mPanel->injectTime(evt.timeSinceLastFrame);
		mSPanel->injectTime(evt.timeSinceLastFrame);
		mSPanel2->injectTime(evt.timeSinceLastFrame);

		mMouse->capture();
 
		// Quit on ESCAPE Keyboard
		mKeyboard->capture();

		if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
			return false;
 
		return true;
	}

	bool keyPressed(const OIS::KeyEvent &e)
	{
		mPanel->injectKeyPressed(e);
		mSPanel->injectKeyPressed(e);
		mSPanel2->injectKeyPressed(e);
		return true;
	}

	bool keyReleased(const OIS::KeyEvent &e)
	{
		mPanel->injectKeyReleased(e);
		mSPanel->injectKeyReleased(e);
		mSPanel2->injectKeyReleased(e);
		return true;
	}

	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		mPanel->injectMousePressed(evt, id);
		mSPanel->injectMousePressed(evt, id);
		mSPanel2->injectMousePressed(evt, id);
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		mPanel->injectMouseReleased(evt, id);
		mSPanel->injectMouseReleased(evt, id);
		mSPanel2->injectMouseReleased(evt, id);
		return true;
	}

	// Calculate and normalize the distance between the middle of the screen 
	//  and the mouse on both X and Y axis.
	// This is used for the smooth movement of the camera
	//  when the mouse is moving.
	Ogre::Vector2 getScreenCenterMouseDistance()
	{
		Ogre::Real posX = (mMousePointer->position().x - mViewport->getActualWidth()) 
			/ mViewport->getActualWidth();
		Ogre::Real posY = (mMousePointer->position().y - mViewport->getActualHeight()) 
			/ mViewport->getActualHeight();

		return Ogre::Vector2(posX + 0.5, posY + 0.5);
	}

	bool mouseMoved(const OIS::MouseEvent &arg)
	{
		// Set the new camera smooth direction movement
		Ogre::Vector2 distance(getScreenCenterMouseDistance());
		mCamera->setDirection(cameraDirection
			+ Ogre::Vector3(distance.x, -distance.y, 0) / 30);
		
		// Raycast for the actual panel
		Ogre::Real xMove = static_cast<Ogre::Real>(arg.state.X.rel);
		Ogre::Real yMove = static_cast<Ogre::Real>(arg.state.Y.rel);
	
		mNormalizedMousePosition.x += xMove / mViewport->getActualWidth();
		mNormalizedMousePosition.y += yMove / mViewport->getActualHeight();

		mNormalizedMousePosition.x = std::max<Ogre::Real>(mNormalizedMousePosition.x, 0);
		mNormalizedMousePosition.y = std::max<Ogre::Real>(mNormalizedMousePosition.y, 0);
		mNormalizedMousePosition.x = std::min<Ogre::Real>(mNormalizedMousePosition.x, 1);
		mNormalizedMousePosition.y = std::min<Ogre::Real>(mNormalizedMousePosition.y, 1);

		mMousePointer->position(
			mNormalizedMousePosition.x * mViewport->getActualWidth(), 
			mNormalizedMousePosition.y * mViewport->getActualHeight());

		mPanel->injectMouseMoved(mCamera->getCameraToViewportRay(
			mNormalizedMousePosition.x, mNormalizedMousePosition.y));

		mSPanel->injectMouseMoved(mNormalizedMousePosition.x * mViewport->getActualWidth(),
			mNormalizedMousePosition.y * mViewport->getActualHeight());

		mSPanel2->injectMouseMoved(mNormalizedMousePosition.x * mViewport->getActualWidth(),
			mNormalizedMousePosition.y * mViewport->getActualHeight());

		return true;
	}
 
	void  _makeOgre()
	{
		mRoot = new Ogre::Root("", "");
		mRoot->addFrameListener(this);
 
#if OGRE_PLATFORM == OGRE_PLATFORM_LINUX
		mRoot->loadPlugin(OGRE_RENDERER);
#else
#if 1
#ifdef _DEBUG
		mRoot->loadPlugin("RenderSystem_Direct3D9_d");
#else
		mRoot->loadPlugin("RenderSystem_Direct3D9");
#endif
#else
#ifdef _DEBUG
		mRoot->loadPlugin("RenderSystem_GL_d.dll");
#else
		mRoot->loadPlugin("RenderSystem_GL.dll");
#endif
#endif
#endif
 
		mRoot->setRenderSystem(mRoot->getAvailableRenderers()[0]);
 
		Ogre::ResourceGroupManager* rgm = Ogre::ResourceGroupManager::getSingletonPtr();
		rgm->addResourceLocation(".", "FileSystem");
		rgm->addResourceLocation("sinbad.zip", "Zip");

		mRoot->initialise(false);
 
		Ogre::NameValuePairList misc;
		misc["FSAA"] = "1";
		mWindow = mRoot->createRenderWindow("Gorilla", 800, 600, false, &misc);
		mSceneMgr = mRoot->createSceneManager(Ogre::ST_EXTERIOR_CLOSE);
		mCamera = mSceneMgr->createCamera("Camera");
		mViewport = mWindow->addViewport(mCamera);
 
		mViewport->setBackgroundColour(Ogre::ColourValue(180./255, 115./255, 175./255, .5f));
 
		rgm->initialiseAllResourceGroups();
 
		mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		mCameraNode->attachObject(mCamera);
 
		mCamera->setNearClipDistance(0.05f);
		mCamera->setFarClipDistance(1000);

		// Add sinbad
		Ogre::SceneNode* sinbadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
		sinbadNode->setPosition(1.5, 2, -6);
		sinbadNode->setScale(Ogre::Vector3(0.15));
		Ogre::Entity* entSinbad = mSceneMgr->createEntity("sinbad.mesh");
		sinbadNode->attachObject(entSinbad);
		
		animDance = entSinbad->getAnimationState("Dance");
		animDance->setEnabled(true);
		animDance->setLoop(true);
	}
 
	void  _makeOIS()
	{
		// Initialise OIS
		OIS::ParamList pl;
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;
		mWindow->getCustomAttribute("WINDOW", &windowHnd);
		windowHndStr << windowHnd;
		pl.insert(std::make_pair(Ogre::String("WINDOW"), windowHndStr.str()));
		mInputManager = OIS::InputManager::createInputSystem(pl);
		mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
		mKeyboard->setEventCallback(this);
		mMouse = static_cast<OIS::Mouse*>(mInputManager->createInputObject(OIS::OISMouse, true));
		mMouse->setEventCallback(this);
		mMouse->getMouseState().width = mViewport->getActualWidth();
		mMouse->getMouseState().height = mViewport->getActualHeight();
	}
 
	void _makeScene()
	{
		Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0);
		Ogre::MeshManager::getSingleton().createPlane("ground", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			plane, 1500, 1500, 20, 20, true, 1, 150, 150, Ogre::Vector3::UNIT_Z);
		Ogre::Entity* entGround = mSceneMgr->createEntity("GroundEntity", "ground");
			mSceneMgr->getRootSceneNode()->createChildSceneNode()->attachObject(entGround);
		entGround->setMaterialName("Gui3DExample/Ground");
		entGround->setCastShadows(false);
	}
 
	Ogre::Root* mRoot;
	Ogre::RenderWindow* mWindow;
	Ogre::Viewport* mViewport;
	Ogre::SceneManager* mSceneMgr;
	Ogre::Camera* mCamera;
	Ogre::SceneNode* mCameraNode;
	OIS::InputManager* mInputManager;
	OIS::Keyboard* mKeyboard;
	OIS::Mouse* mMouse;
};

int main()
{
	Simple2DDemo* demo  = new Simple2DDemo();
	demo->mRoot->startRendering();
	delete demo;
 
	return 0;
}