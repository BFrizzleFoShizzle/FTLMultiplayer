#pragma once
#include <cstdio>

const float charW = 8.0f;

void readBitmapFont(FILE* file);
void drawChar(float x, float y, char text);
void drawString(float x, float y, char* text);

