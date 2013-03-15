Gui3D Library v1.1

*****************************
    INSTALL INSTRUCTIONS
*****************************

To compile this static library using Visual studio 2010 :

This assume that you have an OGRE_HOME environment variable set to OGRE path.

For debug mode : 
Select DebugStatic then click right Gui3DLibrary > Generate

For release mode : 
Then select ReleaseStatic then click right Gui3DLibrary > Generate.

You should now have in Gui3DLibrary\sdk folder the folowing files :
- Gui3DStatic.lib
- Gui3DDebugStatic.lib
- All Gui3D header files + Gorilla header file.



In your project, go in properties > C/C++ > Other includes directories and add "Path\To\Gui3D\Folder\sdk"
Then, in Linker > General > Additional Library Directories add "Path\To\Gui3D\Folder\sdk"


That's it, you're done ! :)