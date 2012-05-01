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



#include "OgreSceneNodeUtils.h"

//
bool OgreSceneNodeUtils::moveSceneNode(Ogre::SceneNode* nodeToMove,
	Ogre::SceneNode* destinationNode,
	Ogre::Real speed)
{
	Ogre::Vector3 vNodeToMove = nodeToMove->_getDerivedPosition();
	Ogre::Vector3 vNodeDestination = destinationNode->_getDerivedPosition();

	// when the translation node is getting closer of the destination
	//  node, reduce its speed
	if (vNodeToMove.positionCloses(vNodeDestination, Ogre::Real(0.001)))
		speed /= 10;

	Ogre::Real translateX = vNodeDestination.x;
	Ogre::Real translateY = vNodeDestination.y;
	Ogre::Real translateZ = vNodeDestination.z;

	// translate on X axis
	if (vNodeToMove.x < vNodeDestination.x)
		translateX = std::min(vNodeDestination.x,
			vNodeToMove.x + (speed));
	else if (vNodeToMove.x > vNodeDestination.x)
		translateX = std::max(vNodeDestination.x,
			vNodeToMove.x - (speed));

	// translate on Y axis
	if (vNodeToMove.y < vNodeDestination.y)
		translateY = std::min(vNodeDestination.y,
			vNodeToMove.y + (speed));
	else if (vNodeToMove.y > vNodeDestination.y)
		translateY = std::max(vNodeDestination.y,
			vNodeToMove.y - (speed));

	// translate on Z axis
	if (vNodeToMove.z < vNodeDestination.z)
		translateZ = std::min(vNodeDestination.z,
			vNodeToMove.z + (speed));
	else if (vNodeToMove.z > vNodeDestination.z)
		translateZ = std::max(vNodeDestination.z,
			vNodeToMove.z - (speed));

	// move the sceneNode
	nodeToMove->setPosition(Ogre::Vector3(translateX, translateY, translateZ));

	// if close enought, consider it's on it
	return !vNodeDestination.positionCloses(vNodeToMove, Ogre::Real(0.00001));
}