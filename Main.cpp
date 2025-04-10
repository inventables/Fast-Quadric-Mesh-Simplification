// Simple wrapper for Sven Forstmann's mesh simplification tool
//
// http://voxels.blogspot.com/2014/05/quadric-mesh-simplification-with-source.html
// https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//
// Forked from
// https://github.com/myMiniFactory/Fast-Quadric-Mesh-Simplification/
//
// This version targets WASM and avoids stdio dependencies and instead exposes
// raw buffer versions of the simplify API
//

#include "Simplify.h"
#include <stdint.h>
#include <vector>

// Define the format types for the raw API
enum MeshFormat {
    FORMAT_OBJ = 0,
    FORMAT_STL = 1
};

// Process buffers directly without file I/O, stdio, or timing functions
int simplify_raw(const uint8_t* input_buffer, size_t input_size, MeshFormat input_format,
                uint8_t** output_buffer, size_t* output_size, MeshFormat output_format,
                float reduceFraction, float agressiveness) {

    // Clear any existing data
    Simplify::vertices.clear();
    Simplify::triangles.clear();

    // Load based on input format
    bool load_success = false;
    if (input_format == FORMAT_OBJ) {
        load_success = Simplify::load_obj(input_buffer, input_size);
    } else if (input_format == FORMAT_STL) {
        load_success = Simplify::load_stl(input_buffer, input_size);
    } else {
        return EXIT_FAILURE;
    }

    if (!load_success) {
        return EXIT_FAILURE;
    }

    // Basic validation
    if ((Simplify::triangles.size() < 3) || (Simplify::vertices.size() < 3)) {
        return EXIT_FAILURE;
    }

    // Calculate target triangle count
    int target_count = Simplify::triangles.size() >> 1;

    if (reduceFraction > 1.0) reduceFraction = 1.0;
    if (reduceFraction <= 0.0) {
        return EXIT_FAILURE;
    }
    target_count = round((float)Simplify::triangles.size() * reduceFraction);

    if (target_count < 4) {
        return EXIT_FAILURE;
    }

    // Perform simplification
    int startSize = Simplify::triangles.size();
    Simplify::simplify_mesh(target_count, agressiveness);

    if (Simplify::triangles.size() >= startSize) {
        return EXIT_FAILURE;
    }

    // Write output to buffer based on format
    bool write_success = false;
    if (output_format == FORMAT_OBJ) {
        write_success = Simplify::write_obj(output_buffer, output_size);
    } else if (output_format == FORMAT_STL) {
        write_success = Simplify::write_stl(output_buffer, output_size);
    } else {
        return EXIT_FAILURE;
    }

    if (!write_success) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

#ifdef __EMSCRIPTEN__

extern "C" {

int simplify(const uint8_t* input_buffer, size_t input_size, int input_format,
                    uint8_t** output_buffer, size_t* output_size, int output_format,
                    float reduceFraction, float agressiveness) {
    return simplify_raw(input_buffer, input_size, (MeshFormat)input_format,
                        output_buffer, output_size, (MeshFormat)output_format,
                        reduceFraction, agressiveness);
}

}

#else

// Stub main for non-WASM builds
int main(int argc, const char * argv[]) {}

#endif
