#include "Mode.hpp"

#include "Scene.hpp"
#include "Sound.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, r, f;

	float touching_seconds = 0.0f;
	float goal_touched_seconds = 0.0f;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *skewer_root = nullptr;
	Scene::Transform *marshmallow_root = nullptr;
	Scene::Transform *marshmallow_golden_root = nullptr;
	Scene::Transform *marshmallow_burnt_root = nullptr;
	Scene::Transform *marshmallow_almost_root = nullptr;
	Scene::Transform *fire_root = nullptr;

	float fire_timer = 0.0f;
	bool fire_visible = false;
	
	// curr marshmallow position
	glm::vec3 marshmallow_pos;
	glm::quat marshmallow_rot;
	glm::vec3 marshmallow_scale;

	//music coming from the tip of the leg (as a demonstration):
	std::shared_ptr< Sound::PlayingSample > fire_loop;
	std::shared_ptr< Sound::PlayingSample > background_loop;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
