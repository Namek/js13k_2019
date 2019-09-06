// gl.cpp : Defines the entry point for the application.
//

#include "gl.h"
#include "framework.h"
#include "window.h"
#include <vector>
#include <functional>
#include <cassert>

const int SHADER_UNIFORM_1i = 0, SHADER_UNIFORM_1f = 1, SHADER_UNIFORM_Matrix4fv = 2;

const uint SIZE_FUNC_RETURN = 100;
static int engine_funcReturnValues[SIZE_FUNC_RETURN];
int VALUES_PER_VERTEX, VALUES_PER_COLOR, VALUES_PER_TEXCOORD = 2;
int SIZE_RENDER_BUFFER_COLOR, SIZE_RENDER_BUFFER_VERTEX, SIZE_RENDER_BUFFER_NORMAL, SIZE_RENDER_BUFFER_INDEX;
int SIZE_RENDER_BUFFER_TEXCOORDS, OFFSET_RENDER_BUFFER_COLOR, OFFSET_RENDER_BUFFER_VERTEX, OFFSET_RENDER_BUFFER_INDEX;
int OFFSET_RENDER_BUFFER_TEXCOOR, OFFSET_RENDER_BUFFER_NORMAL, OFFSET_RENDER_BUFFER_TEXCOORDS;

GLuint buffer_vertex, buffer_normal, buffer_index, buffer_color, buffer_texCoords;
float *engine_colorBuffer, *engine_vertexBuffer, *engine_normalBuffer, *engine_texCoordsBuffer, *engine_indexBuffer;
std::vector<int> textures;
std::vector<std::function<void()>> shaderUniforms;
GLuint shaderProgram;

void compileShaders();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);


  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  preinit(&engine_funcReturnValues);
  generateTextures();
  initEngine();

  VALUES_PER_VERTEX = engine_funcReturnValues[1]; //3
  VALUES_PER_COLOR = engine_funcReturnValues[0];  //4
  VALUES_PER_TEXCOORD = 2;
  SIZE_RENDER_BUFFER_COLOR = engine_funcReturnValues[2];
  SIZE_RENDER_BUFFER_VERTEX = engine_funcReturnValues[3];
  SIZE_RENDER_BUFFER_NORMAL = engine_funcReturnValues[4];
  SIZE_RENDER_BUFFER_INDEX = engine_funcReturnValues[5];
  SIZE_RENDER_BUFFER_TEXCOORDS = engine_funcReturnValues[6];
  OFFSET_RENDER_BUFFER_COLOR = engine_funcReturnValues[7];
  OFFSET_RENDER_BUFFER_VERTEX = engine_funcReturnValues[8];
  OFFSET_RENDER_BUFFER_INDEX = engine_funcReturnValues[9];
  OFFSET_RENDER_BUFFER_TEXCOORDS = engine_funcReturnValues[10];
  OFFSET_RENDER_BUFFER_NORMAL = engine_funcReturnValues[11];

  glCreateBuffers(1, &buffer_vertex);
  glCreateBuffers(1, &buffer_normal);
  glCreateBuffers(1, &buffer_index);
  glCreateBuffers(1, &buffer_texCoords);
  glCreateBuffers(1, &buffer_color);

  engine_colorBuffer = (float*)OFFSET_RENDER_BUFFER_COLOR + SIZE_RENDER_BUFFER_COLOR / 4;
  engine_vertexBuffer = (float*)OFFSET_RENDER_BUFFER_VERTEX + SIZE_RENDER_BUFFER_VERTEX / 4;
  engine_indexBuffer = (float *)OFFSET_RENDER_BUFFER_INDEX + SIZE_RENDER_BUFFER_INDEX / 4;
  engine_texCoordsBuffer = (float*)OFFSET_RENDER_BUFFER_TEXCOORDS, SIZE_RENDER_BUFFER_TEXCOORDS / 4;
  engine_normalBuffer = (float *)OFFSET_RENDER_BUFFER_NORMAL, SIZE_RENDER_BUFFER_NORMAL / 4;

  compileShaders();

  initGame();

  MSG msg;
  bool quit = false;
  LARGE_INTEGER freq, prevTime, curTime, elapsed;

  QueryPerformanceFrequency(&freq); 
  QueryPerformanceCounter(&prevTime);
  while (!quit) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        quit = TRUE;
      }
      else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    else {
      QueryPerformanceCounter(&curTime);
      float elapsed = (curTime.QuadPart - prevTime.QuadPart) / (float)freq.QuadPart;
      prevTime.QuadPart = curTime.QuadPart;

      render(elapsed);
      SwapBuffers(hDC);
    }
  }

  return (int)msg.wParam;
}

void compileShaders() {
  /*
    const vertCode =
        'attribute vec3 pos;' +
        'attribute vec3 nor;' +
        'attribute vec4 color;' +
        'attribute vec2 texC;' +
        'uniform mat4 projMat;' +
        'uniform mat4 viewMat;' +
        'uniform mat4 mdlMat;' +
        'uniform mat4 norMat;' +
        'varying vec3 vNor;' + //transposed(inversed(view*mdl))
        'varying vec4 vColor;' +
        'varying vec2 vTex;' +
        'varying mat4 vMat;' +
        'void main(void) {' +
        'gl_Position = projMat * viewMat * mdlMat * vec4(pos, 1.0);' +
        'vColor = color;' +
        'vTex = texC;' +
        'vMat = viewMat;' +
        'vNor = normalize(vec3(norMat * vec4(nor, 1.0)));' +
        '}'

        const fragCode =
            'precision mediump float;' +
            'varying vec3 vNor;' +
            'varying vec4 vColor;' +
            'varying vec2 vTex;' +
            'varying mat4 vMat;' +
            'uniform sampler2D tex;' +
            'uniform bool useTex;' +
            'uniform float rewind;' +
            'vec4 Sepia(vec4 c) {' +
            'return vec4(' +
            'c.r * 0.393 + c.g * 0.769 + c.b * 0.189,' +
            'c.r * 0.349 + c.g * 0.686 + c.b * 0.168,' +
            'c.r * 0.272 + c.g * 0.534 + c.b * 0.131,' +
            'c.a);' +
            '}' +
            'void main(void) {' +
            'vec4 color;' +
            'if (useTex) {' +
            'color = texture2D(tex, vTex);' +
            'color.a = 1.0;' +
            '}' +
            'else {' +
            'color = vColor;' +
            '}' +
            'vec3 directionToLight = normalize(vec3(0, -0.1, 1));' +
            'float lightStrength = max(0.35, dot(directionToLight, vNor));' +
            'gl_FragColor.rgb = color.rgb * lightStrength;' +
            'gl_FragColor.a = color.a;' +
            'if (rewind > 0.0) {' +
            'gl_FragColor = rewind * Sepia(gl_FragColor) + (1.0 - rewind) * gl_FragColor;' +
            '}' +
            '}'
            */
  shaderProgram = linkShaders(gl, vertCode, fragCode);
  glUseProgram(shaderProgram);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
  auto attrib = glGetAttribLocation(shaderProgram, "pos");
  glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(attrib);
  
  glBindBuffer(GL_ARRAY_BUFFER, buffer_normal);
  auto attrib = glGetAttribLocation(shaderProgram, "nor");
  glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(attrib);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
  auto attrib = glGetAttribLocation(shaderProgram, "color");
  glVertexAttribPointer(attrib, 4, GL_FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(attrib);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_texCoords);
  auto attrib = glGetAttribLocation(shaderProgram, "texC");
  glVertexAttribPointer(attrib, 2, GL_FLOAT, false, 0, 0);
  gl.enableVertexAttribArray(attrib);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_DESTROY: {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hWnd, hDC);
    PostQuitMessage(0);
  } break;
  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    // Parse the menu selections:
    switch (wmId) {
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  } break;
  case WM_KEYDOWN: {
    // TODO
    //onEvent(EVENT_KEYDOWN, keyCode)
  } break;
  case WM_KEYUP: {
    // TODO
    //onEvent(EVENT_KEYUP, keyCode)
  } break;
  //case WM_PAINT: {
  //  PAINTSTRUCT ps;
  //  HDC hdc = BeginPaint(hWnd, &ps);
  //  // TODO: Add any drawing code that uses hdc here...
  //  EndPaint(hWnd, &ps);
  //} break;
  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}


float getCanvasWidth() {
  return CANVAS_WIDTH;
}

float getCanvasHeight() {
  return CANVAS_HEIGHT;
}

void clearFrame() {
  glClearColor(0, 1, 0, 1);
  glClearDepth(1);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//performDrawCall
void triggerDrawCall() {
  int vertexCount = engine_funcReturnValues[0];
  int indexCount = engine_funcReturnValues[1];
  int currentTextureId = engine_funcReturnValues[2];

  glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX, engine_vertexBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_normal);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX, engine_normalBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount, engine_normalBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_COLOR, engine_colorBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_texCoords);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_TEXCOORD, engine_texCoordsBuffer, GL_STATIC_DRAW);

  if (currentTextureId >= 0) {
    glBindTexture(GL_TEXTURE_2D, textures[currentTextureId]);
  }

  for (uint i = 0; i < shaderUniforms.size(); ++i) {
    shaderUniforms[i]();
  }

  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// receive texture from engine
void sendTexture(void *ptr, int width, int height) {
  uint texSize = 4 * width * height;
  uint texId;
  glCreateTextures(GL_TEXTURE_2D, 1, &texId);
  glBindTexture(GL_TEXTURE_2D, texId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ptr);
  glGenerateMipmap(GL_TEXTURE_2D);

  textures.push_back(texId);
}

void registerShaderUniform(const char *name, uint type, void *ptr) {
  auto loc = glGetUniformLocation(shaderProgram, name);

  if (type == SHADER_UNIFORM_1i) {
  }
  else if (type == SHADER_UNIFORM_1f) {
  
  }
  else if (type == SHADER_UNIFORM_Matrix4fv) {
    //TODO
  }

  // TODO
}

void _sendTexture() {
}

void _abort(const char *text) {
  assert(false, text);
}
void _l(int num) {
}
void _lf(float num) {
}
void _lstr(const char *text, int num) {
}
void _lfstr(const char *text, float num) {
}

