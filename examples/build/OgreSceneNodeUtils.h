#ifndef OgreSceneNodeUtils_H
#define OgreSceneNodeUtils_H



#include <OGRE/OgreSceneNode.h>

/*! class. OgreSceneNodeUtils
	desc.
*/
class OgreSceneNodeUtils
{
public:
    
	// Move the SceneNode @nodeToMove to @destinationNode at the
	//  speed @speed
    // return true if @nodeToMove has move (it's not yet at the 
	//  position of @destinationNode
    //  false otherwise, meaning the node is at @destinationNode
	//  position
	static bool moveSceneNode(Ogre::SceneNode* nodeToMove,
		Ogre::SceneNode* destinationNode,
		Ogre::Real speed);
};

#endif