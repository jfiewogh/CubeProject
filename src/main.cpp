#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>

// good example:
// https://github.com/kavan010/gravity_sim/blob/main/gravity_sim.cpp#L59

// learn:
// https://learnopengl.com/book/book_pdf.pdf
// https://learnopengl.com/Getting-started/Hello-Triangle

using namespace std; 

float width = 800.0f;
float height = 600.0f;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection));
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight);

// Shader sources
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "out vec4 vertexColor;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
    "   vertexColor = vec4(0.5, 0.0, 0.0, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor\n;"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";


// more constants

vector<glm::vec3> faceRotations = {
    glm::vec3(90, 0, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 90, 0),
    glm::vec3(0, 180, 0),
    glm::vec3(0, 270, 0),
    glm::vec3(-90, 0, 0),
};

// initial face colors
vector<glm::vec4> faceColors = {
    // top
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
    // front
    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
    // right
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
    // back
    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
    // left
    glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
    // bottom
    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
};

vector<glm::vec2> faceletOffsets = {
    // x is row
    // y is column
    glm::vec2(-1.1f, -1.1f),
    glm::vec2(-1.1f, 0.0f),
    glm::vec2(-1.1f, 1.1f),
    glm::vec2(0.0f, -1.1f),
    glm::vec2(0.0f, 0.0f),
    glm::vec2(0.0f, 1.1f),
    glm::vec2(1.1f, -1.1f),
    glm::vec2(1.1f, 0.0f),
    glm::vec2(1.1f, 1.1f)
};


// THE CODE IS BELOW!

void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void updateCam(float timeValue);
void processInput(GLFWwindow *window);

class FaceletObject {
    public:
    GLuint VAO, VBO;
    size_t vertexCount;
    glm::vec4 color;

    FaceletObject(glm::vec4 color) {
        this->color = color;

        std::vector<float> vertices = draw();
        vertexCount = vertices.size();
        createVBOVAO(VAO, VBO, vertices.data(), vertexCount);
    }

    std::vector<float> draw() {
        std::vector<float> vertices = {
            0.5f,  0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
            -0.5f,  -0.5f, 0.0f
        };
        return vertices;
    }

    void updateVertices() {
        // Get new vertices
        std::vector<float> vertices = draw();

        // Update VBO with new vertices
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    }

    glm::vec3 getPosition(int faceIndex, int faceletIndex) {
        glm::vec2 offset = faceletOffsets[faceletIndex];
        return glm::vec3(offset.x, offset.y, 1.56f);
    }
    glm::vec3 getRotation(int faceIndex) {
        return faceRotations[faceIndex];
    }
};

class Cube {
    public:
    // 0 - U
    // 1 - F
    // 2 - R
    // 3 - B
    // 4 - L
    // 5 - D

    // 0 1 2
    // 3 4 5
    // 6 7 8

    vector<vector<FaceletObject>> facelets;

    void initializeFacelets() {
        for (int f = 0; f < 6; f++) {
            vector<FaceletObject> face;
            for (int fl = 0; fl < 9; fl++) {
                face.push_back(FaceletObject(faceColors[f]));
            }
            facelets.push_back(face);
        }
    }

    void updateVertices() {
        for (vector<FaceletObject> face : facelets) {
            for (FaceletObject facelet : face) {
                facelet.updateVertices();
            }
        }
    }

    void moveFaceCW(int index) {
        FaceletObject temp = facelets[index][0];
        facelets[index][0] = facelets[index][6];
        facelets[index][6] = facelets[index][8];
        facelets[index][8] = facelets[index][2];
        facelets[index][2] = temp;
    }

    void moveFaceCCW(int index) {
        FaceletObject temp = facelets[index][0];
        facelets[index][0] = facelets[index][2];
        facelets[index][2] = facelets[index][8];
        facelets[index][8] = facelets[index][6];
        facelets[index][6] = temp;
    }
};


Cube cube;
glm::mat4 view;
glm::mat4 projection;

int main() {
    GLFWwindow* window;
    if (!glfwInit()) {
        cout << "GLFW couldn't start" << endl;
        return -1;
    }
    window = glfwCreateWindow(width, height, "CubeProject", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwTerminate();
        return -1;
    }

    // Create Shader Program

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    glEnable(GL_DEPTH_TEST);



    // Set background color
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);


    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "ourColor");


    cube.initializeFacelets();

    while (!glfwWindowShouldClose(window)) {
        float timeValue = glfwGetTime();

        processInput(window);

        updateCam(timeValue);

        // clear color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        glUseProgram(shaderProgram);

        // update uniform matrix
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (int f = 0; f < cube.facelets.size(); f++) {
            for (int fl = 0; fl < cube.facelets[0].size(); fl++) {
                FaceletObject facelet = cube.facelets[f][fl];
                
                glm::mat4 model = glm::mat4(1.0f);
                
                glm::vec3 rotation = facelet.getRotation(f);
                model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

                // Translation
                model = glm::translate(model, facelet.getPosition(f, fl));

                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glm::vec4 color = facelet.color;
                glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

                glBindVertexArray(facelet.VAO);
                glDrawArrays(GL_TRIANGLES, 0, facelet.vertexCount);
            }
        }

        // swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}

void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

void updateCam(float timeValue) {
    const float radius = 10.0f;
    float camX = sin(timeValue) * radius;
    float camZ = cos(timeValue) * radius;
    view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

    projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
}

void processInput(GLFWwindow *window) {
    // escape button closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } 
    // R (right face up)
    else if (glfwGetKey(window, GLFW_KEY_I)) {
        cout << "R" << endl;
        cube.moveFaceCW(2);       
    }
    // R' (right face down)
    else if (glfwGetKey(window, GLFW_KEY_K)) {
        cout << "R'" << endl;
        cube.moveFaceCCW(2);
    }
    // L (left face down)
    else if (glfwGetKey(window, GLFW_KEY_E)) {
        cube.moveFaceCW(3);
    }
    // L' (left face up)
    else if (glfwGetKey(window, GLFW_KEY_D)) {
        cube.moveFaceCCW(3);
    }
    // U
    else if (glfwGetKey(window, GLFW_KEY_J)) {
        cube.moveFaceCW(0);
    }
    // U'
    else if (glfwGetKey(window, GLFW_KEY_F)) {
        cube.moveFaceCCW(0);
    }
    // F
    else if (glfwGetKey(window, GLFW_KEY_H)) {
        cube.moveFaceCW(1);
    }
    // F'
    else if (glfwGetKey(window, GLFW_KEY_G)) {
        cube.moveFaceCCW(1);
    }
}