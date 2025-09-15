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

GLuint marshmallow_golden_vao_for_lit = 0;
Load<MeshBuffer> marshmallow_golden_meshes(LoadTagDefault, []() -> MeshBuffer const *
										   {
	MeshBuffer const *ret = new MeshBuffer(data_path("marshmallow-golden.pnct"));
	marshmallow_golden_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

GLuint marshmallow_burnt_vao_for_lit = 0;
Load<MeshBuffer> marshmallow_burnt_meshes(LoadTagDefault, []() -> MeshBuffer const *
										  {
	MeshBuffer const *ret = new MeshBuffer(data_path("marshmallow-burnt.pnct"));
	marshmallow_burnt_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret; });

GLuint marshmallow_almost_vao_for_lit = 0;
Load<MeshBuffer> marshmallow_almost_meshes(LoadTagDefault, []() -> MeshBuffer const *
										   {
	MeshBuffer const *ret = new MeshBuffer(data_path("marshmallow-almost.pnct"));
	marshmallow_almost_vao_for_lit = ret->make_vao_for_program(lit_color_texture_program->program);
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
	load_scene("marshmallow-golden.scene", &*marshmallow_golden_meshes, marshmallow_golden_vao_for_lit);
	load_scene("marshmallow-burnt.scene", &*marshmallow_burnt_meshes, marshmallow_burnt_vao_for_lit);
	load_scene("marshmallow-almost.scene", &*marshmallow_almost_meshes, marshmallow_almost_vao_for_lit);

	return campfire; });

Load<Sound::Sample> fire_crackle_sample(LoadTagDefault, []() -> Sound::Sample const *
										{ return new Sound::Sample(data_path("fire.wav")); });

Load<Sound::Sample> sizzle_sample(LoadTagDefault, []() -> Sound::Sample const *
								  { return new Sound::Sample(data_path("sizzle.wav")); });

Load<Sound::Sample> background_sample(LoadTagDefault, []() -> Sound::Sample const *
									  { return new Sound::Sample(data_path("song.wav")); });

Load<Sound::Sample> win_sample(LoadTagDefault, []() -> Sound::Sample const *
							   { return new Sound::Sample(data_path("win.wav")); });

Load<Sound::Sample> lose_sample(LoadTagDefault, []() -> Sound::Sample const *
								{ return new Sound::Sample(data_path("lose.wav")); });

Load<Sound::Sample> almost_sample(LoadTagDefault, []() -> Sound::Sample const *
								  { return new Sound::Sample(data_path("almost.wav")); });

Load<Sound::Sample> move_sample(LoadTagDefault, []() -> Sound::Sample const *
								{ return new Sound::Sample(data_path("move.wav")); });

PlayMode::PlayMode() : scene(*campfire_scene)
{
	for (auto &transform : scene.transforms)
	{
		if (transform.name == "skewer_root")
			skewer_root = &transform;
		else if (transform.name == "marshmallow_root")
			marshmallow_root = &transform;
		else if (transform.name == "marshmallow_golden_root")
			marshmallow_golden_root = &transform;
		else if (transform.name == "marshmallow_almost_root")
			marshmallow_almost_root = &transform;
		else if (transform.name == "marshmallow_burnt_root")
			marshmallow_burnt_root = &transform;
		else if (transform.name == "fire_root")
			fire_root = &transform;
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> goal_seconds_dist(7.0f, 15.0f);
	goal_touched_seconds = goal_seconds_dist(gen);

	std::uniform_real_distribution<float> fire_dist(-20.0f, 20.0f);
	fire_root->position.x = fire_dist(gen);
	fire_root->position.y = fire_dist(gen);

	if (skewer_root == nullptr)
		throw std::runtime_error("skewer_root not found.");
	if (marshmallow_root == nullptr)
		throw std::runtime_error("marshmallow_root not found.");
	if (marshmallow_golden_root == nullptr)
		throw std::runtime_error("marshmallow_golden_root not found.");
	if (marshmallow_burnt_root == nullptr)
		throw std::runtime_error("marshmallow_burnt_root not found.");
	if (marshmallow_almost_root == nullptr)
		throw std::runtime_error("marshmallow_almost_root not found.");
	if (fire_root == nullptr)
		throw std::runtime_error("fire_root not found.");

	// get pointer to camera for convenience:
	if (scene.cameras.size() != 1)
		throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	// start music loop playing:
	//  (note: position will be over-ridden in update())
	background_loop = Sound::loop_3D(*background_sample, 0.5f, skewer_root->position, 10.0f);
	fire_loop = Sound::loop_3D(*fire_crackle_sample, 3.0f, fire_root->position, 1.0f);
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
		if (evt.key.key == SDLK_SPACE)
		{
			Sound::stop_all_samples();
			Mode::set_current(std::make_shared<PlayMode>());
			return true;
		}
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
	static std::shared_ptr<Sound::PlayingSample> sizzle_sound = nullptr;
	bool is_touching_fire = false;
	if (fire_visible)
	{
		float dist = glm::distance(marshmallow_root->position, fire_root->position);
		float dist2 = glm::distance(marshmallow_golden_root->position, fire_root->position);
		float dist3 = glm::distance(marshmallow_burnt_root->position, fire_root->position);
		float dist4 = glm::distance(marshmallow_almost_root->position, fire_root->position);
		if (dist < 5.0f || dist2 < 5.0f || dist3 < 5.0f || dist4 < 5.0f)
		{

			touching_seconds += elapsed;
			is_touching_fire = true;
		}
		else
		{
			is_touching_fire = false;
		}
	}
	else
	{
		is_touching_fire = false;
	}

	// Sizzle sound logic
	if (is_touching_fire)
	{
		if (sizzle_sound == nullptr || sizzle_sound->stopped)
		{
			sizzle_sound = Sound::play_3D(*sizzle_sample, 0.2f, marshmallow_root->position);
		}
	}
	else
	{
		if (sizzle_sound != nullptr && !sizzle_sound->stopped)
		{
			sizzle_sound->stop();
			sizzle_sound = nullptr;
		}
	}

	// Show and hide fire randomly
	fire_timer += elapsed;
	if (fire_visible)
	{
		if (fire_timer >= 7.0f)
		{
			fire_visible = false;
			fire_timer = 0.0f;
			fire_root->position = glm::vec3(1000.0f, 1000.0f, fire_root->position.z);
		}
	}
	else
	{
		if (fire_timer >= 1.0f)
		{
			fire_visible = true;
			fire_timer = 0.0f;
			static std::random_device rd;
			static std::mt19937 gen(rd());
			static std::uniform_real_distribution<float> fire_dist(-20.0f, 20.0f);
			fire_root->position.x = fire_dist(gen);
			fire_root->position.y = fire_dist(gen);

			// only play louder if we're still trying to get more fire time
			if (touching_seconds < goal_touched_seconds)
			{
				Sound::play_3D(*move_sample, 0.5f, fire_root->position);
			}
			else
			{
				Sound::play_3D(*move_sample, 0.1f, fire_root->position);
			}
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
	glm::vec3 follow_pos = skewer_root->position + skewer_root->rotation * glm::vec3(0, 0, 2.0f);
	glm::quat follow_rot = skewer_root->rotation;
	static bool played_win = false;
	static bool played_lose = false;
	static bool played_almost = false;

	if (touching_seconds < goal_touched_seconds - 2.0f)
	{
		// Show normal marshmallow
		marshmallow_root->position = follow_pos;
		marshmallow_root->rotation = follow_rot;
		marshmallow_almost_root->position = glm::vec3(1000.0f);
		marshmallow_golden_root->position = glm::vec3(1000.0f);
		marshmallow_burnt_root->position = glm::vec3(1000.0f);
		played_win = false;
		played_lose = false;
		played_almost = false;
	}
	else if (touching_seconds < goal_touched_seconds)
	{
		// Show almost marshmallow
		marshmallow_almost_root->position = follow_pos;
		marshmallow_almost_root->rotation = follow_rot;
		marshmallow_root->position = glm::vec3(1000.0f);
		marshmallow_golden_root->position = glm::vec3(1000.0f);
		marshmallow_burnt_root->position = glm::vec3(1000.0f);
		played_win = false;
		played_lose = false;
		if (!played_almost)
		{
			Sound::play_3D(*almost_sample, 0.8f, follow_pos);
			played_almost = true;
		}
	}
	else if (touching_seconds < goal_touched_seconds + 1.0f)
	{
		// Show golden marshmallow
		marshmallow_golden_root->position = follow_pos;
		marshmallow_golden_root->rotation = follow_rot;
		marshmallow_root->position = glm::vec3(1000.0f);
		marshmallow_almost_root->position = glm::vec3(1000.0f);
		marshmallow_burnt_root->position = glm::vec3(1000.0f);
		if (!played_win)
		{
			Sound::play_3D(*win_sample, 0.8f, follow_pos);
			played_win = true;
		}
		played_lose = false;
		played_almost = false;
	}
	else
	{
		// Show burnt marshmallow
		marshmallow_burnt_root->position = follow_pos;
		marshmallow_burnt_root->rotation = follow_rot;
		marshmallow_root->position = glm::vec3(1000.0f);
		marshmallow_golden_root->position = glm::vec3(1000.0f);
		marshmallow_almost_root->position = glm::vec3(1000.0f);
		if (!played_lose)
		{
			Sound::play_3D(*lose_sample, 0.8f, follow_pos);
			played_lose = true;
		}
		played_almost = false;
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

	glClearColor(0.02f, 0.02f, 0.08f, 1.0f);
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
		char buf[64];
		if (touching_seconds < goal_touched_seconds - 2.0f)
		{
			snprintf(buf, sizeof(buf), "Toasted: %.1fs", touching_seconds);
		}
		else if (touching_seconds < goal_touched_seconds)
		{
			snprintf(buf, sizeof(buf), "Toasted: %.1fs - getting toasty..", touching_seconds);
		}
		else if (touching_seconds < goal_touched_seconds + 1.0f)
		{
			snprintf(buf, sizeof(buf), "Toasted: %.1fs - perfection!! :)", touching_seconds);
		}
		else
		{
			snprintf(buf, sizeof(buf), "Toasted: %.1fs -  noOO it's burnt :(", touching_seconds);
		}
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
