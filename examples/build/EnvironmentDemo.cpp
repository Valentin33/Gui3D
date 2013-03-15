#include <iostream>
#include <sstream>
#include <vector>
 
#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include "Gui3D.h"
#include "Gui3DPanel.h"

#include "MyEnvironmentDemoPanelColors.h"
 
typedef struct PanelAndDirection
{
    Gui3D::Panel* panel;
    Ogre::Vector3 cameraDirection;
    int yaw;
} PanelAndDirection;

class EnvironmentDemo : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
    // Gui3D main object
    Gui3D::Gui3D* mGui3D;

    // Keep track of some captions to modify their contents on callback
    Gui3D::Caption* captionButton;
    Gui3D::Caption* captionChecked;
    Gui3D::Caption* captionCombobox;

    Ogre::SceneNode* tvNode;
    Ogre::Vector3 originalTvNodePos;
    Ogre::Entity* entSinbad;
    Ogre::AnimationState* a, *a2;

    Gui3D::Panel* mPanel;

    MyEnvironmentDemoPanelColors mMyEnvironmentDemoPanelColors;

    EnvironmentDemo()
    {
        originalTvNodePos = Ogre::Vector3(0, 1, 10);

        _makeOgre();
        _makeOIS();
        _makeScene();

        mGui3D = new Gui3D::Gui3D(&mMyEnvironmentDemoPanelColors);
        mGui3D->createScreen(mViewport, "environmentDemo", "mainScreen");

        mPanel = _createPanel(Ogre::Vector3(0, 5.3, -2.5), 180);

        mCamera->setPosition(0, 6.f, -8);
        mCamera->setDirection(Ogre::Vector3(0, 0, 1));
    }

    Gui3D::Panel* _createPanel(Ogre::Vector3 pos, int yaw)
    {
        Gui3D::Panel* panel = new Gui3D::Panel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 400), 15, "environmentDemo", "kikoo");

        panel->mNode->setPosition(pos);
        panel->mNode->yaw(Ogre::Degree(yaw));
        
        panel->makeCaption(10, 10, 380, 30, "Move the TV please...", Gorilla::TextAlign_Centre);

        panel->makeCaption(10, 100, 90, 100, "Left", Gorilla::TextAlign_Centre, Gorilla::VerticalAlign_Middle);
        panel->makeCaption(310, 100, 90, 100, "Right", Gorilla::TextAlign_Centre, Gorilla::VerticalAlign_Middle);

        Gui3D::ScrollBar* s = panel->makeScrollBar(100, 100, 200, 100, 0, 15);
        s->setValueChangedCallback(this, &EnvironmentDemo::decalValueChanged);
        s->setStep(0.1);
        s->setDisplayedPrecision(0, false);
        s->setCallCallbackOnSelectingValue(true);
        s->setDisplayValue(false);

        return panel;
    }

    bool decalValueChanged(Gui3D::PanelElement* e)
    {
        Ogre::Vector3 pos = originalTvNodePos;
        pos.x -= ((Gui3D::ScrollBar*)e)->getValue();
        tvNode->setPosition(pos); 
        return true;
    }
 
    ~EnvironmentDemo()
    {
        delete mGui3D;

        std::ostringstream s;
        s << "\n** Average FPS (with FSAA to 1) is " << mWindow->getAverageFPS() << "\n\n";
        Ogre::LogManager::getSingleton().logMessage(s.str());
        delete mRoot;
    }
 
    bool frameStarted(const Ogre::FrameEvent& evt)
    {
        if (mWindow->isClosed())
            return false;
        
        Ogre::Vector3 trans(0,0,0);
   
        if (mKeyboard->isKeyDown(OIS::KC_W))
            trans.z = -1;
        else if (mKeyboard->isKeyDown(OIS::KC_S))
            trans.z =  1;
        if (mKeyboard->isKeyDown(OIS::KC_A))
            trans.x = -1;
        else if (mKeyboard->isKeyDown(OIS::KC_D))
            trans.x =  1;
   
        if (trans.isZeroLength() == false)
        {
            Ogre::Vector3 pos = mCamera->getPosition();
            pos += mCamera->getOrientation() * (trans * 10.0f) * evt.timeSinceLastFrame;
            pos.y = 6.0f;
            mCamera->setPosition(pos);
        }

        a->addTime(evt.timeSinceLastFrame);
        a2->addTime(evt.timeSinceLastFrame);
 
        mPanel->injectMouseMoved(mCamera->getCameraToViewportRay(0.5f, 0.5f));
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

    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        Ogre::Real pitch = Ogre::Real(arg.state.Y.rel) * -0.005f;
        Ogre::Real yaw = Ogre::Real(arg.state.X.rel) * -0.005f;
        mCamera->pitch(Ogre::Radian(pitch));
        mCamera->yaw(Ogre::Radian(yaw));
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
 
        mViewport->setBackgroundColour(Ogre::ColourValue(0./255, 80./255, 160./255, .5f));
 
        rgm->initialiseAllResourceGroups();

        tvNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        tvNode->setPosition(originalTvNodePos);
        tvNode->setScale(2, 2, 2);
        Ogre::Entity* entTV = mSceneMgr->createEntity("TV.mesh");
        tvNode->attachObject(entTV);
 
        Ogre::SceneNode* sinbadNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        sinbadNode->setPosition(0, 2, -2.2);
        sinbadNode->setScale(0.4, 0.4, 0.4);
        sinbadNode->yaw(Ogre::Degree(180));
        entSinbad = mSceneMgr->createEntity("sinbad.mesh");
        sinbadNode->attachObject(entSinbad);

        entSinbad->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
        a = entSinbad->getAnimationState("IdleBase");
        a->setEnabled(true);
        a->setLoop(true);
        a2 = entSinbad->getAnimationState("IdleTop");
        a2->setEnabled(true);
        a2->setLoop(true);


        //mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        //mCameraNode->attachObject(mCamera);
 
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
    OIS::InputManager* mInputManager;
    OIS::Keyboard* mKeyboard;
    OIS::Mouse* mMouse;
};


int main()
{
    EnvironmentDemo* demo  = new EnvironmentDemo();
    demo->mRoot->startRendering();
    delete demo;
 
    return 0;
}