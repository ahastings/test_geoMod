#pragma once
#include "polyhedron.h"
#include <utility>
#include <list>
#include <vector>

class POLYLINE {
public:
	std::list<icVector3> m_vertices;
	icVector3 m_rgb = icVector3(1, 0, 0);
	double m_weight = 1;
	bool isNeighbor(const POLYLINE& line);
	void merge(const POLYLINE& line);
};

void display_polyline(std::vector<POLYLINE>& polylines);
void marchingSquare(
	std::list<POLYLINE>& edges,
	const Polyhedron& poly,
	const double& surface);
void makePolyLineFromEdges(
	std::vector<POLYLINE>& polylines,
	const std::list<POLYLINE>& edges);
Vertex lerpByScalar(const Vertex& v0, const Vertex& v1, const double& surface);
void lookUpTable(
	std::vector<Vertex>& r,
	const Vertex& v0,
	const Vertex& v1,
	const Vertex& v2,
	const Vertex& v3,
	const double& surface);