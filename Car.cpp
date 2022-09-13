//
// Created by William Ozeas on 9/10/22.
//

#include <iostream>
#include "Car.hpp"

Car::Car(Drawable *_drawable, glm::vec2 _bb_size, uint32_t _index) : drawable(_drawable), index(_index), original_pos(_drawable->transform->position) {
    assert(drawable->transform);
    bb_size = _bb_size;
}

Car::Car(Drawable *_drawable, uint32_t _index) : drawable(_drawable), index(_index), original_pos(_drawable->transform->position) {
    Car(_drawable, bb_size, index);
}

Car::Car(const Car &car) : drawable(car.drawable), index(car.index), original_pos(car.drawable->transform->position) {
    Car(car.drawable, car.index);
}

Car::~Car() {
    //delete object
}

void Car::update(float elapsed) {
    drawable->transform->position += drawable->transform->make_local_to_world() * glm::vec4(velocity, 0.0f) * elapsed;
}

bool Car::check_car_collision(Car &other) {
    //rotation code would have to be refactored if we had hierarchy, but we don't! so!
    Transform *other_transform = other.drawable->transform;
    glm::vec3 obb_corner_one = other_transform->position + glm::inverse(other_transform->rotation) * glm::vec3(other.bb_size, 0.0f); //position
    glm::vec3 obb_corner_two  = other_transform->position - glm::inverse(other_transform->rotation) * glm::vec3(other.bb_size, 0.0f); //position

    glm::vec2 obb_corner_greater(std::max(obb_corner_one.x, obb_corner_two.x), std::max(obb_corner_one.y, obb_corner_two.y));
    glm::vec2 obb_corner_lesser(std::min(obb_corner_one.x, obb_corner_two.x), std::min(obb_corner_one.y, obb_corner_two.y));

    Transform *transform = drawable->transform;
    glm::vec3 bb_corner_one = transform->position + glm::inverse(transform->rotation) * glm::vec3(bb_size, 0.0f); //position
    glm::vec3 bb_corner_two  = transform->position - glm::inverse(transform->rotation) * glm::vec3(bb_size, 0.0f); //position

    glm::vec2 bb_corner_greater(std::max(bb_corner_one.x, bb_corner_two.x), std::max(bb_corner_one.y, bb_corner_two.y));
    glm::vec2 bb_corner_lesser(std::min(bb_corner_one.x, bb_corner_two.x), std::min(bb_corner_one.y, bb_corner_two.y));

    bool x_coll = false;
    bool y_coll = false;
    if((obb_corner_lesser.x <= bb_corner_lesser.x && obb_corner_greater.x >= bb_corner_lesser.x)
     || (obb_corner_lesser.x >= bb_corner_lesser.x && obb_corner_lesser.x <= bb_corner_greater.x)) {
        x_coll = true;
    }
    if((obb_corner_lesser.y <= bb_corner_lesser.y && obb_corner_greater.y >= bb_corner_lesser.y)
       || (obb_corner_lesser.y >= bb_corner_lesser.y && obb_corner_lesser.y <= bb_corner_greater.y)) {
        y_coll = true;
    }

    if(x_coll && y_coll) {
        std::cout << transform->name << " collided with " << other_transform->name << std::endl;
    }

    return x_coll && y_coll;
}

void Car::jump_back() {
    if(jump_count < MAX_JUMP_COUNT) {
        drawable->transform->position -= drawable->transform->make_local_to_world() * glm::vec4(jump_vec, 0);
        jump_count++;
    }
}

void Car::reset() {
    drawable->transform->position = original_pos;
    jump_count = 0;
}

