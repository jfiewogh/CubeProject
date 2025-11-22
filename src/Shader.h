#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
    public:
    unsigned int program;

    Shader(const char* vertexPath, const char* fragmentPath) {
        /* Retrieve vertex and fragment source code */
        // Open files
        std::ifstream vertexShaderFile, fragmentShaderFile;
        vertexShaderFile.open(vertexPath);
        fragmentShaderFile.open(fragmentPath);
        // Read files
        std::stringstream vertexShaderStream, fragmentShaderStream;
        vertexShaderStream << vertexShaderFile.rdbuf();
        fragmentShaderStream << fragmentShaderFile.rdbuf();
        // Close files
        vertexShaderFile.close();
        fragmentShaderFile.close();
        // Convert stream into string
        std::string vertexCode = vertexShaderStream.str();
        std::string fragmentCode = fragmentShaderStream.str();
        const char* vertexShaderCode = vertexCode.c_str();
        const char* fragmentShaderCode = fragmentCode.c_str();

        /* Compile shaders */
        unsigned int vertex, fragment;
        // Vertex
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertexShaderCode, NULL);
        glCompileShader(vertex);
        // Fragment
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragmentShaderCode, NULL);
        glCompileShader(fragment);
        // Shader program
        program = glCreateProgram();
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        // Delete shaders
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    // Activate the shader, should be called in main loop
    void use() {
        glUseProgram(program);
    }
};

#endif