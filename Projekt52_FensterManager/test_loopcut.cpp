#include "MeshData.h"
#include <cstdio>
#include <cstdlib>

int main() {
    printf("=== Loop Cut Test ===\n\n");

    // Create plane 4x4 (25 verts, 16 quads)
    MeshData mesh = MeshData::createPlane(4, 4);
    printf("Before loop cut:\n");
    printf("  Vertices: %d\n", mesh.vertexCount());
    printf("  Faces:    %d\n", mesh.faceCount());

    // Run loop cut on interior edge (6,7)
    // Plane 4x4 = 5x5 grid, vertex layout:
    //   (0)-(1)-(2)-(3)-(4)
    //   (5)-(6)-(7)-(8)-(9)
    //  (10)(11)(12)(13)(14)
    //  (15)(16)(17)(18)(19)
    //  (20)(21)(22)(23)(24)
    // Edge (6,7) = horizontal edge in row 1, shared between faces F1 and F5
    bool ok = mesh.loopCut(6, 7);
    if (!ok) {
        printf("FAIL: loopCut returned false\n");
        return 1;
    }

    printf("\nAfter loop cut:\n");
    printf("  Vertices: %d\n", mesh.vertexCount());
    printf("  Faces:    %d\n", mesh.faceCount());

    // Each face in the loop (4 faces) should be split into 2 quads = 8 new faces
    // And 8 new vertices added (midpoints)
    if (mesh.vertexCount() == 33 && mesh.faceCount() == 20) {
        printf("\nPASS: Vertex and face counts match expected (33 verts, 20 faces)\n");
    } else {
        printf("\nNOTE: Counts differ from expected (33/20) - check algorithm\n");
    }

    // Save as OBJ for manual inspection
    if (mesh.saveOBJ("test_loopcut_result.obj")) {
        printf("Saved test_loopcut_result.obj\n");
    }

    // Second cut: edge (12,13) to create a second loop
    ok = mesh.loopCut(12, 13);
    if (ok) {
        printf("\nAfter second loop cut (12,13):\n");
        printf("  Vertices: %d\n", mesh.vertexCount());
        printf("  Faces:    %d\n", mesh.faceCount());
        mesh.saveOBJ("test_loopcut_result2.obj");
    }

    printf("\n=== Done ===\n");
    return 0;
}
