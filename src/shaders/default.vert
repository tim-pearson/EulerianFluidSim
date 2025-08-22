#version 330 core

layout (location=0) in vec2 vertexPos;
layout (location=0) in vec3 vertexCol;

out vec3 fragmentCol;


void main(){
  gl_Position = vec4(vertexPos, 0.0, 1.0);
  fragmentCol = vertexCol;
}
