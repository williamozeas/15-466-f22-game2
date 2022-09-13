#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "Car.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>


GLuint meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("car.pnct"));
    meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > main_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("car.scene"), [&](Scene &scene, Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = meshes->lookup(mesh_name);


		scene.drawables.emplace_back(transform);
		Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;


	});
});

PlayMode::PlayMode() : scene(*main_scene) {
    for(auto &drawable : scene.drawables) {
        if(drawable.transform->name.substr(0,3) == "car") {
            if(drawable.transform->name.substr(4,6) == "player") {
                uint32_t num = (uint32_t)std::stoi(drawable.transform->name.substr(11,drawable.transform->name.size() - 11));
                cars.emplace_back(&drawable, num);
                num_player_cars++;
            } else {
                cars.emplace_back(&drawable);
            }
        }
    }

    //sort for ease of use with player cars
    std::sort(cars.begin(), cars.end(), [&](Car &a, Car &b) {
        return a.index < b.index;
    });

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
    auto numKeys = { SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9 };
    if(currentState == WinState) {
        return false;
    }
	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else {
            for(uint32_t i = 0; i < numKeys.size() && i < num_player_cars; i++) {
                if(evt.key.keysym.sym == *(numKeys.begin() + i)) {
                    cars.at(i).jump_back();
                }
            }
        }
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

bool check_car_collisions(std::vector<Car> cars) {
    for(uint32_t i = 0; i < cars.size(); i++) {
        for(uint32_t j = i + 1; j < cars.size(); j++) {
            if(cars.at(i).check_car_collision(cars.at(j))) {
                return true;
            }
        }
    }

    return false;
}

void PlayMode::update(float elapsed) {
    total_time += elapsed;
    if(total_time >= TIME_TO_WIN) {
        currentState = WinState;
        return;
    }

    //update car pos
    for(Car &car : cars) {
        car.update(elapsed);
    }

    //check car collision
    if(check_car_collisions(cars)) {
        reset();
    }

	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

//		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 right(1/sqrt(2), 1/sqrt(2), 0.0f);
		glm::vec3 up(-1/sqrt(2), 1/sqrt(2), 0.0f);
//		glm::vec3 forward = -frame[2];

		camera->transform->position += move.x * right + move.y * up;
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

    if(currentState == WinState) {
        //win
        glDisable(GL_DEPTH_TEST);
        float aspect = float(drawable_size.x) / float(drawable_size.y);
        DrawLines lines(glm::mat4(
                1.0f / aspect, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        ));

        constexpr float H = 0.9f;
        lines.draw_text("You win!",
                        glm::vec3(-aspect + 0.1f * H, -1.0f + 0.1f * H, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
    } else { //use DrawLines to overlay some text:
        glDisable(GL_DEPTH_TEST);
        float aspect = float(drawable_size.x) / float(drawable_size.y);
        DrawLines lines(glm::mat4(
                1.0f / aspect, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
        ));

        constexpr float H = 0.09f;
        lines.draw_text("WASD moves; number keys move cars",
                        glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0x00, 0x00, 0x00, 0x00));
        float ofs = 2.0f / drawable_size.y;
        lines.draw_text("WASD moves; number keys move cars",
                        glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + + 0.1f * H + ofs, 0.0),
                        glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
                        glm::u8vec4(0xff, 0xff, 0xff, 0x00));
    }
}

void PlayMode::reset() {
    total_time = 0;
    for(Car &car : cars) {
        car.reset();
    }
}
