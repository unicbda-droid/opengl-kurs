#include "MeshData.h"
#include <cstdio>

int main() {
    printf("=== Mirror Test ===\n");
    MeshData m;
    m.addVertex({-2, 0, -2, 0,1,0, 0,0});
    m.addVertex({ 2, 0, -2, 0,1,0, 1,0});
    m.addVertex({ 2, 0,  2, 0,1,0, 1,1});
    m.addVertex({-2, 0,  2, 0,1,0, 0,1});
    m.addFace(0, 1, 2, 3);
    m.recomputeNormals();
    printf("Before: %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.mirror(0); // mirror X
    printf("After mirror X: %d verts, %d faces\n", m.vertexCount(), m.faceCount());
    int pass = (m.vertexCount() == 8 && m.faceCount() == 2) ? 1 : 0;

    m.mirror(1); // mirror Y (now 16 verts, 4 faces should pass through)
    printf("After mirror Y: %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("test_mirror_result.obj");
    printf(pass ? "PASS\n" : "FAIL\n");

    printf("\n=== Weld Test ===\n");
    MeshData w;
    // Create two quads sharing a vertex pair
    // Quad A: 0,1,2,3 at y=0
    w.addVertex({-2,0,-2, 0,1,0, 0,0}); // 0
    w.addVertex({ 2,0,-2, 0,1,0, 1,0}); // 1
    w.addVertex({ 2,0, 2, 0,1,0, 1,1}); // 2
    w.addVertex({-2,0, 2, 0,1,0, 0,1}); // 3
    w.addFace(0,1,2,3);
    // Quad B: 0,4,5,3 (touching quad A at vertices 0 and 3)
    w.addVertex({-3,0,0, 0,1,0, 0,0}); // 4
    w.addVertex({ 3,0,0, 0,1,0, 1,0}); // 5
    w.addFace(0,4,5,3);
    w.recomputeNormals();
    printf("Before weld: %d verts, %d faces\n", w.vertexCount(), w.faceCount());

    // Select vertices 0 and 3 to weld... no, let's weld 0 and 3 (they're already the same point)
    // Actually let's just test welding two different vertices
    w.clearSelection();
    w.select(4, true);
    w.select(0, true); // weld vertex 4 into 0
    bool ok = w.weldSelected();
    printf("After weld (4→0): %d verts, %d faces  ok=%d\n", w.vertexCount(), w.faceCount(), ok);
    // Expected: 5 verts (vertex 4 removed), 2 faces (indices updated)

    pass = (w.vertexCount() == 5 && w.faceCount() == 2) ? 1 : 0;
    w.saveOBJ("test_weld_result.obj");
    printf(pass ? "PASS\n" : "FAIL\n");

    return pass ? 0 : 1;
}
