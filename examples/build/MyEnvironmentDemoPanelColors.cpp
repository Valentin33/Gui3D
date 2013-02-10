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