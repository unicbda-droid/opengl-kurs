#pragma once
#include <vector>
#include <string>
#include <GL/glew.h>

class MeshData {
public:
    struct Vertex {
        float px, py, pz;
        float nx, ny, nz;
        float tu, tv;
    };

    static MeshData createCube();
    static MeshData createPlane(int segX, int segZ);
    int addVertex(const Vertex& v);
    void addFace(int a, int b, int c, int d);

    void drawSolid() const;
    void drawWireframe() const;
    void drawNormal() const;
    void drawTexture() const;
    void drawVertices(float size) const;
    void drawPick(unsigned char r, unsigned char g, unsigned char b) const;

    int vertexCount() const { return (int)m_verts.size(); }
    int faceCount() const { return (int)m_indices.size() / 4; }

    const Vertex& vertex(int i) const { return m_verts[i]; }
    Vertex& vertex(int i) { return m_verts[i]; }
    void setVertex(int i, float x, float y, float z) { m_verts[i].px = x; m_verts[i].py = y; m_verts[i].pz = z; }
    void getVertex(int i, float& x, float& y, float& z) const { x = m_verts[i].px; y = m_verts[i].py; z = m_verts[i].pz; }

    int faceVertex(int faceIdx, int corner) const { return m_indices[faceIdx * 4 + corner]; }

    bool isSelected(int vi) const { return vi < (int)m_sel.size() && m_sel[vi]; }
    void select(int vi, bool s) { if (vi < (int)m_sel.size()) m_sel[vi] = s; }
    void clearSelection();
    int selectedCount() const;
    int firstSelected() const;
    bool getSelectedEdge(int& v0, int& v1) const;
    void translateSelected(float dx, float dy, float dz);
    void getSelectionCenter(float& cx, float& cy, float& cz) const;

    void drawHighlightedEdges() const;
    void drawHighlightedFace() const;

    int pickFace(int mx, int my, int vw, int vh) const;

    int selectedFace() const { return m_selectedFace; }
    void selectFace(int f) { m_selectedFace = f; }
    void clearFaceSelection() { m_selectedFace = -1; }

    static bool loadOBJ(const std::string& path, MeshData& out);
    bool saveOBJ(const std::string& path) const;
    bool loopCut(int edgeV0, int edgeV1);
    bool bridge(int loop1V0, int loop1V1, int loop2V0, int loop2V1);

    void subdivide();
    void extrude(int faceIdx, float dist);
    void inset(int faceIdx, float amount);
    void mirror(int axis);
    bool weldSelected();
    bool bevelEdge(int v0, int v1, float t);
    void recomputeNormals();

    const std::vector<Vertex>& vertices() const { return m_verts; }
    const std::vector<unsigned int>& indices() const { return m_indices; }

private:
    std::vector<Vertex> m_verts;
    std::vector<unsigned int> m_indices;
    std::vector<bool> m_sel;
    int m_selectedFace = -1;
    GLuint m_texId = 0;

    void ensureTexture();
};
