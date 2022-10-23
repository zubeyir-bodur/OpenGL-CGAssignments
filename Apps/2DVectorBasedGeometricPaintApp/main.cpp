#include "ImGuiManager.h"
#include "ErrorManager.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Renderer.h"
#include "Shader.h"
#include "Texture.h"
#include "DrawList.h"
#include "Shape.h"
#include "Camera.h"

#include <nothings-stb/stb_image.h>
#include <dearimgui/imgui.h>
#include "Angel-maths/mat.h"
#include <glew.h>
#include "Input.h"
#include <glfw3.h>
#include <cstdio>
#include <iostream>
#include <string>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "GlFW Error %d: %s\n", error, description);
}


int main(int, char**)
{
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return -1;

	// Set up core profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create GLFW window
	GLFWmonitor* main_monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(main_monitor);
	GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "CS 465 - Assignment 1 - 2D Vector-based Geometric Paint Application", nullptr, nullptr);
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	if (!window || !mode)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwMaximizeWindow(window);
	glfwSwapInterval(1); // Enable vsync

	// Init GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Could not init GLEW..." << std::endl;
		GENERAL_BREAK();
	}
	else
	{
		const unsigned char* opengl_version;
		__glCallReturn(glGetString(GL_VERSION), opengl_version);
		std::cout << "OpenGL version: " << opengl_version << std::endl;

		const unsigned char* glsl_version;
		__glCallReturn(glGetString(GL_SHADING_LANGUAGE_VERSION), glsl_version);
		std::cout << "GLSL version: " << glsl_version << std::endl;
	}

	Input& window_input = Input::get_instance(window);

	// Setup Dear ImGui context
	init_imgui(window);
	SetupImGuiStyle();

	float init_shape_length = width / 8.0f;

	// Enable blending for supporting transparent shapes
	__glCallVoid(glEnable(GL_BLEND));
	__glCallVoid(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

	// Line width for GL_LINES
	__glCallVoid(glLineWidth(5.0f));

	// Specify the colors
	float color_sheet[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	float color_current[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float color_draw[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	Angel::vec4 selector_box_col = { 0, 0.4f, 0.8f, 0.5f };
	Angel::vec4 drawer_box_col = { 0, 0.0f, 0.0f, 0.5f };
	float tmp_degree = 0.0f;
	ImVec4 clear_color = ImVec4(0.3984375f, 0.3984375f, 0.3984375f, 1.0f);

	Shape::init_static_members(width);
	Renderer renderer;
	Angel::vec3 sheet_pos(0, height / 7.0f, 0.0f);

	// Helper rectangle spec for drawing temporary boxes (selection & drawing)
	constexpr float global_z_pos_2d = 0.0f;
	Angel::vec4 rect_0(0.0f, 0.0f, global_z_pos_2d, 1.0f);
	Angel::vec4 rect_1(init_shape_length, 0.0f, global_z_pos_2d, 1.0f);
	Angel::vec4 rect_2(init_shape_length, init_shape_length, global_z_pos_2d, 1.0f);
	Angel::vec4 rect_3(0.0f, init_shape_length, global_z_pos_2d, 1.0f);

	// View matrix - camera
	Camera::init(Angel::vec3(0.0f, 0.0f, global_z_pos_2d), 100.0f);
	auto view_matrix = Camera::view_matrix();

	// Orthographic projection is used
	Angel::mat4 projection_matrix = Angel::Ortho2D(0.0f, (float)width, (float)height, 0.0f);

	// Mouse location
	auto cursor_model_coords = Camera::map_from_global(0, 0);

	// Sheet initializations
	Angel::mat4 model_sheet_matrix = Angel::Translate(sheet_pos)
		* Angel::Scale(Angel::vec3(8.0f, (6.0f / 7.0f) * (8.0f * height / width), 1.0f));
	Angel::mat4 MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;

	// Selection initializations
	Angel::vec3 selector_pos(0.0f, 0.0f, 0.0f);
	Angel::vec3 selector_scale(1, 1, 1.0f); // 1px selector box
	Angel::mat4 model_selector_box = Angel::Translate(selector_pos)
		* Angel::Scale(selector_scale);
	Angel::mat4 MVP_selector_box = projection_matrix * view_matrix * model_selector_box;

	// Drawer Box initializations
	Angel::vec3 drawer_pos(0.0f, 0.0f, 0.0f);
	Angel::vec3 drawer_scale(1, 1, 1.0f); // 1px selector box
	Angel::mat4 model_drawer_box = Angel::Translate(drawer_pos)
		* Angel::Scale(drawer_scale);
	Angel::mat4 MVP_drawer_box = projection_matrix * view_matrix * model_drawer_box;

	// Selection State
	std::vector<ShapeModel*> cur_selections{};

	// Copy & Paste state
	std::vector<ShapeModel*> clipboard{};

	// DrawList
	DrawList list(&renderer, projection_matrix, view_matrix);

	enum class RadioButtons
	{
		Select = 0,
		DrawRect = 1,
		DrawEqTri = 2,
		DrawPoly = 3,
		Delete = 4
	};
	int radio_button_cur = (int)RadioButtons::Select;

	// Draw Polygon State
	enum class PolygonDrawingState
	{
		Idle = 0,
		LTThreeVertices = 1,
		ThirdVertexAdded = 2,
		AddingMoreVertices = 3,
		OnFinish = 4,
	};
	std::vector<Angel::vec3> polygon_mouse_model_coords;
	ShapeModel* new_polygon = nullptr;
	Angel::vec3 polygon_add_vertex_line_pos;
	Angel::vec3 polygon_add_vertex_line_scale;
	Angel::vec3 polygon_add_vertex_line_rotation;
	Angel::mat4 model_polygon_add_vertex_line;
	Angel::mat4 MVP_polygon_add_vertex_line;

	// Other States
	bool drawing_selector_box = false;
	bool drawing_drawer_box = false;
	bool is_dragging = false;
	bool drawing_poly_add_vertex_line = false;
	bool drawing_multiple_selection_box = false;
	std::array<float, 4> bounding_box_selector{};
	std::array<float, 4> bounding_box_drawer{};
	const float& imgui_zoom_ratio = Camera::get_zoom_ratio();
	ImGuiColorEditFlags f = ImGuiColorEditFlags_::ImGuiColorEditFlags_PickerHueWheel
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_NoInputs
		| ImGuiColorEditFlags_::ImGuiColorEditFlags_DisplayHSV;
	Angel::vec3 camera_pos_released;
	float camera_zoom_released;
	Angel::vec3 camera_pos_pressed;
	float camera_zoom_pressed;
	auto map_from_global_any = [](double x, double y, Angel::vec3 c_pos, float c_z) -> Angel::vec3
	{
		return (c_pos + Angel::vec3((float)x, (float)y, 0.0f)) * (100.0f / c_z);
	};
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		// Old mouse pos & state
		Angel::vec2 old_mouse_pos(window_input.m_mouse_x, window_input.m_mouse_y);
		Input::ButtonState mouse_previous_state = window_input.m_lmb_state;

		// Needed for selection and drawing while moving the camera
		const Camera& old_camera = Camera::get_instance();
		const Angel::vec3 old_camera_pos = old_camera.camera_pos();
		const float old_camera_zoom_ratio = old_camera.get_zoom_ratio();
		auto map_from_global_using_old_camera = [&, old_camera_pos, old_camera_zoom_ratio](double x, double y) -> Angel::vec3
		{
			return map_from_global_any(x, y, old_camera_pos, old_camera_zoom_ratio);
		};

		// Reset scroll
		window_input.m_scroll_y = 0.0;

		// Reset copy & paste
		window_input.m_copy_just_pressed = false;
		window_input.m_paste_just_pressed = false;

		glfwPollEvents();

		// Update the viewport
		glfwGetWindowSize(window, &width, &height);

		sheet_pos = Angel::vec3(0, height / 7.0f, 0.0f);

		model_sheet_matrix = Angel::Translate(sheet_pos)
			* Angel::Scale(Angel::vec3(8.0f, (6.0f / 7.0f) * (8.0f * height / width), 1.0f));

		projection_matrix = Angel::Ortho2D(0.0f, (float)width, (float)height, 0.0f);

		// Update cursor
		cursor_model_coords = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
		bool input_on_imgui = ImGui::GetIO().WantCaptureMouse;

		// Keyboard Events
		if (!input_on_imgui)
		{
			// Camera movement - simple
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			{
				Camera::move_vertical(-20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			{
				Camera::move_horizontal(-20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			{
				Camera::move_vertical(20.0f);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			{
				Camera::move_horizontal(20.0f);
			}

			// Camera zooming with mouse wheel
			Camera::zoom(window_input.m_scroll_y, window_input.m_mouse_x, window_input.m_mouse_y);

			// Update view matrix when necessary
			view_matrix = Camera::view_matrix();
		}
		if (window_input.m_copy_just_pressed)
		{
			for (auto* ptr: clipboard)
			{
				delete ptr;
			}
			clipboard.clear();
			if (!cur_selections.empty())
			{
				for (auto selected_item : cur_selections)
				{
					ShapeModel* clipboard_item;
					ShapeModel::StaticShape type = selected_item->shape_def();
					Angel::vec4* colour = new Angel::vec4(selected_item->color());
					if (type == ShapeModel::StaticShape::NONE)
					{
						std::vector<Angel::vec3> poly_coords = selected_item->model_coords();
						clipboard_item = new ShapeModel(poly_coords, colour);
					}
					else
					{
						Angel::vec3* pos = new Angel::vec3(selected_item->position());
						Angel::vec3* rot = new Angel::vec3(selected_item->rotation());
						Angel::vec3* scale = new Angel::vec3(selected_item->scale());
						clipboard_item = new ShapeModel(type, pos, rot, scale, colour);
					}
					clipboard.push_back(clipboard_item);
				}
				std::cout << "Copied!" << std::endl;
				std::cout << "Clipboard size: " << std::to_string(clipboard.size()) << std::endl;
			}
		}
		if (window_input.m_paste_just_pressed)
		{
			if (!clipboard.empty())
			{
				// Clear selections
				drawing_multiple_selection_box = false;
				for (auto* item : cur_selections)
				{
					item->deselect();
				}
				cur_selections.clear();

				// Compute the cursor model position
				Angel::vec3 cursor_pos = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);

				// Compute the center of clipboard
				Angel::vec3 center_clipboard;
				std::array<float, 6> clipboard_bounding_cube = ShapeModel::bounding_cube(clipboard);
				center_clipboard = {
					(clipboard_bounding_cube[0] + clipboard_bounding_cube[1]) / 2.0f,
					(clipboard_bounding_cube[2] + clipboard_bounding_cube[3]) / 2.0f,
					global_z_pos_2d
				};

				std::vector<ShapeModel*> pasted_shapes;
				pasted_shapes.reserve(clipboard.size());

				for (auto* clipboard_item: clipboard)
				{
					ShapeModel* pasted_shape;
					ShapeModel::StaticShape type = clipboard_item->shape_def();
					Angel::vec4* colour = new Angel::vec4(clipboard_item->color());
					if (type == ShapeModel::StaticShape::NONE)
					{
						std::vector<Angel::vec3> poly_coords = clipboard_item->model_coords();
						pasted_shape = new ShapeModel(poly_coords, colour);
					}
					else
					{
						Angel::vec3* pos = new Angel::vec3(clipboard_item->position());
						Angel::vec3* rot = new Angel::vec3(clipboard_item->rotation());
						Angel::vec3* scale = new Angel::vec3(clipboard_item->scale());
						pasted_shape = new ShapeModel(type, pos, rot, scale, colour);
					}
					pasted_shapes.push_back(pasted_shape);
				}

				// Translate the deep clipboard
				// So that deep copies are translated by
				// by v = cursor_pos - center_clipboard
				for (auto* pasted_shape : pasted_shapes)
				{
					pasted_shape->position() += cursor_pos - center_clipboard;
				}
				
				// Add deep copy to the draw list
				for (auto pasted_shape : pasted_shapes)
				{
					list.add_shape(pasted_shape);
				}

				// Select the deep copies & update cur_selections
				for (auto* item : pasted_shapes)
				{
					cur_selections.push_back(item);
					item->select();
				}
				// Draw a rectangular area around if multiple shapes were copied
				if (cur_selections.size() > 1)
				{
					drawing_multiple_selection_box = true;
				}
				else
				{
					drawing_multiple_selection_box = false;
				}
				std::cout << "Pasted!" << std::endl;
			}
		}

		// Process LMB events
		if (mouse_previous_state == Input::ButtonState::BeingPressed
			&& window_input.m_lmb_state == Input::ButtonState::Released)
		{
			if (!input_on_imgui)
			{
				camera_pos_released = Camera::camera_pos();
				camera_zoom_released = Camera::get_zoom_ratio();
				// Update selection
				if (drawing_selector_box)
				{
					Angel::vec4 selector_0 = model_selector_box * rect_0;
					Angel::vec4 selector_1 = model_selector_box * rect_1;
					Angel::vec4 selector_2 = model_selector_box * rect_2;
					Angel::vec4 selector_3 = model_selector_box * rect_3;
					bounding_box_selector = std::array<float, 4>{
						std::min(selector_0.x, selector_1.x),
							std::max(selector_0.x, selector_1.x),
							std::min(selector_0.y, selector_2.y),
							std::max(selector_0.y, selector_2.y)
					};
					if (std::abs(window_input.m_mouse_release_y - window_input.m_mouse_press_y) < 1.0f
						&& std::abs(window_input.m_mouse_release_x - window_input.m_mouse_press_x) < 1.0f)
					{
						std::cout << "THIS IS A CLICK" << std::endl;
						ShapeModel* frontmost = list.frontmost_shape(Camera::map_from_global(window_input.m_mouse_release_x, window_input.m_mouse_release_y));
						if (frontmost != nullptr)
						{
							drawing_multiple_selection_box = false;
							for (auto* item : cur_selections)
							{
								item->deselect();
							}
							cur_selections.clear();
							frontmost->select();
							cur_selections.emplace_back(frontmost);
						}
						else
						{
							drawing_multiple_selection_box = false;
							for (auto* item : cur_selections)
							{
								item->deselect();
							}
							cur_selections.clear();
						}
					}
					else
					{
						std::cout << "THIS IS A DRAG" << std::endl;
						Angel::vec3 selector_pos_centered(
							(bounding_box_selector[0] + bounding_box_selector[1]) / 2.0f,
							(bounding_box_selector[2] + bounding_box_selector[3]) / 2.0f,
							0.0f
						);
						std::vector<ShapeModel*> multiple_selection = list.shapes_contained_in(selector_pos_centered, selector_scale);
						drawing_multiple_selection_box = false;
						for (auto* item : cur_selections)
						{
							item->deselect();
						}
						cur_selections.clear();
						for (const auto& item: multiple_selection)
						{
							cur_selections.push_back(item);
							item->select();
						}

						// Draw a rectangular area around multiple selections
						if (cur_selections.size() > 1)
						{
							drawing_multiple_selection_box = true;
						}
						else
						{
							drawing_multiple_selection_box = false;
						}

					}
				}

				// Process deletion
				if (radio_button_cur == (int)RadioButtons::Delete)
				{
					ShapeModel* s_press = list.frontmost_shape(Camera::map_from_global(window_input.m_mouse_press_x, window_input.m_mouse_press_y));
					ShapeModel* s_release = list.frontmost_shape(Camera::map_from_global(window_input.m_mouse_release_x, window_input.m_mouse_release_y));
					if (s_press != nullptr && s_release != nullptr && s_release == s_press)
					{
						bool in_selections = false;
						for (auto* selection : cur_selections)
						{
							if (selection == s_release)
							{
								in_selections = true;
							}
						}
						if (!in_selections || 
							in_selections && cur_selections.size() < 2)
						{
							list.remove_shape(s_release);
						}
						else
						{
							drawing_multiple_selection_box = false;
							for (auto& item : cur_selections)
							{
								item->deselect();
							}
							for (auto& item: cur_selections)
							{
								list.remove_shape(item);
							}
							cur_selections.clear();
						}
					}
				}

				// Create new predefined shape
				if (drawing_drawer_box)
				{
					if (std::abs(window_input.m_mouse_release_y - window_input.m_mouse_press_y) < 1.0f
						&& std::abs(window_input.m_mouse_release_x - window_input.m_mouse_press_x) < 1.0f)
					{
						std::cout << "THIS IS MOVEMENT WAS SO SMALL, DIDNT DRAW..." << std::endl;
					}
					else
					{
						Angel::vec4 drawer_0 = model_drawer_box * rect_0;
						Angel::vec4 drawer_1 = model_drawer_box * rect_1;
						Angel::vec4 drawer_2 = model_drawer_box * rect_2;
						Angel::vec4 drawer_3 = model_drawer_box * rect_3;
						bounding_box_drawer = std::array<float, 4>{
							std::min(drawer_0.x, drawer_1.x),
								std::max(drawer_0.x, drawer_1.x),
								std::min(drawer_0.y, drawer_2.y),
								std::max(drawer_0.y, drawer_2.y)
						};
						Angel::vec3 mid_point(0.0f, 0.0f, 0.0f);
						mid_point.x = (bounding_box_drawer[0] + bounding_box_drawer[1]) / 2;
						mid_point.y = (bounding_box_drawer[2] + bounding_box_drawer[3]) / 2;
						ShapeModel::StaticShape shape_def;

						if (radio_button_cur == (int)RadioButtons::DrawEqTri)
						{
							shape_def = ShapeModel::StaticShape::ISOSCELES_TRIANGLE;
						}
						else if (radio_button_cur == (int)RadioButtons::DrawRect)
						{
							shape_def = ShapeModel::StaticShape::RECTANGLE;
						}

						if (radio_button_cur != (int)RadioButtons::DrawPoly)
						{
							auto* shape_pos = new Angel::vec3(mid_point);
							auto* shape_rot = new Angel::vec3(0.0f, 0.0f, 0.0f);
							auto* shape_scale = new Angel::vec3(std::abs(drawer_scale.x), std::abs(drawer_scale.y), drawer_scale.z);
							auto* shape_color = new Angel::vec4(color_draw[0],
								color_draw[1],
								color_draw[2],
								color_draw[3]);

							auto* new_shape = new ShapeModel(shape_def, shape_pos, shape_rot, shape_scale, shape_color);
							list.add_shape(new_shape);
							cur_selections.clear();
							new_shape->select();
							cur_selections.push_back(new_shape);
						}
					}
				}

				if (radio_button_cur == (int)RadioButtons::DrawPoly)
				{
					drawing_poly_add_vertex_line = true;
					// Add vertex
					Angel::vec3 poly_vertex = Camera::map_from_global(window_input.m_mouse_release_x, window_input.m_mouse_release_y);
					polygon_add_vertex_line_pos = Angel::vec3(poly_vertex);
					polygon_add_vertex_line_scale.y = (1.0f / init_shape_length) * (Camera::get_zoom_ratio() / 100.0f);
					Angel::vec3 release_pos = map_from_global_any(window_input.m_mouse_release_x, window_input.m_mouse_release_y, camera_pos_released, camera_zoom_released);
					Angel::vec3 current_pos = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
					double dy = current_pos.y - release_pos.y;
					double dx = current_pos.x - release_pos.x;
					double line_length = std::sqrt(std::pow(dy, 2.0f) + std::pow(dx, 2.0f));
					double tetha = std::atan(dy / dx) / Angel::DegreesToRadians;
					if (dx < 0)
					{
						tetha += M_PI / Angel::DegreesToRadians;
					}
					polygon_add_vertex_line_scale.x = (1.0f / init_shape_length) * (line_length);
					polygon_add_vertex_line_rotation.z = tetha;
					polygon_mouse_model_coords.push_back(poly_vertex);
					if (polygon_mouse_model_coords.size() == 1)
					{
						drawing_multiple_selection_box = false;
						for (auto& item : cur_selections)
						{
							item->deselect();
						}
						cur_selections.clear();
					}
					else if (polygon_mouse_model_coords.size() == 3)
					{
						auto* shape_color = new Angel::vec4(color_draw[0],
							color_draw[1],
							color_draw[2],
							color_draw[3]);
						new_polygon = new ShapeModel(polygon_mouse_model_coords, shape_color);
						list.add_shape(new_polygon);
					}
					else if (polygon_mouse_model_coords.size() > 3)
					{
						ASSERT(new_polygon != nullptr);
						new_polygon->push_back_vertex(poly_vertex);
					}
				}
			}

			// Consume the released state
			window_input.m_mouse_press_y = -1.0f;
			window_input.m_mouse_press_x = -1.0f;
			window_input.m_lmb_state = Input::ButtonState::Idle;
			std::cout << "LMB is now Idle" << std::endl;
			drawing_selector_box = false;
			drawing_drawer_box = false;
			selector_pos = { 0, 0, 0 };
			selector_scale = { 1, 1, 1};
			drawer_pos = { 0, 0, 0 };
			drawer_scale = { 1, 1, 1 };
			is_dragging = false;
			camera_pos_pressed = Angel::vec3(0.0f);
			camera_zoom_pressed = 0.0f;
			bounding_box_selector = {};
			bounding_box_drawer = {};

		}
		else if (mouse_previous_state == Input::ButtonState::Idle
			&& window_input.m_lmb_state == Input::ButtonState::JustPressed)
		{
			if (!input_on_imgui)
			{
				camera_pos_pressed = Camera::camera_pos();
				camera_zoom_pressed = Camera::get_zoom_ratio();
				if (radio_button_cur == (int)RadioButtons::Select)
				{
					unsigned int num_selections = cur_selections.size();
					ShapeModel* new_selected = list.frontmost_shape(Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y));
					if (num_selections == 1
						&& cur_selections[0] == new_selected)
					{
						drawing_selector_box = false;
						// Start drag
						is_dragging = true;
						list.move_shape_to_frontview(new_selected);
						Angel::vec3 v_old = map_from_global_using_old_camera(old_mouse_pos.x, old_mouse_pos.y);
						Angel::vec3 v_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
						Angel::vec3 drag_vector = v_new - v_old;
						new_selected->position() += drag_vector;
					}
					else if (num_selections > 1)
					{
						// TO DO Copy Paste?
					}
					else
					{
						drawing_selector_box = true;
						selector_pos = Camera::map_from_global(window_input.m_mouse_press_x, window_input.m_mouse_press_y);
						Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
						Angel::vec3 mouse_model_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
						selector_scale = (1.0f / init_shape_length) * (mouse_model_new - mouse_model_old);
						if (selector_scale.x == 0.0f)
						{
							selector_scale.x = 1.0f / init_shape_length;
						}
						if (selector_scale.y == 0.0f)
						{
							selector_scale.y = 1.0f / init_shape_length;
						}
						selector_scale.z = 1.0f;
					}
				}
				else if (radio_button_cur == (int)RadioButtons::DrawEqTri
					|| radio_button_cur == (int)RadioButtons::DrawRect)
				{
					drawing_multiple_selection_box = false;
					for (auto* item : cur_selections)
					{
						item->deselect();
					}
					cur_selections.clear();
					drawing_drawer_box = true;
					drawer_pos = Camera::map_from_global(window_input.m_mouse_press_x, window_input.m_mouse_press_y);
					Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
					Angel::vec3 mouse_model_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
					drawer_scale = (1.0f / init_shape_length) * (mouse_model_new - mouse_model_old);
					if (drawer_scale.x == 0.0f)
					{
						drawer_scale.x = 1.0f / init_shape_length;
					}
					if (drawer_scale.y == 0.0f)
					{
						drawer_scale.y = 1.0f / init_shape_length;
					}
					if (radio_button_cur == (int)RadioButtons::DrawEqTri)
					{
						float abs_max = std::max(std::abs(drawer_scale.x), std::abs(drawer_scale.y));
						float sign_x = 1.0f;
						float sign_y = 1.0f;
						if (drawer_scale.x < 0.0f)
						{
							sign_x = -1.0f;
						}
						if (drawer_scale.y < 0.0f)
						{
							sign_y = -1.0f;
						}
						drawer_scale.x = abs_max * sign_x;
						drawer_scale.y = abs_max * sign_y;
						drawer_scale.z = 1.0f;
					}
				}
			}

			// Consume the pressed state
			window_input.m_lmb_state = Input::ButtonState::BeingPressed;
			std::cout << "LMB is now BeingPressed" << std::endl;
			window_input.m_mouse_release_y = -1.0f;
			window_input.m_mouse_release_x = -1.0f;
			camera_pos_released = Angel::vec3(0.0f);
			camera_zoom_released = 0.0f;
		}
		else if (mouse_previous_state == Input::ButtonState::BeingPressed)
		{
			if (!input_on_imgui)
			{
				if (radio_button_cur == (int)RadioButtons::Select)
				{
					if (cur_selections.size() == 1 && is_dragging)
					{
						ShapeModel* old_selected = cur_selections[0];
						drawing_selector_box = false;
						// Continue drag
						Angel::vec3 v_old = map_from_global_using_old_camera(old_mouse_pos.x, old_mouse_pos.y);
						Angel::vec3 v_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
						Angel::vec3 drag_vector = v_new - v_old;
						old_selected->position() += drag_vector;
					}
					else
					{
						drawing_selector_box = true;
						Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
						Angel::vec3 mouse_model_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
						selector_scale = (1.0f / init_shape_length) * (mouse_model_new - mouse_model_old);
						if (selector_scale.x == 0.0f)
						{
							selector_scale.x = 1.0f / init_shape_length;
						}
						if (selector_scale.y == 0.0f)
						{
							selector_scale.y = 1.0f / init_shape_length;
						}
						selector_scale.z = 1.0f;
					}
				}
				else if (radio_button_cur == (int)RadioButtons::DrawEqTri
					|| radio_button_cur == (int)RadioButtons::DrawRect)
				{
					drawing_drawer_box = true;
					Angel::vec3 mouse_model_old = map_from_global_any(window_input.m_mouse_press_x, window_input.m_mouse_press_y, camera_pos_pressed, camera_zoom_pressed);
					Angel::vec3 mouse_model_new = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
					drawer_scale = (1.0f / init_shape_length) * (mouse_model_new - mouse_model_old);
					if (drawer_scale.x == 0.0f)
					{
						drawer_scale.x = 1.0f / init_shape_length;
					}
					if (drawer_scale.y == 0.0f)
					{
						drawer_scale.y = 1.0f / init_shape_length;
					}
					if (radio_button_cur == (int)RadioButtons::DrawEqTri)
					{
						float abs_max = std::max(std::abs(drawer_scale.x), std::abs(drawer_scale.y));
						float sign_x = 1.0f;
						float sign_y = 1.0f;
						if (drawer_scale.x < 0.0f)
						{
							sign_x = -1.0f;
						}
						if (drawer_scale.y < 0.0f)
						{
							sign_y = -1.0f;
						}
						drawer_scale.x = abs_max * sign_x;
						drawer_scale.y = abs_max * sign_y;
						drawer_scale.z = 1.0f;
					}

				}
			}
		}
		else if (window_input.m_lmb_state == Input::ButtonState::Idle)
		{
			// Update the add vertex line's end position
			if (drawing_poly_add_vertex_line)
			{
				Angel::vec3 release_pos = map_from_global_any(window_input.m_mouse_release_x, window_input.m_mouse_release_y, camera_pos_released, camera_zoom_released);
				Angel::vec3 current_pos = Camera::map_from_global(window_input.m_mouse_x, window_input.m_mouse_y);
				double dy = current_pos.y - release_pos.y;
				double dx = current_pos.x - release_pos.x;
				double line_length = std::sqrt(std::pow(dy, 2.0f) + std::pow(dx, 2.0f));
				double tetha = std::atan(dy / dx) / Angel::DegreesToRadians;
				if (dx < 0)
				{
					tetha += M_PI / Angel::DegreesToRadians;
				}
				polygon_add_vertex_line_scale.x = (1.0f / init_shape_length) * (line_length);
				polygon_add_vertex_line_scale.y = (1.0f / init_shape_length) * (100.0f / Camera::get_zoom_ratio());
				polygon_add_vertex_line_rotation.z = tetha;
			}
		}

		// Process RMB events
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			if (new_polygon)
			{
				new_polygon->select();
				cur_selections.clear();
				cur_selections.push_back(new_polygon);
			}
			polygon_mouse_model_coords.clear();
			drawing_poly_add_vertex_line = false;
			new_polygon = nullptr;
		}

		// Draw GUI components
		{
			new_imgui_frame();
			ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_::ImGuiCond_Always);
			ImGui::SetNextWindowSize(ImVec2((float)width, height / 7.0f), ImGuiCond_::ImGuiCond_Always);
			ImGuiWindowFlags flags_editor =
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoFocusOnAppearing |
				ImGuiWindowFlags_NoResize;
			ImGui::Begin("Editor Pane", nullptr, flags_editor);
			{
				if (ImGui::BeginTabBar("##tabs"))
				{
					if (ImGui::BeginTabItem("Home"))
					{
						ImGui::RadioButton("Selection Mode", &radio_button_cur, (int)RadioButtons::Select);
						ImGui::SameLine();
						ImGui::RadioButton("Deletion Mode", &radio_button_cur, (int)RadioButtons::Delete);
						ImGui::SameLine();
						ImGui::RadioButton("Draw Rectangle", &radio_button_cur, (int)RadioButtons::DrawRect);
						ImGui::SameLine();
						ImGui::RadioButton("Draw Eq. Triangle", &radio_button_cur, (int)RadioButtons::DrawEqTri);
						ImGui::SameLine();
						ImGui::RadioButton("Draw Convex Poly", &radio_button_cur, (int)RadioButtons::DrawPoly);
						ImGui::SameLine();
						if (radio_button_cur != (int)RadioButtons::DrawPoly)
						{
							polygon_mouse_model_coords.clear();
							new_polygon = nullptr;
							drawing_poly_add_vertex_line = false;
						}
						if (ImGui::Button("Undo"))
						{
							// TO DO Undo operation
						}
						ImGui::SameLine();
						if (ImGui::Button("Redo"))
						{
							// TO DO Redo operation
						}
						ImGui::SameLine();
						if (ImGui::Button("Save"))
						{
							// TO DO Undo operation
						}
						ImGui::SameLine();
						if (ImGui::Button("Load"))
						{
							// TO DO Redo operation
						}
						if (radio_button_cur == (int)RadioButtons::DrawEqTri
							|| radio_button_cur == (int)RadioButtons::DrawRect
							|| radio_button_cur == (int)RadioButtons::DrawPoly)
						{
							ImGui::ColorEdit4("Drawing Color", &color_draw[0], f);
						}
						ImGui::Text("Cursor Coordinates w.r.t Sheet: %f, %f", cursor_model_coords.x - sheet_pos.x, cursor_model_coords.y - sheet_pos.y);
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Selection"))
					{
						unsigned int n_selections = cur_selections.size();
						if (n_selections == 0)
						{
							ImGui::Text("Currently, there is no shape selected");
						}
						else if (n_selections == 1)
						{
							ImGui::ColorEdit4("Shape Color", &(cur_selections[0]->color()).x, f);
							ImGui::SameLine();
							ImGui::SliderFloat("Rotation Degree", &(cur_selections[0]->rotation()).z, 0.0f, 360, "%.3f", 1.0f);
							if (ImGui::Button("Rotate 30 Degrees"))
							{
								(cur_selections[0]->rotation()).z += 30.0f;
							}
							if (ImGui::Button("Rotate -30 Degrees"))
							{
								(cur_selections[0]->rotation()).z -= 30.0f;
							}
							(cur_selections[0]->rotation()).z = (float)(((int)(cur_selections[0]->rotation()).z + 360) % 360);
						}
						else
						{
							ImGui::Text("Multiple shapes were selected...");
						}
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
			}
			ImGui::End();
		}
		
		// Clear background
		renderer.set_viewport(window);
		renderer.clear((float*)&clear_color);

		// Get cursor model coordinates
		ImGui::EndFrame();

		// Draw sheet
		Shape::shader()->bind();
		Shape::shader()->set_uniform_4f("u_color",
			color_sheet[0],
			color_sheet[1],
			color_sheet[2],
			color_sheet[3]);
		MVP_mat_sheet = projection_matrix * view_matrix * model_sheet_matrix;
		Shape::shader()->set_uniform_mat4f("u_MVP", MVP_mat_sheet);
		renderer.draw_triangles(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());

		// Draw the draw list
		list.draw_all();

		// Draw box around multiple selections
		if (drawing_multiple_selection_box)
		{
			// TODO draw a box around selections if there more than one
			ASSERT(cur_selections.size() != 1);
			std::array<float, 6> multiple_selection_bounding_cube = ShapeModel::bounding_cube(cur_selections);
			Angel::vec3 multiple_selection_pos = {
				multiple_selection_bounding_cube[0],
				multiple_selection_bounding_cube[2],
				global_z_pos_2d
			};
			Angel::vec3 multiple_selection_pos_scale = {
				(-multiple_selection_bounding_cube[0] + multiple_selection_bounding_cube[1]) / init_shape_length,
				(-multiple_selection_bounding_cube[2] + multiple_selection_bounding_cube[3]) / init_shape_length,
				1.0f
			};
			Angel::mat4 model_mat_multiple_selection_box = Angel::Translate(multiple_selection_pos)
				* Angel::Scale(multiple_selection_pos_scale);
			Angel::mat4 MVP_mat_multiple_selection_box = projection_matrix * view_matrix * model_mat_multiple_selection_box;
			Shape::shader()->bind();
			Shape::shader()->set_uniform_4f("u_color",
				drawer_box_col[0],
				drawer_box_col[1],
				drawer_box_col[2],
				drawer_box_col[3]);
			Shape::shader()->set_uniform_mat4f("u_MVP", MVP_mat_multiple_selection_box);
			renderer.draw_lines(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());
		}

		// Draw the selector box
		if (drawing_selector_box)
		{
			Shape::shader()->bind();
			Shape::shader()->set_uniform_4f("u_color",
				selector_box_col[0],
				selector_box_col[1],
				selector_box_col[2],
				selector_box_col[3]);
			model_selector_box = Angel::Translate(selector_pos)
				* Angel::Scale(selector_scale);
			MVP_selector_box = projection_matrix * view_matrix * model_selector_box;
			Shape::shader()->set_uniform_mat4f("u_MVP", MVP_selector_box);
			renderer.draw_triangles(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());
		}

		// Draw the drawer box for predefined shapes
		if (drawing_drawer_box)
		{
			Shape::shader()->bind();
			Shape::shader()->set_uniform_4f("u_color",
				drawer_box_col[0],
				drawer_box_col[1],
				drawer_box_col[2],
				drawer_box_col[3]);
			model_drawer_box = Angel::Translate(drawer_pos)
				* Angel::Scale(drawer_scale);
			MVP_drawer_box = projection_matrix * view_matrix * model_drawer_box;
			Shape::shader()->set_uniform_mat4f("u_MVP", MVP_drawer_box);
			renderer.draw_lines(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());
		}

		// Draw the add vertex line for draw polygon command
		if (drawing_poly_add_vertex_line)
		{
			Shape::shader()->bind();
			Shape::shader()->set_uniform_4f("u_color",
				drawer_box_col[0],
				drawer_box_col[1],
				drawer_box_col[2],
				drawer_box_col[3]);
			model_polygon_add_vertex_line = Angel::Translate(polygon_add_vertex_line_pos)
				* Angel::RotateZ(polygon_add_vertex_line_rotation.z)
				* Angel::Scale(polygon_add_vertex_line_scale);
			MVP_polygon_add_vertex_line = projection_matrix * view_matrix * model_polygon_add_vertex_line;
			Shape::shader()->set_uniform_mat4f("u_MVP", MVP_polygon_add_vertex_line);
			renderer.draw_lines(Shape::rectangle()->vertex_array(), Shape::rectangle()->triangles_index_buffer(), Shape::shader());
		}

		// Draw ImGui
		render_imgui();

		// Update the frame buffer
		glfwSwapBuffers(window);
	}
	
	// Clear the draw list & delete the VB/IB/VA objects for polygons
	list.shutdown();

	// Clear heap memory for predefined shapes & delete the VB/IB/VA objects for predefined shapes
	Shape::destroy_static_members_allocated_on_the_heap();

	// Shutdown ImGui & GLFW
	shutdown_imgui();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}