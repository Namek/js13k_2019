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
  , EVENT_KEYDOWN = 1
  , EVENT_KEYUP = 2
  
  let
    performDrawCall = () => {}
  , receiveTexture = () => {}

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

  const logStr = (strPtr, num) => {
    //removeIf(production)
    let str = "";
    let buf = new Uint8Array(memory.buffer, strPtr, 100)

    let i = -1
    while (buf[++i] != 0)
      str += String.fromCharCode(buf[i])

    log("logstr:", str, num);
    //endRemoveIf(production)
  }

  
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
      __l: log, //int
      __lf: log,//float
      __lstr: logStr,
      __lfstr: logStr,
      _randomf: Math.random,
      _triggerDrawCall: () => performDrawCall(),
      _sendTexture: (p,w,h) => receiveTexture(p,w,h),
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
    , gl_createBuffer = () => gl.createBuffer()

    /*========== Preparing WebAssembly memory ==============*/
    const exports = result.instance.exports
    const heap = memory.buffer

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
      'precision highp float;'+
      'varying vec3 vNor;'+
      'varying vec4 vColor;'+
      'varying vec2 vTex;'+
      'varying mat4 vMat;'+
      'uniform sampler2D tex;'+
      'uniform bool useTex;'+
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
      createSwitch("simulate", i++, [0, 3])//3==Simulate
      createInput("time speed", i++, 1, 0.1)
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

    exports._initEngine()
    const VALUES_PER_VERTEX = wasm_funcReturnValues[1]//3
    const VALUES_PER_COLOR = wasm_funcReturnValues[0]//4
    const VALUES_PER_TEXCOORD = 2
    const SIZE_RENDER_BUFFER_COLOR = wasm_funcReturnValues[2]
    const SIZE_RENDER_BUFFER_VERTEX = wasm_funcReturnValues[3]
    const SIZE_RENDER_BUFFER_NORMAL = wasm_funcReturnValues[4]
    const SIZE_RENDER_BUFFER_INDEX = wasm_funcReturnValues[5]
    const SIZE_RENDER_BUFFER_TEXCOORDS = wasm_funcReturnValues[6]
    const NUM_MATRIX4 = 16

    const OFFSET_RENDER_BUFFER_COLOR = wasm_funcReturnValues[7]
    const OFFSET_RENDER_BUFFER_VERTEX = wasm_funcReturnValues[8]
    const OFFSET_RENDER_BUFFER_INDEX = wasm_funcReturnValues[9]
    const OFFSET_RENDER_BUFFER_TEXCOORDS = wasm_funcReturnValues[10]
    const OFFSET_RENDER_BUFFER_NORMAL = wasm_funcReturnValues[11]
    const OFFSET_PROJECTION_MATRIX = wasm_funcReturnValues[12]
    const OFFSET_VIEW_MATRIX = wasm_funcReturnValues[13]
    const OFFSET_MODEL_MATRIX = wasm_funcReturnValues[14]
    const OFFSET_NORMAL_MATRIX = wasm_funcReturnValues[15]

    const OFFSET_DYNAMIC_MEMORY = wasm_funcReturnValues[16]
    dynamicMemoryOffset = dynamicMemoryBreak = OFFSET_DYNAMIC_MEMORY

    const f32buffer = (offset, size) => new Float32Array(heap, offset, size)
    const wasm_colorBuffer = f32buffer(OFFSET_RENDER_BUFFER_COLOR, SIZE_RENDER_BUFFER_COLOR/4)
    const wasm_vertexBuffer = f32buffer(OFFSET_RENDER_BUFFER_VERTEX, SIZE_RENDER_BUFFER_VERTEX/4)
    const wasm_indexBuffer = new Int32Array(heap, OFFSET_RENDER_BUFFER_INDEX, SIZE_RENDER_BUFFER_INDEX/4)
    const wasm_texCoordsBuffer = f32buffer(OFFSET_RENDER_BUFFER_TEXCOORDS, SIZE_RENDER_BUFFER_TEXCOORDS/4)
    const wasm_normalBuffer = f32buffer(OFFSET_RENDER_BUFFER_NORMAL, SIZE_RENDER_BUFFER_NORMAL/4)
    const wasm_projMatrix = f32buffer(OFFSET_PROJECTION_MATRIX, NUM_MATRIX4)
    const wasm_viewMatrix = f32buffer(OFFSET_VIEW_MATRIX, NUM_MATRIX4)
    const wasm_modelMatrix = f32buffer(OFFSET_MODEL_MATRIX, NUM_MATRIX4)
    const wasm_normalMatrix = f32buffer(OFFSET_NORMAL_MATRIX, NUM_MATRIX4)

    const uProjMatrix = gl.getUniformLocation(shaderProgram, 'projMat')
    const uViewMatrix = gl.getUniformLocation(shaderProgram, 'viewMat')
    const uModelMatrix = gl.getUniformLocation(shaderProgram, 'mdlMat')
    const uNormalMatrix = gl.getUniformLocation(shaderProgram, 'norMat');
    const uUseTexture = gl.getUniformLocation(shaderProgram, 'useTex')
  
  
    /*================ Drawing the game =================*/
  
    let previousRenderTimestamp = null
  
    performDrawCall = () => {
      const vertexCount = wasm_funcReturnValues[0]
      const indexCount = wasm_funcReturnValues[1]
      const currentTextureId = wasm_funcReturnValues[2]
      const useTexture = currentTextureId >= 0
  
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_vertex)
      gl.bufferData(gl_ARRAY_BUFFER, wasm_vertexBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_normal)
      gl.bufferData(gl_ARRAY_BUFFER, wasm_normalBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ELEMENT_ARRAY_BUFFER, buffer_index)
      gl.bufferData(gl_ELEMENT_ARRAY_BUFFER, wasm_indexBuffer.subarray(0, indexCount), gl_STATIC_DRAW)

      // There is a choice between vertex-colored pixel and textured pixel.
      // We always need to assign a buffer of non-vertex attributes as big as many vertices are sent to the WebGL.
      // Thus, if we don't use a texture, let's just send the meaningless buffer anyway,
      // so we don't have to have a separate shaders with and without texture support.
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_color)
      gl.bufferData(gl_ARRAY_BUFFER, wasm_colorBuffer.subarray(0, vertexCount*VALUES_PER_COLOR), gl_STATIC_DRAW)
      gl_bindBuffer(gl_ARRAY_BUFFER, buffer_texCoords)
      gl.bufferData(gl_ARRAY_BUFFER, wasm_texCoordsBuffer.subarray(0, vertexCount*VALUES_PER_TEXCOORD), gl_STATIC_DRAW)
      if (useTexture) {
        gl.bindTexture(gl_TEXTURE_2D, textures[currentTextureId])
      }

      gl_bindBuffer(gl_ARRAY_BUFFER, null)
  
      gl.uniformMatrix4fv(uProjMatrix, false, wasm_projMatrix)
      gl.uniformMatrix4fv(uViewMatrix, false, wasm_viewMatrix)
      gl.uniformMatrix4fv(uModelMatrix, false, wasm_modelMatrix)
      gl.uniformMatrix4fv(uNormalMatrix, false, wasm_normalMatrix)
      gl.uniform1i(uUseTexture, useTexture)
  
      gl.drawElements(gl_TRIANGLES, indexCount, gl_UNSIGNED_INT, 0)
    }
  
    const render = (timestamp) => {
      const deltaTime = (timestamp - previousRenderTimestamp) / 1000
      previousRenderTimestamp = timestamp

      gl.clearColor(0.2, 0.2, 0.8, 1)
      gl.clearDepth(1.0)
      gl.enable(gl_DEPTH_TEST)
      gl.depthFunc(gl_LEQUAL)
      gl.clear(gl_BUFFER_COLOR_BIT | gl_DEPTH_BUFFER_BIT)

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