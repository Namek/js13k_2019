document.addEventListener("DOMContentLoaded", (event) => {
  const
    log = console.log
  , canvas = document.getElementById("c")

  //removeIf(!production)
  , DEBUG_TEXTURES = false
  , DEBUG_TWEAK = false
  //endRemoveIf(!production)
  //removeIf(production)
  , DEBUG_TEXTURES = true
  , DEBUG_TWEAK = true
  //endRemoveIf(production)

  , gl_BUFFER_COLOR_BIT = 16384
  , gl_DEPTH_BUFFER_BIT = 256
  , gl_LEQUAL = 515
  , gl_TRIANGLES = 4
  , gl_STATIC_DRAW = 35044
  , gl_ELEMENT_ARRAY_BUFFER = 34963
  , gl_ARRAY_BUFFER = 34962
  , gl_COMPILE_STATUS = 35713
  , gl_FLOAT = 5126
  , gl_DEPTH_TEST = 2929
  , gl_LINK_STATUS = 35714
  , gl_VERTEX_SHADER = 35633
  , gl_FRAGMENT_SHADER = 35632
  , gl_UNSIGNED_INT = 5125
  , gl_TEXTURE_2D = 3553
  , gl_RGB = 6407
  , gl_RGBA = 6408
  , SHADER_UNIFORM_1i = 0
  , SHADER_UNIFORM_1f = 1
  , SHADER_UNIFORM_Matrix4fv = 2
  , NUM_MATRIX4 = 16
  , EVENT_KEYDOWN = 1
  , EVENT_KEYUP = 2

  let
    dumbFunction = () => {}
  , clearFrame = dumbFunction
  , performDrawCall = dumbFunction
  , receiveTexture = dumbFunction
  , registerShaderUniform = dumbFunction

  let
    allocatedPages = 256     //65k-sized WebAssembly pages
  , dynamicMemoryOffset = -1 //start of the program's heap, free to manage by malloc()
  , dynamicMemoryBreak = -1  //refers to "programbreak" as in native sbrk()

  const
    memory = new WebAssembly.Memory({initial:allocatedPages, maximum:allocatedPages})
  , MEMORY_PAGE_SIZE = 65536
  , MEMORY_BASE = MEMORY_PAGE_SIZE
  , HEAP_START = 2*65536
  , STACKTOP = 65536/2

  // implementation mimicking unistd.h::sbrk() for growing and shrinking program's memory
  const _sbrk = (size) => {
    let ret = dynamicMemoryBreak

    if (size != 0) {
      ret += (16 - dynamicMemoryBreak % 16)
      dynamicMemoryBreak = ret + size;

      if (dynamicMemoryBreak > MEMORY_PAGE_SIZE*allocatedPages - dynamicMemoryOffset) {
        allocatedPages += 1
        //removeIf(production)
        throw new Error()
        //endRemoveIf(production)

        // TODO memory.grow(pages)
      }
      else if (size < 0) {
        //removeIf(production)
        throw new Error()
        //endRemoveIf(production)

        // TODO on some condition, memory.drop(pages)
      }
    }

    return ret;
  }

  const readTextFromMemory = (strPtr) => {
    let str = "";
    let buf = new Uint8Array(memory.buffer, strPtr, 100)

    let i = -1
    while (buf[++i] != 0)
      str += String.fromCharCode(buf[i])

    return str
  }
  //removeIf(production)
  const abort = (strPtr) => {
    throw new Error(readTextFromMemory(strPtr))
  }
  const debugValues = {
    // key is name, value is DOM element
  }
  const logStr = (strPtr, num) => {
    let str = readTextFromMemory(strPtr)
    let el = debugValues[str]
    if (!el) {
      el = document.createElement("p")
      debugValues[str] = el
      // `dv` is div instantiated in .html
      dv.appendChild(el)
    }
    el.innerText = "- " + str + ': ' + num
    // log("logstr:", str, num);
  }
  //endRemoveIf(production)
  
  const loadProgram = (wasmProgram) => {
    const getCanvasWidth = () => canvas.width
    const getCanvasHeight = () => canvas.height
    adjustCanvasSizeToHighDpi(canvas)

    const env = {
      __memory_base: MEMORY_BASE,
      __table_base: 0,
      STACKTOP,
      memory,
      table: new WebAssembly.Table({ initial: 0, element: 'anyfunc' }),
      _getCanvasWidth:getCanvasWidth,
      _getCanvasHeight:getCanvasHeight,
      //removeIf(production)
      __abort: abort,
      __l: log, //int
      __lf: log,//float
      __lstr: logStr,
      __lfstr: logStr,
      //endRemoveIf(production)
      _randomf: Math.random,
      _clearFrame: () => clearFrame(),
      _triggerDrawCall: () => performDrawCall(),
      _sendTexture: (p,w,h) => receiveTexture(p,w,h),
      _registerShaderUniform: (name, type, valuePtr) => registerShaderUniform(name, type, valuePtr),
      _sbrk
    }

    for (let fnName of 'round,floor,ceil,cos,acos,sin,tan,sqrt,abs,pow'.split(',')) {
      env['_' + fnName] = Math[fnName]
    }

    return WebAssembly.instantiate(wasmProgram, { env })
  }

  const runProgram = (result) => {
    /*============ Creating WebGL context ==================*/
    const
      gl = canvas.getContext('webgl2')
    , gl_bindBuffer = function() { gl.bindBuffer.apply(gl, arguments) }
    , gl_bufferData = function() { gl.bufferData.apply(gl, arguments) }
    , gl_createBuffer = () => gl.createBuffer()

    /*========== Preparing WebAssembly memory ==============*/
    const exports = result.instance.exports
    const heap = memory.buffer

    const f32buffer = (offset, size) => new Float32Array(heap, offset, size)
    const i32bufer = (offset, size) => new Int32Array(heap, offset, size)

    const passEvent = (eventId) => (evt) => {
      let handled = exports._onEvent(eventId, evt.keyCode)
      if (handled) {
        evt.preventDefault()
        return false;
      }
    }
    window.addEventListener('keydown', passEvent(EVENT_KEYDOWN))
    window.addEventListener('keyup', passEvent(EVENT_KEYUP))

    //removeIf(production)
    log(exports)
    //endRemoveIf(production)
      
    /*============== Defining the geometry ==============*/

    const buffer_vertex = gl_createBuffer()
    const buffer_normal = gl_createBuffer()
    const buffer_index = gl_createBuffer()
    const buffer_color = gl_createBuffer()
    const buffer_texCoords = gl_createBuffer()
    
  
    /*================ Shaders ====================*/

    const vertCode =
    'attribute vec3 pos;'+
    'attribute vec3 nor;'+
    'attribute vec4 color;'+
    'attribute vec2 texC;'+
    'uniform mat4 projMat;'+
    'uniform mat4 viewMat;'+
    'uniform mat4 mdlMat;'+
    'uniform mat4 norMat;'+
    'varying vec3 vNor;'+//transposed(inversed(view*mdl))
    'varying vec4 vColor;'+
    'varying vec2 vTex;'+
    'varying mat4 vMat;'+
    'void main(void) {'+
      'gl_Position = projMat * viewMat * mdlMat * vec4(pos, 1.0);'+
      'vColor = color;'+
      'vTex = texC;'+
      'vMat = viewMat;'+
      'vNor = normalize(vec3(norMat * vec4(nor, 1.0)));'+
    '}'

    const fragCode =
      'precision mediump float;'+
      'varying vec3 vNor;'+
      'varying vec4 vColor;'+
      'varying vec2 vTex;'+
      'varying mat4 vMat;'+
      'uniform sampler2D tex;'+
      'uniform bool useTex;'+
      'uniform float rewind;'+
      `vec4 Sepia(vec4 c) {
          return vec4(
                clamp(c.r * 0.393 + c.g * 0.769 + c.b * 0.189, 0.0, 1.0)
              , clamp(c.r * 0.349 + c.g * 0.686 + c.b * 0.168, 0.0, 1.0)
              , clamp(c.r * 0.272 + c.g * 0.534 + c.b * 0.131, 0.0, 1.0)
              , c.a
          );
      }`+
      'void main(void) {'+
        'vec4 color;'+
        'if (useTex) {'+
          'color = texture2D(tex, vTex);'+
          'color.a = 1.0;'+
        '}'+
        'else {'+
          'color = vColor;'+
        '}'+
        'vec3 directionToLight = normalize(vec3(0, -0.1, 1));'+
        'float lightStrength = max(0.35, dot(directionToLight, vNor));'+
        'gl_FragColor.rgb = color.rgb * lightStrength;'+
        'gl_FragColor.a = color.a;'+
        'if (rewind > 0.0) {'+
          'gl_FragColor = rewind * Sepia(gl_FragColor) + (1.0 - rewind) * gl_FragColor;'+
        '}'+
      '}'

  
    const shaderProgram = linkShaders(gl, vertCode, fragCode)
    gl.useProgram(shaderProgram)
  
    /*======= Associating shaders to buffer objects =======*/
  
    gl_bindBuffer(gl_ELEMENT_ARRAY_BUFFER, buffer_index)

    gl_bindBuffer(gl_ARRAY_BUFFER, buffer_vertex)
    const aPos = gl.getAttribLocation(shaderProgram, "pos")
    gl.vertexAttribPointer(aPos, 3, gl_FLOAT, false, 0, 0)
    gl.enableVertexAttribArray(aPos)

    gl_bindBuffer(gl_ARRAY_BUFFER, buffer_normal)
    const aNormal = gl.getAttribLocation(shaderProgram, "nor")
    gl.vertexAttribPointer(aNormal, 3, gl_FLOAT, false, 0, 0)
    gl.enableVertexAttribArray(aNormal)

    gl_bindBuffer(gl_ARRAY_BUFFER, buffer_color)
    const aColor = gl.getAttribLocation(shaderProgram, "color")
    gl.vertexAttribPointer(aColor, 4, gl_FLOAT, false,0,0) 
    gl.enableVertexAttribArray(aColor)

    gl_bindBuffer(gl_ARRAY_BUFFER, buffer_texCoords)
    const aTexCoords = gl.getAttribLocation(shaderProgram, "texC")
    gl.vertexAttribPointer(aTexCoords, 2, gl_FLOAT, false,0,0)
    gl.enableVertexAttribArray(aTexCoords)


    /*=========== Initialize the engine =============*/

    //removeIf(production)
    if (DEBUG_TWEAK) {
      const addTweaker = (header, el) => {
        let container = document.createElement("p")
        container.innerHTML = header + "<br>"
        container.appendChild(el)
        tv.appendChild(container)
      }
      const createInput = (name, tweakIndex, defaultValue, step) => {
        let input = document.createElement("input")
        input.type = "number";
        input.style.width = '100px'
        input.onchange = () => {
          exports._setTweakValue(tweakIndex, +input.value)
        }
        input.value = defaultValue
        input.step = step || 1.0
        addTweaker(name, input)
        exports._setTweakValue(tweakIndex, defaultValue)
      }
      const createSwitch = (name, tweakIndex, values) => {
        let btn = document.createElement("button")
        let currentValueIndex = -1
        btn.onclick = () => {
          currentValueIndex = (currentValueIndex + 1) % values.length
          btn.textContent = `${name}: ${values[currentValueIndex]}`
          exports._setTweakValue(tweakIndex, values[currentValueIndex])
        }
        btn.onclick()
        addTweaker(name, btn)
      }
      let i = 0
      createInput("cameraChange", i++, 10)
      createInput("go to frame:", i++, -1, 1)
      createSwitch("time speed", i++, [1, 3]);
    }
    //endRemoveIf(production)

    const OFFSET_FUNC_RETURN = exports._preinit(HEAP_START)
    const SIZE_FUNC_RETURN = 100
    const wasm_funcReturnValues = new Int32Array(heap, OFFSET_FUNC_RETURN, SIZE_FUNC_RETURN)
    const textures = []

    receiveTexture = (texOffset, texW, texH) => {
      const texSize = 4 * texW * texH
      const texBytes = new Uint8Array(heap, texOffset, texSize)

      //removeIf(production)
      if (DEBUG_TEXTURES) {
        const canvas2d = document.createElement("canvas")
        t.appendChild(canvas2d) //`t` is an id for a div defined in HTML
        canvas2d.width = texW
        canvas2d.height = texH
        adjustCanvasSizeToHighDpi(canvas2d)
        const ctx = canvas2d.getContext('2d')
        const imgData = ctx.createImageData(texW, texH)
        imgData.data.set(texBytes)
        ctx.putImageData(imgData, 0, 0)
      }
      //endRemoveIf(production)

      const texture = gl.createTexture()
      gl.bindTexture(gl_TEXTURE_2D, texture)
  
      // gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1); // use bottom-down storage
      gl.texImage2D(gl_TEXTURE_2D, 0, gl_RGBA, texW, texH, 0, gl_RGBA, gl.UNSIGNED_BYTE, texBytes);
      gl.generateMipmap(gl_TEXTURE_2D);
      textures.push(texture)
    }
    exports._generateTextures()

    let shaderUniforms = [
      // collection of functions to be called on draw call
    ]
    registerShaderUniform = (namePtr, type, valuePtr) => {
      const name = readTextFromMemory(namePtr)

      //removeIf(production)
      if (shaderUniforms.filter(s => s.name == name).length > 0)
        throw new Error("shader already registered")
      //endRemoveIf(production)

      let suffix, size, bufferFn
      if (type == SHADER_UNIFORM_1i) {
        suffix = "1i"
        size = 1
        bufferFn = i32buffer
      }
      else if (type == SHADER_UNIFORM_1f) {
        suffix = "1f"
        size = 1
        bufferFn = f32buffer
      }
      else if (type == SHADER_UNIFORM_Matrix4fv) {
        suffix = "Matrix4fv"
        size = NUM_MATRIX4
        bufferFn = f32buffer
      }

      const isMatrix = type == SHADER_UNIFORM_Matrix4fv
      const loc = gl.getUniformLocation(shaderProgram, name)
      const wasm_data = bufferFn(valuePtr, size)
      const fetchValue = isMatrix
        ? () => { gl['uniform' + suffix](loc, false, wasm_data) }
        : () => { gl['uniform' + suffix](loc, wasm_data) }

      shaderUniforms.push(fetchValue)
    }

    exports._initEngine()
    const VALUES_PER_VERTEX = wasm_funcReturnValues[1]//3
    const VALUES_PER_COLOR = wasm_funcReturnValues[0]//4
    const VALUES_PER_TEXCOORD = 2
    const SIZE_RENDER_BUFFER_COLOR = wasm_funcReturnValues[2]
    const SIZE_RENDER_BUFFER_VERTEX = wasm_funcReturnValues[3]
    const SIZE_RENDER_BUFFER_NORMAL = wasm_funcReturnValues[4]
    const SIZE_RENDER_BUFFER_INDEX = wasm_funcReturnValues[5]
    const SIZE_RENDER_BUFFER_TEXCOORDS = wasm_funcReturnValues[6]
    const OFFSET_RENDER_BUFFER_COLOR = wasm_funcReturnValues[7]
    const OFFSET_RENDER_BUFFER_VERTEX = wasm_funcReturnValues[8]
    const OFFSET_RENDER_BUFFER_INDEX = wasm_funcReturnValues[9]
    const OFFSET_RENDER_BUFFER_TEXCOORDS = wasm_funcReturnValues[10]
    const OFFSET_RENDER_BUFFER_NORMAL = wasm_funcReturnValues[11]

    const OFFSET_DYNAMIC_MEMORY = wasm_funcReturnValues[12]
    dynamicMemoryOffset = dynamicMemoryBreak = OFFSET_DYNAMIC_MEMORY

    const wasm_colorBuffer = f32buffer(OFFSET_RENDER_BUFFER_COLOR, SIZE_RENDER_BUFFER_COLOR/4)
    const wasm_vertexBuffer = f32buffer(OFFSET_RENDER_BUFFER_VERTEX, SIZE_RENDER_BUFFER_VERTEX/4)
    const wasm_indexBuffer = i32bufer(OFFSET_RENDER_BUFFER_INDEX, SIZE_RENDER_BUFFER_INDEX/4)
    const wasm_texCoordsBuffer = f32buffer(OFFSET_RENDER_BUFFER_TEXCOORDS, SIZE_RENDER_BUFFER_TEXCOORDS/4)
    const wasm_normalBuffer = f32buffer(OFFSET_RENDER_BUFFER_NORMAL, SIZE_RENDER_BUFFER_NORMAL/4)

    const uUseTexture = gl.getUniformLocation(shaderProgram, 'useTex')
  
  
    /*================ Drawing the game =================*/
  
    let previousRenderTimestamp = null

    clearFrame = () => {
      gl.clearColor(0.2, 0.2, 0.8, 1)
      gl.clearDepth(1.0)
      gl.enable(gl_DEPTH_TEST)
      gl.depthFunc(gl_LEQUAL)
      gl.clear(gl_BUFFER_COLOR_BIT | gl_DEPTH_BUFFER_BIT)
    }
  
    performDrawCall = () => {
      const vertexCount = wasm_funcReturnValues[0]
      const indexCount = wasm_funcReturnValues[1]
      const currentTextureId = wasm_funcReturnValues[2]
      const useTexture = currentTextureId >= 0
  

      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_vertex)
      gl_bufferData(gl_ARRAY_BUFFER, wasm_vertexBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_normal)
      gl_bufferData(gl_ARRAY_BUFFER, wasm_normalBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ELEMENT_ARRAY_BUFFER, buffer_index)
      gl_bufferData(gl_ELEMENT_ARRAY_BUFFER, wasm_indexBuffer.subarray(0, indexCount), gl_STATIC_DRAW)

      // There is a choice between vertex-colored pixel and textured pixel.
      // We always need to assign a buffer of non-vertex attributes as big as many vertices are sent to the WebGL.
      // Thus, if we don't use a texture, let's just send the meaningless buffer anyway,
      // so we don't have to have a separate shaders with and without texture support.
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_color)
      gl_bufferData(gl_ARRAY_BUFFER, wasm_colorBuffer.subarray(0, vertexCount*VALUES_PER_COLOR), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_texCoords)
      gl_bufferData(gl_ARRAY_BUFFER, wasm_texCoordsBuffer.subarray(0, vertexCount*VALUES_PER_TEXCOORD), gl_STATIC_DRAW)
      if (useTexture) {
        gl.bindTexture(gl_TEXTURE_2D, textures[currentTextureId])
      }
      gl_bindBuffer(gl_ARRAY_BUFFER, null)

      gl.uniform1i(uUseTexture, useTexture)
      for (let uniform of shaderUniforms) {
        uniform()
      }

      gl.drawElements(gl_TRIANGLES, indexCount, gl_UNSIGNED_INT, 0)
    }
  
    const render = () => {
      const timestamp = performance.now()
      const deltaTime = (timestamp - previousRenderTimestamp) / 1000
      previousRenderTimestamp = timestamp

      // should call performDrawCall() at least once
      exports._render(deltaTime)
  
      gl.viewport(0, 0, canvas.width, canvas.height)
      requestAnimationFrame(render)
    }

    exports._initGame();
    previousRenderTimestamp = performance.now()
    requestAnimationFrame(render)
  }
  const linkShaders = (gl, vertCode, fragCode) => {
    const vertShader = compileShader(gl, gl_VERTEX_SHADER, vertCode)
    const fragShader = compileShader(gl, gl_FRAGMENT_SHADER, fragCode)
  
    const program = gl.createProgram()
    gl.attachShader(program, vertShader)
    gl.attachShader(program, fragShader)
    gl.linkProgram(program)
  
    if (!gl.getProgramParameter(program, gl_LINK_STATUS)) {
      throw new Error(gl.getProgramInfoLog(program))
    }
  
    return program
  }
  
  const compileShader = (gl, type, code) => {
    const shader = gl.createShader(type)
    gl.shaderSource(shader, code)
    gl.compileShader(shader)
  
    const compiled = gl.getShaderParameter(shader, gl_COMPILE_STATUS)
  
    if (!compiled) {
      throw new Error(gl.getShaderInfoLog(shader))
    }
  
    return shader
  }

  const PIXEL_RATIO = (function () {
    let ctx = document.createElement("canvas").getContext("2d"),
        dpr = window.devicePixelRatio || 1,
        bsr = ctx.webkitBackingStorePixelRatio ||
              ctx.mozBackingStorePixelRatio ||
              ctx.msBackingStorePixelRatio ||
              ctx.oBackingStorePixelRatio ||
              ctx.backingStorePixelRatio || 1;

    return dpr / bsr;
  })()

  // scale down for dpi but to keep logical size look sharp
  const adjustCanvasSizeToHighDpi = (canvas, is2d) => {
    let {width, height} = getComputedStyle(canvas)
    width = width.substr(0, width.length-2)    //trim the 'px'
    height = height.substr(0, height.length-2) //trim the 'px'

    canvas.style.width = (canvas.width / PIXEL_RATIO) + "px"
    canvas.style.height = (canvas.height / PIXEL_RATIO) + "px"

    if (is2d) {
      canvas.getContext("2d").setTransform(PIXEL_RATIO, 0, 0, PIXEL_RATIO, 0, 0)
    }
  }


  fetch('game.wasm?' + new Date().toTimeString())
    .then(response => response.arrayBuffer())
    .then(loadProgram)
    .then(runProgram)
  })