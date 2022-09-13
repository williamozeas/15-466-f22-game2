//
// Created by William Ozeas on 9/10/22.
//

#pragma once

//#include "Entities.hpp"
#include "Scene.hpp"
#include <glm/glm.hpp>

struct Drawable;

struct Car {
    Car(Drawable *_drawable, uint32_t _index = UINT32_MAX);
    Car(Drawable *_drawable, glm::vec2 _bb_size, uint32_t _index = UINT32_MAX);
    Car(const Car &car);

    ~Car();

    Drawable *drawable;

    uint16_t MAX_JUMP_COUNT = 3; //const was causing issues with sort/copy/move

    //max int means red car
    uint32_t index;
    uint16_t jump_count = 0;

    glm::vec3 velocity = glm::vec3(2.0f, 0, 0);
    glm::vec2 bb_size = glm::vec2(4.5f / 2, 2.0f / 2);
    glm::vec3 jump_vec = glm::vec3(7.5f, 0.0f, 0.0f);
    glm::vec3 original_pos = glm::vec3(0.0f, 0.0f, 0.0f);

    void update(float elapsed);
    bool check_car_collision(Car &other);
    void jump_back();
    void reset();

};

