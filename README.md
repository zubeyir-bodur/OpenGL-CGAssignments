# Computer Graphics Assignments in OpenGL

This repository is a showcase of my computer graphics assignments which were developed using:
- OpenGL v3.3 (Core Profile)
	- The version was manually set. If your GPU does not support this version, the applications will not work.
- GLFW v3.3.8, for creating windows and retrieving input
- GLEW v2.1.0, for accessing modern OpenGL API
- ImGui v1.88 (docking branch), for drawing basic GUI components.
- FreeType v2.12.1, for better looking fonts for ImGui
- MagicEnum v0.8.1, for converting integer OpenGL errors into strings
- stb-image.h v2.27 from the STB library, for importing images in C++, was used for importing texture assets

## Requirements
You need to have
- C++ 17

minimum installed to run the applications, in addition to the minimum OpenGL version stated above. With a small change in the ErrorManager class, you can run the applications in other compilers, such as GC, CLang etc, because that class uses the following MSVC instrinsic statement:

```c++
//  In ErrorManager.h
// ... Some macros defined before
// At line 6:
#define GENERAL_BREAK() __debugbreak()
#endif
// Other macros and function definitions contn'd
```

If you are using MSVC compiler, you need to have Visual Studio 2017 installed minimum, although
using Visual Studio 2022 is reccommended.

## Assignments

### 2D Vector-based Geometric Paint Application
- To be documented

## Remarks
 Huge shoutout to [TheCherno](https://www.github.com/TheCherno), whom I followed his [OpenGL series](https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) to implement my basic OpenGL framework for the assignments.