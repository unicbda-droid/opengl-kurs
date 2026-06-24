#include "MeshData.h"
#include <cstdio>
#include <cmath>

static const float PI = 3.14159265f;

int main() {
    printf("=== DEMO: Alle Mesh-Features ===\n\n");

    // STEP 1: Create a 4x4 plane
    printf("1. Plane 4x4 erstellen\n");
    MeshData m = MeshData::createPlane(4, 4);
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    // STEP 2: Loop cut twice to get more geometry
    printf("\n2. Loop Cuts (Kanten 6-7 und 12-13)\n");
    m.loopCut(6, 7);
    printf("   -> nach 1. Cut: %d verts, %d faces\n", m.vertexCount(), m.faceCount());
    m.loopCut(12, 13);
    printf("   -> nach 2. Cut: %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    // STEP 3: Inset all faces
    printf("\n3. Inset 0.15 auf alle Faces\n");
    int fc = m.faceCount();
    for (int f = fc - 1; f >= 0; f--) m.inset(f, 0.15f);
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("demo_01_after_inset.obj");

    // STEP 4: Extrude center face
    printf("\n4. Extrude Face 0 (center) um 2.0\n");
    m.extrude(0, 2.0f);
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("demo_02_after_extrude.obj");

    // STEP 5: Mirror on X axis
    printf("\n5. Mirror auf X-Achse\n");
    m.mirror(0);
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("demo_03_after_mirror.obj");

    // STEP 6: Bridge two edge loops
    // Create two separate quads in a new mesh and bridge them
    printf("\n6. Bridge Edge Loops (zwei Quads verbinden)\n");
    MeshData bridgeMesh;
    // Quad A (bottom)
    bridgeMesh.addVertex({-3, 0, -3, 0,1,0, 0,0}); // 0
    bridgeMesh.addVertex({ 3, 0, -3, 0,1,0, 1,0}); // 1
    bridgeMesh.addVertex({ 3, 0,  3, 0,1,0, 1,1}); // 2
    bridgeMesh.addVertex({-3, 0,  3, 0,1,0, 0,1}); // 3
    bridgeMesh.addFace(0, 1, 2, 3);
    // Quad B (top)
    bridgeMesh.addVertex({-3, 5, -3, 0,1,0, 0,0}); // 4
    bridgeMesh.addVertex({ 3, 5, -3, 0,1,0, 1,0}); // 5
    bridgeMesh.addVertex({ 3, 5,  3, 0,1,0, 1,1}); // 6
    bridgeMesh.addVertex({-3, 5,  3, 0,1,0, 0,1}); // 7
    bridgeMesh.addFace(4, 5, 6, 7);
    bridgeMesh.recomputeNormals();
    printf("   -> vor Bridge: %d verts, %d faces\n", bridgeMesh.vertexCount(), bridgeMesh.faceCount());
    bridgeMesh.bridge(0, 1, 4, 5);
    printf("   -> nach Bridge: %d verts, %d faces\n", bridgeMesh.vertexCount(), bridgeMesh.faceCount());
    bridgeMesh.saveOBJ("demo_04_bridge.obj");

    // STEP 7: Subdivide to smooth
    printf("\n7. Subdivide\n");
    m.subdivide();
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("demo_05_subdivided.obj");

    // STEP 8: Weld two vertices
    printf("\n8. Weld (vertices 0 und 10 verschmelzen)\n");
    m.clearSelection();
    m.select(0, true);
    m.select(10, true);
    m.weldSelected();
    printf("   -> %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    m.saveOBJ("demo_06_final.obj");

    printf("\n=== DEMO ENDE ===\n");
    printf("OBJ-Dateien zum Anschauen:\n");
    printf("  demo_01_after_inset.obj\n");
    printf("  demo_02_after_extrude.obj\n");
    printf("  demo_03_after_mirror.obj\n");
    printf("  demo_04_bridge.obj\n");
    printf("  demo_05_subdivided.obj\n");
    printf("  demo_06_final.obj\n");
    printf("\nOeffne sie in Blender / Windows 3D-Viewer / Meshlab\n");
    return 0;
}
