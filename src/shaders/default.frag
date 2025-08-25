#version 330 core

in float vDensity;
out vec4 screenCol;

void main(){
  screenCol =  vec4(vDensity, vDensity, vDensity, 1.0);

}
