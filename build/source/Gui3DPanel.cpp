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

#include "Gui3DPanel.h"
#include "Gui3DPanelColors.h"

namespace Gui3D
{

using namespace std;

Panel::Panel(Gui3D* gui, 
	Ogre::SceneManager* sceneMgr, 
	const Ogre::Vector2& size,
	Ogre::Real distanceFromPanelToInteractWith,
	Ogre::String atlasName,
	Ogre::String name)
	: Container(gui), mSize(size), mFocusedPanelElement(nullptr),
		mDistanceFromPanelToInteractWith(distanceFromPanelToInteractWith)
{
	mScreenRenderable = 
		gui->createScreenRenderable(Ogre::Vector2(mSize.x, mSize.y), atlasName, name);

	mNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
	mNode->attachObject(mScreenRenderable);

	mPanelCameraNode = mNode->createChildSceneNode();
	mPanelCameraNode->setPosition(-1, 0, 7);
	mPanelCameraNode->lookAt(mNode->getPosition(), Ogre::Node::TS_PARENT);
	
	mGUILayer = gui->createLayer(mScreenRenderable, name);
	
	mBackground = mGUILayer->createRectangle(0, 0, mSize.x * 100, mSize.y * 100);

	if (getPanelColors()->panelBackgroundSpriteName.length() == 0 ||
		getPanelColors()->panelBackgroundSpriteName == "none")
	{
		mBackground->background_gradient(mGui3D->getPanelColors()->panelGradientType,
			mGui3D->getPanelColors()->panelGradientColorStart,
			mGui3D->getPanelColors()->panelGradientColorEnd);
		mBackground->border(mGui3D->getPanelColors()->panelBorderSize, 
			mGui3D->getPanelColors()->panelBorder);
	}
	else
		mBackground->background_image(getPanelColors()->panelBackgroundSpriteName);


	// Create an empty mouse pointer which follow the mouse cursor
	mMousePointer = mGUILayer->createRectangle(0, 0, 0, 0);

	if (getPanelColors()->panelCursorSpriteName.length() == 0 ||
		getPanelColors()->panelCursorSpriteName == "none")
	{
		mMousePointer->background_colour(Gorilla::Colours::None);
	}
	else
	{
		mMousePointer->width(getPanelColors()->panelCursorSpriteSizeX);
		mMousePointer->height(getPanelColors()->panelCursorSpriteSizeY);
		mMousePointer->background_image(getPanelColors()->panelCursorSpriteName);
	}
}


Panel::~Panel()
{
	for (size_t i=0; i < mPanelElements.size(); i++)
		delete mPanelElements[i];

	// Destroy all elements that had been created on the screen renderable
	mGui3D->destroyScreenRenderable(mScreenRenderable);
}


void Panel::setFocusedElement(PanelElement* e)
{
	for (size_t i=0; i < mPanelElements.size(); i++)
	{
		if (mPanelElements[i] == e)
		{
			e->setFocus(true);
			mFocusedPanelElement = e;
			break;
		}
	}
}


void Panel::setDistanceFromPanelToInteractWith(Ogre::Real distanceFromPanelToInteractWith)
{
	mDistanceFromPanelToInteractWith = distanceFromPanelToInteractWith;
}


void Panel::addItem(PanelElement* item)
{
	mPanelElements.push_back(item);
}


void Panel::removeItem(PanelElement* item)
{
	vector<PanelElement*>::iterator it = 
		mPanelElements.begin();
	while (it != mPanelElements.end())
	{
		if (*it == item)
		{
			mPanelElements.erase(it);
			return;
		}
		it++;
	}
}


void Panel::injectKeyPressed(const OIS::KeyEvent& evt)
{
	if (mFocusedPanelElement == nullptr)
	{
		// Save the pressed key to eventually send to the futur focusedElement
		mKeyCodes.push_back(evt.key);
		return;
	}
	else
	{
		mFocusedPanelElement->injectKeyPressed(evt);
		if (!mFocusedPanelElement->getFocus())
			mFocusedPanelElement = nullptr;
	}
}


void Panel::injectKeyReleased(const OIS::KeyEvent& evt)
{
	if (mFocusedPanelElement == nullptr)
	{
		// Remove the key from the vector
		vector<OIS::KeyCode>::iterator it = mKeyCodes.begin();
		while (it != mKeyCodes.end())
		{
			if (*it == evt.key)
			{
				mKeyCodes.erase(it);
				return;
			}
			it++;
		}
	}
	else
	{
		mFocusedPanelElement->injectKeyReleased(evt);
		if (!mFocusedPanelElement->getFocus())
			mFocusedPanelElement = nullptr;
	}
}


void Panel::injectMousePressed(const OIS::MouseEvent& evt, 
	OIS::MouseButtonID id)
{
	bool found = false;
	
	// Try to find the element that has been clicked
	for (size_t i=0; i < mPanelElements.size(); i++)
	{
		if (mPanelElements[i]->isOver(mInternalMousePos))
		{
			if (mFocusedPanelElement != nullptr &&
				mFocusedPanelElement != mPanelElements[i])
				mFocusedPanelElement->setFocus(false);

			mFocusedPanelElement = mPanelElements[i];
			mFocusedPanelElement->setFocus(true);
			found = true;
			break;
		}
	}

	// It's a click on the panel.
	if (!found)
	{
		if (mFocusedPanelElement != nullptr)
		{
			mFocusedPanelElement->setFocus(false);
			mFocusedPanelElement = nullptr;
		}
		return;
	}

	// Send key to the new focused Elements
	mFocusedPanelElement->injectKeys(mKeyCodes);
	mKeyCodes.clear();
	mFocusedPanelElement->injectMousePressed(evt, id);
	if (!mFocusedPanelElement->getFocus())
		mFocusedPanelElement = nullptr;
}


void Panel::injectMouseReleased(const OIS::MouseEvent& evt, 
	OIS::MouseButtonID id)
{
	if (mFocusedPanelElement == nullptr)
		return;

	mFocusedPanelElement->injectMouseReleased(evt, id);
	if (!mFocusedPanelElement->getFocus())
		mFocusedPanelElement = nullptr;
}


bool Panel::injectMouseMoved(const Ogre::Ray& ray)
{
	Ogre::Matrix4 transform;
	transform.makeTransform(mNode->getPosition(), mNode->getScale(), mNode->getOrientation());
   
	Ogre::AxisAlignedBox aabb = mScreenRenderable->getBoundingBox();
	aabb.transform(transform);
	pair<bool, Ogre::Real> result = Ogre::Math::intersects(ray, aabb);

	if (result.first == false)
	{
		unOverAllElements();
		return false;
	}

	Ogre::Vector3 a,b,c,d;
	Ogre::Vector2 halfSize = mSize * 0.5f;
	a = transform * Ogre::Vector3(-halfSize.x,-halfSize.y,0);
	b = transform * Ogre::Vector3( halfSize.x,-halfSize.y,0);
	c = transform * Ogre::Vector3(-halfSize.x, halfSize.y,0);
	d = transform * Ogre::Vector3( halfSize.x, halfSize.y,0);
    
	result = Ogre::Math::intersects(ray, c, b, a);
	if (result.first == false)
		result = Ogre::Math::intersects(ray, c, d, b);
	if (result.first == false)
	{
		unOverAllElements();
		return false;
	}
	if (result.second > mDistanceFromPanelToInteractWith)
	{
		unOverAllElements();
		return false;
	}

	Ogre::Vector3 hitPos = (ray.getOrigin() + (ray.getDirection() * result.second));
	Ogre::Vector3 localPos = transform.inverse() * hitPos;
	localPos.x += halfSize.x;
	localPos.y -= halfSize.y;
	localPos.x *= 100;
	localPos.y *= 100;
   
	// Cursor clip
	localPos.x = Ogre::Math::Clamp<Ogre::Real>(localPos.x, 0, (mSize.x * 100) - 10);
	localPos.y = Ogre::Math::Clamp<Ogre::Real>(-localPos.y, 0, (mSize.y * 100) - 18);

	mInternalMousePos = Ogre::Vector2(localPos.x, localPos.y);
	mMousePointer->position(mInternalMousePos);

	// Let's actualize the "over" for each elements
	for (size_t i=0; i < mPanelElements.size(); i++)
		mPanelElements[i]->isOver(mInternalMousePos);

	return true;
}


void Panel::unOverAllElements()
{
	// UnOver all panelElement by putting the mouse outside of 
	//  the panel and actualize the over for each elements
	mInternalMousePos = Ogre::Vector2(-1, -1);
	for (size_t i=0; i < mPanelElements.size(); i++)
		mPanelElements[i]->isOver(mInternalMousePos);

	if (mFocusedPanelElement != nullptr)
	{
		mFocusedPanelElement->setFocus(false);
		mFocusedPanelElement = nullptr;
	}
}


void Panel::injectTime(double time)
{
	for (size_t i=0; i < mPanelElements.size(); i++)
		mPanelElements[i]->injectTimeAndMousePosition(time, mInternalMousePos);
}


Button* Panel::makeButton(Ogre::Real x, 
	Ogre::Real y, 
	size_t width,
	size_t height,
	const Ogre::String& text)
{
	Button* button = new Button(
		x, y, width, height, text, this);
	button->setBackgroundImage(getPanelColors()->buttonOveredSpriteName,
		getPanelColors()->buttonNotOveredSpriteName,
		getPanelColors()->buttonInactiveSpriteName,
		getPanelColors()->buttonClickedSpriteName);
	addItem(button);
	return button;
}


Caption* Panel::makeCaption(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height,
	const Ogre::String& text,
	Gorilla::TextAlignment textAlign,
	Gorilla::VerticalAlignment verticalAlign)
{
	Caption* caption = new Caption(
		x, y, width, height, text, this, textAlign, verticalAlign);
	caption->setBackgroundImage(getPanelColors()->captionBackgroundSpriteName);
	addItem(caption);
	return caption;
}


Checkbox* Panel::makeCheckbox(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height)
{
	Checkbox* checkbox = nullptr;

	// If images weren't defined, use the text checkbox
	if (getPanelColors()->checkboxCheckedNotOveredBackgroundSpriteName.empty())
		checkbox = new CheckboxText(
			x, y, width, height, 
			getPanelColors()->checkboxCheckedSymbol,
			this);
	else
		checkbox = new CheckboxSprite(
			x, y, width, height, 
			getPanelColors()->checkboxCheckedNotOveredBackgroundSpriteName, 
			getPanelColors()->checkboxCheckedOveredBackgroundSpriteName,
			getPanelColors()->checkboxOveredBackgroundSpriteName, 
			getPanelColors()->checkboxNotOveredBackgroundSpriteName, 
			this);
	addItem(checkbox);
	return checkbox;
}


Combobox* Panel::makeCombobox(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height,
	const vector<Ogre::String>& items,
	unsigned int nbDisplayedElements)
{
	Combobox* combobox = new Combobox(x, y, width, height, items, nbDisplayedElements, this);
	combobox->setBackgroundImageButtons(getPanelColors()->comboboxButtonPreviousOveredSpriteName,
		getPanelColors()->comboboxButtonPreviousNotOveredSpriteName,
		getPanelColors()->comboboxButtonPreviousInactiveSpriteName,
		getPanelColors()->comboboxButtonPreviousClickedSpriteName,
		getPanelColors()->comboboxButtonNextOveredSpriteName,
		getPanelColors()->comboboxButtonNextNotOveredSpriteName,
		getPanelColors()->comboboxButtonNextInactiveSpriteName,
		getPanelColors()->comboboxButtonNextClickedSpriteName);
	addItem(combobox);
	return combobox;
}


Listbox* Panel::makeListbox(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height,
	const vector<Ogre::String>& items,
	unsigned int nbDisplayedElements)
{
	Listbox* listBox = new Listbox(x, y, width, height, items, nbDisplayedElements, this);
	listBox->setBackgroundImageButtons(getPanelColors()->listboxButtonPreviousOveredSpriteName,
		getPanelColors()->listboxButtonPreviousNotOveredSpriteName,
		getPanelColors()->listboxButtonPreviousInactiveSpriteName,
		getPanelColors()->listboxButtonPreviousClickedSpriteName,
		getPanelColors()->listboxButtonNextOveredSpriteName,
		getPanelColors()->listboxButtonNextNotOveredSpriteName,
		getPanelColors()->listboxButtonNextInactiveSpriteName,
		getPanelColors()->listboxButtonNextClickedSpriteName);
	addItem(listBox);
	return listBox;
}


InlineSelector* Panel::makeInlineSelector(Ogre::Real x,
	Ogre::Real y,
	size_t width,
	size_t height,
	const vector<Ogre::String>& items)
{
	InlineSelector* inlineSelector = new InlineSelector(
		x, y, width, height, items, this);
	inlineSelector->setBackgroundImageButtons(getPanelColors()->inlineselectorButtonPreviousOveredSpriteName,
		getPanelColors()->inlineselectorButtonPreviousNotOveredSpriteName,
		getPanelColors()->inlineselectorButtonPreviousInactiveSpriteName,
		getPanelColors()->inlineselectorButtonPreviousClickedSpriteName,
		getPanelColors()->inlineselectorButtonNextOveredSpriteName,
		getPanelColors()->inlineselectorButtonNextNotOveredSpriteName,
		getPanelColors()->inlineselectorButtonNextInactiveSpriteName,
		getPanelColors()->inlineselectorButtonNextClickedSpriteName);
	addItem(inlineSelector);
	return inlineSelector;
}


ScrollBar* Panel::makeScrollBar(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height,
	Ogre::Real minValue,
	Ogre::Real maxValue)
{
	ScrollBar* scrollBar = new ScrollBar(
		x, y, width, height, minValue, maxValue, this);
	scrollBar->setCursorSprites(getPanelColors()->scrollbarCursorOveredSpriteName,
		getPanelColors()->scrollbarCursorNotOveredSpriteName,
		getPanelColors()->scrollbarCursorSelectedSpriteName);
	addItem(scrollBar);
	return scrollBar;
}


ProgressBar* Panel::makeProgressBar(Ogre::Real x, 
	Ogre::Real y,
	size_t width,
	size_t height)
{
	ProgressBar* progressBar = new ProgressBar(
		x, y, width, height, this);
	addItem(progressBar);
	return progressBar;
}


TextZone* Panel::makeTextZone(Ogre::Real x,
	Ogre::Real y,
	size_t width,
	size_t height,
	const Ogre::String& text)
{
	TextZone* textZone = new TextZone(
		x, y, width, height, text, this);
	addItem(textZone);
	return textZone;
}


void Panel::destroyButton(Button* b)
{
	removeItem(b);
}
	

void Panel::destroyCaption(Caption* c)
{
	removeItem(c);
}
	

void Panel::destroyCheckbox(Checkbox* c)
{
	removeItem(c);
}
	

void Panel::destroyCombobox(Combobox* c)
{
	removeItem(c);
}
	

void Panel::destroyInlineSelector(InlineSelector* is)
{
	removeItem(is);
}
	

void Panel::destroyProgressBar(ProgressBar* p)
{
	removeItem(p);
}
	

void Panel::destroyScrollBar(ScrollBar* s)
{
	removeItem(s);
}
	

void Panel::destroyTextZoneEditable(TextZone* t)
{
	removeItem(t);
}

}