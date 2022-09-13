#pragma once
#include "Mode.hpp"

#include "Scene.hpp"

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
    virtual void reset();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

    enum GameState {
        PlayState,
        WinState
    };

    GameState currentState = PlayState;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

    const float TIME_TO_WIN = 30.0f;
	float total_time = 0;

    std::vector< Car > cars;
    uint32_t num_player_cars = 0;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
