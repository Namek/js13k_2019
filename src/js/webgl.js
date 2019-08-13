document.addEventListener("DOMContentLoaded", (event) => {
const
  log = console.log
, canvas = document.getElementById("c")

, gl_COLOR_BUFFER_BIT = 16384
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

  /*========== Preparing WebAssembly memory ==============*/
  const exports = result.instance.exports
  const memory = exports.memory
  const { width, height } = canvas
  const wh = width * height
  // const pages = 1
  // memory.grow(pages)
  const heap = memory.buffer

    
  /*============== Defining the geometry ==============*/

  const vertex_buffer = gl.createBuffer()
  const Index_Buffer = gl.createBuffer()
  const color_buffer = gl.createBuffer()
  

  /*================ Shaders ====================*/

  // Vertex shader source code
  const vertCode =
  'attribute vec3 pos;'+
  'attribute vec4 color;'+
  'uniform mat4 projMat;'+
  'uniform mat4 viewMat;'+
  'varying vec4 vColor;'+
  'void main(void) {' +
    'gl_Position = projMat * viewMat * vec4(pos, 1.0);' +
    'vColor = color;'+
  '}'

  // Create a vertex shader object
  
  const fragCode =
    'precision mediump float;'+
    'varying vec4 vColor;'+
    'void main(void) {'+
      'gl_FragColor = vColor;'+
    '}'


  const shaderProgram = linkShaders(gl, vertCode, fragCode)
  gl.useProgram(shaderProgram)

  /*======= Associating shaders to buffer objects =======*/

  gl.bindBuffer(gl_ELEMENT_ARRAY_BUFFER, Index_Buffer)
  
  gl.bindBuffer(gl_ARRAY_BUFFER, vertex_buffer)
  const coord = gl.getAttribLocation(shaderProgram, "pos")
  gl.vertexAttribPointer(coord, 3, gl_FLOAT, false, 0, 0) 
  gl.enableVertexAttribArray(coord)

  gl.bindBuffer(gl_ARRAY_BUFFER, color_buffer)
  const color = gl.getAttribLocation(shaderProgram, "color")
  gl.vertexAttribPointer(color, 4, gl_FLOAT, false,0,0) 
  gl.enableVertexAttribArray(color)

  /*========= Drawing the game ===========*/


  // Note: those numbers should be corresponding to consts in .walt codefile
  const MAX_TRIANGLES = 1000
  const OFFSET_FUNC_RETURN = 0
  const SIZE_FUNC_RETURN = exports.init(MAX_TRIANGLES)
  const wasm_funcReturnValues = new Int32Array(heap, OFFSET_FUNC_RETURN, SIZE_FUNC_RETURN)

  const VALUES_PER_VERTEX = wasm_funcReturnValues[1]//3
  const VALUES_PER_COLOR = wasm_funcReturnValues[0]//4
  const SIZE_RENDER_COLOR_BUFFER = wasm_funcReturnValues[2]
  const SIZE_RENDER_VERTEX_BUFFER = wasm_funcReturnValues[3]
  const SIZE_RENDER_INDEX_BUFFER = wasm_funcReturnValues[4]
  const NUM_MATRIX4 = 16

  const OFFSET_RENDER_COLOR_BUFFER = wasm_funcReturnValues[5]
  const OFFSET_RENDER_VERTEX_BUFFER = wasm_funcReturnValues[6]
  const OFFSET_RENDER_INDEX_BUFFER = wasm_funcReturnValues[7]
  const OFFSET_PROJECTION_MATRIX = wasm_funcReturnValues[8]
  const OFFSET_VIEW_MATRIX = wasm_funcReturnValues[9]
  const OFFSET_SHARED_MEMORY_END = wasm_funcReturnValues[10]

  const wasm_colorBuffer = new Float32Array(heap, OFFSET_RENDER_COLOR_BUFFER, SIZE_RENDER_COLOR_BUFFER/4)
  const wasm_vertexBuffer = new Float32Array(heap, OFFSET_RENDER_VERTEX_BUFFER, SIZE_RENDER_VERTEX_BUFFER/4)
  const wasm_indexBuffer = new Int32Array(heap, OFFSET_RENDER_INDEX_BUFFER, SIZE_RENDER_INDEX_BUFFER/4)
  const wasm_projMatrix = new Float32Array(heap, OFFSET_PROJECTION_MATRIX, NUM_MATRIX4)
  const wasm_viewMatrix = new Float32Array(heap, OFFSET_VIEW_MATRIX, NUM_MATRIX4)

  const uProjMatrix = gl.getUniformLocation(shaderProgram, 'projMat')
  const uViewMatrix = gl.getUniformLocation(shaderProgram, 'viewMat')

  let firstRenderTimestamp = null

  const render = (timestamp) => {
    if (firstRenderTimestamp == null) {
      firstRenderTimestamp = timestamp
    }

    exports.render(firstRenderTimestamp - timestamp)

    const vertexCount = wasm_funcReturnValues[0]
    const indexCount = wasm_funcReturnValues[1]

    // log(vertexCount, indexCount)

    gl.bindBuffer(gl_ARRAY_BUFFER, vertex_buffer)
    gl.bufferData(gl_ARRAY_BUFFER, wasm_vertexBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl_STATIC_DRAW)
    gl.bindBuffer(gl_ARRAY_BUFFER, color_buffer)
    gl.bufferData(gl_ARRAY_BUFFER, wasm_colorBuffer.subarray(0, vertexCount*VALUES_PER_COLOR), gl_STATIC_DRAW)
    gl.bindBuffer(gl_ARRAY_BUFFER, null)
    gl.bindBuffer(gl_ELEMENT_ARRAY_BUFFER, Index_Buffer)
    gl.bufferData(gl_ELEMENT_ARRAY_BUFFER, wasm_indexBuffer.subarray(0, indexCount), gl_STATIC_DRAW)

    gl.uniformMatrix4fv(uProjMatrix, false, wasm_projMatrix)
    gl.uniformMatrix4fv(uViewMatrix, false, wasm_viewMatrix)

    gl.clearColor(0.5, 0.5, 0.5, 1.0)
    gl.clearDepth(1.0)
    gl.enable(gl_DEPTH_TEST)
    gl.depthFunc(gl_LEQUAL)
    gl.clear(gl_COLOR_BUFFER_BIT | gl_DEPTH_BUFFER_BIT)

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