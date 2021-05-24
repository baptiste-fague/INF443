#pragma once
#include "vcl/vcl.hpp" 

class Triangle
{
public :
	vcl::vec3 p[3];

	Triangle(vcl::vec3 p1, vcl::vec3 p2, vcl::vec3 p3) {
		p[0] = p1;
		p[1] = p2;
		p[2] = p3;
	}
};

