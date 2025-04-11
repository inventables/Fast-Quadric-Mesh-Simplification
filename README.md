# Fast-Quadric-Mesh-Simplification

WebAssembly version of Fast Quadric Mesh Simplification

Forked from [myMiniFactory/Fast-Quadric-Mesh-Simplification/](https://github.com/myMiniFactory/Fast-Quadric-Mesh-Simplification/)

Which is a minimal fork of `src.cmd/` from [sp4cerat/Fast-Quadric-Mesh-Simplification](https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification)

## Building

You need to have [emscripten](https://emscripten.org/) installed locally, e.g. `brew install emscripten`.

Run `make` or `make debug`. If updating the `vendor/` files in Easel, you should do the former and copy over the generated `*.js` and `*.wasm` files.

It would be nice to have a non-minified JS bindings file in our vendor directory so it's human readable. Especially since it gets minified by `esbuild` anyway. However, it doesn't appear that's possible while also generating an optimized version of the WASM output.

## Testing

The `index.html` and `worker.js` files provide a basic page to test the simplification library. You have to run a local web server in this directory, e.g.

```sh
python3 -m http.server 8888 --bind 127.0.0.1
```

And then go to `localhost:8888`
