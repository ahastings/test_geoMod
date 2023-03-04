#include "myPolyLine.h"
#include "GL/glew.h"
#include <iostream>
#define EPSILON 1.0e-5

bool POLYLINE::isNeighbor(const POLYLINE& line) {
	if ((m_vertices.front() - line.m_vertices.front()).length() < EPSILON ||
		(m_vertices.front() - line.m_vertices.back()).length() < EPSILON ||
		(m_vertices.back() - line.m_vertices.front()).length() < EPSILON ||
		(m_vertices.back() - line.m_vertices.back()).length() < EPSILON) {
		return true;
	}
	return false;
}
void POLYLINE::merge(const POLYLINE& line) {
	if ((m_vertices.front() - line.m_vertices.front()).length() < EPSILON) {
		POLYLINE line_ = line;
		line_.m_vertices.pop_front();
		for (auto i = line_.m_vertices.begin(); i != line_.m_vertices.end(); i++) {
			m_vertices.push_front(*i);
		}
	}
	else if ((m_vertices.front() - line.m_vertices.back()).length() < EPSILON) {
		POLYLINE line_ = line;
		line_.m_vertices.pop_back();
		line_.m_vertices.reverse();
		for (auto i = line_.m_vertices.begin(); i != line_.m_vertices.end(); i++) {
			m_vertices.push_front(*i);
		}
	}
	else if ((m_vertices.back() - line.m_vertices.front()).length() < EPSILON) {
		POLYLINE line_ = line;
		line_.m_vertices.pop_front();
		for (auto i = line_.m_vertices.begin(); i != line_.m_vertices.end(); i++) {
			m_vertices.push_back(*i);
		}
	}
	else if ((m_vertices.back() - line.m_vertices.back()).length() < EPSILON) {
		POLYLINE line_ = line;
		line_.m_vertices.pop_back();
		line_.m_vertices.reverse();
		for (auto i = line_.m_vertices.begin(); i != line_.m_vertices.end(); i++) {
			m_vertices.push_back(*i);
		}
	}
}

void display_polyline(std::vector<POLYLINE>& polylines) {
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (auto& polyline : polylines) {
		glLineWidth(polyline.m_weight);
		glColor3f(polyline.m_rgb.x, polyline.m_rgb.y, polyline.m_rgb.z);

		glBegin(GL_LINE_STRIP);
		for (auto it = polyline.m_vertices.begin(); it != polyline.m_vertices.end(); it++) {
			glVertex3d(it->x, it->y, it->z);
		}
		glEnd();
	}

	glDisable(GL_BLEND);
	glLineWidth(1);
}

void marchingSquare(std::list<POLYLINE>& edges, const Polyhedron& poly, const double& surface) {
	for (int i = 0; i < poly.nquads; i++) {
		std::vector<Vertex> r;
		lookUpTable(r,
			*poly.qlist[i]->verts[0],
			*poly.qlist[i]->verts[1],
			*poly.qlist[i]->verts[2],
			*poly.qlist[i]->verts[3],
			surface);
		if (r.size() > 0) {
			for (int r_i = 0; r_i < r.size() / 2; r_i++) {
				POLYLINE line;
				auto v0 = icVector3(
					r[r_i * 2].x,
					r[r_i * 2].y,
					r[r_i * 2].z);
				auto v1 = icVector3(
					r[r_i * 2 + 1].x,
					r[r_i * 2 + 1].y,
					r[r_i * 2 + 1].z);
				if ((v0 - v1).length() < EPSILON)
					line.m_vertices.push_back(v0);
				else {
					line.m_vertices.push_back(v0);
					line.m_vertices.push_back(v1);
				}
				//line.m_vertices.push_back(v0);
				//line.m_vertices.push_back(v1);
				edges.push_back(line);
			}
		}
	}
}

void makePolyLineFromEdges(std::vector<POLYLINE>& polylines, const std::list<POLYLINE>& edges) {
	polylines.reserve(edges.size());
	std::list<POLYLINE> edges_temp(edges);
	while (edges_temp.size() > 0) {
		polylines.push_back(edges_temp.front());
		edges_temp.erase(edges_temp.begin());
		int init_size = 0;
		while (init_size != edges_temp.size()) {
			init_size = edges_temp.size();
			for (auto i = edges_temp.begin(); i != edges_temp.end();) {
				if (polylines.back().isNeighbor(*i)) {
					polylines.back().merge(*i);
					i = edges_temp.erase(i);
				}
				else i++;
			}
		}
	}
}

Vertex lerpByScalar(const Vertex& v0, const Vertex& v1, const double& surface) {
	double f0_f1 = v0.scalar - v1.scalar;
	Vertex r(0, 0, 0);
	if (std::abs(f0_f1) < EPSILON) {
		r.x = (v0.x + v1.x) / 2;
		r.y = (v0.y + v1.y) / 2;
		r.z = (v0.z + v1.z) / 2;
	}
	else {
		double t = std::abs((v0.scalar - surface) / ((v0.scalar - surface) - (v1.scalar - surface)));
		r.x = v0.x + t * (v1.x - v0.x);
		r.y = v0.y + t * (v1.y - v0.y);
		r.z = v0.z + t * (v1.z - v0.z);
	}
	return r;
}

void lookUpTable(
	std::vector<Vertex>& r,
	const Vertex& v0,
	const Vertex& v1,
	const Vertex& v2,
	const Vertex& v3,
	const double& surface) {

	r.reserve(2);
	int id = 0;
	if (v0.scalar <= surface + EPSILON) id += 1;
	if (v1.scalar <= surface + EPSILON) id += 2;
	if (v2.scalar <= surface + EPSILON) id += 4;
	if (v3.scalar <= surface + EPSILON) id += 8;

	double center = 0;
	switch (id) {
	case 1:
		//v0 - v1 // v0 - v3
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;
	case 2:
		//v0 - v1 // v1 - v2
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v1, v2, surface));
		break;
	case 3:
		//v1 - v2 // v0 - v3
		r.push_back(lerpByScalar(v1, v2, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;
	case 4:
		//v1 - v2 // v2 - v3
		r.push_back(lerpByScalar(v1, v2, surface));
		r.push_back(lerpByScalar(v2, v3, surface));
		break;
	case 5:
		//v0 - v1 // v1 - v2
		//v2 - v3 // v0 - v3
		center = (v0.scalar + v1.scalar + v2.scalar + v3.scalar) / 4.0;
		if (center <= surface) {
			r.push_back(lerpByScalar(v0, v1, surface));
			r.push_back(lerpByScalar(v1, v2, surface));
			r.push_back(lerpByScalar(v2, v3, surface));
			r.push_back(lerpByScalar(v0, v3, surface));
		}
		else {
			r.push_back(lerpByScalar(v0, v1, surface));
			r.push_back(lerpByScalar(v0, v3, surface));
			r.push_back(lerpByScalar(v1, v2, surface));
			r.push_back(lerpByScalar(v2, v3, surface));
		}
		break;
	case 6:
		//v0 - v1 // v2 - v3
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v2, v3, surface));
		break;
	case 7:
		//v2 - v3 // v0 - v3
		r.push_back(lerpByScalar(v2, v3, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;
	case 8:
		//v2 - v3 // v0 - v3
		r.push_back(lerpByScalar(v2, v3, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;
	case 9:
		//v0 - v1 // v2 - v3
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v2, v3, surface));
		break;
	case 10:
		//v0 - v1 // v0 - v3
		//v1 - v2 // v2 - v3
		center = (v0.scalar + v1.scalar + v2.scalar + v3.scalar) / 4.0;
		if (center <= surface) {
			r.push_back(lerpByScalar(v0, v1, surface));
			r.push_back(lerpByScalar(v0, v3, surface));
			r.push_back(lerpByScalar(v1, v2, surface));
			r.push_back(lerpByScalar(v2, v3, surface));
		}
		else {
			r.push_back(lerpByScalar(v0, v1, surface));
			r.push_back(lerpByScalar(v1, v2, surface));
			r.push_back(lerpByScalar(v2, v3, surface));
			r.push_back(lerpByScalar(v0, v3, surface));
		}
		break;
	case 11:
		//v1 - v2 // v2 - v3
		r.push_back(lerpByScalar(v1, v2, surface));
		r.push_back(lerpByScalar(v2, v3, surface));
		break;

	case 12:
		//v1 - v2 // v0 - v3
		r.push_back(lerpByScalar(v1, v2, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;

	case 13:
		//v0 - v1 // v1 - v2
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v1, v2, surface));
		break;

	case 14:
		//v0 - v1 // v0 - v3
		r.push_back(lerpByScalar(v0, v1, surface));
		r.push_back(lerpByScalar(v0, v3, surface));
		break;
	default:
		//no surface
		break;
	}
}