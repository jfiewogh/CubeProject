/*
Cube Project
David Chau
Comp Sci 3 Period 6
This app allows you to scramble and solve a Rubik's cube and see your time and move count.
*/

/*
CONTROLS
I - right face up
K - right face down
E - left face up
D - left face down
J - top face clockwise
F - top face counterclockwise
H - front face clockwise
G - front face counterclockwise
S - bottom face clockwise
L - bottom face counterclockwise
W - back face clockwise
O - back face counterclockwise

U - right and middle face up
M - right and middle face down
R - left and middle face up
V - left and middle face down

. or X - middle face up
, or C - middle face down
6 - standing layer clockwise (same direction as front)
5 - standing layer counterclockwise
4 - equatorial layer clockwise (same direction as bottom face)
7 - equatorial layer counterclockwise

A - counterclockwise rotation on Y axis (same direction as top face)
; - clockwise rotation on Y axis
T or Y - clockwise rotation on X axis (same direction as right face)
B or N - counterclockwise rotation on X axis
P - clockwise rotation on Z axis (same direction as front face)
Q - counterclockwise on Z axis

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
#include "Shader.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

using namespace std; 

const float width = 1500.0f;
const float height = 1000.0f;

const double PI = 3.14159265358979323846;

const double turnSpeed = 10.0;
const double turnTime = 1.0 / turnSpeed;

const double cubeViewAngle = PI / 3;


const vector<glm::vec3> faceRotations = {
    glm::vec3(-90, 0, 0),
    glm::vec3(0, 0, 0),
    glm::vec3(0, 90, 0),
    glm::vec3(0, 180, 0),
    glm::vec3(0, 270, 0),
    glm::vec3(90, 0, 0),
};

// starting face colors
const vector<glm::vec4> faceColors = {
    // top, white
    glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
    // front, green
    glm::vec4(0.0f, 0.61f, 0.28f, 1.0f),
    // right, red
    glm::vec4(0.72f, 0.07f, 0.20f, 1.0f),
    // back, blue
    glm::vec4(0.0f, 0.27f, 0.68f, 1.0f),
    // left, orange
    glm::vec4(1.0f, 0.345f, 0.0f, 1.0f),
    // bottom, yellow
    glm::vec4(1.0f, 0.835f, 0.0f, 1.0f)
};

const vector<glm::vec2> faceletOffsets = {
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

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


std::map<string, std::function<void()>> possibleMoves;
std::map<string, std::function<void()>> possibleRotations;

std::map<int, string> keyMoves;
std::map<int, string> keyRotations;

void createVBOVAO(GLuint& VAO, GLuint& VBO, const float* vertices, size_t vertexCount);
void updateCam(float radians, float viewAngle);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

void performMove(string moveLetter);

class Facelet {
    public:
    GLuint VAO, VBO;
    size_t vertexCount;
    glm::vec4 color;

    private:
    glm::vec3 previousRotation;
    glm::vec3 desiredRotation;
    float lastMoveTime;

    public:
    int faceIndex;
    int faceletIndex;

    public:
    Facelet(glm::vec4 color) {
        this->color = color;

        std::vector<float> vertices = getVertices();
        vertexCount = vertices.size();
        createVBOVAO(VAO, VBO, vertices.data(), vertexCount);
    }

    private:
    std::vector<float> getVertices() {
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

    public:
    glm::vec3 getPosition(int faceIndex, int faceletIndex) {
        glm::vec2 offset = faceletOffsets[faceletIndex];
        return glm::vec3(offset.x, offset.y, 1.6f);
    }
    glm::vec3 getRotation(int faceIndex) {
        this->faceIndex = faceIndex;
        return faceRotations[faceIndex];
    }

    // SMOOTH ROTATION
    // currently unused
    glm::vec3 getSmoothRotation(int faceIndex, float time) {
        cout << this->faceIndex << " " << faceIndex << endl;
        this->faceIndex = faceIndex;

        glm::vec3 rotation = getRotation(faceIndex);

        if (rotation != desiredRotation) {
            previousRotation = desiredRotation;
            desiredRotation = rotation;
            lastMoveTime = time;
        }
        double interpolation = std::min((time - lastMoveTime) / turnTime, 1.0);
        // cout << interpolation << endl;

        // requires knowing the shortest rotation between two points
        // 
        //
        // glm::vec3 currentRotation = glm::vec3(
        //     std::lerp(),
        //     std::lerp(),
        //     std::lerp()
        // );
        return rotation;
    }
};

class Cube {
    public:
    vector<vector<Facelet>> facelets;

    void initializeFacelets() {
        for (int f = 0; f < 6; f++) {
            vector<Facelet> face;
            for (int fl = 0; fl < 9; fl++) {
                Facelet facelet = Facelet(faceColors[f]);
                facelet.faceIndex = f;
                face.push_back(facelet);
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
        Facelet* first = &facelets[r1][c1];
        Facelet* second = &facelets[r2][c2];
        Facelet* third = &facelets[r3][c3];
        Facelet* fourth = &facelets[r4][c4];
        Facelet temp = *first;
        facelets[r1][c1] = *second;
        facelets[r2][c2] = *third;
        facelets[r3][c3] = *fourth;
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

/* 2D Classes */

class Rectangle {
    public:
    GLuint VAO, VBO;
    size_t vertexCount;
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;

    Rectangle(glm::vec2 position, glm::vec2 size, glm::vec4 color) {
        this->position = position;
        this->size = size;
        this->color = color;

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

    public:
    void draw(GLint& modelLoc, GLint& colorLoc) {
        // set model
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // set color
        glUniform4f(colorLoc, color.r, color.g, color.b, color.a);

        // set vertices
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertexCount / 3);
    }
};

class Button: public Rectangle {
    public:
    std::function<void()> action;

    Button(glm::vec2 position, glm::vec2 size, glm::vec4 color, std::function<void()> action) 
    : Rectangle(position, size, color) {
        this->action = action;
    }

    bool mouseIsHover(double x, double y) {
        y = height - y; // 2D graphics have y flipped (y = 0 at bottom instead of top)
        return x > position.x && x < position.x + size.x && y > position.y && y < position.y + size.y;
    }

    void runAction() {
        action();
    }
};

struct Character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

std::map<char, Character> characters;

class TextRenderer {
    public:
    unsigned int VAO, VBO;

    TextRenderer() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);  
    }

    void loadFont(const char* filePath, int fontSize) {
        FT_Library fontLibrary;
        if (FT_Init_FreeType(&fontLibrary)) {
            cout << "Library init failed" << endl;
        }

        FT_Face fontFace;
        if (FT_New_Face(fontLibrary, filePath, 0, &fontFace)) {
            cout << "Failed to load font" << endl;
        }
        
        FT_Set_Pixel_Sizes(fontFace, 0, fontSize);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // load character glyph
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(fontFace, c, FT_LOAD_RENDER)) {
                cout << "Failed to load glyph" << endl;
                continue;
            }
            // generate texture
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                fontFace->glyph->bitmap.width,
                fontFace->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                fontFace->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture, 
                glm::ivec2(fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows),
                glm::ivec2(fontFace->glyph->bitmap_left, fontFace->glyph->bitmap_top),
                static_cast<unsigned int>(fontFace->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));
        }

        glBindTexture(GL_TEXTURE_2D, 0);

        FT_Done_Face(fontFace);
        FT_Done_FreeType(fontLibrary);
    }

    void renderText(Shader &shader, string text, float x, float y, float scale, glm::vec3 color) {
        glUniform3f(
            glGetUniformLocation(shader.program, "textColor"), 
            color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++) {
            Character character = characters[*c];

            float xPos = x + character.bearing.x * scale;
            float yPos = y - (character.size.y - character.bearing.y) * scale;

            float width = character.size.x * scale;
            float height = character.size.y * scale;

            // Update VBO
            float vertices[6][4] = {
                {xPos, yPos + height, 0.0f, 0.0f},
                {xPos, yPos, 0.0f, 1.0f},
                {xPos + width, yPos, 1.0f, 1.0f},

                {xPos, yPos + height, 0.0f, 0.0f},
                {xPos + width, yPos, 1.0f, 1.0f},
                {xPos + width, yPos + height, 1.0f, 0.0f}
            };

            glBindTexture(GL_TEXTURE_2D, character.textureID);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (character.advance >> 6) * scale;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

Cube cube;

Mode currentMode = Mode::Title;

glm::mat4 view;
glm::mat4 projection;

bool cubeIsSolved = true;
float solveStartTime = 0;

vector<pair<string, float>> performedMoves;

struct SolveResult {
    float time; // seconds
    unsigned int moveCount;
};

std::vector<SolveResult> solves;

vector<Button> titleButtons;
vector<Button> solveButtons;

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

    // Create cube

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
        {"Lw'", std::bind(&Cube::LwPrime, &cube)}
    };

    possibleRotations = {
        {"Y'", std::bind(&Cube::YPrime, &cube)},
        {"Y", std::bind(&Cube::Y, &cube)},
        {"X", std::bind(&Cube::X, &cube)},
        {"X'", std::bind(&Cube::XPrime, &cube)},
        {"Z", std::bind(&Cube::Z, &cube)},
        {"Z'", std::bind(&Cube::ZPrime, &cube)}
    };

    keyMoves = {
        // Face
        {GLFW_KEY_I, "R"},
        {GLFW_KEY_K, "R'"},
        {GLFW_KEY_D, "L"},
        {GLFW_KEY_E, "L'"},
        {GLFW_KEY_J, "U"},
        {GLFW_KEY_F, "U'"},
        {GLFW_KEY_H, "F"},
        {GLFW_KEY_G, "F'"},
        {GLFW_KEY_S, "D"},
        {GLFW_KEY_L, "D'"},
        {GLFW_KEY_W, "B"},
        {GLFW_KEY_O, "B'"},
        // Wide
        {GLFW_KEY_U, "Rw"},
        {GLFW_KEY_M, "Rw'"},
        {GLFW_KEY_R, "Lw'"},
        {GLFW_KEY_V, "Lw"},
        // Slice
        {GLFW_KEY_PERIOD, "M'"},
        {GLFW_KEY_X, "M'"},
        {GLFW_KEY_COMMA, "M"},
        {GLFW_KEY_C, "M"},
        {GLFW_KEY_6, "S"},
        {GLFW_KEY_5, "S'"},
        {GLFW_KEY_4, "E"},
        {GLFW_KEY_7, "E'"},
    };

    keyRotations = {
        // Rotations
        {GLFW_KEY_A, "Y'"},
        {GLFW_KEY_SEMICOLON, "Y"},
        {GLFW_KEY_T, "X"},
        {GLFW_KEY_Y, "X"},
        {GLFW_KEY_B, "X'"},
        {GLFW_KEY_N, "X'"},
        {GLFW_KEY_P, "Z"},
        {GLFW_KEY_Q, "Z'"}
    };

    cube.initializeFacelets();
    

    // Create shader programs
    // relative path not working for some reason

    Shader shader(
        "C:/Users/dchau/Documents/CS3/Project/src/shaders/vertex.txt", 
        "C:/Users/dchau/Documents/CS3/Project/src/shaders/fragment.txt");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader textShader(
        "C:/Users/dchau/Documents/CS3/Project/src/shaders/textVertex.txt", 
        "C:/Users/dchau/Documents/CS3/Project/src/shaders/textFragment.txt");


    // Set background color
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);


    // Get locs
    GLint modelLoc = glGetUniformLocation(shader.program, "model");
    GLint colorLoc = glGetUniformLocation(shader.program, "fillColor");
    GLint viewLoc = glGetUniformLocation(shader.program, "view");
    GLint projectionLoc = glGetUniformLocation(shader.program, "projection");


    /* Initialize graphics */

    // Title
    Rectangle titleBackground(glm::vec2(0, 0), glm::vec2(width, height), glm::vec4(0.25f, 0.25f, 0.25f, 0.4f));

    Button solveModeButton(glm::vec2(50.0f, height - 250.0f), glm::vec2(400, 100), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), []() {
        currentMode = Mode::Solve;
    });
    Button tutorialModeButton(glm::vec2(50.0f, height - 400.0f), glm::vec2(400, 100), glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), []() {
        cout << "no tutorial mode" << endl;
        // currentMode = Mode::Tutorial;
    });
    titleButtons.push_back(solveModeButton);
    titleButtons.push_back(tutorialModeButton);
    
    // Solve
    Rectangle topBar(glm::vec2(0, height - 55), glm::vec2(width, 55), glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
    Button backToTitleButton(glm::vec2(0.0f, height - 55.0f), glm::vec2(55.0f, 55.0f), glm::vec4(0.0f, 1.0f, 1.0f, 1.0f), []() {
        currentMode = Mode::Title;
    });
    solveButtons.push_back(backToTitleButton);


    // Initialize text renderer
    TextRenderer textRenderer;
    textRenderer.loadFont("C:/Users/dchau/Documents/CS3/Project/assets/fonts/LATO-BOLD.TTF", 100);


    // Main loop

    Facelet marker = cube.facelets[0][0];

    while (!glfwWindowShouldClose(window)) {
        float timeValue = glfwGetTime();

        glfwSetKeyCallback(window, keyCallback);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);

        // Handle solving
        if (cubeIsSolved && !cube.isSolved() && performedMoves.size() == 1) {
            cout << "START!" << endl;
            solveStartTime = timeValue;
            cubeIsSolved = false;
        }
        if (!cubeIsSolved && cube.isSolved()) {
            cubeIsSolved = true;

            float time = timeValue - solveStartTime;
            cout << "Yay! Solved in " << timeValue - solveStartTime << endl;

            unsigned int moveCount = performedMoves.size();
            cout << "Moves: " << moveCount << endl;

            cout << "Moves per Sec: " << moveCount / time << endl;

            SolveResult solve;
            solve.time = time;
            solve.moveCount = moveCount;
            solves.push_back(solve);

            performedMoves.clear();
        } 

        // clear color buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // activate shader
        shader.use();

        /* 3D */

        glEnable(GL_DEPTH_TEST);

        
        if (currentMode == Mode::Title) {
            // cool
            updateCam(timeValue, timeValue);
        } else {
            updateCam(PI / 2, cubeViewAngle);
        }

        // update uniform matrix
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (int f = 0; f < cube.facelets.size(); f++) {
            for (int fl = 0; fl < cube.facelets[0].size(); fl++) {
                Facelet* pointer = &cube.facelets[f][fl];
                Facelet facelet = *pointer;
                
                glm::mat4 model = glm::mat4(1.0f);
                
                glm::vec3 rotation;
                // if (pointer == &marker) {
                //     rotation = facelet.getSmoothRotation(f, timeValue);
                // } else {
                    rotation = facelet.getRotation(f);
                // }

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

        cout << marker.faceIndex << endl;

        /* 2D */


        // Set up shader

        glDisable(GL_DEPTH_TEST);

        glViewport(0, 0, width, height);

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

        projection = glm::ortho(0.0f, width, 0.0f, height);  
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


        // Draw 2D graphics

        switch (currentMode) {
            case Title:
                titleBackground.draw(modelLoc, colorLoc);
                solveModeButton.draw(modelLoc, colorLoc);
                tutorialModeButton.draw(modelLoc, colorLoc);
                break;

            case Tutorial:
                break;

            case Solve:
                topBar.draw(modelLoc, colorLoc);
                backToTitleButton.draw(modelLoc, colorLoc);
        }

        // Draw 2D text

        textShader.use();

        glUniformMatrix4fv(
            glGetUniformLocation(textShader.program, "projection"), 
            1, 
            GL_FALSE, 
            glm::value_ptr(projection));

        switch (currentMode) {
            case Title:
                textRenderer.renderText(textShader, 
                    "Welcome to the Cubing App", 
                    15.0f, 
                    height - 75.0f, 
                    0.75f, 
                    glm::vec3(1.0f, 0.0f, 1.0f));

                textRenderer.renderText(textShader, 
                    "Solve", 
                    solveModeButton.position.x + 10.0f, 
                    solveModeButton.position.y + 10.0f, 
                    0.75f, 
                    glm::vec3(0.0f, 0.0f, 0.0f));

                textRenderer.renderText(textShader, 
                    "Tutorial", 
                    tutorialModeButton.position.x + 10.0f, 
                    tutorialModeButton.position.y + 10.0f, 
                    0.75f, 
                    glm::vec3(0.0f, 0.0f, 0.0f));

                break;

            case Tutorial:
                break;

            case Solve:
                textRenderer.renderText(textShader, "<", 7.0f, height - 55.0f, 0.75f, glm::vec3(0.0f, 0.0f, 0.0f));

                textRenderer.renderText(textShader, "Solve Mode", 65.0f, height - 45.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

                if (!cubeIsSolved) {
                    
                    // display time as text
                    float roundedTime = std::round((timeValue - solveStartTime) * 1000.0f) / 1000.0f;
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(3) << roundedTime;
                    string solveTime = stream.str();

                    textRenderer.renderText(
                        textShader,
                        solveTime, 
                        5.0f, 5.0f, 0.75f, glm::vec3(1.0f, 1.0f, 1.0f));

                    // display move count
                    textRenderer.renderText(
                        textShader,
                        "Move Count: " + std::to_string(performedMoves.size()), 
                        width * 0.3f, 5.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
                } else {
                    textRenderer.renderText(
                        textShader,
                        "Press esc to reset the cube", 
                        5.0f, 105.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

                    textRenderer.renderText(
                        textShader,
                        "Press space to scramble", 
                        5.0f, 55.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));

                    textRenderer.renderText(
                        textShader,
                        "Perform any move (excluding cube rotation) to start the timer", 
                        5.0f, 5.0f, 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
                }

                break;
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

// for 3D
void updateCam(float yRad, float viewAngle) {
    const float radius = 7.0f;

    cameraPos.x = cos(yRad) * radius;
    cameraPos.y = sin(viewAngle) * radius;
    cameraPos.z = sin(yRad) * radius;

    view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
}

// Performs and records the move
void performMove(string moveLetter) {
    possibleMoves[moveLetter]();
    performedMoves.push_back(pair<string, float> {moveLetter, glfwGetTime()});
}

// Handles key inputs
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && currentMode == Mode::Solve) {
        // Escape button closes window
        if (key == GLFW_KEY_ESCAPE) {
            cube.resetFacelets();
        // Space scrambles the cube
        } else if (key == GLFW_KEY_SPACE) {
            cube.scramble();
        // Perform move
        } else if (keyMoves.find(key) != keyMoves.end()) {
            performMove(keyMoves.at(key));
        // Perform rotation
        } else if (keyRotations.find(key) != keyRotations.end()) {
            possibleRotations.at(keyRotations.at(key))();
        }
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);

        switch (currentMode) {
            case Title:
                for (Button button : titleButtons) {
                    if (button.mouseIsHover(xPos, yPos)) {
                        button.runAction();
                    }
                }
                break;
            case Solve:
                for (Button button : solveButtons) {
                    if (button.mouseIsHover(xPos, yPos)) {
                        button.runAction();
                    }
                }
                break;
        }
    }
}