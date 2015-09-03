#pragma once

void drawRect (float x,float y,float w,float h);
void drawTriangle (float x1,float y2,float x2,float y1,float x3,float y3);
void addDrawHook(void (void));
void hookGLFinish(void);