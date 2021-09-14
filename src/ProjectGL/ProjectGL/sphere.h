// A class to generate unit circle data array
// reference: http://www.songho.ca/opengl/gl_sphere.html

#ifndef SPHERE_H
#define SPHERE_H

#include "global.h"
#include <vector>
using namespace std;


class Sphere
{
public:
	vector<float> vertices;
	vector<int> indices;

	void buildSphere(int num_stacks=NUM_STACKS, int num_sectors=NUM_SECTORS)
	{
		float sectorStep = 2 * PI / NUM_SECTORS;
		float stackStep = PI / NUM_STACKS;

		for (int i = 0; i < num_stacks; i++) {
			float stackAngle = PI / 2 - i * stackStep;
			float xy = cosf(stackAngle);
			float z = sinf(stackAngle);
			for (int j = 0; j < num_sectors; j++) {
				float sectorAngle = j * sectorStep;
				float x = xy * cosf(sectorAngle);
				float y = xy * sinf(sectorAngle);
				// vertex
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
				// normal
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}
	}

	void generateIndices()
	{
		for (int i = 0; i < NUM_STACKS; i++) {
			int k1 = i * (NUM_SECTORS + 1);
			int k2 = k1 + NUM_SECTORS + 1;
			for (int j = 0; j < NUM_SECTORS; j++, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k2);
					indices.push_back(k1 + 1);
				}
				if (i != NUM_STACKS - 1)
				{
					indices.push_back(k1 + 1);
					indices.push_back(k2);
					indices.push_back(k2 + 1);
				}
			}
		}
	}
};
#endif 

