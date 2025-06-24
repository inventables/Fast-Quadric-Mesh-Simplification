self.addEventListener('message', function(e) {
  prepare_and_simplify(e.data.blob, e.data.percentage);
}, false);

var Module = null;
self.importScripts("Fast-Quadric-Mesh-Simplification.js");

async function initializeModule() {
  if (!Module) {
    Module = await createSimplifyModule();
  }
  return Module;
}

async function prepare_and_simplify(file, percentage) {
  await initializeModule();
  var fr = new FileReader();
  fr.readAsArrayBuffer(file);
  fr.onloadend = async function () {
    const outputData = await simplify(new Uint8Array(fr.result), percentage);
    let file = new Blob([outputData], {type: 'application/sla'});
    self.postMessage({"blob":file});
  }
}

async function simplify(inputArray, percentage) {
  // Allocate memory for the input buffer and copy the data
  const inputPtr = Module._malloc(inputArray.byteLength);
  Module.HEAPU8.set(inputArray, inputPtr);

  // Allocate memory for output pointer and output size
  const outputPtrPtr = Module._malloc(4); // Pointer to pointer (32 bits)
  const outputSizePtr = Module._malloc(4); // Size (32 bits)

  // Initialize the output pointer and size to 0
  Module.HEAPU32[outputPtrPtr/4] = 0;
  Module.HEAPU32[outputSizePtr/4] = 0;

  const reduceFraction = percentage;
  const aggressiveness = 7;

  // Call the C function
  const result = Module._simplify(
    inputPtr,
    inputArray.byteLength,
    outputPtrPtr,
    outputSizePtr,
    reduceFraction,
    aggressiveness
  );

  // Extract the results
  const outputPtr = Module.HEAPU32[outputPtrPtr/4];
  const outputSize = Module.HEAPU32[outputSizePtr/4];

  let outputData = null;

  if (result === 0 && outputPtr !== 0 && outputSize > 0) {
    // Copy the output data to a JavaScript array
    outputData = new Uint8Array(Module.HEAPU8.buffer, outputPtr, outputSize).slice();
  }

  // Clean up allocated memory
  Module._free(inputPtr);
  Module._free(outputPtrPtr);
  Module._free(outputSizePtr);

  // If the C function allocated memory for the output buffer, free it
  if (outputPtr !== 0) {
    Module._free(outputPtr);
  }

  return outputData;
}
