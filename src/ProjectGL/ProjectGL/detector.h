#ifndef DETECTOR_H
#define DETECTOR_H

#include <vector>
#include "octree.h"
#include "global.h"
#include "collide.h"

using namespace glm;

// return a random float normalized to (0, 1)
float randomFloat() {
	return (float)rand() / ((float)RAND_MAX + 1);
}

vec3 randomVec() {
	return vec3(randomFloat(), randomFloat(), randomFloat());
}

vec3 planeDir(int p) {
	switch (p) {
	case LEFT:
		return vec3(-1.0f, 0.0f, 0.0f);
		break;
	case RIGHT:
		return vec3(1.0f, 0.0f, 0.0f);
		break;
	case BACK:
		return vec3(0.0f, 0.0f, -1.0f);
		break;
	case FRONT:
		return vec3(0.0f, 0.0f, 1.0f);
		break;
	case TOP:
		return vec3(0.0f, 1.0f, 0.0f);
		break;
	case BOTTOM:
		return vec3(0.0f, -1.0f, 0.0f);
		break;
	default:
		return vec3(0.0f, 0.0f, 0.0f);
	}
}

class Detector {
private:
	vector<Ball*> balls;
	Octree* octree;

	void updateBallPos(float dt) {
		for (auto b : balls) {
			vec3 oldPos = b->pos;
			b->pos += b->velocity * dt;
			octree->update(b, oldPos);
		}
	}

	void accelerate() {
		for (auto b : balls) {
			b->velocity -= vec3(0.0f, G, 0.0f);
		}
	}

public:
	Detector() { octree = new Octree(); }
	~Detector() { delete octree; }

	void generateBalls(int numBalls) {
		for (int i = 0; i < numBalls; i++) {
			Ball* b = new Ball();
			int index1, index2, index3;
			index1 = i / (NUM_SQUARE);
			index2 = (i - index1 * NUM_SQUARE) / NUM_DIM;
			index3 = i - index1 * NUM_SQUARE - index2 * NUM_DIM;
			b->pos = vec3(-2.0f, -2.0f, -2.0f) + vec3(index1 * 2 * MAX_RADIUS + EPS, index2 * 2 * MAX_RADIUS + EPS, index3 * 2 * MAX_RADIUS + EPS);
			b->velocity = randomVec() * (MAX_SPEED - MIN_SPEED) + MIN_SPEED;
			b->radius = randomFloat() * (MAX_RADIUS - MIN_RADIUS) + MIN_RADIUS;
			b->mass = randomFloat() * (MAX_MASS - MIN_MASS) + MIN_MASS;
			b->cor = randomFloat() * (MAX_COR - MIN_COR) + MIN_COR;
			b->color = randomVec() * 0.6f + 0.2f;
			b->index = i;
			balls.push_back(b);
			octree->insert(b);
		}
		// cuda function to copy the ball information to cuda device
		initBallCuda(balls, numBalls);
	}

	void ballCollideCpu(vector<BallPair> pairs) {
		for (auto pair : pairs) {
			Ball* b1 = balls[pair.b1];
			Ball* b2 = balls[pair.b2];
			vec3 dp = b1->pos - b2->pos;
			float r = b1->radius + b2->radius;
			vec3 v1 = b1->velocity;
			vec3 v2 = b2->velocity;
			float m1 = b1->mass;
			float m2 = b2->mass;
			vec3 dv = v1 - v2;
			float c = min(b1->cor, b2->cor);
			if (dot(dp, dp) < r * r && dot(dv, dp) < EPS) {
				vec3 vec1 = dot(v1, dp) * dp;
				vec3 vec2 = dot(v2, dp) * dp;
				vec3 dv1 = ((1 + c) * m2 * (vec2 - vec1)) / (m1 + m2);
				vec3 dv2 = ((1 + c) * m1 * (vec1 - vec2)) / (m1 + m2);
				b1->velocity += dv1;
				b2->velocity += dv2;
			}
		}
	}

	void ballPlaneCollideCpu(vector<BallPlanePair> pairs) {
		for (auto pair : pairs) {
			vec3 dir = planeDir(pair.p);
			Ball* b = balls[pair.b];
			if (dot(b->pos, dir) + b->radius > SIZE / 2 && dot(b->velocity, dir) > 0) {
				dir = normalize(dir);
				b->velocity -= vec3(1 + b->cor) * dir * dot(b->velocity, dir);
			}
		}
	}
	
	void updateBallAttr() {
		accelerate();
		copyBallVarCuda(balls, balls.size());
		vector<BallPair> bps;
		octree->candidateBallCollision(bps);
		ballCollideCuda(bps, balls);
		vector<BallPlanePair> bpps;
		octree->candidateBallPlaneCollision(bpps);
		ballPlaneCollideCuda(bpps, balls);
		updateVelocityCuda(balls, balls.size());
	}

	void updateBallAttrCpu() {
		accelerate();
		vector<BallPair> bps;
		octree->candidateBallCollision(bps);
		ballCollideCpu(bps);
		vector<BallPlanePair> bpps;
		octree->candidateBallPlaneCollision(bpps);
		ballPlaneCollideCpu(bpps);
	}

	void update(float t, float& dt) {
		while (t > 0) {
			if (dt <= t) {
				updateBallPos(dt);
				updateBallAttr();
				t -= dt;
				dt = UPDATE_INTERVAL;
			}
			else {
				updateBallPos(t);
				dt -= t;
				t = 0;
			}
		}
	}
	
	vector<Ball*>& getBalls() {
		return balls;
	}
};
#endif