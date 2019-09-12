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

GLuint vertexArrayObject, buffer_vertex, buffer_normal, buffer_index, buffer_color, buffer_texCoords;
float *engine_colorBuffer, *engine_vertexBuffer, *engine_normalBuffer, *engine_texCoordsBuffer;
uint *engine_indexBuffer;
std::vector<GLuint> textures;
std::vector<std::function<void()>> shaderUniforms;
GLuint shaderProgram, vertShader, fragShader;

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
    printf(message);
  }
}

void checkGLError() {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    auto a = randomf();
    printf("gl error: %d", err);
  }
}

int main() {
  sf::ContextSettings settings;
  settings.depthBits = 24;
  settings.stencilBits = 8;
  settings.majorVersion = 3;
  settings.minorVersion = 2;
  sf::Window window(sf::VideoMode(CANVAS_WIDTH, CANVAS_HEIGHT, 32), "gl", sf::Style::Titlebar | sf::Style::Close, settings);

  glewExperimental = GL_TRUE;
  glewInit();
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(debugProc, NULL);

  
  glGenVertexArrays(1, &vertexArrayObject);
  glBindVertexArray(vertexArrayObject);
  glGenBuffers(1, &buffer_vertex);
  glGenBuffers(1, &buffer_normal);
  glGenBuffers(1, &buffer_index);
  glGenBuffers(1, &buffer_texCoords);
  glGenBuffers(1, &buffer_color);

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

  engine_colorBuffer = (float *)OFFSET_RENDER_BUFFER_COLOR;
  engine_vertexBuffer = (float *)OFFSET_RENDER_BUFFER_VERTEX;
  engine_indexBuffer = (uint *)OFFSET_RENDER_BUFFER_INDEX;
  engine_texCoordsBuffer = (float *)OFFSET_RENDER_BUFFER_TEXCOORDS;
  engine_normalBuffer = (float *)OFFSET_RENDER_BUFFER_NORMAL;

  initGame();


  bool running = true;

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

  for (auto const &tex : textures)
    glDeleteTextures(1, &tex);

  glDeleteProgram(shaderProgram);
  glDeleteShader(fragShader);
  glDeleteShader(vertShader);

  glDeleteBuffers(1, &buffer_color);
  glDeleteBuffers(1, &buffer_index);
  glDeleteBuffers(1, &buffer_normal);
  glDeleteBuffers(1, &buffer_texCoords);
  glDeleteBuffers(1, &buffer_vertex);

  glDeleteVertexArrays(1, &vertexArrayObject);
  window.close();

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
    printf(errorLog.data());
    glDeleteShader(shader);
  }

  return shader;
}

GLuint linkShaders(const char *vertCode, const char *fragCode) {
  auto program = glCreateProgram();
  vertShader = compileShader(GL_VERTEX_SHADER, vertCode);
  fragShader = compileShader(GL_FRAGMENT_SHADER, fragCode);

  glAttachShader(program, vertShader);
  glAttachShader(program, fragShader);
  glBindFragDataLocation(program, 0, "outColor");
  glLinkProgram(program);
  glUseProgram(program);

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
  const char *vertCode = R"glsl(
      #version 150 core
      in vec3 pos;
      in vec3 nor;
      in vec4 color;
      in vec2 texC;
      uniform mat4 projMat;
      uniform mat4 viewMat;
      uniform mat4 mdlMat;
      uniform mat4 norMat;
      out vec3 vNor; //transposed(inversed(view*mdl))
      out vec4 vColor;
      out vec2 vTex;
      out mat4 vMat;
      void main() {
        gl_Position = projMat * viewMat * mdlMat * vec4(pos, 1.0);
        vColor = color;
        vTex = texC;
        vMat = viewMat;
        vNor = normalize(vec3(norMat * vec4(nor, 1.0)));
      }
  )glsl";

  const char *fragCode = R"glsl(
      #version 150 core
      precision mediump float;
      in vec3 vNor;
      in vec4 vColor;
      in vec2 vTex;
      in mat4 vMat;
      uniform sampler2D tex;
      uniform bool useTex;
      uniform float rewind;
      out vec4 outColor;

      vec4 Sepia(vec4 c) {
        return vec4(
          c.r * 0.393 + c.g * 0.769 + c.b * 0.189,
          c.r * 0.349 + c.g * 0.686 + c.b * 0.168,
          c.r * 0.272 + c.g * 0.534 + c.b * 0.131,
          c.a);
      }
      void main() {
        vec4 color;
        if (useTex) {
          color = texture2D(tex, vTex);
          color.a = 1.0;
        }
        else {
          color = vColor;
        }
        vec3 directionToLight = normalize(vec3(0, -0.1, 1));
        float lightStrength = max(0.35, dot(directionToLight, vNor));
        outColor.rgb = color.rgb * lightStrength;
        outColor.a = color.a;
        if (rewind > 0.0) {
          outColor = rewind * Sepia(outColor) + (1.0 - rewind) * outColor;
        }
      };
  )glsl";
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
  int currentTextureIndex = engine_funcReturnValues[2];
  glBindBuffer(GL_ARRAY_BUFFER, buffer_vertex);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX * sizeof(float), engine_vertexBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_normal);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_VERTEX * sizeof(float), engine_normalBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_index);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(int), engine_indexBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_color);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_COLOR * sizeof(float), engine_colorBuffer, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, buffer_texCoords);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * VALUES_PER_TEXCOORD * sizeof(float), engine_texCoordsBuffer, GL_STATIC_DRAW);

  if (currentTextureIndex >= 0) {
    glBindTexture(GL_TEXTURE_2D, textures[currentTextureIndex]);
  }

  glUseProgram(shaderProgram);
  for (uint i = 0; i < shaderUniforms.size(); ++i) {
    shaderUniforms[i]();
  }

  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
}

// receive texture from engine
void sendTexture(void *ptr, int width, int height) {
  uint texSize = 4 * width * height;
  uint texId;
  glGenTextures(1, &texId);
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
