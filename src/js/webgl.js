document.addEventListener("DOMContentLoaded", (event) => {
const
  log = console.log
, canvas = document.getElementById("c")
, canvas2d = document.getElementById("t")

, gl_buffer_color_BIT = 16384
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
      
const loadProgram = (wasmProgram) => {
  const getCanvasWidth = () => canvas.width
  const getCanvasHeight = () => canvas.height
  const env = {
    getCanvasWidth,
    getCanvasHeight,
    log,
    Math_exp: Math.exp,
    Math_floor: Math.floor,
    Math_tan: Math.tan
  }

  return WebAssembly.instantiate(wasmProgram, { env })
}
  
const runProgram = (result) => {
  /*============ Creating WebGL context ==================*/
  const gl = canvas.getContext('webgl2')
  const ctx = canvas2d.getContext('2d')

  /*========== Preparing WebAssembly memory ==============*/
  const exports = result.instance.exports
  const memory = exports.memory
  const { width, height } = canvas
  const wh = width * height
  // const pages = 1
  // memory.grow(pages)
  const heap = memory.buffer

    
  /*============== Defining the geometry ==============*/

  const buffer_vertex = gl.createBuffer()
  const bufer_index = gl.createBuffer()
  const buffer_color = gl.createBuffer()
  const buffer_texCoords = gl.createBuffer()
  

  /*================ Shaders ====================*/

  // Vertex shader source code
  const vertCode =
  'attribute vec3 pos;'+
  'attribute vec4 color;'+
  'attribute vec2 texC;'+
  'uniform mat4 projMat;'+
  'uniform mat4 viewMat;'+
  'varying vec4 vColor;'+
  'varying vec2 vTex;'+
  'void main(void) {' +
    'gl_Position = projMat * viewMat * vec4(pos, 1.0);' +
    'vColor = color;'+
    'vTex = texC;'+
  '}'

  // Create a vertex shader object
  
  const fragCode =
    'precision mediump float;'+
    'varying vec4 vColor;'+
    'varying vec2 vTex;'+
    'uniform sampler2D tex;'+
    'uniform bool useTex;'+
    'void main(void) {'+
      'if (useTex) {'+
        'gl_FragColor = texture2D(tex, vTex);'+
      '}'+
      'else {'+
        'gl_FragColor = vColor;'+
      '}'+
    '}'

  const shaderProgram = linkShaders(gl, vertCode, fragCode)
  gl.useProgram(shaderProgram)

  /*======= Associating shaders to buffer objects =======*/

  gl.bindBuffer(gl_ELEMENT_ARRAY_BUFFER, bufer_index)
  
  gl.bindBuffer(gl_ARRAY_BUFFER, buffer_vertex)
  const aPos = gl.getAttribLocation(shaderProgram, "pos")
  gl.vertexAttribPointer(aPos, 3, gl_FLOAT, false, 0, 0) 
  gl.enableVertexAttribArray(aPos)

  gl.bindBuffer(gl_ARRAY_BUFFER, buffer_color)
  const aColor = gl.getAttribLocation(shaderProgram, "color")
  gl.vertexAttribPointer(aColor, 4, gl_FLOAT, false,0,0) 
  gl.enableVertexAttribArray(aColor)

  gl.bindBuffer(gl_ARRAY_BUFFER, buffer_texCoords)
  const aTexCoords = gl.getAttribLocation(shaderProgram, "texC")
  gl.vertexAttribPointer(aTexCoords, 2, gl_FLOAT, false,0,0)
  gl.enableVertexAttribArray(aTexCoords)



  /*=========== Initialize data =============*/


  const OFFSET_FUNC_RETURN = 0
  const SIZE_FUNC_RETURN = exports.preinit()
  const wasm_funcReturnValues = new Int32Array(heap, OFFSET_FUNC_RETURN, SIZE_FUNC_RETURN)

  exports.generateTextures()
  let valIdx = 0
  const textureCount = wasm_funcReturnValues[valIdx++]
  
  const textures = []
  
  for (let i = 0; i < textureCount; ++i) {
    const texW = wasm_funcReturnValues[valIdx++]
    const texH = wasm_funcReturnValues[valIdx++]
    const texOffset = wasm_funcReturnValues[valIdx++] //* 4
    const texSize = 4 * texW * texH
    const texBytes = new Uint8Array(heap, texOffset, texSize)

    canvas2d.width = texW
    canvas2d.height = texH
    const imgData = ctx.createImageData(texW, texH)
    imgData.data.set(texBytes)
    ctx.putImageData(imgData, 0, 0)

    const texture = gl.createTexture()
    gl.bindTexture(gl_TEXTURE_2D, texture)
    
    // gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1); // use bottom-down storage
    
    gl.texImage2D( gl_TEXTURE_2D, 0, gl_RGBA,
      texW, texH, 0, gl_RGBA, gl.UNSIGNED_BYTE, texBytes);
    gl.generateMipmap(gl_TEXTURE_2D);
    textures.push(texture)
    
  }

  const MAX_TRIANGLES = 1000
  exports.init(MAX_TRIANGLES)

  const VALUES_PER_VERTEX = wasm_funcReturnValues[1]//3
  const VALUES_PER_COLOR = wasm_funcReturnValues[0]//4
  const VALUES_PER_TEXCOORD = 2
  const SIZE_RENDER_buffer_color = wasm_funcReturnValues[2]
  const SIZE_RENDER_buffer_vertex = wasm_funcReturnValues[3]
  const SIZE_RENDER_bufer_index = wasm_funcReturnValues[4]
  const SIZE_RENDER_TEXCOORDS_BUFFER = wasm_funcReturnValues[5]
  const NUM_MATRIX4 = 16

  const OFFSET_RENDER_buffer_color = wasm_funcReturnValues[6]
  const OFFSET_RENDER_buffer_vertex = wasm_funcReturnValues[7]
  const OFFSET_RENDER_bufer_index = wasm_funcReturnValues[8]
  const OFFSET_RENDER_TEXCOORDS_BUFFER = wasm_funcReturnValues[9]
  const OFFSET_PROJECTION_MATRIX = wasm_funcReturnValues[10]
  const OFFSET_VIEW_MATRIX = wasm_funcReturnValues[11]
  const OFFSET_SHARED_MEMORY_END = wasm_funcReturnValues[12]

  const wasm_colorBuffer = new Float32Array(heap, OFFSET_RENDER_buffer_color, SIZE_RENDER_buffer_color/4)
  const wasm_vertexBuffer = new Float32Array(heap, OFFSET_RENDER_buffer_vertex, SIZE_RENDER_buffer_vertex/4)
  const wasm_indexBuffer = new Int32Array(heap, OFFSET_RENDER_bufer_index, SIZE_RENDER_bufer_index/4)
  const wasm_texCoordsBuffer = new Float32Array(heap, OFFSET_RENDER_TEXCOORDS_BUFFER, SIZE_RENDER_TEXCOORDS_BUFFER/4)
  const wasm_projMatrix = new Float32Array(heap, OFFSET_PROJECTION_MATRIX, NUM_MATRIX4)
  const wasm_viewMatrix = new Float32Array(heap, OFFSET_VIEW_MATRIX, NUM_MATRIX4)

  const uProjMatrix = gl.getUniformLocation(shaderProgram, 'projMat')
  const uViewMatrix = gl.getUniformLocation(shaderProgram, 'viewMat')
  const uUseTexture = gl.getUniformLocation(shaderProgram, 'useTex')


  /*================ Drawing the game =================*/

  let firstRenderTimestamp = null

  const render = (timestamp) => {
    if (firstRenderTimestamp == null) {
      firstRenderTimestamp = timestamp
    }

    exports.render(firstRenderTimestamp - timestamp)

    const vertexCount = wasm_funcReturnValues[0]
    const indexCount = wasm_funcReturnValues[1]

    // log(vertexCount, indexCount)

    gl.bindBuffer(gl_ARRAY_BUFFER, buffer_vertex)
    gl.bufferData(gl_ARRAY_BUFFER, wasm_vertexBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
    gl.bindBuffer(gl_ARRAY_BUFFER, buffer_color)
    gl.bufferData(gl_ARRAY_BUFFER, wasm_colorBuffer.subarray(0, vertexCount*VALUES_PER_COLOR), gl_STATIC_DRAW)
    gl.bindBuffer(gl_ARRAY_BUFFER, buffer_texCoords)
    gl.bufferData(gl_ARRAY_BUFFER, wasm_texCoordsBuffer.subarray(0, vertexCount*VALUES_PER_TEXCOORD), gl_STATIC_DRAW)
    gl.bindBuffer(gl_ARRAY_BUFFER, null)
    gl.bindBuffer(gl_ELEMENT_ARRAY_BUFFER, bufer_index)
    gl.bufferData(gl_ELEMENT_ARRAY_BUFFER, wasm_indexBuffer.subarray(0, indexCount), gl_STATIC_DRAW)

    gl.uniformMatrix4fv(uProjMatrix, false, wasm_projMatrix)
    gl.uniformMatrix4fv(uViewMatrix, false, wasm_viewMatrix)
    gl.uniform1i(uUseTexture, 0)

    gl.clearColor(0.5, 0.5, 0.5, 1.0)
    gl.clearDepth(1.0)
    gl.enable(gl_DEPTH_TEST)
    gl.depthFunc(gl_LEQUAL)
    gl.clear(gl_buffer_color_BIT | gl_DEPTH_BUFFER_BIT)

    // Set the view port
    gl.viewport(0, 0, canvas.width, canvas.height)

    // Draw the triangle
    gl.drawElements(gl_TRIANGLES, indexCount, gl_UNSIGNED_INT,0)

    window.requestAnimationFrame(render)
  }

  window.requestAnimationFrame(render)
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


fetch('triangles.wasm?' + new Date().toTimeString())
  .then(response => response.arrayBuffer())
  .then(loadProgram)
  .then(runProgram)
})