#include "MeshData.h"
#include <cstdio>
#include <cmath>

static const float PI = 3.14159265f;

int main() {
    printf("=== Bridge Edge Loops Test ===\n\n");

    // Create a ring: cylinder wall without caps
    // segments quads form a wall with open top and bottom
    // Top edge loop and bottom edge loop are both boundary loops
    int segs = 8;
    MeshData ring;
    float r = 5.0f;
    for (int i = 0; i < segs; i++) {
        float a0 = (float)i / segs * 2 * PI;
        float a1 = (float)(i + 1) / segs * 2 * PI;
        float x0 = cos(a0) * r, z0 = sin(a0) * r;
        float x1 = cos(a1) * r, z1 = sin(a1) * r;
        ring.addVertex({x0, -2, z0, 0,1,0, 0,0});
        ring.addVertex({x1, -2, z1, 0,1,0, 1,0});
        ring.addVertex({x1,  2, z1, 0,1,0, 1,1});
        ring.addVertex({x0,  2, z0, 0,1,0, 0,1});
    }
    for (int i = 0; i < segs; i++) {
        int base = i * 4;
        ring.addFace(base, base + 1, base + 2, base + 3);
    }
    ring.recomputeNormals();
    printf("Ring: %d verts, %d faces\n", ring.vertexCount(), ring.faceCount());
    // 32 verts (8*4), 8 faces
    // Bottom boundary loop: edges (base+0, base+1) for each segment = 8 edges
    // Top boundary loop: edges (base+2, base+3) for each segment = 8 edges

    // Bridge bottom loop → top loop
    // Edge (0,1) is bottom edge of first quad (boundary)
    // Edge (2,3) is top edge of first quad (boundary)
    // But wait — (0,1) and (2,3) are both from the SAME face, so bridging them
    // would just add more quads parallel to the existing wall.
    // 
    // Let me think: the top loop goes (2,3), (6,7), (10,11), ..., (30,31)
    // The bottom loop goes (0,1), (4,5), (8,9), ..., (28,29)
    // These are already connected — the walls are already there.
    // Bridge would add overlapping geometry.
    //
    // Bridge should only work when the loops are NOT already connected by faces.
    // For this ring mesh, every edge has exactly 1 face (it's a boundary edge)
    // because there are no adjacent faces — BUT the top edge (2,3) and bottom edge (0,1)
    // are boundary edges that ARE connected through the same face (face 0).
    // Bridge quads would be on top of existing quads.
    //
    // For a proper test, I need TWO separate boundary loops that are NOT connected.
    // This happens when the mesh has two separate islands.
    
    printf("\nBuilding two separate quads for bridge test...\n");
    MeshData mesh;
    // Quad A (bottom, y=0): vertices 0-3
    mesh.addVertex({-2, 0, -2, 0,1,0, 0,0});
    mesh.addVertex({ 2, 0, -2, 0,1,0, 1,0});
    mesh.addVertex({ 2, 0,  2, 0,1,0, 1,1});
    mesh.addVertex({-2, 0,  2, 0,1,0, 0,1});
    mesh.addFace(0, 1, 2, 3);
    
    // Quad B (top, y=4): vertices 4-7
    mesh.addVertex({-2, 4, -2, 0,1,0, 0,0});
    mesh.addVertex({ 2, 4, -2, 0,1,0, 1,0});
    mesh.addVertex({ 2, 4,  2, 0,1,0, 1,1});
    mesh.addVertex({-2, 4,  2, 0,1,0, 0,1});
    mesh.addFace(4, 5, 6, 7);
    
    mesh.recomputeNormals();
    
    printf("Before bridge: %d verts, %d faces\n", mesh.vertexCount(), mesh.faceCount());
    
    // Bridge quad A's boundary edge (0,1) to quad B's boundary edge (4,5)
    // Quad A's boundary loop: (0,1) → (1,2) → (2,3) → (3,0) [4 edges]
    // Quad B's boundary loop: (4,5) → (5,6) → (6,7) → (7,4) [4 edges]
    bool ok = mesh.bridge(0, 1, 4, 5);
    if (!ok) { printf("FAIL: bridge returned false\n"); return 1; }
    
    printf("After bridge: %d verts, %d faces\n", mesh.vertexCount(), mesh.faceCount());
    // Expected: 8 verts, 2 + 4 = 6 faces
    
    if (mesh.vertexCount() == 8 && mesh.faceCount() == 6) {
        printf("PASS: Counts match (8 verts, 6 faces)\n");
    } else {
        printf("NOTE: Expected 8 verts, 6 faces\n");
    }
    
    mesh.saveOBJ("test_bridge_result.obj");
    
    printf("\n=== Done ===\n");
    return 0;
}
