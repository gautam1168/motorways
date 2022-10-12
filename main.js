function main() {
  const canvas = document.createElement("canvas");
  document.body.appendChild(canvas);
  canvas.width = 64 * 15; //window.innerWidth;
  canvas.height = 64 * 8; //window.innerHeight;

  startGame(canvas);
}

function startGame(canvas) {
  const ctx = canvas.getContext("2d");
  const HousePinkP = new Promise((resolve, reject) => {
    const HousePink = new Image();
    HousePink.src = "/housepink.png";
    HousePink.onload = data => {
      resolve({ HousePink });
    };
    HousePink.onerror = reject;
  });

  const BuildingPinkP = new Promise((resolve, reject) => {
    const BuildingPink = new Image();
    BuildingPink.src = "/buildingpink.png";
    BuildingPink.onload = data => {
      resolve({ BuildingPink });
    };
    BuildingPink.onerror = reject;
  });

  const CarPinkP = new Promise((resolve, reject) => {
    const CarPink = new Image();
    CarPink.src = "/carpink.png";
    CarPink.onload = data => {
      resolve({ CarPink });
    };
    CarPink.onerror = reject;
  });
  
  const pagesPerMB = 1024*1024 / 64000;
  const memory = new WebAssembly.Memory({ initial: 10 * pagesPerMB })

  const GameCode = fetch("/game.wasm")
    .then(res => res.arrayBuffer())
    .then(bytes => WebAssembly.instantiate(bytes, { env: { memory } }))

  Promise.all([GameCode, BuildingPinkP, HousePinkP, CarPinkP])
    .then(([{ instance }, { BuildingPink }, { HousePink }, { CarPink }]) => {

      let bufferLength = canvas.width * canvas.height * 4;
      const JSMemoryView = new Uint8Array(memory.buffer);
      const Game = instance.exports;

      const ofcanvas = new OffscreenCanvas(BuildingPink.width, BuildingPink.height);
      const ofctx = ofcanvas.getContext("2d");
      ofctx.drawImage(BuildingPink, 0, 0);

      const HEAP_BASE = Game.__heap_base;

      let bytes = ofctx.getImageData(0, 0, ofcanvas.width, ofcanvas.height);
      
      let byteIndex = 0;
      for (let i = HEAP_BASE + bufferLength; i < HEAP_BASE + bufferLength + bytes.data.length; i++) {
        JSMemoryView[i] = bytes.data[byteIndex++];
      }

      ofcanvas.width = HousePink.width;
      ofcanvas.height = HousePink.height;
      ofctx.clearRect(0, 0, ofcanvas.width, ofcanvas.height);
      ofctx.drawImage(HousePink, 0, 0);

      bytes = ofctx.getImageData(0, 0, ofcanvas.width, ofcanvas.height);
      const buildingLength = BuildingPink.width * BuildingPink.height * 4;
      const houseLength = HousePink.width * HousePink.height * 4;
      byteIndex = 0;
      for (let i = HEAP_BASE + bufferLength + buildingLength;
        i < HEAP_BASE + bufferLength + buildingLength + houseLength;
        i++) {
        JSMemoryView[i] = bytes.data[byteIndex++];
      }

      ofcanvas.width = CarPink.width;
      ofcanvas.height = CarPink.height;
      ofctx.clearRect(0, 0, ofcanvas.width, ofcanvas.height);
      ofctx.drawImage(CarPink, 0, 0);

      bytes = ofctx.getImageData(0, 0, ofcanvas.width, ofcanvas.height);
      const carLength = CarPink.width * CarPink.height * 4;
      byteIndex = 0;
      for (let i = HEAP_BASE + bufferLength + buildingLength + houseLength;
        i < HEAP_BASE + bufferLength + buildingLength + houseLength + carLength;
        i++) {
        JSMemoryView[i] = bytes.data[byteIndex++];
      }

      
      /*
      Game.UpdateAndRender(HEAP_BASE, 
        BuildingPink.width, BuildingPink.height, 
        HousePink.width, HousePink.height,
        CarPink.width, CarPink.height,
        canvas.width, canvas.height,
        64, 64, 0);
      

      const BufferMemory = JSMemoryView.slice(HEAP_BASE, 
        HEAP_BASE + (canvas.width * canvas.height * 4));


      const Frame = new ImageData(
        new Uint8ClampedArray(BufferMemory), 
        canvas.width, canvas.height);

      ctx.putImageData(Frame, 0, 0);
      */

      let mouseIsDown = 0, mouseX = 0, mouseY = 0;
      canvas.addEventListener("mousedown", ev => {
        mouseIsDown = 1;
      });

      canvas.addEventListener("mouseup", ev => {
        mouseIsDown = 0;
      });

      canvas.addEventListener("mousemove", ev => {
        mouseX = ev.offsetX;
        mouseY = ev.offsetY;
        
      });

      let dateprev = Date.now();
      renderLoop();
      function renderLoop() {
        const datenow = Date.now();
        // console.log(1000/(datenow - dateprev));
        dateprev = datenow;
        Game.UpdateAndRender(HEAP_BASE, 
          BuildingPink.width, BuildingPink.height, 
          HousePink.width, HousePink.height,
          CarPink.width, CarPink.height,
          canvas.width, canvas.height,
          mouseX, mouseY, mouseIsDown);

        const BufferMemory = JSMemoryView.slice(HEAP_BASE, 
          HEAP_BASE + (canvas.width * canvas.height * 4));

        const Frame = new ImageData(
          new Uint8ClampedArray(BufferMemory), 
          canvas.width, canvas.height);

        ctx.putImageData(Frame, 0, 0);

        requestAnimationFrame(renderLoop);
      }
    });
}
