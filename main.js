function main() {
  const canvas = document.createElement("canvas");
  document.body.appendChild(canvas);
  canvas.width = window.innerWidth;
  canvas.height = window.innerHeight;

  startGame(canvas);
}

function startGame(canvas) {
  const ctx = canvas.getContext("2d");
  const pagesPerGB = 1024*1024*1024 / 64000;
  const memory = new WebAssembly.Memory({ initial: pagesPerGB })
  const view = new Uint8Array(memory.buffer);
  
  return fetch("/game.wasm")
    .then(res => res.arrayBuffer())
    .then(bytes => WebAssembly.instantiate(bytes, { env: { memory } }))
    .then(({ instance }) => {
      const Game = instance.exports;
      const Memory = view;
      const HEAP_BASE = Game.__heap_base;

      Game.UpdateAndRender(HEAP_BASE, canvas.width, canvas.height);

      const BufferMemory = Memory.slice(HEAP_BASE, HEAP_BASE + (canvas.width * canvas.height * 4));
      const Frame = new ImageData(new Uint8ClampedArray(BufferMemory.buffer), canvas.width, canvas.height);
      ctx.putImageData(Frame, 0, 0);
    });
}
