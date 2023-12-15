#include "util.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int loadTexture(const char* path, int pic_type) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	int width, height, nrChannels;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, pic_type, width, height, 0, pic_type, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		printf("Failed to load texture\n");
	}
	stbi_image_free(data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}

unsigned int loadCubemap(vector<string> faces) {
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < faces.size(); i++) {
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else {
			printf("Failed to load cubemap texture\n");
		}
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return textureID;
}

glm::mat4 GetTrunkModelMat(glm::vec3 start, glm::vec3 end, float radius_dec_scale, float len_dec_scale) {
	glm::mat4 mat = glm::mat4(1.0f);
	glm::vec3 dir = end - start;
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 axis = glm::cross(up, dir);
	float angle = glm::acos(glm::dot(up, dir) / glm::length(dir));

	glm::mat4 scale = glm::scale(mat, glm::vec3(radius_dec_scale, len_dec_scale, radius_dec_scale));

	glm::mat4 trans = glm::translate(mat, start);
	if (glm::length(axis) < 0.001f)
		return trans * scale;
	glm::mat4 rotas = glm::rotate(mat, angle, axis);
	// model = glm::scale(model, glm::vec3(radius, glm::length(dir), radius));
	return trans * rotas * scale;
}

glm::mat4 GetLeafModelMat(glm::vec3 start, glm::vec3 dir, glm::vec3 normal, float scale, float height_offset) {
	glm::mat4 mat(1.0f);
	glm::vec3 ori = glm::vec3(0.5f, 0.0f, -0.5f);
	glm::vec3 tar = dir + normal;
	glm::vec3 axis = glm::cross(ori, tar);

	glm::mat4 scale_mat = glm::scale(mat, glm::vec3(1.5f * scale));
	glm::mat4 trans_mat = glm::translate(mat, start - glm::vec3(0.0f, fminf(start.y+0.4f, height_offset), 0.0f));
	if(glm::length(axis) < 0.001f)
		return trans_mat * scale_mat;
	float angle = glm::acos(glm::dot(ori, tar) / glm::length(tar) / glm::length(ori));
	glm::mat4 rotas_mat = glm::rotate(mat, angle, axis);
	ori = glm::vec3(rotas_mat * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	tar = dir - normal;
	axis = glm::cross(ori, tar);
	if(glm::length(axis) < 0.001f)
		return trans_mat * rotas_mat * scale_mat;
	angle = glm::acos(glm::dot(ori, tar) / glm::length(tar) / glm::length(ori));
	glm::mat4 rotas_mat2 = glm::rotate(mat, angle, axis);
	return trans_mat * rotas_mat2 * rotas_mat * scale_mat;
}

void genVAOVBO_pos(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t len) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, len, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);
}

void genVAOVBO_pos_nor_tex(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t len) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, len, vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);
}

void printVec3(glm::vec3& v) {
	printf("%.3f %.3f %.3f\n", v.x, v.y, v.z);
}

float genRandom() {
	return (float)rand() / RAND_MAX;
}

float genRandom(float min, float max) {
	return min + (max - min) * genRandom();
}