#ifndef UTIL_H
#define UTIL_H

#include<glm/glm/glm.hpp>
#include<glm/glm/gtc/matrix_transform.hpp>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<vector>

using std::string;
using std::vector;

unsigned int loadTexture(const char*, int);

unsigned int loadCubemap(vector<string> faces);

glm::mat4 GetTrunkModelMat(glm::vec3 start, glm::vec3 end, float radius_dec_scale, float len_dec_scale);

glm::mat4 GetLeafModelMat(glm::vec3 start, glm::vec3 dir, float angle_offset, float scale, float height_offset);

glm::mat4 GetLeafModelMat(glm::vec3 start, glm::vec3 dir, glm::vec3 normal, float scale, float height_offset);

void genVAOVBO_pos(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t len);

void genVAOVBO_pos_nor_tex(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t len);

void printVec3(glm::vec3& v);

float genRandom();

float genRandom(float min, float max);

#endif