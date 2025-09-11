#pragma once
/* const int WIDTH = 6; // needs to be even, or fix div clearance algo */
/* const int HEIGHT = 6; // needs to be even, or fix div clearance algo */
/* const int CELL_SIZE = 30; */

const int WIDTH = 512;  // needs to be even, or fix div clearance algo
const int HEIGHT = 512; // needs to be even, or fix div clearance algo
const int CELL_SIZE = 8;

const int SCREEN_WIDTH = WIDTH * CELL_SIZE;
const int SCREEN_HEIGHT = HEIGHT * CELL_SIZE;

const bool OVERRELAXATION = true;

enum class ColorMapType { Original = 0, CoolWarm, Viridis, Rainbow, COUNT };

