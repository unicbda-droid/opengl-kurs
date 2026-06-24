#include "MeshData.h"
#include <cstdio>

int main() {
    printf("=== Inset Test ===\n\n");

    // Single quad
    MeshData m;
    m.addVertex({-2, 0, -2, 0,1,0, 0,0});
    m.addVertex({ 2, 0, -2, 0,1,0, 1,0});
    m.addVertex({ 2, 0,  2, 0,1,0, 1,1});
    m.addVertex({-2, 0,  2, 0,1,0, 0,1});
    m.addFace(0, 1, 2, 3);
    m.recomputeNormals();

    printf("Before inset: %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.inset(0, 0.3f);

    printf("After inset (0.3): %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    if (m.vertexCount() == 8 && m.faceCount() == 5) {
        printf("PASS: Counts match (8 verts, 5 faces)\n");
    } else {
        printf("NOTE: Expected 8 verts, 5 faces\n");
    }

    m.saveOBJ("test_inset_result.obj");

    printf("\n=== Done ===\n");
    return 0;
}
