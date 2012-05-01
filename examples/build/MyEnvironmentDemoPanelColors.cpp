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

#include "MyEnvironmentDemoPanelColors.h"

MyEnvironmentDemoPanelColors::MyEnvironmentDemoPanelColors()
	: Gui3D::PanelColors()
{
	// General
	transparent = Gorilla::rgb(0, 0, 0, 0);

	// Panel
	panelBackgroundSpriteName = "panelBackground";
	panelCursorSpriteName = "mousepointer";
	panelCursorSpriteSizeX = 12;
	panelCursorSpriteSizeY = 18;

	// Scrollbar
	scrollbarCursorOveredSpriteName = "scrollbarOvered";
	scrollbarCursorNotOveredSpriteName = "scrollbarNotOvered";
	scrollbarCursorSelectedSpriteName = "scrollbarSelected";

	scrollbarBackgroundGradientType = Gorilla::Gradient_NorthSouth;
	scrollbarBackgroundGradientStart = Gorilla::rgb(255, 255, 255, 80);
	scrollbarBackgroundGradientEnd = Gorilla::rgb(255, 255, 255, 40);

	scrollbarProgressbarGradientType = Gorilla::Gradient_WestEast;
	scrollbarProgressbarGradientStart = Gorilla::rgb(0, 104, 0, 140);
	scrollbarProgressbarGradientEnd = Gorilla::rgb(0, 120, 0, 180);

	scrollbarText = Ogre::ColourValue::White;
	scrollbarTextSize = 14;

	scrollbarBorder = Gorilla::rgb(170, 170, 170, 150);
	scrollbarBorderSize = 1;

	// Caption
	captionBackgroundGradientType = Gorilla::Gradient_NorthSouth;
	captionBackgroundGradientStart = transparent;
	captionBackgroundGradientEnd = transparent;

	captionBorder = transparent;
	captionText = Ogre::ColourValue::White;
	captionTextSize = 14;
	captionBorderSize = 0;
}