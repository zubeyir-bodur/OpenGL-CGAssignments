Student Name: Zübeyir Bodur
Bilkent ID:   21702382


Build Instructions:

1 - Install Visual Studio 2017 or later. Make sure a valid Windows 10 SDK for Visual Studio 2017 
    was installed, otherwise the project will not compile. This can be done by making sure that 
    "Desktop Development with C++" is selected on the Visual Studio Installer pane, while you 
    are downlading Visual Studio 2017.

2 - Open OpenGL-CGAssignments.sln.

3 - Build the solution, in either Debug or Release configuration, and with either x86 or x64
	platform. Other configurations and platforms come from FreeType project, which were not
	set up for the rest of the project.

4 - Since all the dependencies are already included with the zip file, it should compile if
	the Windows SDK is installed properly.
	
	There are rare occasions where Visual Studio will choose the wrong directory due to some 
	bugs on some versions of the SDK. For instance, installing multiple SDK's for Visual 
	Studio 2017 and Visual Studio might trigger such a bug, and Visual Studio 2017 may search
	for the ProgramFiles folder instead, where the SDK is not actually located.
	
	If that happens, you may choose	the SDK that is recommended by the installed version of 
	the Visual Studio.

5 - Set 2DVectorBasedGeometricPaintApp as the startup project.

6 - Run the startup project.

Important Notes for the Assignment Regarding Third Party Libraries and References

Since there are a lot of third parties used for this assignment, all important note was written into this
file, instead of writing it to the header file of each library, or the source code of the assignment

1. Third Parties

	The third parties below were used without any significant change, or without any change.
	
	1.1 - GLEW & GLFW

		This dependency was required for any part of the application to work. Without
		these libraries, nothing will work. Therefore, these dependencies were used
		without any change.

		1.1.1 - GLFW
		https://www.glfw.org

		GLFW was required for creating windows, OpenGL context and receiving input from the machine.
		It should be noted that the examples in the book use FreeGLUT instead. It was my choice to
		use a different API for the same purpose, since both APIs do the same job. Below are the main
		reasons of my choice:
			- GLFW has 20 times more stars and 60 times more forks in GitHub, and therefore is more common
			in the computer graphics industry than FreeGLUT.
			- ImGui has no support for FreeGLUT with OpenGL 3, altough it supports OpenGL 2.

		1.1.2 - GLEW
		https://glew.sourceforge.net

		GLEW was used to access the modern, shader based OpenGL API. The same API was also used in the examples
		in the textbook.

	1.2 Angel-maths
		http://www.cs.bilkent.edu.tr/~gudukbay/cs465_lectures/Angel_VC11.zip

		This small "library" was basically for the matrix operations and creating necessary 
		transformation matrices, which were taken from the examples in the book. There were
		small changes necessary for me to use these two files easier:
			- Replacing GLfloat data type with float.
			- Removing Angel.h folder, as it was not necessary for me to use.
			- Adding Angel-maths.cpp, which includes mat.h, for the MSVC compiler to create a static library
			  so that I can use these two files as a seperate project inside the ThirdParty folder


	1.3 dearimgui & FreeType
		This libarires' sole purpose was for creating GUI components, and making the GUI components
		look good. Since both libraries were huge and complex ones, they were used without any change.
		
		1.3.1 ImGui
			https://github.com/ocornut/imgui
			
			ImGui were used for creating GUI components, and only for creating GUI components. The scene 
			was rendered in OpenGL from scratch.
			
		1.3.2 FreeType
			https://gitlab.freedesktop.org/freetype/freetype/-/tree/master/
			
			FreeType was used for rendering fonts better in ImGui. Although ImGui uses TrueTyped fonts,
			they look blurry.
			
	1.4 magic_enum
		https://github.com/Neargye/magic_enum

		MagicEnum library was used for debugging purposes. It was also used in the DSerializer class. OpenGL 
		Errors are not actual enums, they were actual integer macros. Therefore, reflection was required to
		convert the enum values into their strings, and vice versa.
		
	1.5 tinyfiledialogs
		https://sourceforge.net/p/tinyfiledialogs/code/ci/master/tree/

		TinyFileDialogs library was used for the Save/Load functionality of the assignment. Since C++ does
		not support a GUI interface for selecting folders and files, neither ImGui, and using Windows.h 
		functions would make the application platform dependent, a ThirdParty library was used for this
		functionality.

2. Engine
	
	The "Engine" was implemented in OpenGL from the scratch for abstracting OpenGL code into classes,
	instead of writing repetitive OpenGL code in the main file, or any other file in the Engine project.
	The abstraction, and other design paradigms in my code, was possible thanks to TheCherno's OpenGL and C++ series:

	OpenGL Series: https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2

	C++ Series: https://www.youtube.com/watch?v=18c3MTX0PK0&list=PLlrATfBNZ98dudnM48yfGUldqGD0S4FFb

	In addition to this, for the following functions, I had used external source code:

		- ImGuiManager.h
			- The dearimgui repository gives examples on how to setup ImGui for GLFW & OpenGL. From these examples, the necessary parts
			of the example codes were abstracted as four functions:
				- init_imgui()
				- new_imgui_frame()
				- shutdown_imgui()
				- render_imgui()
			- SetupImGuiStyle() function:
				- The customized coloring style for ImGui was taken from: 
				https://gist.github.com/dougbinks/8089b4bbaccaaf6fa204236978d165a9
				later, the colors were changed accordingly
		- contains() function that checks if a point is contained by a simple polygon
			- The code was taken from:
				https://www.geeksforgeeks.org/how-to-check-if-a-given-point-lies-inside-a-polygon/
			- The code was changed in the following way:
				- Other functions were converted into lambdas
				- Since the point inclusion in the code was for integers, it was converted to floats
				- The code in the link works for statically allocated arrays of Points, which is not compatible
				  with the API. It was modified so that it works with the Angel::vec3 data type
				- Some of the names of the variables were changed so that code is cleaner