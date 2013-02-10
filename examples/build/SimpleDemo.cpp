#include <iostream>
#include <sstream>
#include <vector>
 
#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include "Gui3D.h"
#include "Gui3DPanel.h"

#include "MySimpleDemoPanelColors.h"
 
class SimpleDemo : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
	// Gui3D main object
	Gui3D::Gui3D* mGui3D;

	// The main panel
	Gui3D::Panel* mPanel;

	// Keep track of some captions to modify their contents on callback
	Gui3D::Caption* captionButton;
	Gui3D::Caption* captionChecked;
	Gui3D::Caption* captionCombobox;

	// As Gui3D doesn't fully abstract Gorilla, you still have to deal with it. 
	// See http://www.valentinfrechaud.fr/Gui3DWiki/index.php/Gui3D_and_Gorilla for more infos.
	Gorilla::Layer* mMousePointerLayer;
	Gorilla::Rectangle* mMousePointer;
	Ogre::Vector2 mNormalizedMousePosition;

	// For the smooth movment when moving the mouse
	Ogre::Vector3 cameraDirection;

	// Keep track of number of click on the button
	int mClicksOnButton;

	SimpleDemo()
	{
		_makeOgre();
		_makeOIS();
		_makeScene();

		mGui3D = new Gui3D::Gui3D(new MySimpleDemoPanelColors());
		mGui3D->createScreen(mViewport, "simpleDemo", "mainScreen");

		// Create a layer for the mousePointer
		mNormalizedMousePosition = Ogre::Vector2(0.5, 0.5);
		mMousePointerLayer = mGui3D->getScreen("mainScreen")->createLayer();
		mMousePointer = mMousePointerLayer->createRectangle(mViewport->getActualWidth()/2, 
			mViewport->getActualHeight()/2, 12, 18);
		mMousePointer->background_image("mousepointer");

		_createDemoPanel();

		mCameraNode->setPosition(0, 2, 1);
		cameraDirection = Ogre::Vector3(0, 0, -1);
		mCamera->setDirection(cameraDirection);
	}

	void _createDemoPanel()
	{
		Gui3D::Panel* panel = new Gui3D::Panel(
			mGui3D, mSceneMgr, Ogre::Vector2(400, 400), 10, "simpleDemo", "test_panel");

		panel->makeCaption(5, -30, 390, 30, "Simple Demo Panel");

		panel->makeButton(100, 10, 200, 30, "click me!")->setPressedCallback(this, &SimpleDemo::buttonPressed);
		panel->makeCaption(10, 60, 100, 30, "check it ! ");
		panel->makeCheckbox(110, 60, 30, 30)->setSelecteStateChangedCallback(this, &SimpleDemo::checkboxChecked);
		
		std::vector<Ogre::String> items;
		items.push_back("item1");
		items.push_back("item2");
		items.push_back("item3");
		items.push_back("item4");
		items.push_back("item5");
		items.push_back("item6");
		panel->makeCombobox(10, 110, 380, 100, items, 4)->setValueChangedCallback(this, &SimpleDemo::comboboxValueChanged);

		captionButton = panel->makeCaption(10, 250, 380, 30, "The button hasn't been clicked yet");
		captionChecked = panel->makeCaption(10, 300, 380, 30, "The checkbox hasn't been checked");
		captionCombobox = panel->makeCaption(10, 350, 380, 30, "No selected values yet");
		
		mPanel = panel;
		mPanel->mNode->setPosition(0, 2.1, -8);
	}
 
	~SimpleDemo()
	{
		delete mGui3D;

		std::ostringstream s;
		s << "\n** Average FPS (with FSAA to 1) is " << mWindow->getAverageFPS() << "\n\n";
		Ogre::LogManager::getSingleton().logMessage(s.str());
		delete mRoot;
	}

	bool buttonPressed(Gui3D::PanelElement* e)
	{
		mClicksOnButton++;
		std::ostringstream s;
		s << "The button has been pressed " << mClicksOnButton << " times";
		captionButton->text(s.str());
		return true;
	}

	bool checkboxChecked(Gui3D::PanelElement* e)
	{
		Gui3D::Checkbox* c = (Gui3D::Checkbox*) e;
		if (c->getChecked())
			captionChecked->text("The checkbox is checked");
		else
			captionChecked->text("The checkbox isn't checked");
		return true;
	}

	bool comboboxValueChanged(Gui3D::PanelElement* e)
	{
		Gui3D::Combobox* c = (Gui3D::Combobox*) e;
		std::ostringstream s;
		s << "Combobox selected value : " << c->getValue();
		captionCombobox->text(s.str());
		return true;
	}
 
	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		if (mWindow->isClosed())
			return false;
 
		mPanel->injectTime(evt.timeSinceLastFrame);

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
		return true;
	}

	bool keyReleased(const OIS::KeyEvent &e)
	{
		mPanel->injectKeyReleased(e);
		return true;
	}

	bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		mPanel->injectMousePressed(evt, id);
		return true;
	}

	bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
	{
		mPanel->injectMouseReleased(evt, id);
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
	SimpleDemo* demo  = new SimpleDemo();
	demo->mRoot->startRendering();
	delete demo;
 
	return 0;
}