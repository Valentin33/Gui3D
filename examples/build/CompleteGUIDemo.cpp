// Fix by mkultra333 for compilers that doesn't support the nullptr
#ifndef __CPP09NULLPTRSUPPORTED
#define NULLPTR (0)
#else
#define NULLPTR (nullptr)
#endif


#include <iostream>
#include <sstream>
#include <vector>

#include <OGRE/Ogre.h>
#include <OIS/OIS.h>

#include "Gui3D.h"
#include "Gui3DPanel.h"

#include "MyCompleteGUIDemoPanelColors.h"

// Used to move the sceneNode of the camera
#include "OgreSceneNodeUtils.h"

/*! class. MyLinkedPanel
    desc.
        A specialization of a panel to a linked panel
*/
class MyLinkedPanel : public Gui3D::Panel
{
public:
    //
    MyLinkedPanel(Gui3D::Gui3D* gui, 
        Ogre::SceneManager* sceneMgr, 
        const Ogre::Vector2& size,
        Ogre::String atlasName,
        Ogre::String name)
    : Gui3D::Panel(gui, sceneMgr, size, 10, atlasName, name), 
        next(nullptr), previous(nullptr)
    {}

    ~MyLinkedPanel()
    {}

    // Link the panels
    MyLinkedPanel* next;
    MyLinkedPanel* previous;
};

class DemoLinkedPanels : public Ogre::FrameListener, public OIS::KeyListener, public OIS::MouseListener
{
public:
    // OGRE Related
    Ogre::Root* mRoot;
    Ogre::RenderWindow* mWindow;
    Ogre::Viewport* mViewport;
    Ogre::SceneManager* mSceneMgr;
    Ogre::Camera* mCamera;
    Ogre::SceneNode* mCameraNode;
    OIS::InputManager* mInputManager;
    OIS::Keyboard* mKeyboard;
    OIS::Mouse* mMouse;

    // The panel that is actually focused
    MyLinkedPanel* mActualPanel;

    // For going from a panel to another, we move the camera node
    Ogre::SceneNode* mNodeToMove;
    Ogre::SceneNode* mNodeDestination;

    Gui3D::Gui3D* mGui3D;

    // Some panel element that are importants
    Gui3D::TextZone* mNicknameTextZone;
    Gui3D::TextZone* mCityTextZone;

    // The summary panel that contains all the users infos
    MyLinkedPanel* mSummaryPanel;
    // Captions of the summaryPanel to use thoses in callbacks
    Gui3D::Caption* mNicknameSummary;
    Gui3D::Caption* mCitySummary;
    Gui3D::Caption* mGenderSummary;
    Gui3D::Caption* mResolutionSummary;
    Gui3D::Caption* mMouseSpeedSummary;
    Gui3D::Caption* mMouseAxesSummary;
    Gui3D::Caption* mVisitedCitySummary;

    // Some content...
    std::vector<Ogre::String> mAvailableResolutions;

    // User values
    Ogre::String mUserNickname;
    Ogre::String mUserCity;
    Ogre::String mUserResolution;
    Ogre::String mUserGender;
    bool mUserInvertMouseAxes;
    double mUserMouseMultiplier;
    std::vector<Ogre::String> mUserVisitedCountry;

    // Keep track of the progress bar object and the button
    //  that will be activated when loading will be complete
    // Plus, keep a clock for the loading simulator + a boolean
    //  to start the fake loading
    bool fakeLoading;
    Gui3D::ProgressBar* mLoadingProgressbar;
    Gui3D::Button* mNextButtonLoadingPanel;
    Gui3D::Caption* mLoadingStateText;
    Gui3D::Clock mLoadingSimulatorClock;

    // The user mouse
    Gorilla::Layer* mMousePointerLayer;
    Gorilla::Rectangle* mMousePointer;
    Ogre::Vector2 mNormalizedMousePosition;

    // Base Direction for the camera
    Ogre::Vector3 cameraDirection;

    Gorilla::Screen* mScreen;

    // Sinbad animations to refresh on each frame
    std::vector<Ogre::AnimationState*> v_animations;

    MyCompleteGUIDemoPanelColors mMyCompleteGUIDemoPanelColors;

    DemoLinkedPanels()
    {
        _makeOgre();
        _makeOIS();

        mGui3D = new Gui3D::Gui3D(&mMyCompleteGUIDemoPanelColors);
        mScreen = mGui3D->createScreen(mViewport, "completeGUIDemo", "mainScreen");

        mNormalizedMousePosition = Ogre::Vector2(0.5, 0.5);
        mMousePointerLayer = mGui3D->getScreen("mainScreen")->createLayer();
        mMousePointer = mMousePointerLayer->createRectangle(
            mViewport->getActualWidth() / 2, 
            mViewport->getActualHeight() / 2 , 
            12,
            18);
        mMousePointer->background_image("mousepointer");

        // Let's give some value to the available resolutions
        mAvailableResolutions.push_back("800x600 16bits");
        mAvailableResolutions.push_back("1024x700 16bits");
        mAvailableResolutions.push_back("1366x700 16bits");
        mAvailableResolutions.push_back("1600x900 16bits");
        mAvailableResolutions.push_back("800x600 32bits");
        mAvailableResolutions.push_back("1024x700 32bits");
        mAvailableResolutions.push_back("1366x700 32bits");
        mAvailableResolutions.push_back("1600x900 32bits");

        // Set some defaults values...
        mUserNickname = "";
        mUserCity = "";
        mUserGender = "male";
        mUserResolution = mAvailableResolutions[2];
        mUserInvertMouseAxes = true;
        mUserMouseMultiplier = 0.70;

        fakeLoading = false;

        _createPanels();

        mNodeDestination = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mNodeDestination->setPosition(Ogre::Vector3::ZERO);
        mNodeToMove = nullptr;
    }

    ~DemoLinkedPanels()
    {
        std::ostringstream s;
        s << "\n** Average FPS (with FSAA to 1) is " << mWindow->getAverageFPS() << "\n\n";
        Ogre::LogManager::getSingleton().logMessage(s.str());
        delete mGui3D;
        delete mRoot;
    }

    bool frameStarted(const Ogre::FrameEvent& evt)
    {
        if (mWindow->isClosed())
            return false;

        mActualPanel->injectTime(evt.timeSinceLastFrame);
        
        // Fake loading for the loading panel
        if (fakeLoading)
            if (mLoadingSimulatorClock.clockAction(0.01, evt.timeSinceLastFrame))
                mLoadingProgressbar->setValue(mLoadingProgressbar->getValue() + 0.005);

        // Do we need to move some node ?
        if (mNodeDestination != nullptr && mNodeToMove != nullptr)
        {
            if (!OgreSceneNodeUtils::moveSceneNode(mNodeToMove, 
                mNodeDestination, evt.timeSinceLastFrame * 200))
            {
                mNodeDestination = nullptr;
                mNodeToMove = nullptr;
            }
        }

        // Just for fun, animate all sinbads :)
        for (int i=0; i<v_animations.size(); i++)
            v_animations[i]->addTime(evt.timeSinceLastFrame);

        mMouse->capture();

        // Quit on ESCAPE Keyboard
        mKeyboard->capture();
        if (mKeyboard->isKeyDown(OIS::KC_ESCAPE))
            return false;

        return true;
    }

    bool keyPressed(const OIS::KeyEvent &e)
    {
        mActualPanel->injectKeyPressed(e);
        return true;
    }

    bool keyReleased(const OIS::KeyEvent &e)
    {
        mActualPanel->injectKeyReleased(e);
        return true;
    }

    // Calculate and normalize the distance between the middle of the screen 
    //    and the mouse on both X and Y axis.
    //    This is used for the smooth movement of the camera
    //    when the mouse is moving.
    Ogre::Vector2 getScreenCenterMouseDistance()
    {
        double posX = (mMousePointer->position().x - mViewport->getActualWidth()) 
            / mViewport->getActualWidth();
        double posY = (mMousePointer->position().y - mViewport->getActualHeight()) 
            / mViewport->getActualHeight();

        return Ogre::Vector2(posX + 0.5, posY + 0.5);
    }

    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        // Set the new camera smooth direction movement
        Ogre::Vector2 distance(getScreenCenterMouseDistance());
        mCamera->setDirection(cameraDirection
            + Ogre::Vector3(distance.x, -distance.y, 0) / 30);
        
        float xMove = static_cast<float>(arg.state.X.rel);
        float yMove = static_cast<float>(arg.state.Y.rel);
    
        mNormalizedMousePosition.x += xMove / mViewport->getActualWidth();
        mNormalizedMousePosition.y += yMove / mViewport->getActualHeight();

        // Make sur the mouse isn't out of the screen
        mNormalizedMousePosition.x = std::min<Ogre::Real>(
            std::max<Ogre::Real>(mNormalizedMousePosition.x, 0), 1);
        mNormalizedMousePosition.y = std::min<Ogre::Real>(
            std::max<Ogre::Real>(mNormalizedMousePosition.y, 0), 1);

        // Mouse position on the screen
        mMousePointer->position(
            mNormalizedMousePosition.x * mViewport->getActualWidth(), 
            mNormalizedMousePosition.y * mViewport->getActualHeight());

        // Ray for the actual panel to know where the mouse is
        mActualPanel->injectMouseMoved(
            mCamera->getCameraToViewportRay(
                mNormalizedMousePosition.x, mNormalizedMousePosition.y));

        return true;
    }

    bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
    {
        mActualPanel->injectMousePressed(evt, id);
        return true;
    }

    bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id)
    {
        mActualPanel->injectMouseReleased(evt, id);
        return true;
    }


    /**
        Callback methods
    */

    bool goNextPanel(Gui3D::PanelElement* e)
    {
        if (mActualPanel->next == nullptr)
            return false;
        
        mActualPanel = mActualPanel->next;
        mActualPanel->unOverAllElements();
        mActualPanel->getGUILayer()->show();
        mNodeDestination = mActualPanel->mPanelCameraNode;
        mNodeToMove = mCameraNode;
        return true;
    }

    bool goPreviousPanel(Gui3D::PanelElement* e)
    {
        if (mActualPanel->previous == nullptr)
            return false;

        mActualPanel->getGUILayer()->hide();
        mActualPanel = mActualPanel->previous;
        mActualPanel->unOverAllElements();
        mNodeDestination = mActualPanel->mPanelCameraNode;
        mNodeToMove = mCameraNode;
        return true;
    }

    bool validNicknameCityPanel(Gui3D::PanelElement* e)
    {
        if (mUserNickname.size() == 0)
        {
            mActualPanel->setFocusedElement(mNicknameTextZone);
            mNicknameTextZone->highlight();
            return false;
        }
        else if (mUserCity.size() == 0)
        {
            mActualPanel->setFocusedElement(mCityTextZone);
            mCityTextZone->highlight();
            return false;
        }

        return goNextPanel(nullptr);
    }

    bool nicknameChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::TextZone* tz = (Gui3D::TextZone*) e;
        mUserNickname = tz->getValue();
        Ogre::String s("Nickname : ");
        s.append(mUserNickname);
        mNicknameSummary->text(s);
        return true;
    }

    bool cityChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::TextZone* tz = (Gui3D::TextZone*) e;
        mUserCity = tz->getValue();
        Ogre::String s("City : ");
        s.append(mUserCity);
        mCitySummary->text(s);
        return true;
    }

    bool genderChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::InlineSelector* iSelector = (Gui3D::InlineSelector*) e;
        mUserGender = iSelector->getValue();
        Ogre::String s("Gender : ");
        s.append(mUserGender);
        mGenderSummary->text(s);
        return true;
    }

    bool invertMouseAxesChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::Checkbox* ch = (Gui3D::Checkbox*) e;
        mUserInvertMouseAxes = ch->getChecked();
        Ogre::String s("Invert mouse axes : ");
        mUserInvertMouseAxes ? s.append("Yes") : s.append("No");
        mMouseAxesSummary->text(s);
        return true;
    }
    
    bool resolutionChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::Combobox* cb = (Gui3D::Combobox*) e;
        mUserResolution = cb->getValue();
        Ogre::String s("Resolution : ");
        s.append(mUserResolution);
        mResolutionSummary->text(s);
        return true;
    }

    bool countryVisitedChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::Listbox* cb = (Gui3D::Listbox*) e;
        mUserVisitedCountry = cb->getValues();
        Ogre::String s("Visited country : ");
        for (size_t i=0; i < mUserVisitedCountry.size(); i++)
        {
            s.append(mUserVisitedCountry[i].substr(0, 2));
            if (i != mUserVisitedCountry.size() - 1)
                s.append(",");
        }
        mVisitedCitySummary->text(s);
        return true;
    }

    bool mouseMultiplierChangedCallback(Gui3D::PanelElement* e)
    {
        Gui3D::ScrollBar* sb = (Gui3D::ScrollBar*) e;
        mUserMouseMultiplier = sb->getValue();
        Ogre::String s("Mouse multiplier : ");
        std::ostringstream _s;
        _s << mUserMouseMultiplier;
        s.append(_s.str());
        mMouseSpeedSummary->text(s);
        return true;
    }

    bool valueChangedProgressBarCallback(Gui3D::PanelElement* e)
    {
        Gui3D::ProgressBar* pb = (Gui3D::ProgressBar*) e;
        double value = pb->getValue();

        // Is it complete ?
        if (value == 1)
        {
            fakeLoading = false;
            mNextButtonLoadingPanel->setActive(true);

            mLoadingStateText->text(Ogre::String("Everything is loaded :)"));
            mLoadingStateText->textColor(Gorilla::rgb(0, 255, 30, 200));
        }
        else
        {
            // It's loading
            if (mLoadingStateText != nullptr)
            {
                std::ostringstream s;
                s << "Loading... (" << std::fixed << std::setprecision(2) << (1 - value)*100 << "% remains)";
                mLoadingStateText->text(s.str());
                mLoadingStateText->textColor(Gorilla::rgb(250, 200, 100, 200));
            }
        }
        
        return true;
    }

    bool startLoading(Gui3D::PanelElement* e)
    {
        fakeLoading = true;
        return true;
    }

    bool restartLoading(Gui3D::PanelElement* e)
    {
        mLoadingStateText->text(Ogre::String("Nothing is loaded yet..."));
        mLoadingStateText->textColor(Gorilla::rgb(255, 0, 0, 200));
        
        fakeLoading = false;
        mLoadingSimulatorClock.reset();
        mLoadingProgressbar->reset();
        mNextButtonLoadingPanel->setActive(false);
        return true;
    }




    /**
        Create Panels methods
    */

    MyLinkedPanel* createSummaryPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 260), "completeGUIDemo", "panelSummary");

        panel->makeCaption(5, -30, 390, 30, "Summary Panel");

        std::ostringstream s;
        s << "Nickname : " << mUserNickname;
        mNicknameSummary = panel->makeCaption(5, 10, 300, 30, s.str());
        s.str("");
        s << "City : " << mUserCity;
        mCitySummary = panel->makeCaption(5, 40, 300, 30, s.str());
        s.str("");
        s << "Gender : " << mUserGender;
        mGenderSummary = panel->makeCaption(5, 70, 300, 30, s.str());
        s.str("");
        s << "Resolution : " << mUserResolution;
        mResolutionSummary = panel->makeCaption(5, 100, 300, 30, s.str());
        s.str("");
        s << "Mouse speed : " << mUserMouseMultiplier;
        mMouseSpeedSummary = panel->makeCaption(5, 130, 300, 30, s.str());
        s.str("");
        s << "Invert mouse axes: " << mUserInvertMouseAxes;
        mMouseAxesSummary = panel->makeCaption(5, 160, 300, 30, s.str());
        s.str("");
        s << "Visited country : ";
        for (size_t i=0; i < mUserVisitedCountry.size(); i++)
        {
            s << mUserVisitedCountry[i].substr(0, 2);
            if (i != mUserVisitedCountry.size() - 1)
                s << ",";
        }
        mVisitedCitySummary = panel->makeCaption(5, 190, 395, 30, s.str());

        panel->makeButton(100, 220, 200, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);

        return panel;
    }

    MyLinkedPanel* createInfosPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 200), "completeGUIDemo", "panelInfo");
        
        panel->makeCaption(5, -30, 390, 30, "Infos Panel");

        std::vector<Ogre::String> txtDesc;
        txtDesc.push_back("Hi ! Welcome on the Gui3D demo.");
        txtDesc.push_back("It'll show you what you can do with Gui3D !");
        txtDesc.push_back("There's a summary panel at the end.");
        txtDesc.push_back("Click on the Start button to discover it !");
        for (size_t i=0; i < txtDesc.size(); i++)
        {
            panel->makeCaption(2, 20 + 25*i, 400, 30, txtDesc[i], 
                Gorilla::TextAlign_Centre, Gorilla::VerticalAlignment::VerticalAlign_Middle);
        }
        
        panel->makeButton(100, 160, 200, 30, "Start")
            ->setPressedCallback(this, &DemoLinkedPanels::goNextPanel);

        return panel;
    }

    MyLinkedPanel* createTestPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 400), "completeGUIDemo", "panelInfo");

        /*
        Gui3D::Button* b = panel->makeButton(200, 200, 100, 100, "TEST");
        b->setSize(200, 50);
        b->setPosition(200, 200);
        */

        /*
        Gui3D::Caption* c = panel->makeCaption(40, 100, 100, 20, "test");
        c->setSize(40, 200);
        c->setPosition(200, 200);
        c->getCaption()->background(Gorilla::rgb(255, 0, 0));
        */

        //Gui3D::Checkbox* ck = panel->makeCheckbox(100, 100, 50, 50);

        /*
        Gui3D::Combobox* c = 
            panel->makeCombobox(100, 100, 300, 210, mAvailableResolutions, 5);
        c->setPosition(100, 100);
        c->setSize(100, 200);
        */

        /*
        std::vector<Ogre::String> genders;
        genders.push_back("Male");
        genders.push_back("Female");
        genders.push_back("Unknow");
        Gui3D::InlineSelector* inlineSelector = panel->makeInlineSelector(0, 100, 300, 50, genders);
        inlineSelector->setPosition(100, 125);
        inlineSelector->setSize(300, 40);
        */

        /*
        std::vector<Ogre::String> mCountry;
        mCountry.push_back("Canada");
        mCountry.push_back("England");
        mCountry.push_back("Europe");
        mCountry.push_back("France");
        mCountry.push_back("USA");
        mCountry.push_back("Mexico");
        mCountry.push_back("South America");
        mCountry.push_back("Others");

        Gui3D::Listbox* listBox = 
            panel->makeListbox(50, 60, 300, 210, mCountry, 5);
        listBox->getRectangleDesign()->background_colour(Gorilla::rgb(255, 0, 0));
        listBox->setPosition(50,0);
        listBox->setSize(350, 100);
        listBox->setPosition(150,0);
        */

        /*
        Gui3D::ProgressBar* p = panel->makeProgressBar(100, 100, 200, 50);
        p->setValue(0.3);
        p->setPosition(200, 200);
        p->setSize(100, 40);
        */

        /*
        Gui3D::ScrollBar* s = panel->makeScrollBar(100, 100, 200, 120, 21.01, 199.62);
        s->setStep(50.09);
        s->setPosition(0, 0);
        s->setValue(64.49);
        */

        /*
        Gui3D::TextZone* t = panel->makeTextZone(0, 0, 100, 100, "TEST");
        t->setPosition(100, 100);
        t->setSize(200, 50);
        */

        return panel;
    }

    MyLinkedPanel* createVisitedCountryPanels()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 330), "completeGUIDemo", "travelPanel");

        Gui3D::Caption* c = panel->makeCaption(5, -30, 390, 30, "Travel Panel");

        panel->makeCaption(5, 20, 390, 30, "Where have you ever traveled", 
            Gorilla::TextAlign_Centre);

        std::vector<Ogre::String> mCountry;
        mCountry.push_back("Canada");
        mCountry.push_back("England");
        mCountry.push_back("Europe");
        mCountry.push_back("France");
        mCountry.push_back("USA");
        mCountry.push_back("Mexico");
        mCountry.push_back("South America");
        mCountry.push_back("Others");

        // The user has already visited some country..
        mUserVisitedCountry.push_back(mCountry[0]);
        mUserVisitedCountry.push_back(mCountry[1]);
        mUserVisitedCountry.push_back(mCountry[5]);

        Gui3D::Listbox* listBox = 
            panel->makeListbox(50, 60, 300, 210, mCountry, 4);
        listBox->setValueChangedCallback(this, &DemoLinkedPanels::countryVisitedChangedCallback);
        listBox->setCurrentValues(mUserVisitedCountry);

        panel->makeButton(10, 290, 180, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);
        panel->makeButton(200, 290, 180, 30, "Next")
            ->setPressedCallback(this, &DemoLinkedPanels::goNextPanel);

        return panel;
    }

    MyLinkedPanel* createLoadingPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 220), "completeGUIDemo", "loadingPanel");
        
        panel->makeCaption(5, -30, 390, 30, "Loading Panel");

        panel->makeCaption(5, 10, 390, 30, 
            "Click on the \"Loading button\" to start.", Gorilla::TextAlign_Centre);

        mLoadingProgressbar = panel->makeProgressBar(10, 40, 380, 30);
        mLoadingProgressbar
            ->setValueChangedCallback(this, &DemoLinkedPanels::valueChangedProgressBarCallback);
        mLoadingSimulatorClock.reset();

        mLoadingStateText = panel->makeCaption(10, 80, 380, 30, "Nothing is loaded yet...", Gorilla::TextAlign_Centre);
        mLoadingStateText->textColor(Gorilla::rgb(255, 0, 0, 200));

        panel->makeButton(10, 120, 180, 30, "Restart Loading")
            ->setPressedCallback(this, &DemoLinkedPanels::restartLoading);
        panel->makeButton(200, 120, 190, 30, "Start Loading")
            ->setPressedCallback(this, &DemoLinkedPanels::startLoading);

        panel->makeButton(10, 180, 180, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);
        mNextButtonLoadingPanel = panel->makeButton(200, 180, 180, 30, "Next");
        mNextButtonLoadingPanel->setActive(false);
        mNextButtonLoadingPanel->setPressedCallback(this, &DemoLinkedPanels::goNextPanel);

        return panel;
    }

    MyLinkedPanel* createProfilPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 290), "completeGUIDemo", "panelProfil");

        panel->makeCaption(5, -30, 390, 30, "Profil Panel");

        panel->makeCaption(5, 5, 390, 30, "Nickname (max 10 chars), required : ", Gorilla::TextAlign_Centre);

        mNicknameTextZone = panel->makeTextZone(50, 40, 300, 30, mUserNickname);
        mNicknameTextZone->setValueChangedCallback(this, &DemoLinkedPanels::nicknameChangedCallback);
        mNicknameTextZone->setMaxStringLength(10);

        panel->makeCaption(5, 90, 390, 30, "City (max 15 chars), required : ", Gorilla::TextAlign_Centre);

        mCityTextZone = panel->makeTextZone(50, 125, 300, 30, mUserCity);
        mCityTextZone->setValueChangedCallback(this, &DemoLinkedPanels::cityChangedCallback);
        mCityTextZone->setMaxStringLength(15);

        panel->makeCaption(5, 165, 390, 30, "Gender : ", Gorilla::TextAlign_Centre);

        std::vector<Ogre::String> genders;
        genders.push_back("Male");
        genders.push_back("Female");
        genders.push_back("Unknow");
        Gui3D::InlineSelector* inlineSelector = panel->makeInlineSelector(50, 200, 300, 30, genders);
        inlineSelector->setValueChangedCallback(this, &DemoLinkedPanels::genderChangedCallback);

        panel->makeButton(10, 250, 180, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);
        panel->makeButton(200, 250, 180, 30, "Next")
            ->setPressedCallback(this, &DemoLinkedPanels::validNicknameCityPanel);

        return panel;
    }

    MyLinkedPanel* createMousePanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 200), "completeGUIDemo", "panelMouse");

        panel->makeCaption(5, -30, 390, 30, "Mouse Panel");

        panel->makeCaption(5, 20, 390, 30, "Set the mouse speed multiplier : ", 
            Gorilla::TextAlign_Centre);

        Gui3D::ScrollBar* scrollBarMouseMultiplier = 
            panel->makeScrollBar(20, 50, 360, 30, 0, 1);
        scrollBarMouseMultiplier->setValueChangedCallback(this, &DemoLinkedPanels::mouseMultiplierChangedCallback);
        scrollBarMouseMultiplier->setValue(mUserMouseMultiplier);
        scrollBarMouseMultiplier->setStep(0.01);

        panel->makeCaption(10, 100, 200, 30, "Invert mouse axes : ", Gorilla::TextAlign_Centre);

        Gui3D::Checkbox* checkboxMouseInvertAxes = 
            panel->makeCheckbox(210, 100, 30, 30); 
        checkboxMouseInvertAxes->setSelecteStateChangedCallback(this, &DemoLinkedPanels::invertMouseAxesChangedCallback);
        checkboxMouseInvertAxes->setChecked(mUserInvertMouseAxes);
        
        panel->makeButton(10, 160, 180, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);
        panel->makeButton(200, 160, 180, 30, "Next")
            ->setPressedCallback(this, &DemoLinkedPanels::goNextPanel);

        return panel;
    }

    MyLinkedPanel* createResolutionPanel()
    {
        MyLinkedPanel* panel = new MyLinkedPanel(
            mGui3D, mSceneMgr, Ogre::Vector2(400, 340), "completeGUIDemo", "panelResolution");

        panel->makeCaption(5, -30, 390, 30, "Resolution Panel");

        panel->makeCaption(5, 10, 390, 30, "Choose your prefered resolution", Gorilla::TextAlign_Centre);
        
        Gui3D::Combobox* comboboxResolution = 
            panel->makeCombobox(50, 50, 300, 210, mAvailableResolutions, 4);
        comboboxResolution->setValueChangedCallback(this, &DemoLinkedPanels::resolutionChangedCallback);
        comboboxResolution->setCurrentValue(mUserResolution);

        panel->makeButton(10, 300, 180, 30, "Back")
            ->setPressedCallback(this, &DemoLinkedPanels::goPreviousPanel);
        panel->makeButton(200, 300, 180, 30, "Next")
            ->setPressedCallback(this, &DemoLinkedPanels::goNextPanel);

        return panel;
    }

    void _createPanels()
    {
        int rotationDegree = 30;

        struct panelCreationInfo
        {
        public:
            panelCreationInfo::panelCreationInfo(
                MyLinkedPanel* (DemoLinkedPanels::*method)(),
                Ogre::Vector3 pos,
                Ogre::Degree rot) : createMethod(method), position(pos), rotation(rot)
                {}
            MyLinkedPanel* (DemoLinkedPanels::*createMethod)();
            Ogre::Vector3 position;
            Ogre::Degree rotation;
        };

        //
        Ogre::Vector3 shift(28, 0, 25);

        // Create the summary panel first as callback will refresh it
        //  on creation of all panels for defaults values
        mSummaryPanel = createSummaryPanel();
        mSummaryPanel->mNode->setPosition(Ogre::Vector3(0, 2, -10) + 6*shift);
        mSummaryPanel->mNode->yaw(Ogre::Degree(rotationDegree));
        mSummaryPanel->next = nullptr;
        
        std::vector<struct panelCreationInfo> panels;

        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createInfosPanel, 
            Ogre::Vector3(0, 2, 0), 
            Ogre::Degree(rotationDegree)));
        
        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createResolutionPanel,
            Ogre::Vector3(0, 2, -10) + 1*shift, 
            Ogre::Degree(rotationDegree)));
        
        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createProfilPanel,
            Ogre::Vector3(0, 2, -10) + 2*shift, 
            Ogre::Degree(rotationDegree)));
        
        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createVisitedCountryPanels,
            Ogre::Vector3(0, 2, -10) + 3*shift, 
            Ogre::Degree(rotationDegree)));
        
        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createMousePanel, 
            Ogre::Vector3(0, 2, -10) + 4*shift, 
            Ogre::Degree(rotationDegree)));
        
        panels.push_back(struct panelCreationInfo(&DemoLinkedPanels::createLoadingPanel, 
            Ogre::Vector3(0, 2, -10) + 5*shift, 
            Ogre::Degree(rotationDegree)));
        
        MyLinkedPanel* lastCreatedPanel = nullptr;

        Ogre::SceneNode* m = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        m->attachObject(mSceneMgr->createEntity("tudorhouse.mesh"));
        m->setPosition(80, 27, -100);
        m->setScale(0.05, 0.05, 0.05);

        // Put the panels in the world
        for (size_t i=0; i < panels.size(); i++)
        {
            if (i > 0)
            {
                Ogre::SceneNode* n = nullptr;
                
                n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
                n->setScale(0.03, 0.03, 0.03);
                n->setPosition(panels[i].position + Ogre::Vector3(3, -2, 0));
                n->attachObject(mSceneMgr->createEntity("arbre.mesh"));
                
                n = mSceneMgr->getRootSceneNode()->createChildSceneNode();
                n->setScale(0.2, 0.2, 0.2);
                n->setPosition(panels[i].position + Ogre::Vector3(2, -1, 0));
                Ogre::Entity* ent = mSceneMgr->createEntity("sinbad.mesh");

                if (i%2 == 0)
                {
                    Ogre::AnimationState* a = ent->getAnimationState("Dance");
                    a->setEnabled(true);
                    v_animations.push_back(a);
                }
                else
                {
                    ent->getSkeleton()->setBlendMode(Ogre::ANIMBLEND_CUMULATIVE);
                    Ogre::AnimationState* a = ent->getAnimationState("IdleBase");
                    a->setEnabled(true);
                    a->setLoop(true);
                    v_animations.push_back(a);
                    Ogre::AnimationState* a2 = ent->getAnimationState("IdleTop");
                    a2->setEnabled(true);
                    a2->setLoop(true);
                    v_animations.push_back(a2);
                }
                n->attachObject(ent);
            }

            MyLinkedPanel* panel = (this->*(panels[i].createMethod))();
            panel->mNode->setPosition(panels[i].position);
            panel->mNode->yaw(panels[i].rotation);
            panel->previous = lastCreatedPanel;
            // Next will be set on next loop
            panel->next = nullptr;

            // First panel created ? Setup things
            if (lastCreatedPanel == nullptr)
            {
                mCameraNode->setPosition(panel->mPanelCameraNode->_getDerivedPosition());

                // This will be the actual panel
                mActualPanel = panel;
            }
            else
            {
                panel->getGUILayer()->hide();
                // Chain it with previous panels
                lastCreatedPanel->next = panel;
            }

            lastCreatedPanel = panel;
        }

        // Don't forget to chain the summary panel
        mSummaryPanel->previous = lastCreatedPanel;
        lastCreatedPanel->next = mSummaryPanel;
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
        mWindow = mRoot->createRenderWindow("Gorilla", (int)800/1, (int)600/1, false, &misc);
        mSceneMgr = mRoot->createSceneManager(Ogre::ST_GENERIC);
        mCamera = mSceneMgr->createCamera("Camera");
        mViewport = mWindow->addViewport(mCamera);

        mCamera->setNearClipDistance(5);
        mCamera->setFarClipDistance(30000);
        mSceneMgr->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));
        mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE);
        Ogre::Light* light = mSceneMgr->createLight("Light");
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(-1, -1, -1);
        light->setDiffuseColour(Ogre::ColourValue(200.f / 255.f, 
            200.f / 255.f, 
            191.f / 255.f));
        
        mViewport->setBackgroundColour(Ogre::ColourValue(180./255, 115./255, 175./255, .5f));

        rgm->initialiseAllResourceGroups();

        cameraDirection = Ogre::Vector3(-0.1, 0, -0.3);

        mCameraNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCameraNode->attachObject(mCamera);

        mCamera->setDirection(cameraDirection);

        mCamera->setNearClipDistance(0.05f);
        mCamera->setFarClipDistance(1000);

        _makeScene();
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
};

int main()
{
    DemoLinkedPanels* demo  = new DemoLinkedPanels();
    demo->mRoot->startRendering();
    delete demo;

    return 0;
}