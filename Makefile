CXX := Main.cpp
O3 := -O3
WASM := -s NO_FILESYSTEM=1 -s ALLOW_MEMORY_GROWTH=1 -s WASM=1 \
        -s EXPORTED_FUNCTIONS='["_simplify", "_malloc", "_free"]' \
        -s ENVIRONMENT='web,worker' \
        -s MODULARIZE=1 \
        -s EXPORT_ES6=0 \
        -s SINGLE_FILE=0 \
        -s EXPORT_NAME="createSimplifyModule"

# Might also need these too
# -s NODEJS_CATCH_EXIT=0 -s NODEJS_CATCH_REJECTION=0

OUT := -o Fast-Quadric-Mesh-Simplification.js

all: release

# Unfortunately there's now good way to generate optimized WASM with unmangled
# JS bindings.
release:
	em++ ${CXX} ${WASM} ${O3} ${OUT}
debug:
	em++ ${CXX} ${WASM} ${OUT}
