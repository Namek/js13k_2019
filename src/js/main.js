document.addEventListener("DOMContentLoaded", function(event) {
  fetch('main.wasm?' + new Date().toTimeString()).then(response => response.arrayBuffer())
    .then((buffer) => {

      const canvas = document.getElementById("canvas");
      const ctx = canvas.getContext("2d");
      const imgData = ctx.createImageData(canvas.width, canvas.height);

      const getCanvasWidth = () => canvas.width;
      const getCanvasHeight = () => canvas.height;
      const env = {
        getCanvasWidth,
        getCanvasHeight,
        log: console.log,
        Math_exp: Math.exp,
        Math_floor: Math.floor
      };

      WebAssembly.instantiate(buffer, { env }).then(result => {
        const exports = result.instance.exports;
        const memory = exports.memory;
        const { width, height } = canvas;
        const wh = width * height;
        const pages = 1 + ((20 * wh) >> 16);
        memory.grow(pages);
        const heap = memory.buffer;
        const HEAP_START = 0;
        const IMAGE_START = exports.init(HEAP_START);

        const imageArray = new Uint8ClampedArray(heap, IMAGE_START, 4 * wh);
        const forceArray = new Int32Array(heap, IMAGE_START + 4 * wh, wh);

        let lastMouseX = null;
        let lastMouseY = null;

        canvas.onmousedown = (e) => {
          e.preventDefault();
          let bbox = canvas.getBoundingClientRect();
          const mouseX = Math.round(e.clientX - bbox.left * (width / bbox.width));
          const mouseY = Math.round(e.clientY - bbox.top * (height / bbox.height));
          lastMouseX = mouseX;
          lastMouseY = mouseY;
          exports.applyBrush(mouseX, mouseY);
        };

        canvas.onmousemove = (e) => {
          e.preventDefault();
          let bbox = canvas.getBoundingClientRect();
          const mouseX = Math.round(e.clientX - bbox.left * (width / bbox.width));
          const mouseY = Math.round(e.clientY - bbox.top * (height / bbox.height));
          if (lastMouseX !== null && lastMouseY !== null) {
            const r = Math.sqrt((mouseX - lastMouseX) * (mouseX - lastMouseX) + (mouseY - lastMouseY) * (mouseY - lastMouseY));
            for (let t = 0; t <= r; t += 5) {
              const currX = Math.round(lastMouseX + ((mouseX - lastMouseX) * (t / r)));
              const currY = Math.round(lastMouseY + ((mouseY - lastMouseY) * (t / r)));
              exports.applyBrush(currX, currY);
              forceArray[currY * width + currX] = 0x3FFFFFFF;
            }
            //applyBrush(mouseX);
            //exports.applyBrush(mouseY);
            lastMouseX = mouseX;
            lastMouseY = mouseY;
          }
        };

        canvas.onmouseout = canvas.onmouseup = (e) => {
          e.preventDefault();
          lastMouseX = null;
          lastMouseY = null;
        };

        function step() {
          setTimeout(step, 0);
          const pre = performance.now();
          exports.step();
          const post = performance.now();
          imgData.data.set(imageArray);
          ctx.putImageData(imgData, 0, 0);
        }
        step();
      });
    });
});

