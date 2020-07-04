// import the emscripten glue code
import emscripten from './build/module.js'

addEventListener('fetch', (event) => {
  event.respondWith(handleRequest(event))
})

// this is where the magic happens
// we send our own instantiateWasm function
// to the emscripten module
// so we can initialize the WASM instance ourselves
// since Workers puts your wasm file in global scope
// as a binding. In this case, this binding is called
// `wasm` as that is the name Wrangler uses
// for any uploaded wasm module
let emscripten_module = new Promise((resolve, reject) => {
  emscripten({
    instantiateWasm(info, receive) {
      let instance = new WebAssembly.Instance(wasm, info)
      receive(instance)
      return instance.exports
    },
  }).then((module) => {
    resolve({
      init: module.cwrap('init', 'number', ['number']),
      execute: module.cwrap('execute', 'number', []),
      retrieve: module.cwrap('retrieve', 'number', []),
      module: module,
    })
  })
})

function parseQueryString(request) {
  const params = {}
  const url = new URL(request.url)
  const queryString = url.search.slice(1).split('&')
  queryString.forEach((item) => {
    const kv = item.split('=')
    if (kv[0]) params[kv[0]] = kv[1] || true
  })
  return params
}

async function handleRequest(event) {
  const request = event.request
  const response = await fetch(request)

  const params = parseQueryString(request)

  if (params.noexecute) {
    const plainResponse = new Response(response.body, response)
    plainResponse.headers.set('Content-Type', 'text/plain')
    return plainResponse
  }

  const evaluator = await emscripten_module

  // Pass request body to evaluator
  const script = new Uint8Array(await response.arrayBuffer())
  const scriptPtr = evaluator.init(script.length)
  evaluator.module.HEAPU8.set(script, scriptPtr)

  // Execute and retrieve result
  const resultSize = evaluator.execute()
  if (resultSize <= 1) {
    return response
  }
  const resultPtr = evaluator.retrieve()
  const result = new Uint8Array(
    evaluator.module.HEAPU8.subarray(resultPtr, resultPtr + resultSize),
  )

  return new Response(result, response)
}
