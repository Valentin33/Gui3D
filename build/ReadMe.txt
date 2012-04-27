Gui3D Library

To compile this static library using Visual studio 2010 :

This assume that you have an OGRE_HOME environment variable set to OGRE path.

For debug mode : 
Select DebugStatic then click right Gui3DLibrary > Generate
Then, copy the .lib file from Gui3D3DLibrary\build\DebugStatic\Gui3DDebugStatic.lib to Gui3DLibrary\sdk folder

For release mode : 
Then select ReleaseStatic then click right Gui3DLibrary > Generate.
Then, copy the .lib file from Gui3DLibrary\build\FinalStatic\Gui3DStatic.lib to Gui3DLibrary\sdk folder


In your project, go in properties > C/C++ > Other includes directories and add "Path\To\Gui3D\Folder\sdk"
Then, in Linker > General > Additional Library Directories add "Path\To\Gui3D\Folder\sdk"


That's it, you're done ! :)