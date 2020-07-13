#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__
#include "cgmath.h"
#include <queue>
#include <mmsystem.h>
#include <Windows.h>

#pragma comment(lib, "winmm.lib")

//*******************************************************************
// common structures
struct camera
{
	vec3	eye = vec3(-100, -150, 100);
	vec3	at = vec3(0, 0, 0);
	vec3	up = vec3(0, 0, 1);
	mat4	view_matrix = mat4::look_at(eye, at, up);

	float	fovy = PI / 4.0f; // must be in radian
	float	aspect_ratio;
	float	dnear = 1.0f;
	float	dfar = 1000.0f;
	mat4	projection_matrix = mat4{ 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };
};

struct step_t {
	vec3 center = vec3(0.0f, 0.0f, 0.0f);
	int angle_status = 0;
	vec3 radius = vec3(10.0f, 20.0f, 2.0f);		// radius
	int box_status = -1;			// Default
	vec4 color=vec4(107/255.0f, 236/225.0f, 213/225.0f, 0.3f);					// RGBA color in [0,1]

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
};

struct cube_t {
	vec3 center = vec3(0.0f,0.0f,0.0f);
	vec3 radius = vec3(10.0f, 10.0f, 10.0f);		// radius
	float angle = 0.0f;							// For rotation
	float last_center_x = 0.0f;					// For rotation
	float last_angle = 0.0f;
	int before_index = 0;							// Main cube is placed on here
	int now_index = 1;
	int next_index = 2;
	int score = 0;
	bool music_on = false;

	int		flag = 0;
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 0.0f);				// RGBA color in [0,1]
	int		timer = 0;			// For bpm

	mat4	model_matrix;		// modeling transformation

	// public functions
	void	update(float t);
	float	roll(std::vector<step_t>* steps, std::queue<int>* map, bool* start);
};

inline cube_t create_cube() {
	cube_t main = { vec3(0.0f, 0.0f, 0.0f), };
	return main;
}

inline std::vector<step_t> create_steps() {
	std::vector<step_t> steps;

	step_t step8 = { vec3(-20.0f,0,-12.0f),};
	step_t step8_square = { vec3(-20.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};

	step_t step1 = { vec3(0,0,-12.0f)};
	step_t step1_square = { vec3(0,0,0), 0,vec3(0.0f, 0.0f, 0.0f),0};
	
	step_t step2 = { vec3(20.0f,0, -12.0f)};
	step_t step2_square = { vec3(20.0f,0, 0), 0,vec3(0.0f, 0.0f, 0.0f),0};

	step_t step3 = { vec3(40.0f,0,-12.0f)};
	step_t step3_square = { vec3(40.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};

	step_t step4 = { vec3(60.0f,0,-12.0f)};
	step_t step4_square = { vec3(60.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};

	step_t step5 = { vec3(80.0f,0,-12.0f)};
	step_t step5_square = { vec3(80.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};

	step_t step6 = { vec3(100.0f,0,-12.0f)};
	step_t step6_square = { vec3(100.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};

	step_t step7 = { vec3(120.0f,0,-12.0f)};
	step_t step7_square = { vec3(120.0f,0,0), 0, vec3(0.0f, 0.0f, 0.0f),0};
	
	steps.emplace_back(step8);                                                                                                                                                                                                  
	steps.emplace_back(step1);
	steps.emplace_back(step2);
	steps.emplace_back(step3);
	steps.emplace_back(step4);
	steps.emplace_back(step5);
	steps.emplace_back(step6);
	steps.emplace_back(step7);

	steps.emplace_back(step8_square);
	steps.emplace_back(step1_square);
	steps.emplace_back(step2_square);
	steps.emplace_back(step3_square);
	steps.emplace_back(step4_square);
	steps.emplace_back(step5_square);
	steps.emplace_back(step6_square);
	steps.emplace_back(step7_square);
	return steps;
}

inline void cube_t::update(float t)
{
	float c = cos(t), s = sin(t);

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius.x, 0, 0, 0,
		0, radius.y, 0, 0,
		0, 0, radius.z, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		cos(angle), 0, sin(angle), 0,
		0, 1, 0, 0,
		-sin(angle), 0, cos(angle), 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, 0,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

inline void step_t::update(float t)
{
	float angle = PI / 8 * angle_status;
	angle_status %= 8;
	if (angle_status==0||angle_status==8) {
		color = vec4(107 / 255.0f, 236 / 225.0f, 213 / 225.0f, 0.3f);
	}
	else if(angle_status<=3){
		color = vec4(128 / 255.0f, 0 / 225.0f, 0 / 225.0f, 0.7f);
	}
	else {
		color = vec4(30 / 255.0f, 100 / 225.0f, 0 / 225.0f, 0.7f);
	}


	if (box_status%10 == 0) {
		radius.x = 0.0f;
		radius.y = 0.0f;
		radius.z = 0.0f;
	}
	else if (box_status%10 == 1) {
		radius.x = 2.0f;
		radius.y = 2.0f;
		radius.z = 2.0f;
		color = vec4(107 / 255.0f, 236 / 225.0f, 219 / 225.0f, 0.9f);
	}
	else if (box_status%10 == 2) {
		radius.x = 5.0f;
		radius.y = 5.0f;
		radius.z = 5.0f;
		color = vec4(8 / 255.0f, 97 / 225.0f, 179 / 225.0f, 0.9f);
	}

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius.x, 0, 0, 0,
		0, radius.y, 0, 0,
		0, 0, radius.z, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, cos(angle), -sin(angle), 0,
		0, sin(angle), cos(angle), 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, 0,
		0, 0, 1, center.z,
		0, 0, 0, 1
	};

	model_matrix = translate_matrix * rotation_matrix * scale_matrix;
}

inline float cube_t::roll(std::vector<step_t>* steps, std::queue<int>* map, bool* start) {
	if (!music_on) {
		sndPlaySound(TEXT(".\\ForgiveMe.wav"), SND_ASYNC);
		music_on = true;
	}
	if (next_index==before_index) {
		*start = false;
		sndPlaySound(0, 0);
	}
	else {
		timer = (timer + 1) % 35;
		center.z = (float)(radius.x / 2 * (sqrt(2) * sin(PI / 4 * (1 + timer / 17.0f)) - 1));
		center.x = last_center_x + (float)(radius.x * (1 - sqrt(2) * cos(PI / 4 * (1 + timer / 17.0f))));
		angle = last_angle + timer * PI / 2 / 34;
	}

	if (timer == 0) {
		center.z -= 1.0f;
		steps->at(now_index).center.z -= 1.0f;
	}
	else if (timer == 1) {
		center.z -= 0.5f;
		steps->at(now_index).center.z -= 0.5f;
	}
	else if (timer == 3) {
		if (steps->at(now_index).angle_status > 0 || steps->at(now_index + 8).box_status > 0) {
			score -= 500;
		}
		else score += 200;
	}
	else if (timer == 6) {
		center.z += 1.5f;
		steps->at(now_index).center.z += 1.5f;
	}
	else if (timer == 34) {
		last_center_x = center.x;
		last_angle = angle;

		if (!map->empty()) {
			int tmp = map->front();
			map->pop();
	
			steps->at(before_index).center.x += 160;
			steps->at(before_index).center.z = -12.0f;
			steps->at(before_index).angle_status = tmp / 10;
			steps->at(before_index + 8).center.x += 160;
			steps->at(before_index + 8).box_status = tmp % 10;
			before_index = (before_index + 1) % 8;
		}
		now_index = (now_index + 1) % 8;
		next_index = (next_index + 1) % 8;
	}
	return center.x;
}
#endif
