// header.h : include file for standard system include files,
// or project specific include files
//

#pragma once

#include "../src/wasm/common.hpp"

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

//#include <windows.h>

// C RunTime Header Files
/*#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>*/

#include <GL/glew.h> 
#pragma comment(lib,"opengl32.lib")
//#include <GL/glu.h>
//#include <GL\glaux.h> 

#define SFML_STATIC
#include <SFML/Window.hpp>
