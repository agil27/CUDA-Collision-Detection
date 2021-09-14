// cuda functions to execute collision detection
// and synchrozie data between the detector class 
// and the gpu hosted memory
// reference: https://zhuanlan.zhihu.com/p/34587739

#include "collide.h"
#include <iostream>
#include <cstdio>
#include <glm/glm.hpp>
using namespace std;
using namespace glm;

// define host data
glm::vec3 pos[MAX_BALL_COUNT], velocity[MAX_BALL_COUNT];
float mass[MAX_BALL_COUNT], radius[MAX_BALL_COUNT], cor[MAX_BALL_COUNT];
int b1[MAX_COLLISIONS], b2[MAX_COLLISIONS];
int b[MAX_COLLISIONS], p[MAX_COLLISIONS];

// define device data
__device__ glm::vec3 _pos[MAX_BALL_COUNT], _velocity[MAX_BALL_COUNT];
__device__ float _mass[MAX_BALL_COUNT], _radius[MAX_BALL_COUNT], _cor[MAX_BALL_COUNT];
__device__ int _b1[MAX_COLLISIONS], _b2[MAX_COLLISIONS];
__device__ int _b[MAX_COLLISIONS], _p[MAX_COLLISIONS];

// sychronize data between device and host 
void reverseSyncVelocity(int n) {
	cudaMemcpyFromSymbol(velocity, _velocity, n * sizeof(vec3));
}

__device__ void printv(vec3 val) {
	printf("%f %f %f\n", val.x, val.y, val.z);
}

__device__ void printfl(float val) {
	printf("%f\n", val);
}

void syncVars(int n) {
	cudaMemcpyToSymbol(_pos, pos, n * sizeof(vec3), 0);
	cudaMemcpyToSymbol(_velocity, velocity, n * sizeof(vec3), 0);
}

void syncConsts(int n) {
	cudaMemcpyToSymbol(_mass, mass, n * sizeof(float), 0);
	cudaMemcpyToSymbol(_radius, radius, n * sizeof(float), 0);
	cudaMemcpyToSymbol(_cor, cor, n * sizeof(float), 0);
}

void syncBallPairs(int n) {
	cudaMemcpyToSymbol(_b1, b1, n * sizeof(int), 0);
	cudaMemcpyToSymbol(_b2, b2, n * sizeof(int), 0);
}

void syncBallPlanePairs(int n) {
	cudaMemcpyToSymbol(_b, b, n * sizeof(int), 0);
	cudaMemcpyToSymbol(_p, p, n * sizeof(int), 0);
}

// synchronization between cuda and detector class
void initBallCuda(vector<Ball*> balls, int n) {
    for (int i = 0; i < n; i++) {
        pos[i] = balls[i]->pos;
        velocity[i] = balls[i]->velocity;
        mass[i] = balls[i]->mass;
        cor[i] = balls[i]->cor;
        radius[i] = balls[i]->radius;
    }
	syncConsts(n);
	syncVars(n);
}

void copyBallVarCuda(vector<Ball*> balls, int n) {
    for (int i = 0; i < n; i++) {
        pos[i] = balls[i]->pos;
        velocity[i] = balls[i]->velocity;
    }
	syncVars(n);
	return;
}

void updateVelocityCuda(vector<Ball*> balls, int n) {
	reverseSyncVelocity(n);
    for (int i = 0; i < n; i++) {
		balls[i]->velocity = velocity[i];
    }
}

void copyBallPairCuda(vector<BallPair> pairs, int numPairs) {
    for (int i = 0; i < numPairs && i < MAX_COLLISIONS; i++) {
        b1[i] = pairs[i].b1;
        b2[i] = pairs[i].b2;
    }
	syncBallPairs(numPairs);
}

void copyBallPlanePairCuda(vector<BallPlanePair> pairs, int numPairs) {
    for (int i = 0; i < numPairs && i < MAX_COLLISIONS; i++) {
        b[i] = pairs[i].b;
        p[i] = pairs[i].p;
    }
	syncBallPlanePairs(numPairs);
}

// utility function to compute the plane normal vector
__device__
vec3 _planeDir(int p) {
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

// kernel functions
__global__
void ballCollideKernel(int numPairs) {
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < numPairs; i += stride) {
        int index1 = _b1[i];
        int index2 = _b2[i];
		vec3 pos1 = _pos[index1];
		vec3 pos2 = _pos[index2];
        float r = _radius[index1] + _radius[index2];
		vec3 dp = pos1 - pos2;
		vec3 v1 = _velocity[index1];
		vec3 v2 = _velocity[index2];
		vec3 dv = v1 - v2;
        if (dot(dp, dp) < r * r && dot(dv, dp) <= 0) {
			// balls are close enough and are approaching
			// so the collision will happen
			float cor1 = _cor[index1];
			float cor2 = _cor[index2];
			float c = min(cor1, cor2);
			float m1 = _mass[index1];
			float m2 = _mass[index2];

			// use momentum & energy preservation theorem
			// to solve the velocities
			vec3 dpvec = normalize(dp);
			vec3 proj1 = dot(v1, dpvec) * dpvec;
			vec3 proj2 = dot(v2, dpvec) * dpvec;
			vec3 dv1 = ((1 + c) * m2 * (proj2 - proj1)) / (m1 + m2);
			vec3 dv2 = ((1 + c) * m1 * (proj1 - proj2)) / (m1 + m2);
			_velocity[index1] += dv1;
			_velocity[index2] += dv2;
        }
    }
}

__global__
void ballPlaneCollideKernel(int numPairs) {
    int index = threadIdx.x + blockIdx.x * blockDim.x;
    int stride = blockDim.x * gridDim.x;
    for (int i = index; i < numPairs; i += stride) {
        int ballIndex = _b[i];
        int planeIndex = _p[i];
		vec3 dir = _planeDir(planeIndex);
		vec3 p = _pos[ballIndex];
		vec3 v = _velocity[ballIndex];
        float r = _radius[ballIndex];
        if (dot(p, dir) + r > SIZE / 2 && dot(v, dir) > 0) {
			// the ball and the plane are close enough and the ball is approaching
			// so the collision will happen
            float c = _cor[ballIndex];

			// assume the plane is of infinity mass
			vec3 dv = (1 + c) * dir * dot(v, dir);
			_velocity[ballIndex] -= dv;
        }
    }
}

// interfaces to the detector
void ballCollideCuda(vector<BallPair>& pairs, vector<Ball*> balls) {
	int numBalls = balls.size();
	int numPairs = pairs.size();
	copyBallPairCuda(pairs, numPairs);

	dim3 blockSize(64);
	dim3 gridSize((numBalls + blockSize.x - 1) / blockSize.x);

	// call kernel function
	ballCollideKernel <<<gridSize, blockSize>>> (numPairs);
}

void ballPlaneCollideCuda(vector<BallPlanePair>& pairs, vector<Ball*> balls) {
	int numBalls = balls.size();
	int numPairs = pairs.size();
	copyBallPlanePairCuda(pairs, numPairs);

	dim3 blockSize(64);
	dim3 gridSize((numBalls + blockSize.x - 1) / blockSize.x);

	// call kernel function
	ballPlaneCollideKernel <<<gridSize, blockSize>>> (numPairs);
	auto result = cudaGetLastError();
}

