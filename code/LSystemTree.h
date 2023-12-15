#ifndef LSYSTEMTREE_H
#define LSYSTEMTREE_H

#include "glm/glm/glm.hpp"
#include "glm/glm/gtc/matrix_transform.hpp"
#include "grammar.h"
#include "util.h"
#include<stack>
using std::stack;

struct state {
	glm::vec3 pos;
	glm::vec3 dir;
	float len;
	float radius;
	int level;
};

struct Leaf {
	glm::vec3 pos;
	glm::vec3 dir;
	float len; // 树叶附着的树干的长度
};

struct Trunk {
	glm::vec3 start;
	glm::vec3 end;
	float radius_scale;
	float len_scale;
};

class LSystemTree {
public:
	LSystemTree() {
		clear();
	}

	LSystemTree(glm::vec3 ini_pos, glm::vec3 ini_dir, float len, float radius, float angle, float radius_dec_scale, float len_dec_scale) {
		cur_state.pos = ini_pos;
		cur_state.dir = ini_dir;
		cur_state.len = len;
		cur_state.radius = radius;
		cur_state.level = 1;
		ori_radius = radius;
		ori_len = len;
		this->angle = angle;
		this->radius_dec_scale = radius_dec_scale;
		this->len_dec_scale = len_dec_scale;
	}

	const state& get_state() const { return cur_state; }
	const vector<Trunk>& get_trunks() const { return trunks; }
	const vector<Leaf>& get_leafs() const { return leafs; }

	void clear() {
		cur_state.pos = glm::vec3(0.0f, -0.5f, 0.0f);
		cur_state.dir = glm::vec3(0.0f, 1.0f, 0.0f);
		cur_state.len = 0.4f;
		cur_state.radius = 0.05f;
		cur_state.level = 1;
		radius_dec_scale = 0.8f;
		len_dec_scale = 0.8f;
		ori_radius = 0.05f;
		angle = 30.0f;
		ori_len = 0.4f;
		trunks.clear();
		leafs.clear();
	}

	void generate(const Grammar& g) {
		stack<state> state_stack;
		string result = g.get_result();
		for (auto c : result) {
			if (c == 'F') {
				Trunk trunk;
				trunk.start = cur_state.pos;
				cur_state.pos += cur_state.dir * cur_state.len;
				trunk.end = cur_state.pos;
				trunk.radius_scale = cur_state.radius / ori_radius;
				trunk.len_scale = cur_state.len / ori_len;
				trunks.push_back(trunk);
			}
			else if (c == '$') { // rotate around y axis
				cur_state.dir = rotate(cur_state.dir, glm::radians(angle), glm::vec3(0, 1, 0));
			}
			else if (c == '%') {
				cur_state.dir = rotate(cur_state.dir, glm::radians(-angle), glm::vec3(0, 1, 0));
			}
			else if (c == '^') { // rotate around x axis
				cur_state.dir = rotate(cur_state.dir, glm::radians(angle), glm::vec3(1, 0, 0));
			}
			else if (c == '&') {
				cur_state.dir = rotate(cur_state.dir, glm::radians(-angle), glm::vec3(1, 0, 0));
			}
			else if (c == '*') { // rotate around z axis
				cur_state.dir = rotate(cur_state.dir, glm::radians(angle), glm::vec3(0, 0, 1));
			}
			else if (c == '/') {
				cur_state.dir = rotate(cur_state.dir, glm::radians(-angle), glm::vec3(0, 0, 1));
			}
			else if (c == '[') {
				state_stack.push(cur_state);
				cur_state.len *= len_dec_scale;
				cur_state.radius *= radius_dec_scale;
				cur_state.level++;
			}
			else if (c == ']') {
				if (cur_state.level == g.get_level()) {
					Trunk& t = trunks[trunks.size() - 1];
					Leaf leaf;
					leaf.pos = t.end;
					leaf.dir = cur_state.dir; 
					leaf.len = cur_state.len;
					leafs.push_back(leaf);
				}
				cur_state = state_stack.top();
				state_stack.pop();
			}
			else if (c == 'L') {
				Leaf leaf;
				leaf.pos = cur_state.pos;
				leaf.dir = cur_state.dir;// +genRandom(-1.0f, 1.0f);
				leaf.len = cur_state.len;
				leafs.push_back(leaf);
			}
		}
	}
	
	static glm::vec3 rotate(glm::vec3 v, float angle, glm::vec3 axis) {
		glm::vec4 rotate_vector = glm::rotate(glm::mat4(1.0f), angle, axis) * glm::vec4(v, 1.0f);
		return glm::vec3(rotate_vector);
	}

public:
	state cur_state;
	vector<Trunk> trunks;
	vector<Leaf> leafs;
	float radius_dec_scale;
	float len_dec_scale;
	float ori_radius;
	float ori_len;
	float angle; // in degrees
};

#endif
