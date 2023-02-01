# Computer Graphics Applications in OpenGL

This repository is a showcase of my computer graphics assignments which were developed using:
- OpenGL v3.3 (Core Profile)
	- The version was manually set. If your GPU does not support this version, the applications will not work.
- GLFW v3.3.8, for creating windows and retrieving input
- GLEW v2.1.0, for accessing modern OpenGL API
- ImGui v1.88 (docking branch), for drawing basic GUI components
- FreeType v2.12.1, for better looking fonts for ImGui
- MagicEnum v0.8.1, for converting integer OpenGL errors into strings and for serializing enums
- stb-image.h v2.27 from the STB library, for importing images in C++, was used for importing texture assets
- tinyfiledialogs v3.8.8, for creating file dialogs for save & load purposes for the 1st assignment

## Requirements
You need to have
- C++ 17

minimum installed to run the applications, in addition to the minimum OpenGL version stated above. 
With a small change in the ErrorManager class, you can run the applications in other compilers,
such as GCC, CLang etc, because that class uses the following MSVC instrinsic statement:

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

## Applications

### 2D Vector-based Geometric Paint Application


<img src="Data/docs/assignment 1.gif" alt="Figure 1: 2D Vector-based Geometric Paint Application" align="centered">
<br>
<p align="center"><i>Figure 1: 2D Vector-based Geometric Paint Application</i></p>

- Features:
   - Drawing predefined shapes, rectangulars and equilateral triangles
   - Changing the drawing color using a HSV color picker, as well as support for transparent shapes
   - 2D selection system, which supports single and multiple selection
       - Drawing a rectangle box around selected multiple shapes, in addition to highlighting any selected shape by drawing edges
       of the shape
   - Moving a selected shape, does not support moving multiple shapes
   - Removing shapes
       - Supports removing multiple shapes if there are multiple selections
       - Clears both the undo and redo stack
   - Support for drawing convex polygons
       - Drawing a line whenever a vertex is being added, from the previous created vertex to the current mouse position
       - Also the polygon is drawn if there are at least 3 vertices
   - Selection pane, which controls:
       - Changing the colour of the shape
       - Rotation of the shape, with buttons and a slider float. Rrotation with the slider float does not support
       undo/redo
   - Support for undo/redo for the following operations:
       - Drawing a predefined shape, or <u>finishing</u> a polygon shape
       - Moving a shape
       - Rotating a shape
   - Interactive 2D Camera:
       - Zooming, minimum zoom is 50 %, maximum zoom is 1000 %. The values can be changed very easily.
       - Zooming towards mouse position - the world coordinate the cursor is poingting to won't change while zooming
       - Camera movement and zoom also works when:
           - a selection is being made
           - a predefined shape is being drawn
           - a vertex is added to a polygon
           - a shape is being moved
   - Support for copying selected shapes, and pasting shapes that are copied towards the mouse position

<br>

### Modeling and Animating Randomly Generated Tree Models

<img src="Data/docs/assignment 2.gif" alt="Figure 2: Modeling and Animating Randomly Generated Tree Models Demo" align="centered">
<br>
<p align="center"><i>Figure 2: Modeling and Animating Randomly Generated Tree Models Demo</i></p>

- Features:
    - A simple 3D camera to move around the scene, using raw, pitch and roll angles
    - Rendering hierarchical models, such as a tree, and changing the orientation and position of the model
    - Smooth shading using the Phong model, along with textures
    - Randomly generating hierarchical models so that these models look more like a tree
    - Mouse picking for selecting a part of the model, or anything entity in the drawlist
        - Also supports selection highlighting by rendering the green channel only (smoothly shaded entities only)

### Realistic Rendering of Parametric Surfaces


<img src="Data/docs/assignment 3.gif" alt="Figure 3: Realistic Rendering of Parametric Surfaces Demo" align="centered">
<br>
<p align="center"><i>Figure 3: Realistic Rendering of Parametric Surfaces Demo</i></p>

- Features:
    - 3D camera which can zoom in and out
    - Generating parametric surfaces 
    - Bump Mapping
    - Display options:
        - Wireframe
        - Gouraud Shaded, bump mapping and shading per vertex
        - Phong Shaded, bump mapping and shading per fragment
    - Parameter tuning on the fly for shading and curve parameters

## Remarks
 Huge shoutout to [TheCherno](https://www.github.com/TheCherno), whom I followed his [OpenGL series](https://www.youtube.com/watch?v=W3gAzLwfIP0&list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2) to implement my basic OpenGL framework for the assignments.