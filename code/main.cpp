#include "grammar.h"
#include "LSystemTree.h"
#include "Cone.h"
#include "shader.h"
#include "camera.h"
#include "data.h"
#include "util.h"
#include "ParticleSystem.h"
#include<stdio.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void renderGround(const Shader& shader);
void renderTree(const Shader& shader, size_t len);
void renderLeaf(const Shader& shader, size_t len, float leaf_mix_rate, float scale, float height_offset);
void renderSnow(const Shader& shader, size_t len);
void renderQuad();
void update_season();
void update_light();

ParticleSystem ps(1000);
LSystemTree l;
float radius = 0.03f; // initial radius of trunk
float init_len = 0.4f;   // initial length of trunk
float radius_dec_scale = 0.7f; // radius decay rate
float len_dec_scale = 0.8f; // length decay rate
float angle = 30.0f; // angle of each rotation
int iter = 8; // iteration times
int preset = 1;

void generate_LSystemTree(
    int iter = 8,
    glm::vec3 ini_pos = glm::vec3(0.0f, -0.5f, 0.0f),
    glm::vec3 ini_dir = glm::vec3(0.0f, 1.0f, 0.0f)
    )
{
    Grammar g = Grammar(iter, "S");
    if (preset == 1) {
        g.add_rule('S', "F[^$X][*%X][&%X]");
        g.add_rule('X', "F[^%D][&$D][/$D][*%D]");
        g.add_rule('X', "F[&%D][*$D][/$D][^%D]");
        g.add_rule('D', "F[^$X][*%X][&%X]");
    }
    else if(preset == 2) {
        init_len = 0.3f;
        angle = 25.0f;
        len_dec_scale = 0.7f;
        g.add_rule('S', "[**$X][*%Y][&%Z][//$O][/$$P][**$Z][^$$P][&&$Y][///%X]");
        g.add_rule('X', "F[^XL]");
        g.add_rule('Y', "F[&YL]");
        g.add_rule('Z', "F[*ZL]");
        g.add_rule('O', "F[/OL]");
        g.add_rule('P', "F[^PL]");
    }

    g.generate();

    l = LSystemTree(ini_pos, ini_dir, init_len, radius, angle, radius_dec_scale, len_dec_scale);
    l.generate(g);
    // printf("%d\n", l.leafs.size());
}

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.3f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// global VAO
unsigned int treeVAO, leafVAO, groundVAO, snowVAO, quadVAO = 0, quadVBO;

// season control
const float Period = 20.0f;
float season_time = 0.0f;
float leaf_mix_rate = 0.0f, ground_mix_rate = 0.0f;
float scale = 1.0f;
float height_offset = 0.0f;

// light control
float lightStrength = 1.0f;
float strengthflag = 1.0f;
float lightAngle = 45.0f;
float angleflag = -1.0f;
glm::vec3 lightPos = glm::vec3(1.0f, 1.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
bool shadows = true; 

int main() {
    //-------------------------configure and initialize-------------------------

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 2);

    generate_LSystemTree();
    ps.InitParticles();
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LSystemTree", NULL, NULL);
    if (window == NULL) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hide cursor

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ------------------------shaders--------------------------
    Shader skyboxShader("shaders/skybox.vs", "shaders/skybox.fs");
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    Shader trunkShader("shaders/shader.vs", "shaders/shader.fs");
    trunkShader.use();
    trunkShader.setInt("diffuseTexture", 0);
    trunkShader.setInt("shadowMap", 1);

    Shader leafShader("shaders/shader.vs", "shaders/mixShader.fs");
    leafShader.use();
    leafShader.setInt("texture1", 0);
    leafShader.setInt("texture2", 1);
    leafShader.setInt("shadowMap", 2);

    Shader lightShader("shaders/light.vs", "shaders/light.fs"); // light object

    Shader snowShader("shaders/snow.vs", "shaders/snow.fs");

    Shader depthShader("shaders/depthShader.vs", "shaders/depthShader.fs");

    Shader debugDepthQuad("shaders/debugDepthQuad.vs", "shaders/debugDepthQuad.fs");
    debugDepthQuad.use();
    debugDepthQuad.setInt("depthMap", 0);

    //-----------------------------------textures-----------------------------------
    vector<string> faces{ "resources/right.jpg", "resources/left.jpg", "resources/top.jpg",
                          "resources/bottom.jpg", "resources/front.jpg", "resources/back.jpg" };
    unsigned int cubemapTexture = loadCubemap(faces);
    unsigned int leaf_texture = loadTexture("resources/leaf.png", GL_RGBA);
    unsigned int yellow_leaf_texture = loadTexture("resources/yellowLeaf.png", GL_RGBA);
    unsigned int trunk_texture = loadTexture("resources/trunk.jpg", GL_RGB);
    unsigned int ground_texture = loadTexture("resources/grassGround.jpg", GL_RGB);
    unsigned int snow_ground_texture = loadTexture("resources/snowGround.jpg", GL_RGB);
    unsigned int yellow_ground_texture = loadTexture("resources/yellowGrassGround.jpg", GL_RGB);
    unsigned int snow_texture = loadTexture("resources/snow.png", GL_RGBA);

    //-----------------------------------VAO VBO-----------------------------------
    unsigned int skyboxVAO, skyboxVBO;
    genVAOVBO_pos(skyboxVAO, skyboxVBO, skyboxVertices, sizeof(skyboxVertices));

    unsigned int groundVBO;
    genVAOVBO_pos_nor_tex(groundVAO, groundVBO, planeVertices, sizeof(planeVertices));

    unsigned int snowVBO;
    genVAOVBO_pos_nor_tex(snowVAO, snowVBO, snowVertices, sizeof(snowVertices));

    vector<TVertex> vertices;
    Cone cone = Cone(radius, init_len, 25, radius_dec_scale); // trunk
    cone.buildConeVertices(vertices);
    unsigned int treeVBO;
    genVAOVBO_pos_nor_tex(treeVAO, treeVBO, (const float*)vertices.data(), vertices.size() * sizeof(TVertex));

    unsigned int leafVBO;
    genVAOVBO_pos_nor_tex(leafVAO, leafVBO, leafVertices, sizeof(leafVertices));

    unsigned int lightCubeVAO, lightCubeVBO;
    genVAOVBO_pos(lightCubeVAO, lightCubeVBO, lightVertices, sizeof(lightVertices));

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    float lastTime = glfwGetTime();

    // render loop
    while (!glfwWindowShouldClose(window)) {
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        processInput(window);

        if (currentTime - lastTime >= 1.0f) {
            int frameRate = static_cast<int>(1.0f / deltaTime);
            lastTime = currentTime;
            char title[30];
            sprintf_s(title, "LSystemTree FPS: %d", frameRate);
            glfwSetWindowTitle(window, title);
        }

        update_season();
        update_light();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // set clear color
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color buffer and depth buffer
        // render depth of scene to texture
        glCullFace(GL_FRONT);
        glm::mat4 lightProjection, lightView;
        glm::mat4 lightSpaceMatrix;
        glm::vec3 parallei_light = glm::normalize(lightPos - glm::vec3(0.0f));
        float near_plane = 1.0f, far_plane = 7.5f;
        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
        lightSpaceMatrix = lightProjection * lightView;
         
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        renderTree(depthShader, vertices.size());
        renderGround(depthShader);
        renderLeaf(depthShader, sizeof(leafVertices), leaf_mix_rate, scale, height_offset);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);

        /*
        // render depthMap visually
        // 如果想要渲染阴影贴图，将下面的所有渲染过程注释掉(到glfwswapbuffer为止)
        // 并将下面括号内的内容取消注释
        
        {
            debugDepthQuad.use();
            debugDepthQuad.setFloat("near_plane", near_plane);
            debugDepthQuad.setFloat("far_plane", far_plane);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            renderQuad();
        }
        */
        
        // render skybox
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        // render trunks
        trunkShader.use();
        glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        view = camera.GetViewMatrix();
        trunkShader.setVec3("viewPos", camera.Position);
        trunkShader.setMat4("view", view);
        trunkShader.setMat4("projection", projection);
        trunkShader.setVec3("lightPos", lightPos);
        trunkShader.setVec3("lightColor", lightColor);
        trunkShader.setVec3("lightDir", parallei_light);
        trunkShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        trunkShader.setBool("shadows", shadows);
        trunkShader.setFloat("lightStrength", lightStrength);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, trunk_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderTree(trunkShader, vertices.size());

        // render leaves
        leafShader.use();
        leafShader.setVec3("viewPos", camera.Position);
        leafShader.setMat4("view", view);
        leafShader.setMat4("projection", projection);
        leafShader.setVec3("lightPos", lightPos);
        leafShader.setVec3("lightColor", lightColor);
        leafShader.setVec3("lightDir", parallei_light);
        leafShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        leafShader.setBool("shadows", shadows);
        leafShader.setFloat("lightStrength", lightStrength);
        leafShader.setFloat("mix_rate", leaf_mix_rate);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, leaf_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, yellow_leaf_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderLeaf(leafShader, sizeof(leafVertices), leaf_mix_rate, scale, height_offset);

        // render ground
        leafShader.setFloat("mix_rate", ground_mix_rate);
        if(season_time < Period / 8.0f || season_time >= 3.0f * Period / 4.0f)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, snow_ground_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, yellow_ground_texture);
            glActiveTexture(GL_TEXTURE2);
        }
        else 
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, ground_texture);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, yellow_ground_texture);
            glActiveTexture(GL_TEXTURE2);
        }
        glBindTexture(GL_TEXTURE_2D, depthMap);
        renderGround(leafShader);

        // render light cube
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.1f)); // a smaller cube
        lightShader.setMat4("model", model);
        lightShader.setVec3("lightColor", lightColor);
        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        if (season_time > 3.0f * Period / 5.0f) 
        { // render snow
            ps.update();
            snowShader.use();
            snowShader.setMat4("view", view);
            snowShader.setMat4("projection", projection);
            snowShader.setFloat("alphaValue", fminf(1.0f, Period - season_time));
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, snow_texture);
            renderSnow(snowShader, sizeof(snowVertices));
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &treeVAO);
    glDeleteBuffers(1, &treeVBO);
    glDeleteVertexArrays(1, &lightCubeVAO);
    glDeleteBuffers(1, &lightCubeVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &groundVAO);
    glDeleteBuffers(1, &groundVBO);
    glDeleteVertexArrays(1, &leafVAO);
    glDeleteBuffers(1, &leafVBO);

    glfwTerminate();
    return 0;
}


void renderGround(const Shader& shader) {

    glm::mat4 model = glm::mat4(1.0f);
    shader.setMat4("model", model);
    glBindVertexArray(groundVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void renderTree(const Shader& shader, size_t len) {
    glBindVertexArray(treeVAO);
    auto& trunks = l.get_trunks();
    for (unsigned int i = 0; i < trunks.size(); i++) {
        glm::mat4 model = GetTrunkModelMat(trunks[i].start, trunks[i].end, trunks[i].radius_scale, trunks[i].len_scale);
        shader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, len);
    }
}

void renderLeaf(const Shader& shader, size_t len, float leaf_mix_rate, float scale, float height_offset) {
    glBindVertexArray(leafVAO);
    auto& leafs = l.get_leafs();
    const int pieces = 5; // number of leaves in a trunk
    for (unsigned int i = 0; i < leafs.size(); ++i) {
        glm::vec3 dir = leafs[i].dir;
        glm::vec3 nor1 = glm::vec3(0.0f, 1.0f, 0.0f);
        if(abs(dir.x)>0.0001f || abs(dir.y)>0.0001f){
            nor1 = glm::normalize(glm::vec3(-dir.y, dir.x, 0.0f));
        }
        glm::vec3 nor2 = glm::normalize(glm::cross(dir, nor1));
    
        for (int j = 0; j < pieces; ++j) {
            //glm::mat4 model = GetLeafModelMat(leafs[i].pos, leafs[i].dir, 0.0f, 1.0f, 0.0f);
            float angle = glm::radians(360.0f / pieces * j);
            glm::vec3 normal = glm::normalize(nor1 * cos(angle) + nor2 * sin(angle));
            glm::vec3 start = leafs[i].pos - leafs[i].len * j / pieces * leafs[i].dir;
            glm::mat4 model = GetLeafModelMat(start, dir, normal, scale, height_offset);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, len);
        }
    }
}

void renderSnow(const Shader& shader, size_t len) {
	glBindVertexArray(snowVAO);
	auto* snows = ps.getParticles();
    for (unsigned int i = 0; i < ps.getNum(); ++i) {
        glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, snows[i].pos);
        model = glm::rotate(model, glm::radians(snows[i].angle), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(snows[i].size));
		shader.setMat4("model", model);
		glDrawArrays(GL_TRIANGLES, 0, len);
	}
}

void update_season() {
    season_time += deltaTime;
    if (season_time >= Period) {
        ps.InitParticles();
        season_time -= Period;
    }

    if (season_time < Period / 4.0f) { // spring
		scale = season_time / (Period / 4.0f);
		leaf_mix_rate = 0.0f;
        ground_mix_rate = fminf(season_time/(Period / 8.0f), 2.0f - season_time / (Period / 8.0f));
        height_offset = 0.0f;
	}
    else if (Period / 4.0f <= season_time and season_time < 2.0f * Period / 4.0f) { // summer
		scale = 1.0f;
	}
    else if (2.0f * Period / 4.0f <= season_time and season_time < 3.0f * Period / 4.0f) { // autumn
		leaf_mix_rate = season_time / (Period / 4.0f) - 2.0f;
        ground_mix_rate = leaf_mix_rate;
	}
    else { // winter
		leaf_mix_rate = 1.0f;
		ground_mix_rate = fmaxf(7.0f - 8.0f/Period * season_time, 0.0f);
		height_offset = 2.0f * (season_time - 3.0f * Period / 4.0f) / (Period / 4.0f);
	}
}

void update_light()
{
    lightAngle += angleflag * 4.5f * deltaTime;
    if (glm::cos(glm::radians(lightAngle)) < 0.5f)
    {
        // lightAngle = 60.0f;
        angleflag *= -1;
    }
    lightPos = glm::vec3(std::sqrt(5) * glm::sin(glm::radians(lightAngle)), std::sqrt(5) * glm::cos(glm::radians(lightAngle)), -2.0f);

    lightStrength += strengthflag * 0.8f / 20 * deltaTime;
    if (lightStrength > 1.0f)
        strengthflag *= -1, lightStrength = 1.0f;
    if (lightStrength < 0.5f)
        strengthflag *= -1, lightStrength = 0.5f;
}

void renderQuad()
{
    if (quadVAO == 0)
    {
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        shadows = !shadows;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}