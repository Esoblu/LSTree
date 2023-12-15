#ifndef CONE_H
#define CONE_H

#include "glm/glm/glm.hpp"
#include<vector>
using std::vector;

struct TVertex {
	glm::vec3 pos;
	glm::vec3 nor;
	glm::vec2 tex;
};

const float PI = 3.1415926f;

class Cone {
public:
	float radius;
	float height;
	int sectorCount;
	float scale;
public:
	Cone():radius(0.1f), height(0.5f), sectorCount(36), scale(1.0f) {}

	Cone(float radius, float height, int sectorCount, float scale):radius(radius), height(height), sectorCount(sectorCount), scale(scale) {}

	vector<TVertex> getUnitCircleVertices() {
		float sectorStep = 2 * PI / sectorCount;
		float sectorAngle = 0.0f;

		glm::vec3 position;
		glm::vec3 normal;
		TVertex tVertex;

		float cos_Slope = height / sqrt(height * height + (1 - scale) * (1 - scale) * radius * radius);
		float sin_Slope = sqrt(1 - cos_Slope * cos_Slope);

		vector<TVertex> unitCircleVertices;
		for (int i = 0; i <= sectorCount; ++i) {
			sectorAngle = i * sectorStep;
			normal.x = cosf(sectorAngle) * cos_Slope;
			normal.y = sin_Slope;
			normal.z = sinf(sectorAngle) * cos_Slope;

			position.x = radius * normal.x;
			position.y = 0.0f;
			position.z = radius * normal.z;

			tVertex.pos = position;
			tVertex.nor = normal;

			tVertex.tex.x = (float) i / sectorCount;
			tVertex.tex.y = 0.0f;

			unitCircleVertices.push_back(tVertex);
		}
		return unitCircleVertices;
	}

	void buildConeVertices(vector<TVertex>& vertices) {
		auto vctBot = getUnitCircleVertices();

		vector<TVertex> vctTop(vctBot.size()); // 上下圆周顶点数组
		TVertex tVertex;
		for (int i = 0; i < vctBot.size(); ++i) {
			tVertex.nor = vctBot[i].nor;
			tVertex.pos = vctBot[i].pos * scale;
			tVertex.pos.y = height;
			tVertex.tex.x = vctBot[i].tex.x;
			tVertex.tex.y = 1.0f;
			vctTop[i] = tVertex;
		}

		// 圆柱侧面
		for (int i = 0; i < vctTop.size() - 1; ++i) {
			vertices.push_back(vctTop[i]);
			vertices.push_back(vctBot[i]);
			vertices.push_back(vctBot[i + 1]);

			vertices.push_back(vctTop[i]);
			vertices.push_back(vctTop[i + 1]);
			vertices.push_back(vctBot[i + 1]);
		}

		// 顶部圆形
		glm::vec3 pos(0.0f, height, 0.0f);
		glm::vec3 nor(0.0f, 1.0f, 0.0f);
		for (int i = 0; i < vctTop.size() - 1; ++i) {
			tVertex.nor = nor;
			tVertex.pos = pos;
			vertices.push_back(tVertex);
			tVertex.pos = vctTop[i].pos;
			vertices.push_back(tVertex);
			tVertex.pos = vctTop[i + 1].pos;
			vertices.push_back(tVertex);
		}

		pos.y = 0.0f;
		nor.y = -1.0f;
		for (int i = 0; i < vctBot.size() - 1; ++i) {
			tVertex.pos = pos;
			tVertex.nor = nor;
			vertices.push_back(tVertex);
			tVertex.pos = vctBot[i].pos;
			vertices.push_back(tVertex);
			tVertex.pos = vctBot[i + 1].pos;
			vertices.push_back(tVertex);
		}
	}
};

#endif