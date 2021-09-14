#ifndef GLOBAL_H
#define GLOBAL_H
#include <glm/glm.hpp>
#include <cmath>
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

const float EPS = 1e-3;

// scene
__constant__ const float SIZE = 12.0f;
const glm::vec3 MAX_POS = glm::vec3(SIZE / 2, SIZE / 2, SIZE / 2);
const glm::vec3 MIN_POS = glm::vec3(-SIZE / 2, -SIZE / 2, -SIZE / 2);

// constants w.r.t the balls
const int NUM_DIM = 4;
const int NUM_SQUARE = NUM_DIM * NUM_DIM;
const int NUM_BALLS = NUM_DIM * NUM_DIM * NUM_DIM;
const float MAX_RADIUS = 0.8f;
const float MIN_RADIUS = 0.6f;
const float MAX_MASS = 5.0f;
const float MIN_MASS = 1.0f;
const float MAX_COR = 1.0f;
const float MIN_COR = 0.5f;
const float MAX_SPEED = 1.0f;
const float MIN_SPEED = 0.3f;
__constant__ const int MAX_BALL_COUNT = (int)1e4;
const int MAX_COLLISIONS = (int)1e7;

// sphere modeling
const int NUM_STACKS = 40;
const int NUM_SECTORS = 40;
const float PI = acosf(-1);

// environment physics
const float G = 6.0f;

// square plane
const float PLANE_VERTICES[24] = {
	 0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f,
	 0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
	-0.5f, 0.0f,  0.5f, 0.0f, 1.0f, 0.0f
};

const int PLANE_INDICES[12] = {
	 0, 1, 3,
	 1, 2, 3
};

// octree
const int MAX_DEPTH = 6;
const int MIN_BALLS_PER_OCTREE = 3;
const int MAX_BALLS_PER_OCTREE = 6;

// update
const float UPDATE_INTERVAL = 0.01f;
const int UPDATE_TIMES = 25;
#endif