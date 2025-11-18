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

vector<glm::vec4> faceColors = {
    // top
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
    // front
    glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
    // left
    glm::vec4(1.0f, 0.5f, 0.0f, 1.0f),
    // back
    glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),
    // right
    glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
    // bottom
    glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
};


// THE EPIC CODE IS BELOW!

class Cube {
    public:
    vector<vector<FaceletObject>> facelets;

    Cube() {
        // Initialize facelets
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
};

class FaceletObject {
    public:
    GLuint VAO, VBO;
    glm::vec4 color;

    FaceletObject(glm::vec4 color) {
        this->color = color;
    }

    std::vector<float> draw() {
        std::vector<float> vertices = {
            0.5f,  0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f
        };
        return vertices;
    }

    void updateVertices() {
        std::vector<float> vertices = draw();
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data());
    }
};

struct Facelet {
    glm::vec4 color;
    glm::vec3 position;
};

struct Face {
    // 0 is top left
    // 8 is bottom right
    vector<Facelet> facelets;
};

vector<Face> faces(6);

void processInput(GLFWwindow *window) {
    // escape button closes window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    } 

    // define cube interact buttons
    // only when in solving mode

    // R (right face clockwise/up)
    else if (glfwGetKey(window, GLFW_KEY_I)) {       
    }
    // R' (right face counterclockwise/down)
    else if (glfwGetKey(window, GLFW_KEY_K)) {
    }
    // L
    else if (glfwGetKey(window, GLFW_KEY_E)) {
    }
    // L'
    else if (glfwGetKey(window, GLFW_KEY_D)) {
    }
    // U
    else if (glfwGetKey(window, GLFW_KEY_J)) {
    }
    // U'
    else if (glfwGetKey(window, GLFW_KEY_F)) {
    }
    // F
    else if (glfwGetKey(window, GLFW_KEY_H)) {
    }
    // F'
    else if (glfwGetKey(window, GLFW_KEY_G)) {
    }
}

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

    // positions of each facelet relative to center
    glm::vec3 positions[] = {
        glm::vec3(-0.5f, -0.5f, 0.5f),
        glm::vec3(-0.5f, 0.0f, 0.5f),
        glm::vec3(-0.5f, 0.5f, 0.5f),
        glm::vec3(0.0f, -0.5f, 0.5f),
        glm::vec3(0.0f, 0.0f, 0.5f),
        glm::vec3(0.0f, 0.5f, 0.5f),
        glm::vec3(0.5f, -0.5f, 0.5f),
        glm::vec3(0.5f, 0.0f, 0.5f),
        glm::vec3(0.5f, 0.5f, 0.5f),
    };


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


    Cube cube;



    float vertices[] = {
        // positions
        0.5f,  0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    // Set background color
    glClearColor(0.25f, 0.5f, 0.75f, 1.0f);


    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint colorLoc = glGetUniformLocation(shaderProgram, "ourColor");


    while (!glfwWindowShouldClose(window)) {
        processInput(window);


        float timeValue = glfwGetTime();



        const float radius = 10.0f;
        float camX = sin(timeValue) * radius;
        float camZ = cos(timeValue) * radius;
        glm::mat4 view;
        view = glm::lookAt(glm::vec3(camX, 0.0, camZ), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);

        // clear color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // activate shader
        glUseProgram(shaderProgram);

        // update uniform matrix
        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // bind vertex array
        glBindVertexArray(VAO);

        cube.updateVertices();

        // loop through faces
        for (unsigned int f = 0; f < 6; f++) {
            Face face = faces[f];
            glm::vec4 color = faceColors[f];
            glUniform4f(colorLoc, color.r, color.g, color.b, 1.0f);

            // should be based on the face
            glm::vec3 rotation = faceRotations[f];
            float xRadians = glm::radians(rotation.x);
            float yRadians = glm::radians(rotation.y);
            float zRadians = glm::radians(rotation.z);
    
            // loop through facelets
            for (unsigned int i = 0; i < 9; i++) {
                glm::mat4 model = glm::mat4(1.0f);

                // rotate x
                model = glm::rotate(model, xRadians, glm::vec3(1.0f, 0.0f, 0.0f));
                // rotate y
                model = glm::rotate(model, yRadians, glm::vec3(0.0f, 1.0f, 0.0f));
                // rotate z
                model = glm::rotate(model, zRadians, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::translate(model, positions[i]);     
                
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }

        // swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();    
    }

    glfwTerminate();
    return 0;
}