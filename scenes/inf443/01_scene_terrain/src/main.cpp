#include "vcl/vcl.hpp"
#include <iostream>
#include <time.h>

#include "terrain.hpp"
#include "tree.hpp"
#include "Gridcell.hpp"
#include "Boids.hpp"



using namespace vcl;

struct keyboard_state_parameters {
	bool left = false;
	bool right = false;
	bool up = false;
	bool down = false;
};

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
	keyboard_state_parameters keyboard_state;
	float speed = 2.0f;
};
user_interaction_parameters user;

struct scene_environment
{
	camera_head camera;;
	mat4 projection;
	vec3 light;

	std::vector<vcl::vec3> tree_position;
	std::vector<vcl::vec3> billboard_position;
};
scene_environment scene;


float cohesion_strength = 4.0f;
float alignment_strength = 2.3f;
float separation_strength = 8.0f;
std::vector<mesh_drawable> terrain_marching_cubes;
mesh_drawable sphere;
timer_event_periodic timer(0.6f);
int boid_number = 300;
std::vector<Boid> flock;
float isolevel = 1.0f;
int terrain_size = 60;
float scale = 0.025f;

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
//void mouse_move_callback(GLFWwindow* window, double xpos, double ypos);
void window_size_callback(GLFWwindow* window, int width, int height);

void initialize_data();
void display_scene();
void display_interface();



int main(int, char* argv[])
{
	std::cout << "Run " << argv[0] << std::endl;

	int const width = 1280, height = 1024;
	GLFWwindow* window = create_window(width, height);
	window_size_callback(window, width, height);
	std::cout << opengl_info_display() << std::endl;;

	imgui_init(window);
	//glfwSetCursorPosCallback(window, mouse_move_callback);
	glfwSetKeyCallback(window, keyboard_callback);
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
		
		glClearColor(0.0f, 0.25f, 0.4f, 0.2f);
		glClear(GL_COLOR_BUFFER_BIT);
		glClear(GL_DEPTH_BUFFER_BIT);
		imgui_create_frame();
		if(user.fps_record.event) {
			std::string const title = "VCL Display - "+str(user.fps_record.fps)+" fps";
			glfwSetWindowTitle(window, title.c_str());
		}

		ImGui::Begin("GUI",NULL,ImGuiWindowFlags_AlwaysAutoResize);
		//user.cursor_on_gui = ImGui::IsAnyWindowFocused();
		user.cursor_on_gui = ImGui::GetIO().WantCaptureMouse;

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
	//scene.camera.distance_to_center = 20.0f;
	//scene.camera.look_at({ 4,3,2 }, { 0,0,0 }, { 0,0,1 });
	scene.camera.position_camera = { 0.5f, 0.5f, -2.0f };
	scene.camera.manipulator_rotate_roll_pitch_yaw(0, 0, pi / 2.0f);

	// Create visual terrain surface
	terrain_marching_cubes = create_terrain(terrain_size, isolevel, scale);
	//Generate flock of boids
	flock = create_flock(boid_number);
	sphere = mesh_drawable(mesh_primitive_cone(0.05f, 0.2f));
}



void display_scene()
{
	for (int i = 0; i < 10; i++) {
		//draw(terrain_marching_cubes[i], scene);
	}
	float const dt = timer.update();
	std::vector<Boid> flock_dt;
	for (Boid boid : flock) {
		Boid new_boid;
		vcl::vec3 steer;
		vcl::vec3 new_position;
		steer = cohesion_strength * cohesion(boid, flock) + alignment_strength * alignment(boid, flock) + separation_strength * separation(boid, flock);
		float steer_norm = (float) steer[0] * steer[0] + steer[1] * steer[1] + steer[2] * steer[2];
		if (steer_norm > maxforce) {
			steer = steer * (float) maxforce / steer_norm;
		}
		vcl::vec3 new_speed = boid.speed + dt * steer;
		float new_speed_norm = new_speed[0] * new_speed[0] + new_speed[1] * new_speed[1] + new_speed[2] * new_speed[2];
		if (new_speed_norm > maxspeed) {
			new_speed = new_speed * (float) maxspeed / new_speed_norm;
		}
		if (new_speed_norm < minspeed) {
			new_speed = new_speed * (float)minspeed / new_speed_norm;
		}
		new_position = boid.position + dt * new_speed;
		new_boid.position = new_position;
		new_boid.speed = new_speed;
		flock_dt.push_back(new_boid);
	}
	flock = flock_dt;
	for (int i = 0; i < flock.size(); i++) {
		if (flock[i].position[0] < 0) { 
			flock[i].speed[0] = -flock[i].speed[0];
			flock[i].position[0] = 0;
		}
		if (flock[i].position[1] < 0) { 
			flock[i].speed[1] = -flock[i].speed[1];
			flock[i].position[1] = 0;
		}
		if (flock[i].position[2] < 0) { 
			flock[i].speed[2] = -flock[i].speed[2];
			flock[i].position[2] = 0;
		}
		if (flock[i].position[0] > terrain_size) { 
			flock[i].speed[0] = -flock[i].speed[0];
			flock[i].position[0] = terrain_size;
		}
		if (flock[i].position[1] > terrain_size) { 
			flock[i].speed[1] = -flock[i].speed[1]; 
			flock[i].position[1] = terrain_size;
		}
		if (flock[i].position[2] > terrain_size) { 
			flock[i].speed[2] = -flock[i].speed[2];
			flock[i].position[2] = terrain_size;
		}
		sphere.transform.translate = flock[i].position;
		sphere.transform.rotate = rotation(cross({ 0,0,1 }, flock[i].speed) / norm(flock[i].speed), acos(scalar({ 0,0,1 }, flock[i].speed)/norm(flock[i].speed)));
		sphere.shading.color = vec3( 0.0f,1.0f,0.1f );
		draw(sphere, scene);
	}
	scene.camera.position_camera += user.speed * 1.0f * dt * scene.camera.front();
	if (user.keyboard_state.up)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0, -0.5f * dt, 0);
	if (user.keyboard_state.down)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0, 0.5f * dt, 0);
	if (user.keyboard_state.right)
		scene.camera.manipulator_rotate_roll_pitch_yaw(0.7f * dt, 0, 0);
	if (user.keyboard_state.left)
		scene.camera.manipulator_rotate_roll_pitch_yaw(-0.7f * dt, 0, 0);
}

void keyboard_callback(GLFWwindow*, int key, int, int action, int)
{
	if (key == GLFW_KEY_UP) {
		if (action == GLFW_PRESS) user.keyboard_state.up = true;
		if (action == GLFW_RELEASE) user.keyboard_state.up = false;
	}

	if (key == GLFW_KEY_DOWN) {
		if (action == GLFW_PRESS) user.keyboard_state.down = true;
		if (action == GLFW_RELEASE) user.keyboard_state.down = false;
	}

	if (key == GLFW_KEY_LEFT) {
		if (action == GLFW_PRESS) user.keyboard_state.left = true;
		if (action == GLFW_RELEASE) user.keyboard_state.left = false;
	}

	if (key == GLFW_KEY_RIGHT) {
		if (action == GLFW_PRESS) user.keyboard_state.right = true;
		if (action == GLFW_RELEASE) user.keyboard_state.right = false;
	}
}


void display_interface()
{
	bool update = false;
	ImGui::Checkbox("Frame", &user.gui.display_frame);
	ImGui::SliderFloat("Speed", &user.speed, 0.2f, 5.0f);
	update |= ImGui::SliderFloat("Isolevel", &isolevel,0,1);
	update |= ImGui::SliderFloat("Cohesion", &cohesion_strength, 0, 10);
	update |= ImGui::SliderFloat("Alignment", &alignment_strength, 0, 10);
	update |= ImGui::SliderFloat("Separation", &separation_strength, 0, 10);
	update |= ImGui::SliderFloat("MaxSpeed", &maxspeed, 1, 20);
	update |= ImGui::SliderFloat("Vision angle", &vision_angle, -1, 1);
	update |= ImGui::SliderFloat("MaxForce", &maxforce, 0, 5);
	update |= ImGui::SliderFloat("Perception Distance", &perception, 0, terrain_size);
	update |= ImGui::SliderFloat("Avoid Distance", &avoid, 0, 10);

	if (update) {
		terrain_marching_cubes = create_terrain(terrain_size, isolevel, scale);
		float const dt = timer.update();
		std::vector<Boid> flock_dt;
		for (Boid boid : flock) {
			Boid new_boid;
			vcl::vec3 steer;
			vcl::vec3 new_position;
			steer = cohesion_strength * cohesion(boid, flock) + alignment_strength * alignment(boid, flock) + separation_strength * separation(boid, flock);
			float steer_norm = (float) steer[0] * steer[0] + steer[1] * steer[1] + steer[2] * steer[2];
			if (steer_norm > maxforce) {
				steer = steer* (float)maxforce / steer_norm;
			}
			vcl::vec3 new_speed = boid.speed + dt * steer;
			float new_speed_norm = (float) new_speed[0] * new_speed[0] + new_speed[1] * new_speed[1] + new_speed[2] * new_speed[2];
			if (new_speed_norm > maxspeed) {
				new_speed = new_speed * (float)maxspeed / new_speed_norm;
			}
			if (new_speed_norm < minspeed) {
				new_speed = new_speed * (float)minspeed / new_speed_norm;
			}
			new_position = boid.position + dt * new_speed;
			new_boid.position = new_position;
			new_boid.speed = new_speed;
			flock_dt.push_back(new_boid);
		}
		flock = flock_dt;
	}
}


void window_size_callback(GLFWwindow* , int width, int height)
{
	glViewport(0, 0, width, height);
	float const aspect = width / static_cast<float>(height);
	scene.projection = projection_perspective(50.0f*pi/180.0f, aspect, 0.1f, 1000.0f);
}


//void mouse_move_callback(GLFWwindow* window, double xpos, double ypos)
//{
//	vec2 const  p1 = glfw_get_mouse_cursor(window, xpos, ypos);
//	vec2 const& p0 = user.mouse_prev;
//	glfw_state state = glfw_current_state(window);
//
//	auto& camera = scene.camera;
//	if(!user.cursor_on_gui){
//		if(state.mouse_click_left && !state.key_ctrl)
//			scene.camera.manipulator_rotate_trackball(p0, p1);
//		if(state.mouse_click_left && state.key_ctrl)
//			camera.manipulator_translate_in_plane(p1-p0);
//		if(state.mouse_click_right)
//			camera.manipulator_scale_distance_to_center( (p1-p0).y );
//	}
//
//	user.mouse_prev = p1;
//}

void opengl_uniform(GLuint shader, scene_environment const& current_scene)
{
	opengl_uniform(shader, "projection", current_scene.projection);
	opengl_uniform(shader, "view", scene.camera.matrix_view());
	opengl_uniform(shader, "light", scene.light, false);
}



