// gl.cpp : Defines the entry point for the application.
//

#include "gl.h"
#include "framework.h"
#include "window.h"
#include <cassert>
#include <functional>
#include <vector>

const uint SIZE_FUNC_RETURN = 100;
static int engine_funcReturnValues[SIZE_FUNC_RETURN];
int VALUES_PER_VERTEX, VALUES_PER_COLOR, VALUES_PER_TEXCOORD = 2;
int SIZE_RENDER_BUFFER_COLOR, SIZE_RENDER_BUFFER_VERTEX, SIZE_RENDER_BUFFER_NORMAL, SIZE_RENDER_BUFFER_INDEX;
int SIZE_RENDER_BUFFER_TEXCOORDS, OFFSET_RENDER_BUFFER_COLOR, OFFSET_RENDER_BUFFER_VERTEX, OFFSET_RENDER_BUFFER_INDEX;
int OFFSET_RENDER_BUFFER_TEXCOOR, OFFSET_RENDER_BUFFER_NORMAL, OFFSET_RENDER_BUFFER_TEXCOORDS;

GLuint buffer_vertex, buffer_normal, buffer_index, buffer_color, buffer_texCoords;
float *engine_colorBuffer, *engine_vertexBuffer, *engine_normalBuffer, *engine_texCoordsBuffer;
uint *engine_indexBuffer;
std::vector<int> textures;
std::vector<std::function<void()>> shaderUniforms;
GLuint shaderProgram;

void compileShaders();

void GLAPIENTRY debugProc(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam) {
  if (type == GL_DEBUG_TYPE_ERROR) {
    auto a = randomf();
  }
}

void checkGLError() {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    auto a = randomf();
    //std::cout << err;
  }
}

int main() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.majorVersion = 3;
  settings.minorVersion = 2;
  sf::Window window(sf::VideoMode(CANVAS_WIDTH, CANVAS_HEIGHT, 32), "gl", sf::Style::Titlebar | sf::Style::Close, settings);

  glewExperimental = GL_TRUE;
  glewInit();

  glDebugMessageCallback(debugProc, NULL);

  compileShaders();

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

  engine_colorBuffer = (float *)OFFSET_RENDER_BUFFER_COLOR;
  engine_vertexBuffer = (float *)OFFSET_RENDER_BUFFER_VERTEX;
  engine_indexBuffer = (uint *)OFFSET_RENDER_BUFFER_INDEX;
  engine_texCoordsBuffer = (float *)OFFSET_RENDER_BUFFER_TEXCOORDS;
  engine_normalBuffer = (float *)OFFSET_RENDER_BUFFER_NORMAL;

  initGame();

  bool running = false;

  sf::Clock clock;
  while (running) {
    sf::Event windowEvent;
    while (window.pollEvent(windowEvent)) {
      switch (windowEvent.type) {
      case sf::Event::Closed:
        running = false;
        break;
      }
    }

    sf::Time dt = clock.restart();
    render(dt.asSeconds());
    window.display();
  }

  return 0;
}

GLuint compileShader(GLuint type, const char *code) {
  auto shader = glCreateShader(type);
  GLint lengths[] = {strlen(code)};
  glShaderSource(shader, 1, &code, lengths);
  glCompileShader(shader);

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if (success == GL_FALSE) {
    GLint maxLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);
    glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
    glDeleteShader(shader);
  }

  return shader;
}

GLuint linkShaders(const char *vertCode, const char *fragCode) {
  auto program = glCreateProgram();
  auto vertShader = compileShader(GL_VERTEX_SHADER, vertCode);
  auto fragShader = compileShader(GL_FRAGMENT_SHADER, fragCode);

  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glLinkProgram(program);

  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);

  if (success == GL_FALSE) {
    GLint maxLength = 0;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);
    std::vector<GLchar> errorLog(maxLength);
    glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);
  }

  return program;
}

void compileShaders() {
  // clang-format off
  const char *vertCode =
      "attribute vec3 pos;"
      "attribute vec3 nor;"
      "attribute vec4 color;"
      "attribute vec2 texC;"
      "uniform mat4 projMat;"
      "uniform mat4 viewMat;"
      "uniform mat4 mdlMat;"
      "uniform mat4 norMat;"
      "varying vec3 vNor;" //transposed(inversed(view*mdl))
      "varying vec4 vColor;"
      "varying vec2 vTex;"
      "varying mat4 vMat;"
      "void main(void) {"
        "gl_Position = projMat * viewMat * mdlMat * vec4(pos, 1.0);"
        "vColor = color;"
        "vTex = texC;"
        "vMat = viewMat;"
        "vNor = normalize(vec3(norMat * vec4(nor, 1.0)));"
      "}";

  const char *fragCode =
      "precision mediump float;"
      "varying vec3 vNor;"
      "varying vec4 vColor;"
      "varying vec2 vTex;"
      "varying mat4 vMat;"
      "uniform sampler2D tex;"
      "uniform bool useTex;"
      "uniform float rewind;"
      "vec4 Sepia(vec4 c) {"
        "return vec4("
          "c.r * 0.393 + c.g * 0.769 + c.b * 0.189,"
          "c.r * 0.349 + c.g * 0.686 + c.b * 0.168,"
          "c.r * 0.272 + c.g * 0.534 + c.b * 0.131,"
          "c.a);"
      "}"
      "void main(void) {"
        "vec4 color;"
        "if (useTex) {"
          "color = texture2D(tex, vTex);"
          "color.a = 1.0;"
        "}"
        "else {"
          "color = vColor;"
        "}"
        "vec3 directionToLight = normalize(vec3(0, -0.1, 1));"
        "float lightStrength = max(0.35, dot(directionToLight, vNor));"
        "gl_FragColor.rgb = color.rgb * lightStrength;"
        "gl_FragColor.a = color.a;"
        "if (rewind > 0.0) {"
        "gl_FragColor = rewind * Sepia(gl_FragColor) + (1.0 - rewind) * gl_FragColor;"
        "}"
      "}";
  // clang-format on

  shaderProgram = linkShaders(vertCode, fragCode);
  glUseProgram(shaderProgram);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
  auto attrib = glGetAttribLocation(shaderProgram, "pos");
  glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(attrib);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_normal);
  attrib = glGetAttribLocation(shaderProgram, "nor");
  glVertexAttribPointer(attrib, 3, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(attrib);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
  attrib = glGetAttribLocation(shaderProgram, "color");
  glVertexAttribPointer(attrib, 4, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(attrib);

  glBindBuffer(GL_ARRAY_BUFFER, buffer_texCoords);
  attrib = glGetAttribLocation(shaderProgram, "texC");
  glVertexAttribPointer(attrib, 2, GL_FLOAT, false, 0, 0);
  glEnableVertexAttribArray(attrib);
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
/*LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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
  return DefWindowProc(hWnd, message, wParam, lParam);
  //return 0;
}*/

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
  checkGLError();
  glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
  checkGLError();
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX * sizeof(float), engine_vertexBuffer, GL_STATIC_DRAW);
  //glGetBufferParameteriv(GL_ARRAY_BUFFER,  buffer_vertex)
  checkGLError();
  glBindBuffer(GL_ARRAY_BUFFER, buffer_normal);
  checkGLError();
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX * sizeof(float), engine_normalBuffer, GL_STATIC_DRAW);
  checkGLError();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
  checkGLError();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(int), engine_indexBuffer, GL_STATIC_DRAW);
  checkGLError();
  glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
  checkGLError();
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_COLOR * sizeof(float), engine_colorBuffer, GL_STATIC_DRAW);
  checkGLError();
  glBindBuffer(GL_ARRAY_BUFFER, buffer_texCoords);
  checkGLError();
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_TEXCOORD * sizeof(float), engine_texCoordsBuffer, GL_STATIC_DRAW);
  checkGLError();

  if (currentTextureId >= 0) {
    glBindTexture(GL_TEXTURE_2D, textures[currentTextureId]);
    checkGLError();
  }

  glUseProgram(shaderProgram);
  for (uint i = 0; i < shaderUniforms.size(); ++i) {
    shaderUniforms[i]();
    checkGLError();
  }

  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
  //glDrawArrays(GL_TRIANGLES, 0, indexCount);
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
  assert(loc >= 0);

  if (type == SHADER_UNIFORM_1i) {
    int iv = *(int *)ptr;
    std::function<void()> fun = [loc, iv]() {
      glUniform1i(loc, iv);
    };
    shaderUniforms.push_back(fun);
  }
  else if (type == SHADER_UNIFORM_1f) {
    float fv = *(float *)ptr;
    std::function<void()> fun = [loc, fv]() {
      glUniform1f(loc, fv);
    };
    shaderUniforms.push_back(fun);
  }
  else if (type == SHADER_UNIFORM_Matrix4fv) {
    std::function<void()> fun = [loc, ptr]() {
      glUniformMatrix4fv(loc, 1, false, (float *)ptr);
    };
    shaderUniforms.push_back(fun);
  }
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
