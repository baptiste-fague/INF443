#pragma once
#include "Position.h"

class Gridcell
{
public:
	vcl::vec3 p[8];
	double values[8];
	Gridcell() {}

	Gridcell(const vcl::vec3 pos[8], float val[8]) {
		for (int i = 0; i < 8; i++) {
			p[i] = pos[i];
			values[i] = val[i];
		}
}
};

