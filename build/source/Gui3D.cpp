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


#include "Gui3D.h"

namespace Gui3D
{

using namespace std;

struct Gui3DScreen
{
	Gui3DScreen(Gorilla::Screen* screen)
		: mScreen(screen)
	{}
	Gorilla::Screen* mScreen;
	map<Ogre::String, Gorilla::Layer*> mLayers;
};


struct Gui3DScreenRenderable
{
	Gui3DScreenRenderable(Gorilla::ScreenRenderable* screenRenderable)
		: mScreenRenderable(screenRenderable)
	{}
	Gorilla::ScreenRenderable* mScreenRenderable;
	map<Ogre::String, Gorilla::Layer*> mLayers;
};


Gui3D::Gui3D(PanelColors* panelColors)
	: mPanelColors(NULLPTR)
{
	mSilverback = new Gorilla::Silverback();
	setPanelColors(panelColors);
}


Gui3D::~Gui3D()
{
	delete mPanelColors;

	// Implicit delete of screens, screenRenderables and atlas
	delete mSilverback;
}


Gorilla::Silverback* Gui3D::getSilverback()
{
	return mSilverback;
}


//
// ScreenRenderables methods (2D Screens rendered in 3D)
//


Gorilla::Layer* Gui3D::getLayer(Ogre::String screenRenderableName,
	Ogre::String layerName)
{
	if (mScreenRenderables[screenRenderableName] != NULLPTR)
		return mScreenRenderables[screenRenderableName]->mLayers[layerName];
	
	return NULLPTR;
}


Gorilla::Layer* Gui3D::getLayer(Gorilla::ScreenRenderable* screenRenderable,
	Ogre::String layerName)
{
	map<Ogre::String, Gui3DScreenRenderable*>::iterator it =
		mScreenRenderables.begin();

	for (; it != mScreenRenderables.end(); ++it)
		if (it->second->mScreenRenderable == screenRenderable)
			return getLayer(it->first, layerName);

	return NULLPTR;
}


Gorilla::Layer* Gui3D::createLayer(Gorilla::ScreenRenderable* screenRenderable,
	Ogre::String layerName)
{
	// Tests if already exists
	Gorilla::Layer* layer = getLayer(screenRenderable, layerName);
	if (layer != NULLPTR)
		return layer;

	map<Ogre::String, Gui3DScreenRenderable*>::iterator it = 
		mScreenRenderables.begin();

	// Try to find the screenRenderable
	for (; it != mScreenRenderables.end(); ++it)
		if (it->second->mScreenRenderable == screenRenderable)
		{
			Gorilla::Layer* layer = screenRenderable->createLayer(1);
			it->second->mLayers[layerName] = layer;
			return layer;
		}

	return NULLPTR;
}


Gorilla::Layer* Gui3D::createLayer(Ogre::String screenRenderableName,
	Ogre::String layerName)
{
	// Tests if already exists
	Gorilla::Layer* layer = getLayer(screenRenderableName, layerName);
	if (layer != NULLPTR)
		return layer;

	// Try to find the screenRenderable
	Gorilla::ScreenRenderable* screenR = 
		mScreenRenderables[screenRenderableName]->mScreenRenderable;

	if (screenR != NULLPTR)
	{
		Gorilla::Layer* layer = screenR->createLayer();

		mScreenRenderables[screenRenderableName]->mLayers[layerName] = layer;
		return layer;
	}

	return NULLPTR;
}


Gorilla::ScreenRenderable* Gui3D::createScreenRenderable(
	const Ogre::Vector2& pos, Ogre::String atlasName, Ogre::String name)
{
	if (!hasAtlas(atlasName))
		loadAtlas(atlasName);

	mScreenRenderables[name] = new Gui3DScreenRenderable(
		mSilverback->createScreenRenderable(pos, atlasName));

	return mScreenRenderables[name]->mScreenRenderable;
}
	

Gorilla::ScreenRenderable* Gui3D::getScreenRenderable(Ogre::String name)
{
	if (mScreenRenderables[name] != NULLPTR)
		return mScreenRenderables[name]->mScreenRenderable;

	return NULLPTR;
}


//
// Screens methods (2D screen)
//


Gorilla::Layer* Gui3D::getLayerScreen(Ogre::String screenName, 
	Ogre::String layerName)
{
	if (mScreens[screenName] != 0)
		return mScreens[screenName]->mLayers[layerName];

	return NULLPTR;
}


Gorilla::Layer* Gui3D::getLayerScreen(Gorilla::Screen* screen, 
	Ogre::String layerName)
{
	map<Ogre::String, Gui3DScreen*>::iterator it = mScreens.begin();

	for (; it != mScreens.end(); ++it)
		if (it->second->mScreen == screen)
			return getLayerScreen(it->first, layerName);

	return NULLPTR;
}


Gorilla::Layer* Gui3D::createLayerScreen(Gorilla::Screen* screen, Ogre::String layerName)
{
	// Tests if already exists
	Gorilla::Layer* layer = getLayerScreen(screen, layerName);
	if (layer != NULLPTR)
		return layer;

	map<Ogre::String, Gui3DScreen*>::iterator it = mScreens.begin();

	// Try to find the screen
	for (; it != mScreens.end(); ++it)
		if (it->second->mScreen == screen)
		{
			Gorilla::Layer* layer = screen->createLayer(1);
			it->second->mLayers[layerName] = layer;
			return layer;
		}

	return NULLPTR;
}


Gorilla::Layer* Gui3D::createLayerScreen(Ogre::String screenName, Ogre::String layerName)
{
	// Tests if already exists
	Gorilla::Layer* layer = getLayerScreen(screenName, layerName);
	if (layer != NULLPTR)
		return layer;

	// Try to find the screen
	Gorilla::Screen* screen = mScreens[screenName]->mScreen;

	if (screen != NULLPTR)
	{
		Gorilla::Layer* layer = screen->createLayer();

		mScreens[screenName]->mLayers[layerName] = layer;
		return layer;
	}

	return NULLPTR;
}


Gorilla::Screen* Gui3D::createScreen(Ogre::Viewport* vp, 
	Ogre::String atlasName, Ogre::String name)
{
	if (!hasAtlas(atlasName))
		loadAtlas(atlasName);

	mScreens[name] = new Gui3DScreen(mSilverback->createScreen(vp, atlasName));
	return mScreens[name]->mScreen;
}


Gorilla::Screen* Gui3D::getScreen(Ogre::String name)
{
	if (mScreens[name] != NULLPTR)
		return mScreens[name]->mScreen;
	return NULLPTR;
}


//
// General methods
//


bool Gui3D::hasAtlas(Ogre::String name)
{
	for (size_t i=0; i < mAtlas.size(); i++)
		if (mAtlas[i] == name)
			return true;
	return false;
}


void Gui3D::loadAtlas(Ogre::String name)
{
	mSilverback->loadAtlas(name);
	mAtlas.push_back(name);
}


void Gui3D::destroyScreenRenderable(Ogre::String screenRenderableName)
{
	// Will implicitly destroy its layers
	mSilverback->destroyScreenRenderable(
		mScreenRenderables[screenRenderableName]->mScreenRenderable);
	mScreenRenderables.erase(screenRenderableName);
}


void Gui3D::destroyScreenRenderable(Gorilla::ScreenRenderable* screenRenderable)
{
	map<Ogre::String, Gui3DScreenRenderable*>::iterator it =
		mScreenRenderables.begin();

	for (; it != mScreenRenderables.end(); ++it)
		if (it->second->mScreenRenderable == screenRenderable)
		{
			destroyScreenRenderable(it->first);
			break;
		}
}


void Gui3D::destroyScreen(Ogre::String screenName)
{
	// Will implicitly destroy its layers
	mSilverback->destroyScreen(
		mScreens[screenName]->mScreen);
	mScreens.erase(screenName);
}


void Gui3D::destroyScreen(Gorilla::Screen* screen)
{
	map<Ogre::String, Gui3DScreen*>::iterator it =
		mScreens.begin();

	for (; it != mScreens.end(); ++it)
		if (it->second->mScreen == screen)
		{
			destroyScreen(it->first);
			break;
		}
}


void Gui3D::setPanelColors(PanelColors* panelColors)
{
	if (mPanelColors != NULLPTR)
		delete mPanelColors;
	mPanelColors = panelColors;
}


PanelColors* Gui3D::getPanelColors()
{
	return mPanelColors;
}

}