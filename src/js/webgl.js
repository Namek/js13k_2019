document.addEventListener("DOMContentLoaded", (event) => {
  fetch('triangles.wasm?' + new Date().toTimeString())
    .then(response => response.arrayBuffer())
    .then(loadProgram)
    .then(runProgram)
})

const log = console.log
      
const loadProgram = (wasmProgram) => {
  const getCanvasWidth = () => canvas.width
  const getCanvasHeight = () => canvas.height
  const env = {
    getCanvasWidth,
    getCanvasHeight,
    log,
    Math_exp: Math.exp,
    Math_floor: Math.floor
  }

  return WebAssembly.instantiate(wasmProgram, { env })
}
  
const runProgram = (result) => {
  /*============== Creating a canvas ====================*/
  const canvas = document.getElementById("canvas")
  const gl = canvas.getContext('webgl')
  var ext = gl.getExtension('OES_element_index_uint')


  /*========== Preparing WebAssembly memory =============*/
  const exports = result.instance.exports
  const memory = exports.memory
  const { width, height } = canvas
  const wh = width * height
  // const pages = 1
  // memory.grow(pages)
  const heap = memory.buffer

    
  /*======== Defining and storing the geometry ===========*/

  const vertex_buffer = gl.createBuffer()
  const Index_Buffer = gl.createBuffer()
  const color_buffer = gl.createBuffer()
  

  /*================ Shaders ====================*/

  // Vertex shader source code
  var vertCode =
  'uniform vec4 translation;'+
  'attribute vec4 coordinates;'+
  'attribute vec4 color;'+
  'varying vec4 vColor;'+
  'void main(void) {' +
    'gl_Position = coordinates + translation;' +
    'vColor = color;'+
  '}'

  // Create a vertex shader object
  
  var fragCode =
    'precision mediump float;'+
    'varying vec4 vColor;'+
    'void main(void) {'+
      'gl_FragColor = vColor;'+
    '}'


  var shaderProgram = linkShaders(gl, vertCode, fragCode)
  gl.useProgram(shaderProgram)

  /*======= Associating shaders to buffer objects =======*/

  gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, Index_Buffer)
  
  gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer)
  var coord = gl.getAttribLocation(shaderProgram, "coordinates")
  gl.vertexAttribPointer(coord, 3, gl.FLOAT, false, 0, 0) 
  gl.enableVertexAttribArray(coord)

  gl.bindBuffer(gl.ARRAY_BUFFER, color_buffer)
  let color = gl.getAttribLocation(shaderProgram, "color")
  gl.vertexAttribPointer(color, 4, gl.FLOAT, false,0,0) 
  gl.enableVertexAttribArray(color)

  /*========= Drawing the game ===========*/


  // Note: those numbers should be corresponding to consts in .walt codefile
  const VALUES_PER_COLOR = 4
  const VALUES_PER_VERTEX = 3
  const BYTES_PER_COLOR = VALUES_PER_COLOR * 4
  const BYTES_PER_VERTEX = VALUES_PER_VERTEX * 4
  const SIZE_FUNC_RETURN = 100
  const SIZE_RENDER_COLOR_BUFFER = 1000 * BYTES_PER_COLOR
  const SIZE_RENDER_VERTEX_BUFFER = 1000 * BYTES_PER_VERTEX
  const SIZE_RENDER_INDEX_BUFFER = SIZE_RENDER_VERTEX_BUFFER / 3

  const OFFSET_FUNC_RETURN = 0
  const OFFSET_RENDER_COLOR_BUFFER = OFFSET_FUNC_RETURN + SIZE_FUNC_RETURN
  const OFFSET_RENDER_VERTEX_BUFFER = OFFSET_RENDER_COLOR_BUFFER + SIZE_RENDER_COLOR_BUFFER
  const OFFSET_RENDER_INDEX_BUFFER = OFFSET_RENDER_VERTEX_BUFFER + SIZE_RENDER_VERTEX_BUFFER
  const OFFSET_SHARED_MEMORY_END = OFFSET_RENDER_INDEX_BUFFER + SIZE_RENDER_INDEX_BUFFER
  log(`offsets: COLOR = ${OFFSET_RENDER_COLOR_BUFFER}, VERTEX = ${OFFSET_RENDER_VERTEX_BUFFER}, INDEX = ${OFFSET_RENDER_INDEX_BUFFER}, _END = ${OFFSET_SHARED_MEMORY_END}`)

  const wasm_funcReturnValues = new Int32Array(heap, OFFSET_FUNC_RETURN, SIZE_FUNC_RETURN)
  const wasm_colorBuffer = new Float32Array(heap, OFFSET_RENDER_COLOR_BUFFER, SIZE_RENDER_COLOR_BUFFER)
  const wasm_vertexBuffer = new Float32Array(heap, OFFSET_RENDER_VERTEX_BUFFER, SIZE_RENDER_VERTEX_BUFFER)
  const wasm_indexBuffer = new Int32Array(heap, OFFSET_RENDER_INDEX_BUFFER, SIZE_RENDER_INDEX_BUFFER)

  
  let firstRenderTimestamp = null

  const render = (timestamp) => {
    if (firstRenderTimestamp == null) {
      firstRenderTimestamp = timestamp

    // console.log(wasm _colorBuffer.subarray(0, vertexCount*BYTES_PER_COLOR))
    }

    exports.render(firstRenderTimestamp - timestamp)

    const vertexCount = wasm_funcReturnValues[0]
    const indexCount = wasm_funcReturnValues[1]

    // log(vertexCount, indexCount)

    gl.bindBuffer(gl.ARRAY_BUFFER, vertex_buffer)
    gl.bufferData(gl.ARRAY_BUFFER, wasm_vertexBuffer.subarray(0, vertexCount*VALUES_PER_VERTEX), gl.STATIC_DRAW)
    gl.bindBuffer(gl.ARRAY_BUFFER, color_buffer)
    gl.bufferData(gl.ARRAY_BUFFER, wasm_colorBuffer.subarray(0, vertexCount*VALUES_PER_COLOR), gl.STATIC_DRAW)
    gl.bindBuffer(gl.ARRAY_BUFFER, null)
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, Index_Buffer)
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, wasm_indexBuffer.subarray(0, indexCount), gl.STATIC_DRAW)

    var Tx = 0.5, Ty = 0.5, Tz = 0.0
    var translation = gl.getUniformLocation(shaderProgram, 'translation')
    gl.uniform4f(translation, Tx, Ty, Tz, 0.0)

    gl.clearColor(0.5, 0.5, 0.5, 1.0)
    gl.clearDepth(1.0)
    gl.enable(gl.DEPTH_TEST)
    gl.depthFunc(gl.LEQUAL)
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

    // Set the view port
    gl.viewport(0,0,canvas.width,canvas.height)

    // Draw the triangle
    gl.drawElements(gl.TRIANGLES, indexCount, gl.UNSIGNED_INT,0)

    window.requestAnimationFrame(render)
  }

  window.requestAnimationFrame(render)
}

const linkShaders = (gl, vertCode, fragCode) => {
  let vertShader = compileShader(gl, gl.VERTEX_SHADER, vertCode)
  let fragShader = compileShader(gl, gl.FRAGMENT_SHADER, fragCode)

  var program = gl.createProgram()
  gl.attachShader(program, vertShader)
  gl.attachShader(program, fragShader)
  gl.linkProgram(program)

  if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
    throw new Error(gl.getProgramInfoLog(program))
  }

  return program
}

const compileShader = (gl, type, code) => {
  var shader = gl.createShader(type)
  gl.shaderSource(shader, code)
  gl.compileShader(shader)

  const compiled = gl.getShaderParameter(shader, gl.COMPILE_STATUS)

  if (!compiled) {
    throw new Error(gl.getShaderInfoLog(shader))
  }

  return shader
}