/*
Cube Project
David Chau
Comp Sci 3 Period 6
This app allows you to scramble and solve a Rubik's cube and see your time.
*/

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <vector>
#include <functional>
#include <random>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace std; 

float width = 1000.0f;
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
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}\n\0";


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

std::map<string, std::function<void()>> possibleMoves;


// THE CODE IS BELOW!

void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void updateCam(float radians);
void processInput(GLFWwindow *window);
void renderText();

class Facelet {
    public:
    GLuint VAO, VBO;
    size_t vertexCount;
    glm::vec4 color;

    Facelet(glm::vec4 color) {
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
    vector<vector<Facelet>> facelets;

    void initializeFacelets() {
        for (int f = 0; f < 6; f++) {
            vector<Facelet> face;
            for (int fl = 0; fl < 9; fl++) {
                face.push_back(Facelet(faceColors[f]));
            }
            facelets.push_back(face);
        }
    }

    void resetFacelets() {
        facelets.clear();
        initializeFacelets();
    }

    private:
    // previous is assigned to next 
    void swapFour(int r1, int c1, int r2, int c2, int r3, int c3, int r4, int c4) {
        Facelet temp = facelets[r1][c1];
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

    public:
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
    // Wide
    void Rw() {
        R();
        MPrime();
    }
    void RwPrime() {
        RPrime();
        M();
    }
    void LwPrime() {
        LPrime();
        MPrime();
    }
    void Lw() {
        L();
        M();
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

    void Y() {
        U(); 
        EPrime(); 
        DPrime();
    }
    void YPrime() {
        UPrime(); 
        E(); 
        D();
    }
    void X() {
        LPrime();
        MPrime();
        R();
    }
    void XPrime() {
        L();
        M();
        RPrime();
    }
    void Z() {
        F();
        S();
        BPrime();
    }
    void ZPrime() {
        FPrime();
        SPrime();
        B();
    }
    
    void scramble() {
        vector<string> movesList = {"R", "R'", "L", "L'", "F", "F'", "B", "B'", "D", "D'", "U", "U'"};

        std::random_device rd; // obtain a random number from hardware
        std::mt19937 gen(rd()); // seed the generator
        std::uniform_int_distribution<> distr(0, movesList.size() - 1); // define the range

        for (int i = 0; i < 40; i++) {
            int index = distr(gen);
            possibleMoves.at(movesList.at(index))();
        }
    }
    
    bool isSolved() {
        for (vector<Facelet> face : facelets) {
            for (int i = 1; i < face.size(); i++) {
                if (face[i].color != face[i - 1].color) {
                    return false;   
                }
            }
        }
        return true;
    }

    void printSolved() {
        cout << isSolved() << endl;
    }
};

enum Mode {
    Title,
    Tutorial,
    Solve
};

enum TutorialPage {

};

/* 2D Classes */

class Rectangle {
    public:
    GLuint VAO, VBO;
    size_t vertexCount;
    glm::vec2 position;
    glm::vec2 size;

    Rectangle(glm::vec2 position, glm::vec2 size) {
        this->position = position;
        this->size = size;

        std::vector<float> vertices = getVertices();
        vertexCount = vertices.size();
        createVBOVAO(VAO, VBO, vertices.data(), vertexCount);
    }

    private:
    std::vector<float> getVertices() {
        std::vector<float> vertices = {
            // first triangle
            position.x, position.y, 0.0f,
            position.x, position.y + size.y, 0.0f,
            position.x + size.x, position.y + size.y, 0.0f,
            // second triangle
            position.x, position.y, 0.0f,
            position.x + size.x, position.y, 0.0f,
            position.x + size.x, position.y + size.y, 0.0f
        };
        return vertices;
    }
};

// text
// FTbrary ft;
// FT_Face face;_Li

struct Character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

std::map<char, Character> characters;

class TextRenderer {
    void load(string font, unsigned int fontSize) {
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char) {

            }
        }
    }


    void renderText(string text, float x, float y, float scale, glm::vec3 color) {

    }
};

void loadFont(const char* filePath, int fontSize) {
    FT_Library fontLibrary;
    if (FT_Init_FreeType(&fontLibrary)) {
        cout << "font library is not good" << endl;
    }

    FT_Face fontFace;
    if (FT_New_Face(fontLibrary, filePath, 0, &fontFace)) {
        cout << "font face is not good" << endl;
    }
    
    FT_Set_Pixel_Sizes(fontFace, 0, fontSize);
}

// Instances

Cube cube;

glm::mat4 view;
glm::mat4 projection;

bool cubeIsSolved = true;
float solveStartTime = 0;

vector<pair<string, float>> performedMoves;

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

    // Set background color
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);


    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "ourColor");

    // reset facelets
    cube.initializeFacelets();

    possibleMoves = {
        {"R", std::bind(&Cube::R, &cube)},
        {"R'", std::bind(&Cube::RPrime, &cube)},
        {"L", std::bind(&Cube::L, &cube)},
        {"L'", std::bind(&Cube::LPrime, &cube)},
        {"U", std::bind(&Cube::U, &cube)},
        {"U'", std::bind(&Cube::UPrime, &cube)},
        {"F", std::bind(&Cube::F, &cube)},
        {"F'", std::bind(&Cube::FPrime, &cube)},
        {"B", std::bind(&Cube::B, &cube)},
        {"B'", std::bind(&Cube::BPrime, &cube)},
        {"D", std::bind(&Cube::D, &cube)},
        {"D'", std::bind(&Cube::DPrime, &cube)},
        {"S", std::bind(&Cube::S, &cube)}, 
        {"S'", std::bind(&Cube::SPrime, &cube)}, 
        {"E", std::bind(&Cube::E, &cube)}, 
        {"E'", std::bind(&Cube::EPrime, &cube)}, 
        {"M", std::bind(&Cube::M, &cube)}, 
        {"M'", std::bind(&Cube::MPrime, &cube)}, 
        {"Rw", std::bind(&Cube::Rw, &cube)}, 
        {"Rw'", std::bind(&Cube::RwPrime, &cube)}, 
        {"Lw", std::bind(&Cube::Lw, &cube)}, 
        {"Lw'", std::bind(&Cube::LwPrime, &cube)}, 
    };
    Rectangle top(glm::vec2(0, 0), glm::vec2(width, 50));

    // text
    loadFont("C:/Users/dchau/Documents/CS3/Project/assets/fonts/arial.ttf", 20);


    while (!glfwWindowShouldClose(window)) {
        float timeValue = glfwGetTime();

        if (cubeIsSolved && !cube.isSolved() && performedMoves.size() == 1) {
            cout << "START!" << endl;
            solveStartTime = timeValue;
            cubeIsSolved = false;
        }
        if (!cubeIsSolved) {
            // display time as text
            // cout << timeValue - solveStartTime << endl;
        }
        if (!cubeIsSolved && cube.isSolved()) {
            cout << "YAY! Solved in " << timeValue - solveStartTime << endl;
            cubeIsSolved = true;
            cout << "Moves: " << performedMoves.size() << endl;
            performedMoves.clear();
        }

        processInput(window);

        /* 3D */

        glEnable(GL_DEPTH_TEST);

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
                Facelet facelet = cube.facelets[f][fl];
                
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
                glDrawArrays(GL_TRIANGLES, 0, facelet.vertexCount / 3);
            }
        }

        /* 2D */

        glDisable(GL_DEPTH_TEST);

        glViewport(0, 0, width, height);

        view = glm::mat4(1.0f);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);  
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


        // do for each 2d element
        glm::mat4 model = glm::mat4(1.0f);
        glUniform4f(colorLoc, 0.5f, 0.6f, 0.7f, 1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glBindVertexArray(top.VAO);
        glDrawArrays(GL_TRIANGLES, 0, top.vertexCount / 3);


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
    cameraPos.y = sin(PI / 3) * radius;
    cameraPos.z = sin(yRad) * radius;

    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
}

std::map<int, bool> keys;

// Use when you want to bind a key to a certain action
void keyInput(GLFWwindow *window, int key, std::function<void()> action) {
    // Add key to map if not there
    if (keys.find(key) == keys.end()) {
        keys[key] = false;
    }
    // When pressed, perform action (holding down does not repeat action)
    if (glfwGetKey(window, key) == GLFW_PRESS) {
        if (!keys[key]) {
            action();
            keys[key] = true;
        }
    } else if (glfwGetKey(window, key) == GLFW_RELEASE) {
        keys[key] = false;
    }
}

// Performs and records the move
void performMove(string moveLetter) {
    possibleMoves[moveLetter]();
    performedMoves.push_back(pair<string, float> {moveLetter, glfwGetTime()});
}

// Use when you want to bind a key to a certain move
void keyInputMove(GLFWwindow *window, int key, string moveLetter) {
    // Add key to map if not there
    if (keys.find(key) == keys.end()) {
        keys[key] = false;
    }
    // When pressed, perform action (holding down does not repeat action)
    if (glfwGetKey(window, key) == GLFW_PRESS) {
        if (!keys[key]) {
            performMove(moveLetter);
            keys[key] = true;
        }
    } else if (glfwGetKey(window, key) == GLFW_RELEASE) {
        keys[key] = false;
    }
}

// Handles key inputs
void processInput(GLFWwindow *window) {
    // Escape button closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } 

    keyInput(window, GLFW_KEY_1, std::bind(&Cube::scramble, &cube));
    keyInput(window, GLFW_KEY_2, std::bind(&Cube::printSolved, &cube));

    /* Move Face */
    // R
    keyInputMove(window, GLFW_KEY_I, "R");
    // R'
    keyInputMove(window, GLFW_KEY_K, "R'");
    // L
    keyInputMove(window, GLFW_KEY_D, "L");
    // L'
    keyInputMove(window, GLFW_KEY_E, "L'");
    // U
    keyInputMove(window, GLFW_KEY_J, "U");
    // U'
    keyInputMove(window, GLFW_KEY_F, "U'");
    // F
    keyInputMove(window, GLFW_KEY_H, "F");
    // F'
    keyInputMove(window, GLFW_KEY_G, "F'");
    // D
    keyInputMove(window, GLFW_KEY_S, "D");
    // D'
    keyInputMove(window, GLFW_KEY_L, "D'");
    // B
    keyInputMove(window, GLFW_KEY_W, "B");
    // B'
    keyInputMove(window, GLFW_KEY_O, "B'");

    /* Wide Moves */
    // Rw
    keyInputMove(window, GLFW_KEY_U, "Rw");
    // Rw'
    keyInputMove(window, GLFW_KEY_M, "Rw'");
    // Lw'
    keyInputMove(window, GLFW_KEY_R, "Lw'");
    // Lw
    keyInputMove(window, GLFW_KEY_V, "Lw");

    /* Slice */
    // M'
    keyInputMove(window, GLFW_KEY_PERIOD, "M'");
    keyInputMove(window, GLFW_KEY_X, "M'");
    // M
    keyInputMove(window, GLFW_KEY_COMMA, "M");
    keyInputMove(window, GLFW_KEY_C, "M");
    // S
    keyInputMove(window, GLFW_KEY_6, "S");
    // S'
    keyInputMove(window, GLFW_KEY_5, "S'");
    keyInputMove(window, GLFW_KEY_4, "E");
    keyInputMove(window, GLFW_KEY_7, "E'");

    /* Rotate Cube */
    // Y'
    keyInput(window, GLFW_KEY_A, std::bind(&Cube::YPrime, &cube));
    // Y
    keyInput(window, GLFW_KEY_SEMICOLON, std::bind(&Cube::Y, &cube));
    // X
    keyInput(window, GLFW_KEY_T, std::bind(&Cube::X, &cube));
    keyInput(window, GLFW_KEY_Y, std::bind(&Cube::X, &cube));
    // X'
    keyInput(window, GLFW_KEY_B, std::bind(&Cube::XPrime, &cube));
    keyInput(window, GLFW_KEY_N, std::bind(&Cube::XPrime, &cube));
    // Z
    keyInput(window, GLFW_KEY_P, std::bind(&Cube::Z, &cube));
    // Z'
    keyInput(window, GLFW_KEY_Q, std::bind(&Cube::ZPrime, &cube));
}