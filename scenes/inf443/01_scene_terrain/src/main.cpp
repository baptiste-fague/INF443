#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>

#include "terrain.hpp"
#include "tree.hpp"
#include "Gridcell.h"



using namespace vcl;

struct gui_parameters {
	bool display_frame = false;
	bool display_wireframe = false;
	//bool add_sphere = true;
	//float sphereRadius;
	//float color[3];

	//NoiseSettings noiseSettings;
};

struct user_interaction_parameters {
	vec2 mouse_prev;
	timer_fps fps_record;
	mesh_drawable global_frame;
	gui_parameters gui;
	bool cursor_on_gui;
};
user_interaction_parameters user;

struct scene_environment
{
	camera_around_center camera;
	mat4 projection;
	vec3 light;

	std::vector<vcl::vec3> tree_position;
	std::vector<vcl::vec3> billboard_position;
};
scene_environment scene;


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();

mesh_drawable terrain;
Gridcell test;
float isolevel = 0.5f;

int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);
	
	std::cout<<"Initialize data ..."<<std::endl;
	initialize_data();


	std::cout<<"Start animation loop ..."<<std::endl;
	user.fps_record.start();
	glEnable(GL_DEPTH_TEST);
	while (!glfwWindowShouldClose(window))
	{
		scene.light = scene.camera.position();
		user.fps_record.update();
		
		glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		user.cursor_on_gui = ImGui::IsAnyWindowFocused();

		if(user.gui.display_frame) draw(user.global_frame, scene);

		display_interface();
		display_scene();

		ImGui::End();
		imgui_render_frame(window);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	imgui_cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void display_terrain() {
	draw(terrain, scene);
	if (user.gui.display_wireframe)
		draw_wireframe(terrain, scene, { 1, 0, 0 });
}



void initialize_data()
{
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	GLuint const shader_uniform_color = opengl_create_shader_program(opengl_shader_preset("single_color_vertex"), opengl_shader_preset("single_color_fragment"));
	GLuint const texture_white = opengl_texture_to_gpu(image_raw{ 1,1,image_color_type::rgba,{255,255,255,255} });
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = texture_white;
	curve_drawable::default_shader = shader_uniform_color;
	segments_drawable::default_shader = shader_uniform_color;

	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
	scene.camera.distance_to_center = 2.5f;
	scene.camera.look_at({ 4,3,2 }, { 0,0,0 }, { 0,0,1 });

	// Create visual terrain surface
	vcl::vec3 cube1_positions[8] = { vcl::vec3(4, 0, 0), vcl::vec3(2, 1, 0), vcl::vec3(0, 3, 0), vcl::vec3(0, 0, 0),  vcl::vec3(5, 0, 1),  vcl::vec3(2, 3, 4),  vcl::vec3(0, 1, 1),  vcl::vec3(0, 0, 1) };
	float cube1_valeurs[8] = { 0.4f,0.6f,0.7f,0.2f,0.8f,0.1f,1.0f,0.0f };
	//float cube1_valeurs[8] = { 0.1f,0.1f,0.9f,0.1f,0.8f,0.1f,0.1f,0.1f };
	test = Gridcell(cube1_positions, cube1_valeurs );
	std::vector<Triangle> vecteur = polygonize(test,isolevel);
	mesh first_try;
	first_try.position.resize(3*vecteur.size());
	for (int i = 0; i < vecteur.size(); i++) {
		first_try.position[3 * i] = vecteur[i].p[0];
		first_try.position[3 * i + 1] = vecteur[i].p[1];
		first_try.position[3 * i + 2] = vecteur[i].p[2];
		first_try.connectivity.push_back({ 3 * i, 3 * i + 1, 3 * i + 2 });
	}
	for (int i = 0; i < first_try.position.size(); i++) {
		std::cout << first_try.position[i].x << ' ' << first_try.position[i].y << ' ' << first_try.position[i].z << std::endl;
	}

	first_try.fill_empty_field();
	terrain = mesh_drawable(first_try);
	terrain.shading.color = { 0.6f,0.85f,0.5f };
	terrain.shading.phong.specular = 0.0f; // non-specular terrain material

}



void display_scene()
{
	draw(terrain, scene);
}


void display_interface()
{
	bool update = false;
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	update |= ImGui::SliderFloat("Isolevel", &isolevel,0,1);

	if (update) {
		std::vector<Triangle> vecteur = polygonize(test, isolevel);
		mesh first_try;
		first_try.position.resize(3 * vecteur.size());
		for (int i = 0; i < vecteur.size(); i++) {
			first_try.position[3 * i] = vecteur[i].p[0];
			first_try.position[3 * i + 1] = vecteur[i].p[1];
			first_try.position[3 * i + 2] = vecteur[i].p[2];
			first_try.connectivity.push_back({ 3 * i, 3 * i + 1, 3 * i + 2 });
			
		}
		first_try.fill_empty_field();
		terrain = mesh_drawable(first_try);
		terrain.shading.color = { 0.6f,0.85f,0.5f };
		terrain.shading.phong.specular = 0.0f; // non-specular terrain material
	}
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 100.0f);
}


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
{
	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
	vec2 const& p0 = user.mouse_prev;
	glfw_state state = glfw_current_state(window);

	auto& camera = scene.camera;
	if(!user.cursor_on_gui){
		if(state.mouse_click_left && !state.key_ctrl)
			scene.camera.manipulator_rotate_trackball(p0, p1);
		if(state.mouse_click_left && state.key_ctrl)
			camera.manipulator_translate_in_plane(p1-p0);
		if(state.mouse_click_right)
			camera.manipulator_scale_distance_to_center( (p1-p0).y );
	}

	user.mouse_prev = p1;
}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", scene.camera.matrix_view());
	opengl_uniform(shader, "light", scene.light, false);
}



