// a class to recursively find colliding pairs with octo-tree
// reference: https://github.com/YDCarry/OctreeCollisionDetection/blob/master/CodeForOctreeCollisionDetection/octree.cpp

#ifndef OCTREE_H
#define OCTREE_H


#include "global.h"
#include <vector>
#include <set>
#include <glm/glm.hpp>


using namespace std;
using namespace glm;


struct Ball {
	vec3 pos;
	vec3 velocity;
	vec3 color;
	float radius;
	float mass;
	float cor;
	int index;
};

enum Plane {
	LEFT=0, RIGHT, BACK, FRONT, TOP, BOTTOM
};

enum Coordinate {
	X, Y, Z
};

struct BallPair {
	int b1;
	int b2;
};

struct BallPlanePair {
	int b;
	int p;
};


class Octree {
private:
	vec3 minPos; // bottom left back corner
	vec3 maxPos; // top right front corner
	vec3 center; // center of the cubical room
	int numBalls;
	int depth;
	bool leaf;
	Octree* children[2][2][2];
	set<Ball*> balls;

	void clearChildren() {
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					children[i][j][k] = nullptr;
				}
			}
		}
	}

	// take all the balls of its children
	// and put them in a separate set of balls
	// used when deleting or inserting balls
	void collectBalls(set<Ball*>& result) {
		if (!leaf) {
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					for (int k = 0; k < 2; k++) {
						children[i][j][k]->collectBalls(result);
					}
				}
			}
		}
		else {
			for (Ball* b : balls) {
				result.insert(b);
			}
		}
	}

	void deleteChildren() {
		collectBalls(balls);
		clearChildren();
		leaf = true;
	}

	// recursive insert a ball into the correct partition of octree
	// or remove a ball from the correct partition
	void recursiveTravel(Ball* ball, vec3 pos, bool insert) {
		for (int i = 0; i < 2; i++) {
			if (i == 0 && pos.x > center.x + ball->radius) {
				continue;
			}
			if (i == 1 && pos.x < center.x - ball->radius) {
				continue;
			}

			for (int j = 0; j < 2; j++) {
				if (j == 0 && pos.y > center.y + ball->radius) {
					continue;
				}
				if (j == 1 && pos.y < center.y - ball->radius) {
					continue;
				}
				
				for (int k = 0; k < 2; k++) {
					if (k == 0 && pos.z > center.z + ball->radius) {
						continue;
					}
					if (k == 1 && pos.z < center.z - ball->radius) {
						continue;
					}
					if (insert) {
						children[i][j][k]->insert(ball);
					}
					else {
						children[i][j][k]->remove(ball, pos);
					}
				}
			}
		}
	}

	void recursiveInsert(Ball* ball, vec3 pos) {
		recursiveTravel(ball, pos, true);
	}

	void recursiveRemove(Ball* ball, vec3 pos) {
		recursiveTravel(ball, pos, false);
	}

	void ballPlaneCollide(vector<BallPlanePair>& result, Plane p, Coordinate coord, int i) {
		if (!leaf) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					Octree* child = nullptr;
					switch (coord) {
					case X:
						child = children[i][j][k];
						break;
					case Y:
						child = children[j][i][k];
						break;
					case Z:
						child = children[j][k][i];
						break;
					default:
						break;
					}
					child->ballPlaneCollide(result, p, coord, i);
				}
			}
		}
		else {
			for (auto b : balls) {
				BallPlanePair bpp;
				bpp.b = b->index;
				bpp.p = static_cast<int>(p);
				result.push_back(bpp);
			}
		}
	}

public:
	Octree(vec3 minPos=MIN_POS, vec3 maxPos=MAX_POS, int depth=0):
		minPos(minPos), maxPos(maxPos), depth(depth), numBalls(0), leaf(true)
	{
		clearChildren();
	}

	~Octree() {
		if (!leaf) {
			deleteChildren();
		}
	}

	// create subspace and children nodes recursively
	// and sort the ball pointers into leaf nodes
	void createChildren() {
		for (int i = 0; i < 2; i++) {
			float minX = (i == 0 ? minPos.x : center.x);
			float maxX = (i == 0 ? center.x : maxPos.x);
			for (int j = 0; j < 2; j++) {
				float minY = (j == 0 ? minPos.y : center.y);
				float maxY = (j == 0 ? center.y : maxPos.y);
				for (int k = 0; k < 2; k++) {
					float minZ = (k == 0 ? minPos.z : center.z);
					float maxZ = (k == 0 ? center.z : maxPos.z);
					children[i][j][k] = new Octree(
						vec3(minX, minY, minZ),
						vec3(maxX, maxY, maxZ),
						depth + 1
					);
				}
			}
		}
		for (Ball* b : balls) {
			recursiveInsert(b, b->pos);
		}
		balls.clear();
		leaf = false;
	}

	void insert(Ball* ball) {
		numBalls++;
		if (leaf && depth < MAX_DEPTH && numBalls > MAX_BALLS_PER_OCTREE) {
			createChildren();
		}
		if (!leaf) {
			recursiveInsert(ball, ball->pos);
		}
		else {
			balls.insert(ball);
		}
	}

	void remove(Ball* ball, vec3 pos) {
		numBalls--;
		if (!leaf) {
			if (numBalls < MIN_BALLS_PER_OCTREE) {
				deleteChildren();
			}
			else {
				recursiveRemove(ball, pos);
			}
		}
		else {
			balls.erase(ball);
		}
	}

	// update the position of a ball
	void update(Ball* ball, vec3 oldPos) {
		remove(ball, oldPos);
		insert(ball);
	}

	// recursively search every possible pair of colliding objects
	void candidateBallPlaneCollision(vector<BallPlanePair>& result) {
		ballPlaneCollide(result, LEFT, X, 0);
		ballPlaneCollide(result, RIGHT, X, 1);
		ballPlaneCollide(result, BOTTOM, Y, 0);
		ballPlaneCollide(result, TOP, Y, 1);
		ballPlaneCollide(result, BACK, Z, 0);
		ballPlaneCollide(result, FRONT, Z, 1);
	}

	void candidateBallCollision(vector<BallPair>& result) {
		if (!leaf) {
			for (int i = 0; i < 2; i++) {
				for (int j = 0; j < 2; j++) {
					for (int k = 0; k < 2; k++) {
						children[i][j][k]->candidateBallCollision(result);
					}
				}
			}
		}
		else {
			for (auto iter1 = balls.begin(); iter1 != balls.end(); iter1++) {
				for (auto iter2 = balls.begin(); iter2 != balls.end(); iter2++) {
					if (*iter1 < *iter2) {
						BallPair bp;
						bp.b1 = (*iter1)->index;
						bp.b2 = (*iter2)->index;
						result.push_back(bp);
					}
				}
			}
		}
	}
};

#endif