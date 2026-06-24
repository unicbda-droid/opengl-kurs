#include "MeshData.h"
#include <cstdio>
#include <cstring>

static int failed = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("  FAIL: %s\n", msg); failed++; } \
    else { printf("  PASS: %s\n", msg); } \
} while(0)

int main() {
    printf("=== Face Selection Test ===\n\n");

    // Create a cube (6 faces)
    MeshData m = MeshData::createCube();

    printf("Cube: %d verts, %d faces\n", m.vertexCount(), m.faceCount());

    // Test: selectFace / selectedFace
    CHECK(m.selectedFace() == -1, "no face selected initially");
    m.selectFace(0);
    CHECK(m.selectedFace() == 0, "selectFace(0) works");
    m.clearFaceSelection();
    CHECK(m.selectedFace() == -1, "clearFaceSelection works");

    // Test: clearSelection also clears face
    m.selectFace(2);
    m.clearSelection();
    CHECK(m.selectedFace() == -1, "clearSelection clears face");
    CHECK(m.vertexCount() == 24, "clearSelection preserves mesh");

    // Test: targeted extrude on single face
    printf("\n-- Targeted extrude --\n");
    m.selectFace(0);
    int beforeVerts = m.vertexCount();
    int beforeFaces = m.faceCount();
    m.extrude(0, 0.5f);
    CHECK(m.vertexCount() == beforeVerts + 4, "extrude adds 4 verts");
    CHECK(m.faceCount() == beforeFaces + 4, "extrude adds 4 faces");
    m.saveOBJ("test_facesel_extrude.obj");
    printf("  saved test_facesel_extrude.obj (%d verts, %d faces)\n",
           m.vertexCount(), m.faceCount());

    // Test: targeted inset on single face
    // (cube = 24 verts, 6 faces; inset adds 4 verts + 4 side faces = 28/10)
    printf("\n-- Targeted inset --\n");
    MeshData m2 = MeshData::createCube();
    m2.selectFace(0);
    m2.inset(0, 0.2f);
    CHECK(m2.vertexCount() == 28, "inset on face 0: 24+4=28 verts");
    CHECK(m2.faceCount() == 10, "inset on face 0: 6+4=10 faces");
    m2.saveOBJ("test_facesel_inset.obj");
    printf("  saved test_facesel_inset.obj (%d verts, %d faces)\n",
           m2.vertexCount(), m2.faceCount());

    // Test: operations that change topology clear face selection
    printf("\n-- Topology changes clear face selection --\n");
    m2.selectFace(2);
    CHECK(m2.selectedFace() == 2, "face 2 selected before subdivide");
    m2.subdivide();
    CHECK(m2.selectedFace() == -1, "subdivide clears face selection");

    // Test: mirror clears face selection
    MeshData m3 = MeshData::createCube();
    m3.selectFace(0);
    m3.mirror(0);
    CHECK(m3.selectedFace() == -1, "mirror clears face selection");

    // Test: weld clears face selection (need 2 selected verts first)
    MeshData m4 = MeshData::createCube();
    m4.selectFace(0);
    m4.select(0, true);
    m4.select(1, true);
    m4.weldSelected();
    CHECK(m4.selectedFace() == -1, "weld clears face selection");

    printf("\n=== Results: ");
    if (failed == 0) printf("ALL PASSED");
    else printf("%d FAILED", failed);
    printf(" ===\n");
    return failed;
}
