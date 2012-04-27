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

#ifndef Gui3DPanel_H
#define Gui3DPanel_H



#include <vector>

#include <OIS\OIS.h>

#include "Gorilla.h"

#include "Gui3D.h"
#include "Gui3DContainer.h"

#include "Gui3DPanelElement.h"

#include "Gui3DButton.h"
#include "Gui3DCaption.h"
#include "Gui3DCheckbox.h"
#include "Gui3DCheckboxText.h"
#include "Gui3DCheckboxSprite.h"
#include "Gui3DCombobox.h"
#include "Gui3DInlineSelector.h"
#include "Gui3DListbox.h"
#include "Gui3DProgressBar.h"
#include "Gui3DScrollBar.h"
#include "Gui3DTextZone.h"

namespace Gui3D
{

//
class Panel : public Container
{
public:
	Panel(Gui3D* gui, 
		Ogre::SceneManager* sceneMgr, 
		const Ogre::Vector2& size, 
		Ogre::String atlasName,
		Ogre::String name);

	~Panel();

	/** \brief Give the focus to an element of this panel.
		note.
			If the element isn't contained by this panel, nothing will happened.
	*/
	void setFocusedElement(PanelElement* e);
	
	void injectKeyPressed(const OIS::KeyEvent& evt);

	void injectKeyReleased(const OIS::KeyEvent& evt);

	void injectMousePressed(const OIS::MouseEvent& evt,
		OIS::MouseButtonID id);

	void injectMouseReleased(const OIS::MouseEvent& evt, 
		OIS::MouseButtonID id);

	/** \brief Send a ray from a part of the screen to this panel to
	actualize the internal mouse position on the panel.
	*/
	void injectMouseMoved(const Ogre::Ray& ray);

	/** \brief Inject time to have animations with panel elements, such as
	a flash on and off of a cursor in Textzones
	*/
	void injectTime(double time);

	/** \brief UnOver all elements.
	*/
	void unOverAllElements();

	Button* makeButton(Ogre::Real x, 
		Ogre::Real y, 
		size_t width,
		size_t height,
		const Ogre::String& text);

	Caption* makeCaption(Ogre::Real x, 
		Ogre::Real y,
		size_t width,
		size_t height,
		const Ogre::String& text,
		Gorilla::TextAlignment textAlign = Gorilla::TextAlign_Left,
		Gorilla::VerticalAlignment verticalAlign = Gorilla::VerticalAlign_Top);

	Checkbox* makeCheckbox(Ogre::Real x,
		Ogre::Real y,
		size_t width,
		size_t height);

	Combobox* makeCombobox(Ogre::Real x, 
		Ogre::Real y,
		size_t width,
		size_t height,
		const std::vector<Ogre::String>& items,
		unsigned int nbDisplayedElements);

	InlineSelector* makeInlineSelector(Ogre::Real x,
		Ogre::Real y,
		size_t width,
		size_t height,
		const std::vector<Ogre::String>& items);

	Listbox* makeListbox(Ogre::Real x, 
		Ogre::Real y,
		size_t width,
		size_t height,
		const std::vector<Ogre::String>& items,
		unsigned int nbDisplayedElements);

	ProgressBar* makeProgressBar(Ogre::Real x, 
		Ogre::Real y,
		size_t width,
		size_t height);

	ScrollBar* makeScrollBar(Ogre::Real x, 
		Ogre::Real y,
		size_t width,
		size_t height,
		Ogre::Real minValue,
		Ogre::Real maxValue);

	TextZone* makeTextZone(Ogre::Real x,
		Ogre::Real y,
		size_t width,
		size_t height,
		const Ogre::String& text);

	void destroyButton(Button*);
	
	void destroyCaption(Caption*);
	
	void destroyCheckbox(Checkbox*);
	
	void destroyCombobox(Combobox*);
	
	void destroyInlineSelector(InlineSelector*);
	
	void destroyProgressBar(ProgressBar*);
	
	void destroyScrollBar(ScrollBar*);
	
	void destroyTextZoneEditable(TextZone*);

	Ogre::SceneNode* mNode; //!< \brief The panel node
	Ogre::SceneNode* mPanelCameraNode; //!< \brief The node the camera should be attached to and look at mNode for this panel

protected:
	Gorilla::ScreenRenderable* mScreenRenderable;
	Gorilla::Rectangle* mBackground;
	Ogre::Vector2 mSize;

	Ogre::Vector2 mInternalMousePos; //!< \brief The Internal position of the mouse in the panel

	std::vector<PanelElement*> mPanelElements;
	PanelElement* mFocusedPanelElement; //!< \brief The element that actually have the focus

	/** \brief Save key pressed when there's no focused element
	If a focusedElement is set before keyReleased, these keys are then sent
	*/
	std::vector<OIS::KeyCode> mKeyCodes;

	/** \brief Internal use. Use make[XXX] to add element to this panel.
	Add an item to the panel.
	*/
	void addItem(PanelElement* item);

	/** \brief Internal use. Use destroy[XXX] to remove an element to this panel.
	Remove an item to the panel.
	*/
	void removeItem(PanelElement* item);
};

}

#endif