#pragma once
#include "vcl/vcl.hpp"
#include <stdlib.h>
#include <vector>

class Boid
{
public:
	vcl::vec3 position;
	vcl::vec3 speed;
};
float vision_angle = -0.7;
float maxspeed = 6.0f;
float minspeed = 2.0f;
float maxforce = 2.5f;
float perception = 4.0f;
float avoid = 0.9f;

float dist(Boid a, Boid b) {
	return (float)sqrt((b.position[0] - a.position[0]) * (b.position[0] - a.position[0]) + (b.position[1] - a.position[1]) * (b.position[1] - a.position[1]) + (b.position[2] - a.position[2]) * (b.position[2] - a.position[2]));
}

float scalar(vcl::vec3 a, vcl::vec3 b) {
	return a[0]*b[0] + a[1] * b[1] + a[2] * b[2];
}

float norm(vcl::vec3 vecteur) {
	return (float)sqrt(vecteur[0] * vecteur[0] + vecteur[1] * vecteur[1] + vecteur[2] * vecteur[2]);
}

std::vector<Boid> create_flock(int const N) {
	std::vector<Boid> flock;
	for (int i = 0; i < N; i++) {
		Boid boid;
		boid.position = { (float)10 * rand() / RAND_MAX, (float)10 * rand() / RAND_MAX, (float)10 * rand() / RAND_MAX };
		boid.speed = { (float)5*rand() / RAND_MAX, (float)5*rand() / RAND_MAX, (float)5*rand() / RAND_MAX };
		flock.push_back(boid);
	}
	return flock;
}

vcl::vec3 alignment(Boid boid, std::vector<Boid> others) {
	vcl::vec3 steering = { 0, 0, 0 };
	int neighbours = 0;
	for (int i = 0; i < others.size(); i++) {
		if (dist(boid, others[i]) < perception && (float) scalar(boid.speed,boid.position-others[i].position)/(dist(boid, others[i])*norm(boid.speed))>vision_angle) {
			steering += others[i].speed;
			neighbours++;
		}
	}
	if (neighbours > 0) {
		return (steering / neighbours) - boid.speed;
	}
	return steering;
}

vcl::vec3 cohesion(Boid boid, std::vector<Boid> others) {
	vcl::vec3 steering = { 0, 0, 0 };
	int neighbours = 0;
	for (int i = 0; i < others.size(); i++) {
		if (dist(boid, others[i]) < perception && (float) scalar(boid.speed, boid.position - others[i].position) / (dist(boid, others[i]) * norm(boid.speed)) > vision_angle) {
			steering += others[i].position;
			neighbours++;
		}
	}
	if (neighbours > 0) {
		return steering / neighbours - boid.position;
	}
	return steering;
}

vcl::vec3 separation(Boid boid, std::vector<Boid> others) {
	vcl::vec3 steering = { 0,0,0 };
	int compteur = 0;
	for (int i = 0; i < others.size(); i++) {
		if (boid.position[0] != others[i].position[0] && boid.position[1] != others[i].position[1] && boid.position[2] != others[i].position[2] && dist(boid, others[i]) < avoid ) {
			steering -= (others[i].position - boid.position) / dist(boid, others[i]);
			compteur++;
		}
	}
	if (compteur > 0) {
		return steering / compteur;
	}
	return steering;
}
