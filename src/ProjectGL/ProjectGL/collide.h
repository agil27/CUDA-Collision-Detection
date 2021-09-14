#ifndef COLLIDE_H
#define COLLIDE_H

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "global.h"
#include "octree.h"
#include <cstdio>


void initBallCuda(vector<Ball*> balls, int numBalls);
void copyBallVarCuda(vector<Ball*> balls, int numBalls);
void updateVelocityCuda(vector<Ball*> balls, int numBalls);
void copyBallPairCuda(vector<BallPair> pairs, int numPairs);
void copyBallPlanePairCuda(vector<BallPlanePair> pairs, int numPairs);
void ballCollideCuda(vector<BallPair>& pairs, vector<Ball*> balls);
void ballPlaneCollideCuda(vector<BallPlanePair>& pairs, vector<Ball*> balls);

#endif