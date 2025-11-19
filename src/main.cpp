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

const double PI = 3.14159265358979323846;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

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
    glm::vec3(-90, 0, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 90, 0),
    glm::vec3(0, 180, 0),
    glm::vec3(0, 270, 0),
    glm::vec3(90, 0, 0),
};

// initial face colors
vector<glm::vec4> faceColors = {
    // top
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
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
    glm::vec2(-1.1f, 1.1f),
    glm::vec2(0.0f, 1.1f),
    glm::vec2(1.1f, 1.1f),

    glm::vec2(-1.1f, 0.0f),
    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.1f, 0.0f),

    glm::vec2(-1.1f, -1.1f),
    glm::vec2(0.0f, -1.1f),
    glm::vec2(1.1f, -1.1f)
};


// THE CODE IS BELOW!

void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void updateCam(float radians);
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

    glm::vec3 getPosition(int faceIndex, int faceletIndex) {
        glm::vec2 offset = faceletOffsets[faceletIndex];
        return glm::vec3(offset.x, offset.y, 1.6f);
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

    void resetFacelets() {
        facelets.clear();
        initializeFacelets();
    }

    // previous is next
    void swapFour(int r1, int c1, int r2, int c2, int r3, int c3, int r4, int c4) {
        FaceletObject temp = facelets[r1][c1];
        facelets[r1][c1] = facelets[r2][c2];
        facelets[r2][c2] = facelets[r3][c3];
        facelets[r3][c3] = facelets[r4][c4];
        facelets[r4][c4] = temp;
    }

    void moveFaceCW(int index) {
        swapFour(index, 0, index, 6, index, 8, index, 2);
        swapFour(index, 1, index, 3, index, 7, index, 5);
    }
    void moveFaceCCW(int index) {
        swapFour(index, 0, index, 2, index, 8, index, 6);
        swapFour(index, 5, index, 7, index, 3, index, 1);
    }

    // Face
    void U() {
        moveFaceCW(0);
        swapFour(1, 0, 2, 0, 3, 0, 4, 0);
        swapFour(1, 1, 2, 1, 3, 1, 4, 1);
        swapFour(1, 2, 2, 2, 3, 2, 4, 2);
    }
    void UPrime() {
        moveFaceCCW(0);
        swapFour(4, 0, 3, 0, 2, 0, 1, 0);
        swapFour(4, 1, 3, 1, 2, 1, 1, 1);
        swapFour(4, 2, 3, 2, 2, 2, 1, 2);
    }
    void R() {
        moveFaceCW(2);
        swapFour(0, 2, 1, 2, 5, 2, 3, 6);
        swapFour(0, 5, 1, 5, 5, 5, 3, 3);
        swapFour(0, 8, 1, 8, 5, 8, 3, 0);
    }
    void RPrime() {
        moveFaceCCW(2);
        swapFour(3, 6, 5, 2, 1, 2, 0, 2);
        swapFour(3, 3, 5, 5, 1, 5, 0, 5);
        swapFour(3, 0, 5, 8, 1, 8, 0, 8);
    }
    void L() {
        moveFaceCW(4);
        swapFour(3, 8, 5, 0, 1, 0, 0, 0);
        swapFour(3, 5, 5, 3, 1, 3, 0, 3);
        swapFour(3, 2, 5, 6, 1, 6, 0, 6);
    }
    void LPrime() {
        moveFaceCCW(4);
        swapFour(0, 0, 1, 0, 5, 0, 3, 8);
        swapFour(0, 3, 1, 3, 5, 3, 3, 5);
        swapFour(0, 6, 1, 6, 5, 6, 3, 2);
    }
    void F() {
        moveFaceCW(1);
        swapFour(0, 6, 4, 8, 5, 2, 2, 0);
        swapFour(0, 7, 4, 5, 5, 1, 2, 3);
        swapFour(0, 8, 4, 2, 5, 0, 2, 6);
    }
    void FPrime() {
        moveFaceCCW(1);
        swapFour(2, 0, 5, 2, 4, 8, 0, 6);
        swapFour(2, 3, 5, 1, 4, 5, 0, 7);
        swapFour(2, 6, 5, 0, 4, 2, 0, 8);
    }
    void D() {
        moveFaceCW(5);
        swapFour(1, 6, 4, 6, 3, 6, 2, 6);
        swapFour(1, 7, 4, 7, 3, 7, 2, 7);
        swapFour(1, 8, 4, 8, 3, 8, 2, 8);
    }
    void DPrime() {
        moveFaceCCW(5);
        swapFour(2, 6, 3, 6, 4, 6, 1, 6);
        swapFour(2, 7, 3, 7, 4, 7, 1, 7);
        swapFour(2, 8, 3, 8, 4, 8, 1, 8);
    }
    void B() {
        moveFaceCW(3);
        swapFour(2, 8, 5, 6, 4, 0, 0, 2);
        swapFour(2, 5, 5, 7, 4, 3, 0, 1);
        swapFour(2, 2, 5, 8, 4, 6, 0, 0);
    }
    void BPrime() {
        moveFaceCCW(3);
        swapFour(0, 2, 4, 0, 5, 6, 2, 8);
        swapFour(0, 1, 4, 3, 5, 7, 2, 5);
        swapFour(0, 0, 4, 6, 5, 8, 2, 2);
    }

    // Slice
    
    void M() {
        swapFour(0, 4, 3, 4, 5, 4, 1, 4);
        swapFour(0, 1, 3, 7, 5, 1, 1, 1);
        swapFour(0, 7, 3, 1, 5, 7, 1, 7);
    }
    void MPrime() {
        swapFour(1, 4, 5, 4, 3, 4, 0, 4);
        swapFour(1, 1, 5, 1, 3, 7, 0, 1);
        swapFour(1, 7, 5, 7, 3, 1, 0, 7);

    }
    void S() {
        swapFour(4, 1, 5, 3, 2, 7, 0, 5);
        swapFour(4, 4, 5, 4, 2, 4, 0, 4);
        swapFour(4, 7, 5, 5, 2, 1, 0, 3);
    }
    void SPrime() {
        swapFour(0, 5, 2, 7, 5, 3, 4, 1);
        swapFour(0, 4, 2, 4, 5, 4, 4, 4);
        swapFour(0, 3, 2, 1, 5, 5, 4, 7);
    }
    void E() {
        swapFour(4, 3, 3, 3, 2, 3, 1, 3);
        swapFour(4, 4, 3, 4, 2, 4, 1, 4);
        swapFour(4, 5, 3, 5, 2, 5, 1, 5);
    }
    void EPrime() {
        swapFour(1, 3, 2, 3, 3, 3, 4, 3);
        swapFour(1, 4, 2, 4, 3, 4, 4, 4);
        swapFour(1, 5, 2, 5, 3, 5, 4, 5);
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


    // reset facelets
    cube.initializeFacelets();

    while (!glfwWindowShouldClose(window)) {
        float timeValue = glfwGetTime();

        processInput(window);

        updateCam(PI / 2);

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

void updateCam(float yRad) {
    const float radius = 7.0f;

    cameraPos.x = cos(yRad) * radius;
    cameraPos.y = sin(PI / 4) * radius;
    cameraPos.z = sin(yRad) * radius;

    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
}

bool keyIPressed = false;
bool keyKPressed = false;
bool keyEPressed = false;
bool keyDPressed = false;
bool keyJPressed = false;
bool keyFPressed = false;
bool keyHPressed = false;
bool keyGPressed = false;
bool keyPeriodPressed = false;
bool keyCommaPressed = false;
bool keySPressed = false;
bool keyLPressed = false;
bool keyWPressed = false;
bool keyOPressed = false;
bool key6Pressed = false;
bool key5Pressed = false;
bool key4Pressed = false;
bool key7Pressed = false;
bool keyXPressed = false;
bool keyCPressed = false;

void processInput(GLFWwindow *window) {
    // escape button closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } 

    /* Moves */

    // R (right face up)
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        if (!keyIPressed) {
            cube.R();   
            keyIPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) {
        keyIPressed = false;
    }
    // R' (right face down)
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        if (!keyKPressed) {
            cube.RPrime();   
            keyKPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_RELEASE) {
        keyKPressed = false;
    }

    // L (left face down)
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        if (!keyDPressed) {
            cube.L();   
            keyDPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_RELEASE) {
        keyDPressed = false;
    }
    // L' (left face up)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        if (!keyEPressed) {
            cube.LPrime();   
            keyEPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_RELEASE) {
        keyEPressed = false;
    }

    // U
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        if (!keyJPressed) {
            cube.U();
            keyJPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_RELEASE) {
        keyJPressed = false;
    }
    // U'
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!keyFPressed) {
            cube.UPrime();
            keyFPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        keyFPressed = false;
    }

    // F
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        if (!keyHPressed) {
            cube.F();   
            keyHPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        keyHPressed = false;
    }
    // F'
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        if (!keyGPressed) {
            cube.FPrime();   
            keyGPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        keyGPressed = false;
    }

    // M'
    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
        if (!keyPeriodPressed) {
            cube.MPrime();   
            keyPeriodPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_RELEASE) {
        keyPeriodPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!keyXPressed) {
            cube.MPrime();   
            keyXPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_RELEASE) {
        keyXPressed = false;
    }
    // M
    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
        if (!keyCommaPressed) {
            cube.M();   
            keyCommaPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_RELEASE) {
        keyCommaPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        if (!keyCPressed) {
            cube.M();   
            keyCPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE) {
        keyCPressed = false;
    }

    // D
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        if (!keySPressed) {
            cube.D();   
            keySPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_RELEASE) {
        keySPressed = false;
    }
    // D'
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!keyLPressed) {
            cube.DPrime();   
            keyLPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        keyLPressed = false;
    }

    // B
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        if (!keyWPressed) {
            cube.B();   
            keyWPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_RELEASE) {
        keyWPressed = false;
    }
    // B'
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        if (!keyOPressed) {
            cube.BPrime();   
            keyOPressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        keyOPressed = false;
    }

    // S
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (!key6Pressed) {
            cube.S();   
            key6Pressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_RELEASE) {
        key6Pressed = false;
    }
    // S'
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (!key5Pressed) {
            cube.SPrime();   
            key5Pressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_RELEASE) {
        key5Pressed = false;
    }

    // E 4
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        if (!key4Pressed) {
            cube.E();   
            key4Pressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
        key4Pressed = false;
    }
    // E' 7
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (!key7Pressed) {
            cube.EPrime();   
            key7Pressed = true; 
        }
    }
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_RELEASE) {
        key7Pressed = false;
    }

    /* Rotate Cube */

    // a and ; for y
    // b and t for x
    // z and / for z
}