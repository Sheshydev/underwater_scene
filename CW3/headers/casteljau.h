#pragma once
#include <list>
#include <vector>
#include <algorithm>

#include "point.h"

point evaluate(float t, std::list<point> P)
{
	std::list<point> Q = P;
	std::list<point>::iterator p1;
	std::list<point>::iterator p2;

	while (Q.size() > 1) {
		std::list<point> R;

		p1 = Q.begin();
		p2 = Q.begin();
		std::advance(p2, 1);

		for (int i = 1; i < Q.size(); i++) {
			point p = ((1 - t) * (*p1)) + (t * (*p2));
			R.push_back(p);
			std::advance(p1, 1);
			std::advance(p2, 1);
		}
		Q = R;
	}
	return *(Q.begin());
}

std::vector<point> EvaluateBezierCurve(std::vector<point>ctrl_points, int num_evaluations)
{
	std::list<point> ps(ctrl_points.begin(), ctrl_points.end());
	std::vector<point> curve;
	curve.push_back(*(ps.begin()));

	float offset = 1.f / num_evaluations;

	for (int i = 0; i < num_evaluations; i++) {
		point p = evaluate(offset * (i + 1), ps);
		curve.push_back(p);
	}

	return curve;
}

float* MakeFloatsFromVector(std::vector<point> curve, int& num_verts, int& num_floats, float r, float g, float b)
{
	num_verts = curve.size();
	if (num_verts == 0)
		return NULL;

	num_floats = num_verts * 6;
	float* vertices = (float*)malloc(sizeof(float) * num_floats);

	for (int i = 0; i < num_verts; i++) {
		vertices[i * 6] = curve[i].x;
		vertices[i * 6 + 1] = curve[i].y;
		vertices[i * 6 + 2] = curve[i].z;

		vertices[i * 6 + 3] = r;
		vertices[i * 6 + 4] = g;
		vertices[i * 6 + 5] = b;
	}
	return vertices;
}
