//
// Created by William Ozeas on 9/10/22.
//

#pragma once

//#include "Entities.hpp"
#include "Scene.hpp"
#include <glm/glm.hpp>

struct Drawable;

struct Car {
    Car(Drawable *_drawable);
    Car(Drawable *_drawable, glm::vec2 _bb_size);

    Drawable *drawable;

    glm::vec3 velocity = glm::vec3(1.0f, 0, 0);
    glm::vec2 bb_size = glm::vec2(2.0f / 2, 4.5f / 2);
    glm::vec3 jump_vec = glm::vec3(0.0f, 5.0f, 0.0f);
    glm::vec3 original_pos;

    void update(float elapsed);
    bool check_car_collision(Car &other);
    void jump_back();
    void reset();

};

