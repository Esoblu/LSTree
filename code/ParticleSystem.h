#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "util.h"


struct Particle {
	glm::vec3 pos;
	glm::vec3 velocity;
	// glm::vec3 color;
    glm::vec3 acce;
	float size;
	float angle;
};

class ParticleSystem {
private:
	Particle* particles;
	int num;
public:
	ParticleSystem() {
		particles = nullptr;
		num = 0;
	}

	ParticleSystem(int num) {
		particles = (Particle*)malloc(sizeof(Particle) * num);
		this->num = num;
	}

	// void setColor(glm::vec3 color, int index) {
	// 	particles[index].color = color;
	// }

	void setVelocity(glm::vec3 velocity, int index) {
		particles[index].velocity = velocity;
	}

    void setPos(glm::vec3 pos, int index) {
		particles[index].pos = pos;
	}

	void setSize(float size, int index) {
		particles[index].size = size;
	}

	void setAng(float angle, int index) {
		particles[index].angle = angle;
	}

    bool InitParticles()
    {
        // glm::vec3 color(1.0f, 1.0f, 1.0f);
        float x, y, z, vx, vy, vz, ax, ay, az, angle;
        for (int i = 0; i < num; ++i)
        {
            // 初始化颜色（白色）
            
            //particles[i].color = color;

            // 初始化坐标
            x = genRandom(-2.5f, 2.5f);
            y = 2.0f + genRandom(0.0f, 0.2f);
            z = genRandom(-1.0f, 3.0f);
            particles[i].pos = glm::vec3(x, y, z);

            // 初始化速度
            vx = 0.00001 * genRandom(-50.0f, 50.0f);
            vy = -0.0000004 * (rand() % 28000);
            vz = 0.00001 * genRandom(-50.0f, 50.0f);
            particles[i].velocity = glm::vec3(vx, vy, vz);

            // 初始化加速度
            ax = 0;
            ay = -0.000005f;
            az = 0;
            particles[i].acce = glm::vec3(ax, ay, az);

            angle = genRandom(0.0f, 360.0f);
            particles[i].angle = angle;

            // 初始化大小
            particles[i].size = genRandom(0.05f, 0.15f);
        }
        return true;
    }

    void update() {
        for (int i = 0; i < num; ++i) {
            particles[i].pos += particles[i].velocity;
            particles[i].velocity += particles[i].acce;

            if (particles[i].pos.y < -0.5f) {
                particles[i].pos.x = genRandom(-2.5f, 2.5f);
                particles[i].pos.y = 2.0f + genRandom(0.0f, 0.2f);
                particles[i].pos.z = genRandom(-1.0f, 3.0f);
                particles[i].velocity.x = 0.00001 * genRandom(-50.0f, 50.0f);
                particles[i].velocity.y = -0.0000004 * (rand() % 28000);
                particles[i].velocity.z = 0.00001 * genRandom(-50.0f, 50.0f);
                particles[i].size = genRandom(0.1f, 0.2f);
                particles[i].angle = genRandom(0.0f, 360.0f);
            }
        }
    }

	const Particle* getParticles() const { return particles; }
    const int getNum() const { return num; }
};

#endif