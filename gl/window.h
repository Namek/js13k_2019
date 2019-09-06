#pragma once

#include "framework.h"
#include "resource.h"

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

extern HINSTANCE hInst;
extern HDC hDC;
extern HGLRC hRC;

#define CANVAS_WIDTH 1200
#define CANVAS_HEIGHT 675
