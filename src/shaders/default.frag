#version 330 core

in vec3 fragmentCol;

out vec4 screenCol;

void main(){
  screenCol = vec4(fragmentCol, 1.0);
}
