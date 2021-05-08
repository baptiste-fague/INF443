#include "vcl/vcl.hpp"
#include <iostream>

#include "models_textures.hpp"

using namespace vcl;

struct gui_parameters {
	bool display_frame = true;
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
};
scene_environment scene;


void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_interface();

mesh_drawable cyl;
mesh_drawable disc_a;
mesh_drawable disc_b;

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
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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
		
		draw(cyl, scene);
		draw(disc_a, scene);
		draw(disc_b, scene);
		//draw_wireframe(visual, scene, {1,0,0});

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



void initialize_data()
{
	// Basic setups of shaders and camera
	GLuint const shader_mesh = opengl_create_shader_program(opengl_shader_preset("mesh_vertex"), opengl_shader_preset("mesh_fragment"));
	mesh_drawable::default_shader = shader_mesh;
	mesh_drawable::default_texture = opengl_texture_to_gpu(image_raw{1,1,image_color_type::rgba,{255,255,255,255}});

	user.global_frame = mesh_drawable(mesh_primitive_frame());
	user.gui.display_frame = false;
	scene.camera.distance_to_center = 2.5f;
	scene.camera.look_at({-4,3,2}, {0,0,0}, {0,0,1});

	// Geometry creation
	//-----------------------------------
	// Create a quadrangle as a mesh
	mesh quadrangle;
	quadrangle.position     = {{-1,-1,0}, { 1,-1,0}, { 1, 1,0}, {-1, 1,0}};
	quadrangle.uv = { {0,2}, {2,2}, {2,0}, {0,0} }; // Associate Texture-Coordinates to the vertices of the quadrangle
	quadrangle.connectivity = {{0,1,2}, {0,2,3}};

	quadrangle.fill_empty_field(); // (fill with some default values the other buffers (colors, normals) that we didn't filled before)
	
	mesh cylindre = cylinder_with_texture();
	mesh disque = disc_with_texture();
	
	// Convert the mesh structure into a mesh_drawable structure
	//visual = mesh_drawable(quadrangle);
	cyl = mesh_drawable(cylinder_with_texture());
	disc_a = mesh_drawable(disque);
	disc_b = mesh_drawable(disque);
	disc_a.transform.translate = { 0.0f, 0.0f, -2.0f };
	disc_b.transform.translate = { 0.0f, 0.0f, 2.0f };


	// Texture Image load and association
	//-----------------------------------	
	// Load an image from a file
	image_raw const im = image_load_png("assets/trunk.png");
	image_raw const img = image_load_png("assets/tree-ring.png");

	// Send this image to the GPU, and get its identifier texture_image_id
	GLuint const texture_image_id = opengl_texture_to_gpu(im, 
		GL_REPEAT, 
		GL_REPEAT);
	GLuint const texture_img_id = opengl_texture_to_gpu(img,
		GL_REPEAT,
		GL_REPEAT);

	// Associate the texture_image_id to the image texture used when displaying visual
	cyl.texture = texture_image_id;
	disc_a.texture = texture_img_id;
	disc_b.texture = texture_img_id;

}




void display_interface()
{
	ImGui::Checkbox("Frame", &user.gui.display_frame);
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
	opengl_uniform(shader, "view", current_scene.camera.matrix_view());
	opengl_uniform(shader, "light", current_scene.light, false);
}



