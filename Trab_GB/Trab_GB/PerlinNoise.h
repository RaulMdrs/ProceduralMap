#pragma once
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
class PerlinNoise
{
public:

	PerlinNoise();
	~PerlinNoise();

	float interpolate(float a0, float a1, float w);
	glm::vec2 randomGradient(int ix, int iy);
	float dotGridGradient(int ix, int iy, float x, float y);
	float perlin(float x, float y);

private:
	std::vector<int> permutation;
};

