CXX := Main.cpp
O3 := -O3
WASM := -s NO_FILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 -s EXPORTED_FUNCTIONS='["_simplify", "_malloc", "_free"]'
OUT := -o Fast-Quadric-Mesh-Simplification.js

all: release

# Unfortunately there's now good way to generate optimized WASM with unmangled
# JS bindings. Instead, you can `make wasm_debug` to get the human readable JS
# first and include that when vendoring since it'll be run through esbuild.
release:
	em++ ${CXX} ${WASM} ${O3} ${OUT}
debug:
	em++ ${CXX} ${WASM} ${OUT}
