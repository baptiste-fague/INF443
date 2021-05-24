#pragma once
#include "Triangle.hpp"
#include "Gridcell.hpp"
#include <stdlib.h>
#include <vector>
#include "vcl/vcl.hpp"


struct perlin_noise_parameters
{
	float persistency = 0.35f;
	float frequency_gain = 2.0f;
	int octave = 6;
	float terrain_height = 0.5f;
};


void update_tree_position(std::vector<vcl::vec3>& tree_positions, vcl::mesh& terrain, int count, bool minDist = true);
bool pointFarEnoughFrom(vcl::vec2 point, std::vector<vcl::vec3>& positions, float distance);
std::vector<Triangle> polygonize(Gridcell grid, float isolevel);
vcl::vec3 VertexInterp(vcl::vec3 p1, vcl::vec3  p2, float valp1, float valp2, float isolevel);
std::vector<vcl::mesh_drawable> create_terrain(int terrain_size_, float isolevel_, float scale_);
void create_terrain_thread(int thread_number);