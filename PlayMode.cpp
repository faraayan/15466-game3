#include <string>
#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint skewer_vao_for_lit = 0;
Load<MeshBuffer> skewer_meshes(LoadTagDefault, []() -> MeshBuffer const *
							   {
	MeshBuffer const *ret = new MeshBuffer(data_path("skewer.pnct"));
	skewer_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

GLuint marshmallow_vao_for_lit = 0;
Load<MeshBuffer> marshmallow_meshes(LoadTagDefault, []() -> MeshBuffer const *
									{
	MeshBuffer const *ret = new MeshBuffer(data_path("marshmallow.pnct"));
	marshmallow_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

GLuint ground_vao_for_lit = 0;
Load<MeshBuffer> ground_meshes(LoadTagDefault, []() -> MeshBuffer const *
							   {
	MeshBuffer const *ret = new MeshBuffer(data_path("ground.pnct"));
	ground_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

GLuint fire_vao_for_lit = 0;
Load<MeshBuffer> fire_meshes(LoadTagDefault, []() -> MeshBuffer const *
							 {
	MeshBuffer const *ret = new MeshBuffer(data_path("fire.pnct"));
	fire_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

// Used from my game 2 code format
Load<Scene> campfire_scene(LoadTagDefault, []() -> Scene const *
						   {
	Scene *campfire = new Scene();
	
	auto load_scene = [&](const char* scene_path,
									 MeshBuffer const* buf, GLuint vao) {
		campfire->load(data_path(scene_path),
			[&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
				Mesh const &mesh = buf->lookup(mesh_name);

				scene.drawables.emplace_back(transform);
				Scene::Drawable &drawable = scene.drawables.back();
				
				drawable.pipeline = lit_color_texture_program_pipeline;
				
				drawable.pipeline.vao   = vao;
				drawable.pipeline.type  = mesh.type;
				drawable.pipeline.start = mesh.start;
				drawable.pipeline.count = mesh.count;
			}
		);
	};
	
	load_scene("skewer.scene",   &*skewer_meshes,   skewer_vao_for_lit);
	load_scene("fire.scene",   &*fire_meshes,   fire_vao_for_lit);
	load_scene("ground.scene", &*ground_meshes, ground_vao_for_lit);
	load_scene("marshmallow.scene", &*marshmallow_meshes, marshmallow_vao_for_lit);

	return campfire; });

Load<Sound::Sample> dusty_floor_sample(LoadTagDefault, []() -> Sound::Sample const *
									   { return new Sound::Sample(data_path("dusty-floor.opus")); });

Load<Sound::Sample> honk_sample(LoadTagDefault, []() -> Sound::Sample const *
								{ return new Sound::Sample(data_path("honk.wav")); });

PlayMode::PlayMode() : scene(*campfire_scene)
{
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "skewer_root")
			skewer_root = &transform;
		else if (transform.name == "marshmallow_root")
			marshmallow_root = &transform;
		else if (transform.name == "fire_root")
			fire_root = &transform;
	}

	// Goal touched seconds
	goal_touched_seconds = 10.0f + 5.0f * (float(rand()) / float(RAND_MAX));

	// Set fire to random initial position
	if (fire_root)
	{
		fire_root->position.x = -20.0f + 40.0f * (float(rand()) / float(RAND_MAX));
		fire_root->position.y = -20.0f + 40.0f * (float(rand()) / float(RAND_MAX));
	}

	if (skewer_root == nullptr)
		throw std::runtime_error("skewer_root not found.");
	if (marshmallow_root == nullptr)
		throw std::runtime_error("marshmallow_root not found.");
	if (fire_root == nullptr)
		throw std::runtime_error("fire_root not found.");

	// get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// start music loop playing:
	//  (note: position will be over-ridden in update())
	// leg_tip_loop = Sound::loop_3D(*dusty_floor_sample, 1.0f, get_leg_tip_position(), 10.0f);
}

PlayMode::~PlayMode()
{
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size)
{

	if (evt.type == SDL_EVENT_KEY_DOWN)
	{
		if (evt.key.key == SDLK_ESCAPE)
		{
			SDL_SetWindowRelativeMouseMode(Mode::window, false);
			return true;
		}
		else if (evt.key.key == SDLK_A)
		{
			left.downs += 1;
			left.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_D)
		{
			right.downs += 1;
			right.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_W)
		{
			up.downs += 1;
			up.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_S)
		{
			down.downs += 1;
			down.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_R)
		{
			r.downs += 1;
			r.pressed = true;
			return true;
		}
		else if (evt.key.key == SDLK_F)
		{
			f.downs += 1;
			f.pressed = true;
			return true;
		}

		// else if (evt.key.key == SDLK_SPACE)
		// {
		// 	if (honk_oneshot)
		// 		honk_oneshot->stop();
		// 	honk_oneshot = Sound::play_3D(*honk_sample, 0.3f, glm::vec3(4.6f, -7.8f, 6.9f)); // hardcoded position of front of car, from blender
		// }
	}
	else if (evt.type == SDL_EVENT_KEY_UP)
	{
		if (evt.key.key == SDLK_A)
		{
			left.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_D)
		{
			right.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_W)
		{
			up.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_S)
		{
			down.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_R)
		{
			r.pressed = false;
			return true;
		}
		else if (evt.key.key == SDLK_F)
		{
			f.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed)
{
	// Track num seconds marshmallow touches fire
	if (marshmallow_root && fire_root && fire_visible)
	{
		float dist = glm::distance(marshmallow_root->position, fire_root->position);
		if (dist < 5.0f)
		{
			touching_seconds += elapsed;
		}
	}

	// Show and hide fire randomly
	fire_timer += elapsed;
	if (fire_visible)
	{
		if (fire_timer >= 10.0f)
		{
			fire_visible = false;
			fire_timer = 0.0f;
			// Hide fire away
			fire_root->position = glm::vec3(1000.0f, 1000.0f, fire_root->position.z);
		}
	}
	else
	{
		if (fire_timer >= 1.0f)
		{
			fire_visible = true;
			fire_timer = 0.0f;
			// Random position in (-20, 20)
			fire_root->position.x = -20.0f + 40.0f * (float(rand()) / float(RAND_MAX));
			fire_root->position.y = -20.0f + 40.0f * (float(rand()) / float(RAND_MAX));
		}
	}

	static float rotation = 0.0f;
	float amt = 0.0f;
	if (left.pressed && !right.pressed)
		amt += 1.0f;
	if (!left.pressed && right.pressed)
		amt -= 1.0f;
	rotation += amt * 1.8f * elapsed;

	// Get direction away
	glm::vec3 tilt_dir = glm::normalize(glm::cross(glm::vec3(0, 0, 1), -glm::normalize(skewer_root->position - camera->transform->position)));
	float tilt_angle = glm::radians(-70.0f);

	// Tilt rotation - for style
	glm::mat4 tilt_mat = glm::rotate(glm::mat4(1.0f), tilt_angle, tilt_dir);

	// Actual Z rotation
	glm::mat4 zrot_mat = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));

	// Also rotate skewer and marshmallow for style
	glm::mat4 visual_rot = tilt_mat * zrot_mat;
	skewer_root->rotation = glm::quat_cast(visual_rot);

	// Camera does z rotation only
	glm::mat4 cam_rot_mat = zrot_mat;
	glm::quat cam_rot = glm::quat_cast(cam_rot_mat);

	amt = 0.0f;
	if (up.pressed && !down.pressed)
		amt = +1.0f;
	else if (!up.pressed && down.pressed)
		amt = -1.0f;
	glm::vec3 move_vec = skewer_root->rotation * glm::vec3(0, -1, 0);
	move_vec.z = 0.0f;
	skewer_root->position += move_vec * (amt * 20.0f * elapsed);

	// Move up and down
	amt = 0.0f;
	if (r.pressed && !f.pressed)
		amt = +1.0f;
	else if (!r.pressed && f.pressed)
		amt = -1.0f;
	float new_z = skewer_root->position.z + amt * 20.0f * elapsed;
	if (new_z > 10.f)
		new_z = 10.0f;
	if (new_z < 0.0f)
		new_z = 0.0f;
	skewer_root->position.z = new_z;

	// Make marshmallow follow skewer with offset
	if (marshmallow_root && skewer_root)
	{
		marshmallow_root->position = skewer_root->position + skewer_root->rotation * glm::vec3(0, 0, 2.0f);
		marshmallow_root->rotation = skewer_root->rotation;
	}

	// Set camera to follow skewer
	{
		const glm::vec3 camera_offset(0.0f, 23.0f, 8.0f);
		camera->transform->position = skewer_root->position + cam_rot * camera_offset;
		camera->transform->rotation = glm::quat_cast(glm::inverse(glm::lookAt(camera->transform->position, skewer_root->position, glm::vec3(0, 0, 1))));
	}

	{ // update listener to camera position:
		glm::mat4x3 frame = camera->transform->make_parent_from_local();
		glm::vec3 frame_right = frame[0];
		glm::vec3 frame_at = frame[3];
		Sound::listener.set_position_right(frame_at, frame_right, 1.0f / 60.0f);
	}

	// reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size)
{
	// update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	// set up light type and position for lit_color_texture_program:
	//  TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f, -1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); // 1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); // this is the default depth comparison function, but FYI you can change it.

	scene.draw(*camera);

	{ // use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f));

		constexpr float H = 0.09f;
		char buf[32];
		snprintf(buf, sizeof(buf), "Toasted: %.1fs, Goal: %.1fs", touching_seconds, goal_touched_seconds);
		std::string touch_text = buf;
		lines.draw_text(touch_text,
						glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(touch_text,
						glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
						glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
						glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
	GL_ERRORS();
}

// glm::vec3 PlayMode::get_leg_tip_position()
// {
// 	// the vertex position here was read from the model in blender:
// 	return glm::vec4(-1.26137f, -11.861f, 0.0f, 1.0f);
// }
