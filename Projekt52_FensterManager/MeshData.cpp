#include "MeshData.h"
#include <GL/glew.h>
#include <cmath>
#include <cfloat>
#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>

static const float PI = 3.14159265f;

MeshData MeshData::createCube() {
    MeshData m;
    m.m_verts = {
        { -1,-1, 1,  0,0,1,  0,0 }, {  1,-1, 1,  0,0,1,  1,0 },
        {  1, 1, 1,  0,0,1,  1,1 }, { -1, 1, 1,  0,0,1,  0,1 },
        {  1,-1,-1,  0,0,-1, 0,0 }, { -1,-1,-1,  0,0,-1, 1,0 },
        { -1, 1,-1,  0,0,-1, 1,1 }, {  1, 1,-1,  0,0,-1, 0,1 },
        { -1, 1, 1,  0,1,0,  0,0 }, {  1, 1, 1,  0,1,0,  1,0 },
        {  1, 1,-1,  0,1,0,  1,1 }, { -1, 1,-1,  0,1,0,  0,1 },
        { -1,-1,-1,  0,-1,0, 0,0 }, {  1,-1,-1,  0,-1,0, 1,0 },
        {  1,-1, 1,  0,-1,0, 1,1 }, { -1,-1, 1,  0,-1,0, 0,1 },
        {  1,-1, 1,  1,0,0,  0,0 }, {  1,-1,-1,  1,0,0,  1,0 },
        {  1, 1,-1,  1,0,0,  1,1 }, {  1, 1, 1,  1,0,0,  0,1 },
        { -1,-1,-1, -1,0,0,  0,0 }, { -1,-1, 1, -1,0,0,  1,0 },
        { -1, 1, 1, -1,0,0,  1,1 }, { -1, 1,-1, -1,0,0,  0,1 },
    };
    m.m_indices = {
        0,1,2,3, 4,5,6,7, 8,9,10,11,
        12,13,14,15, 16,17,18,19, 20,21,22,23
    };
    m.m_sel.resize(24, false);
    return m;
}

MeshData MeshData::createPlane(int segX, int segZ) {
    MeshData m;
    for (int z = 0; z <= segZ; z++) {
        for (int x = 0; x <= segX; x++) {
            float fx = (float)x / segX - 0.5f;
            float fz = (float)z / segZ - 0.5f;
            m.m_verts.push_back({ fx*10, 0, fz*10, 0,1,0, (float)x/segX, (float)z/segZ });
        }
    }
    for (int z = 0; z < segZ; z++) {
        for (int x = 0; x < segX; x++) {
            int a = z * (segX+1) + x;
            int b = a + 1;
            int c = (z+1) * (segX+1) + x;
            int d = c + 1;
            m.m_indices.push_back(a); m.m_indices.push_back(b);
            m.m_indices.push_back(d); m.m_indices.push_back(c);
        }
    }
    m.m_sel.resize(m.m_verts.size(), false);
    m.recomputeNormals();
    return m;
}

int MeshData::addVertex(const Vertex& v) {
    int idx = (int)m_verts.size();
    m_verts.push_back(v);
    m_sel.push_back(false);
    return idx;
}

void MeshData::addFace(int a, int b, int c, int d) {
    m_indices.push_back(a);
    m_indices.push_back(b);
    m_indices.push_back(c);
    m_indices.push_back(d);
}

static void emitQuad(const std::vector<MeshData::Vertex>& verts, const std::vector<unsigned int>& indices, size_t i) {
    int i0 = indices[i+0], i1 = indices[i+1], i2 = indices[i+2], i3 = indices[i+3];
    const auto& v0 = verts[i0];
    const auto& v1 = verts[i1];
    const auto& v2 = verts[i2];
    // First triangle: v0,v1,v2
    glVertex3f(v0.px, v0.py, v0.pz);
    glVertex3f(v1.px, v1.py, v1.pz);
    glVertex3f(v2.px, v2.py, v2.pz);
    // Second triangle: v0,v2,v3 (skip if degenerate, e.g. triangle loaded as quad)
    if (i2 != i3) {
        const auto& v3 = verts[i3];
        glVertex3f(v0.px, v0.py, v0.pz);
        glVertex3f(v2.px, v2.py, v2.pz);
        glVertex3f(v3.px, v3.py, v3.pz);
    }
}
static void emitQuadNormal(const std::vector<MeshData::Vertex>& verts, const std::vector<unsigned int>& indices, size_t i) {
    int i0 = indices[i+0], i1 = indices[i+1], i2 = indices[i+2], i3 = indices[i+3];
    const auto& v0 = verts[i0], & v1 = verts[i1], & v2 = verts[i2];
    glNormal3f(v0.nx, v0.ny, v0.nz); glVertex3f(v0.px, v0.py, v0.pz);
    glNormal3f(v1.nx, v1.ny, v1.nz); glVertex3f(v1.px, v1.py, v1.pz);
    glNormal3f(v2.nx, v2.ny, v2.nz); glVertex3f(v2.px, v2.py, v2.pz);
    if (i2 != i3) {
        const auto& v3 = verts[i3];
        glNormal3f(v0.nx, v0.ny, v0.nz); glVertex3f(v0.px, v0.py, v0.pz);
        glNormal3f(v2.nx, v2.ny, v2.nz); glVertex3f(v2.px, v2.py, v2.pz);
        glNormal3f(v3.nx, v3.ny, v3.nz); glVertex3f(v3.px, v3.py, v3.pz);
    }
}
static void emitQuadColor(const std::vector<MeshData::Vertex>& verts, const std::vector<unsigned int>& indices, size_t i) {
    int i0 = indices[i+0], i1 = indices[i+1], i2 = indices[i+2], i3 = indices[i+3];
    const auto& v0 = verts[i0], & v1 = verts[i1], & v2 = verts[i2];
    auto emit = [](const auto& v) { glColor3f(v.nx*0.5f+0.5f, v.ny*0.5f+0.5f, v.nz*0.5f+0.5f); glNormal3f(v.nx, v.ny, v.nz); glVertex3f(v.px, v.py, v.pz); };
    emit(v0); emit(v1); emit(v2);
    if (i2 != i3) { emit(v0); emit(v2); emit(verts[i3]); }
}
static void emitQuadTex(const std::vector<MeshData::Vertex>& verts, const std::vector<unsigned int>& indices, size_t i) {
    int i0 = indices[i+0], i1 = indices[i+1], i2 = indices[i+2], i3 = indices[i+3];
    const auto& v0 = verts[i0], & v1 = verts[i1], & v2 = verts[i2];
    auto emit = [](const auto& v) { glTexCoord2f(v.tu, v.tv); glNormal3f(v.nx, v.ny, v.nz); glVertex3f(v.px, v.py, v.pz); };
    emit(v0); emit(v1); emit(v2);
    if (i2 != i3) { emit(v0); emit(v2); emit(verts[i3]); }
}

void MeshData::drawSolid() const {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < m_indices.size(); i += 4)
        emitQuadNormal(m_verts, m_indices, i);
    glEnd();
}

void MeshData::drawWireframe() const {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < m_indices.size(); i += 4)
        emitQuad(m_verts, m_indices, i);
    glEnd();
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void MeshData::drawNormal() const {
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < m_indices.size(); i += 4)
        emitQuadColor(m_verts, m_indices, i);
    glEnd();
}

void MeshData::ensureTexture() {
    if (m_texId != 0) return;
    glGenTextures(1, &m_texId);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    unsigned char data[256*256*3];
    for (int y = 0; y < 256; y++) {
        for (int x = 0; x < 256; x++) {
            int idx = (y * 256 + x) * 3;
            bool w = ((x / 32) + (y / 32)) % 2 == 0;
            data[idx+0] = w ? 255 : 60;
            data[idx+1] = w ? 255 : 60;
            data[idx+2] = w ? 255 : 60;
        }
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void MeshData::drawTexture() const {
    const_cast<MeshData*>(this)->ensureTexture();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_texId);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glColor3f(1,1,1);
    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < m_indices.size(); i += 4)
        emitQuadTex(m_verts, m_indices, i);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void MeshData::drawVertices(float size) const {
    glPointSize(size);
    glBegin(GL_POINTS);
    for (int i = 0; i < (int)m_verts.size(); i++) {
        if (m_sel[i]) glColor3f(1,1,0);
        else glColor3f(1,0.5f,0);
        glVertex3f(m_verts[i].px, m_verts[i].py, m_verts[i].pz);
    }
    glEnd();
    glPointSize(1.0f);
}

void MeshData::drawPick(unsigned char r, unsigned char g, unsigned char b) const {
    glColor3ub(r, g, b);
    drawSolid();
}

void MeshData::clearSelection() {
    for (auto& s : m_sel) s = false;
    m_selectedFace = -1;
}

int MeshData::selectedCount() const {
    int c = 0;
    for (auto s : m_sel) if (s) c++;
    return c;
}

int MeshData::firstSelected() const {
    for (int i = 0; i < (int)m_sel.size(); i++)
        if (m_sel[i]) return i;
    return -1;
}

bool MeshData::getSelectedEdge(int& v0, int& v1) const {
    v0 = v1 = -1;
    for (int i = 0; i < (int)m_sel.size(); i++) {
        if (m_sel[i]) {
            if (v0 < 0) v0 = i;
            else { v1 = i; return true; }
        }
    }
    return false;
}

void MeshData::drawHighlightedEdges() const {
    int v0, v1;
    if (!getSelectedEdge(v0, v1)) return;
    glDisable(GL_DEPTH_TEST);
    glColor3f(1, 0.2f, 0.2f);
    glLineWidth(4.0f);
    glBegin(GL_LINES);
    glVertex3f(m_verts[v0].px, m_verts[v0].py, m_verts[v0].pz);
    glVertex3f(m_verts[v1].px, m_verts[v1].py, m_verts[v1].pz);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_DEPTH_TEST);
}

void MeshData::drawHighlightedFace() const {
    if (m_selectedFace < 0) return;
    int fi = m_selectedFace;
    int idx0 = m_indices[fi * 4 + 0];
    int idx1 = m_indices[fi * 4 + 1];
    int idx2 = m_indices[fi * 4 + 2];
    int idx3 = m_indices[fi * 4 + 3];
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor4f(1.0f, 0.8f, 0.0f, 0.4f);
    glBegin(GL_TRIANGLES);
    glVertex3f(m_verts[idx0].px, m_verts[idx0].py, m_verts[idx0].pz);
    glVertex3f(m_verts[idx1].px, m_verts[idx1].py, m_verts[idx1].pz);
    glVertex3f(m_verts[idx2].px, m_verts[idx2].py, m_verts[idx2].pz);
    if (idx2 != idx3) {
        glVertex3f(m_verts[idx0].px, m_verts[idx0].py, m_verts[idx0].pz);
        glVertex3f(m_verts[idx2].px, m_verts[idx2].py, m_verts[idx2].pz);
        glVertex3f(m_verts[idx3].px, m_verts[idx3].py, m_verts[idx3].pz);
    }
    glEnd();
    glColor4f(1, 0.5f, 0, 0.5f);
    glLineWidth(3.0f);
    glBegin(GL_LINE_LOOP);
    glVertex3f(m_verts[idx0].px, m_verts[idx0].py, m_verts[idx0].pz);
    glVertex3f(m_verts[idx1].px, m_verts[idx1].py, m_verts[idx1].pz);
    glVertex3f(m_verts[idx2].px, m_verts[idx2].py, m_verts[idx2].pz);
    glVertex3f(m_verts[idx3].px, m_verts[idx3].py, m_verts[idx3].pz);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
}

static void projectPoint(float x, float y, float z,
    const GLdouble* model, const GLdouble* proj, const GLint* view,
    double& sx, double& sy) {
    // transform by modelview matrix
    double ex = model[0]*x + model[4]*y + model[8]*z + model[12];
    double ey = model[1]*x + model[5]*y + model[9]*z + model[13];
    double ez = model[2]*x + model[6]*y + model[10]*z + model[14];
    double ew = model[3]*x + model[7]*y + model[11]*z + model[15];
    // transform by projection matrix
    double px = proj[0]*ex + proj[4]*ey + proj[8]*ez + proj[12]*ew;
    double py = proj[1]*ex + proj[5]*ey + proj[9]*ez + proj[13]*ew;
    double pz = proj[2]*ex + proj[6]*ey + proj[10]*ez + proj[14]*ew;
    double pw = proj[3]*ex + proj[7]*ey + proj[11]*ez + proj[15]*ew;
    if (pw == 0) pw = 1;
    // perspective divide + viewport
    sx = view[0] + view[2] * (px / pw + 1.0) / 2.0;
    sy = view[1] + view[3] * (py / pw + 1.0) / 2.0;
}

int MeshData::pickFace(int mx, int my, int vw, int vh) const {
    int bestFace = -1;
    float bestDist = 20.0f;
    GLdouble model[16], proj[16];
    GLint view[4];
    glGetDoublev(GL_MODELVIEW_MATRIX, model);
    glGetDoublev(GL_PROJECTION_MATRIX, proj);
    glGetIntegerv(GL_VIEWPORT, view);
    int nFaces = (int)m_indices.size() / 4;
    for (int fi = 0; fi < nFaces; fi++) {
        float cx = 0, cy = 0, cz = 0;
        for (int c = 0; c < 4; c++) {
            int vi = m_indices[fi * 4 + c];
            cx += m_verts[vi].px;
            cy += m_verts[vi].py;
            cz += m_verts[vi].pz;
        }
        cx /= 4; cy /= 4; cz /= 4;
        double sx, sy;
        projectPoint(cx, cy, cz, model, proj, view, sx, sy);
        double dx = sx - mx;
        double dy = sy - my;
        double dist = sqrt(dx * dx + dy * dy);
        if (dist < bestDist) {
            bestDist = (float)dist;
            bestFace = fi;
        }
    }
    return bestFace;
}

void MeshData::translateSelected(float dx, float dy, float dz) {
    for (int i = 0; i < (int)m_verts.size(); i++) {
        if (m_sel[i]) {
            m_verts[i].px += dx;
            m_verts[i].py += dy;
            m_verts[i].pz += dz;
        }
    }
}

void MeshData::getSelectionCenter(float& cx, float& cy, float& cz) const {
    cx = cy = cz = 0;
    int n = 0;
    for (int i = 0; i < (int)m_verts.size(); i++) {
        if (m_sel[i]) {
            cx += m_verts[i].px;
            cy += m_verts[i].py;
            cz += m_verts[i].pz;
            n++;
        }
    }
    if (n > 0) { cx /= n; cy /= n; cz /= n; }
}

struct OBJVertKey {
    int v, vt, vn;
    bool operator==(const OBJVertKey& o) const { return v == o.v && vt == o.vt && vn == o.vn; }
};
struct OBJVertKeyHash {
    size_t operator()(const OBJVertKey& k) const {
        return ((size_t)(unsigned int)k.v << 32) | ((size_t)(unsigned int)k.vt << 16) | (size_t)(unsigned int)k.vn;
    }
};

bool MeshData::loadOBJ(const std::string& path, MeshData& out) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) { fprintf(stderr, "OBJ: kann %s nicht oeffnen\n", path.c_str()); return false; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    char* buf = (char*)malloc(len + 1);
    if (!buf) { fclose(f); return false; }
    fread(buf, 1, len, f); buf[len] = 0;
    fclose(f);

    std::vector<float> pos, tex, norm;
    std::vector<Vertex> verts;
    std::vector<unsigned int> indices;
    std::unordered_map<OBJVertKey, unsigned int, OBJVertKeyHash> vertCache;
    bool hasNormals = false;

    auto getOrCreateVert = [&](int vi, int ti, int ni) -> unsigned int {
        OBJVertKey key{vi, ti, ni};
        auto it = vertCache.find(key);
        if (it != vertCache.end()) return it->second;
        Vertex v{};
        if (vi > 0 && (vi-1)*3+2 < pos.size()) {
            v.px = pos[(vi-1)*3];
            v.py = pos[(vi-1)*3+1];
            v.pz = pos[(vi-1)*3+2];
        }
        if (ti > 0 && (ti-1)*2+1 < tex.size()) {
            v.tu = tex[(ti-1)*2];
            v.tv = tex[(ti-1)*2+1];
        }
        if (ni > 0 && (ni-1)*3+2 < norm.size()) {
            v.nx = norm[(ni-1)*3];
            v.ny = norm[(ni-1)*3+1];
            v.nz = norm[(ni-1)*3+2];
        }
        unsigned int idx = (unsigned int)verts.size();
        verts.push_back(v);
        vertCache[key] = idx;
        return idx;
    };

    char* line = buf;
    char* next;
    while (line && *line) {
        if ((next = strchr(line, '\n'))) { *next++ = 0; } else { next = nullptr; }
        // trim
        char* e = line + strlen(line) - 1;
        while (e >= line && (*e == ' ' || *e == '\r' || *e == '\n')) *e-- = 0;
        if (line[0] == '#' || line[0] == 0) { line = next; continue; }

        if (line[0] == 'v' && line[1] == ' ') {
            float x, y, z;
            if (sscanf(line, "v %f %f %f", &x, &y, &z) == 3) {
                pos.push_back(x); pos.push_back(y); pos.push_back(z);
            }
        } else if (line[0] == 'v' && line[1] == 't') {
            float u, v;
            if (sscanf(line, "vt %f %f", &u, &v) == 2) {
                tex.push_back(u); tex.push_back(v);
            }
        } else if (line[0] == 'v' && line[1] == 'n') {
            float x, y, z;
            if (sscanf(line, "vn %f %f %f", &x, &y, &z) == 3) {
                norm.push_back(x); norm.push_back(y); norm.push_back(z);
                hasNormals = true;
            }
        } else if (line[0] == 'f' && line[1] == ' ') {
            int v[8], vt[8], vn[8];
            int cnt = 0;
            char* tok = line + 2;
            while (*tok == ' ') tok++;
            while (cnt < 8 && *tok) {
                char* rest = strchr(tok, ' ');
                if (rest) *rest++ = 0;
                v[cnt] = vt[cnt] = vn[cnt] = 0;
                sscanf(tok, "%d/%d/%d", &v[cnt], &vt[cnt], &vn[cnt]);
                if (v[cnt] < 0) v[cnt] = (int)pos.size() / 3 + v[cnt] + 1;
                if (vt[cnt] < 0) vt[cnt] = (int)tex.size() / 2 + vt[cnt] + 1;
                if (vn[cnt] < 0) vn[cnt] = (int)norm.size() / 3 + vn[cnt] + 1;
                cnt++;
                tok = rest ? rest : tok + strlen(tok);
                while (tok && *tok == ' ') tok++;
            }
            if (cnt < 3) { line = next; continue; }

            unsigned int vi[8];
            for (int i = 0; i < cnt; i++)
                vi[i] = getOrCreateVert(v[i], vt[i], vn[i]);

            if (cnt == 3) {
                indices.push_back(vi[0]); indices.push_back(vi[1]);
                indices.push_back(vi[2]); indices.push_back(vi[2]);
            } else if (cnt == 4) {
                indices.push_back(vi[0]); indices.push_back(vi[1]);
                indices.push_back(vi[2]); indices.push_back(vi[3]);
            } else {
                // N-gon: triangle fan, each triangle becomes degenerate quad
                for (int i = 1; i + 1 < cnt; i++) {
                    indices.push_back(vi[0]); indices.push_back(vi[i]);
                    indices.push_back(vi[i+1]); indices.push_back(vi[i+1]);
                }
            }
        }
        line = next;
    }
    free(buf);

    if (verts.empty()) { fprintf(stderr, "OBJ: keine Vertices\n"); return false; }

    out.m_verts = verts;
    out.m_indices = indices;
    out.m_sel.resize(verts.size(), false);
    out.m_texId = 0;
    if (!hasNormals) out.recomputeNormals();
    printf("OBJ geladen: %s | Verts: %zu | Faces: %zu | Normals: %s\n",
        path.c_str(), verts.size(), indices.size()/4, hasNormals ? "from file" : "computed");
    return true;
}

void MeshData::subdivide() {
    int origFaces = faceCount();
    std::vector<Vertex> newVerts = m_verts;
    std::vector<unsigned int> newIdx;

    for (int f = 0; f < origFaces; f++) {
        int i0 = m_indices[f*4+0];
        int i1 = m_indices[f*4+1];
        int i2 = m_indices[f*4+2];
        int i3 = m_indices[f*4+3];

        auto avg = [&](int a, int b) -> int {
            const auto& va = m_verts[a];
            const auto& vb = m_verts[b];
            Vertex v;
            v.px = (va.px + vb.px) * 0.5f;
            v.py = (va.py + vb.py) * 0.5f;
            v.pz = (va.pz + vb.pz) * 0.5f;
            v.tu = (va.tu + vb.tu) * 0.5f;
            v.tv = (va.tv + vb.tv) * 0.5f;
            newVerts.push_back(v);
            return (int)newVerts.size() - 1;
        };

        auto avg4 = [&](int a, int b, int c, int d) -> int {
            const auto& va = m_verts[a], &vb = m_verts[b];
            const auto& vc = m_verts[c], &vd = m_verts[d];
            Vertex v;
            v.px = (va.px + vb.px + vc.px + vd.px) * 0.25f;
            v.py = (va.py + vb.py + vc.py + vd.py) * 0.25f;
            v.pz = (va.pz + vb.pz + vc.pz + vd.pz) * 0.25f;
            v.tu = (va.tu + vb.tu + vc.tu + vd.tu) * 0.25f;
            v.tv = (va.tv + vb.tv + vc.tv + vd.tv) * 0.25f;
            newVerts.push_back(v);
            return (int)newVerts.size() - 1;
        };

        int m01 = avg(i0, i1);
        int m12 = avg(i1, i2);
        int m23 = avg(i2, i3);
        int m30 = avg(i3, i0);
        int c = avg4(i0, i1, i2, i3);

        newIdx.push_back(i0); newIdx.push_back(m01); newIdx.push_back(c); newIdx.push_back(m30);
        newIdx.push_back(m01); newIdx.push_back(i1); newIdx.push_back(m12); newIdx.push_back(c);
        newIdx.push_back(c); newIdx.push_back(m12); newIdx.push_back(i2); newIdx.push_back(m23);
        newIdx.push_back(m30); newIdx.push_back(c); newIdx.push_back(m23); newIdx.push_back(i3);
    }

    m_verts = newVerts;
    m_indices = newIdx;
    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
}

void MeshData::extrude(int faceIdx, float dist) {
    if (faceIdx < 0 || faceIdx >= faceCount()) return;

    int base = (int)m_verts.size();
    int i0 = m_indices[faceIdx*4+0];
    int i1 = m_indices[faceIdx*4+1];
    int i2 = m_indices[faceIdx*4+2];
    int i3 = m_indices[faceIdx*4+3];

    // Face normal from first 3 verts
    const auto& a = m_verts[i0];
    const auto& b = m_verts[i1];
    const auto& c = m_verts[i2];
    float ex = b.px - a.px, ey = b.py - a.py, ez = b.pz - a.pz;
    float fx = c.px - a.px, fy = c.py - a.py, fz = c.pz - a.pz;
    float nx = ey*fz - ez*fy;
    float ny = ez*fx - ex*fz;
    float nz = ex*fy - ey*fx;
    float nl = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (nl > 0) { nx /= nl; ny /= nl; nz /= nl; }

    auto pushVert = [&](int src) {
        Vertex v = m_verts[src];
        v.px += nx * dist;
        v.py += ny * dist;
        v.pz += nz * dist;
        m_verts.push_back(v);
    };

    pushVert(i0); pushVert(i1); pushVert(i2); pushVert(i3);

    // Replace original face with pushed face
    m_indices[faceIdx*4+0] = base+0;
    m_indices[faceIdx*4+1] = base+1;
    m_indices[faceIdx*4+2] = base+2;
    m_indices[faceIdx*4+3] = base+3;

    // Side quads
    int sides[4][4] = {
        {i0, i1, base+1, base+0},
        {i1, i2, base+2, base+1},
        {i2, i3, base+3, base+2},
        {i3, i0, base+0, base+3},
    };
    for (auto& s : sides) {
        m_indices.push_back(s[0]);
        m_indices.push_back(s[1]);
        m_indices.push_back(s[2]);
        m_indices.push_back(s[3]);
    }

    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
}

bool MeshData::saveOBJ(const std::string& path) const {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) { fprintf(stderr, "saveOBJ: kann %s nicht oeffnen\n", path.c_str()); return false; }

    fprintf(f, "# exported from Germanische Bibel Engine\n");
    fprintf(f, "o mesh\n");

    for (const auto& v : m_verts)
        fprintf(f, "v %.6f %.6f %.6f\n", v.px, v.py, v.pz);
    for (const auto& v : m_verts)
        fprintf(f, "vt %.6f %.6f\n", v.tu, v.tv);
    for (const auto& v : m_verts)
        fprintf(f, "vn %.4f %.4f %.4f\n", v.nx, v.ny, v.nz);

    int faceCount = (int)m_indices.size() / 4;
    for (int fIdx = 0; fIdx < faceCount; fIdx++) {
        fprintf(f, "f");
        for (int j = 0; j < 4; j++) {
            int idx = (int)m_indices[fIdx * 4 + j] + 1; // OBJ is 1-based
            fprintf(f, " %d/%d/%d", idx, idx, idx);
        }
        fprintf(f, "\n");
    }

    fclose(f);
    printf("saveOBJ: %s geschrieben (%d verts, %d faces)\n", path.c_str(), (int)m_verts.size(), faceCount);
    return true;
}

bool MeshData::loopCut(int edgeV0, int edgeV1) {
    int faceCount = (int)m_indices.size() / 4;
    if (faceCount == 0) return false;

    // Build edge -> {(face, corner)} map
    struct EdgeKey { int a, b; };
    auto mkKey = [](int v0, int v1) {
        if (v0 < v1) return EdgeKey{v0, v1};
        return EdgeKey{v1, v0};
    };
    auto hash = [](const EdgeKey& k) { return (unsigned)k.a * 73856093u ^ (unsigned)k.b * 19349663u; };
    auto eq = [](const EdgeKey& a, const EdgeKey& b) { return a.a == b.a && a.b == b.b; };
    std::unordered_map<EdgeKey, std::vector<int>, decltype(hash), decltype(eq)> edgeFaces(0, hash, eq);
    std::unordered_map<EdgeKey, int, decltype(hash), decltype(eq)> edgeCorner(0, hash, eq);

    for (int f = 0; f < faceCount; f++) {
        int i0 = (int)m_indices[f*4+0], i1 = (int)m_indices[f*4+1];
        int i2 = (int)m_indices[f*4+2], i3 = (int)m_indices[f*4+3];
        int vv[4] = {i0,i1,i2,i3};
        for (int c = 0; c < 4; c++) {
            EdgeKey k = mkKey(vv[c], vv[(c+1)%4]);
            edgeFaces[k].push_back(f);
            edgeCorner[k] = c; // store corner for first occurrence
        }
    }

    EdgeKey startKey = mkKey(edgeV0, edgeV1);
    if (edgeFaces.find(startKey) == edgeFaces.end()) return false;

    // Walk the edge loop in both directions from start edge
    // For each face visited, we record: {faceIdx, edgeV0, edgeV1, oppV0, oppV1}
    struct LoopEdge { int face; int e0, e1; int o0, o1; };
    std::vector<LoopEdge> loop;

    // Walk from a given face, in a given direction
    // collect faces until boundary or loop
    auto walk = [&](int startFace, EdgeKey startEdge, int maxSteps) {
        std::vector<LoopEdge> result;
        int curFace = startFace;
        EdgeKey curKey = startEdge;
        for (int step = 0; step < maxSteps; step++) {
            int i0 = (int)m_indices[curFace*4+0], i1 = (int)m_indices[curFace*4+1];
            int i2 = (int)m_indices[curFace*4+2], i3 = (int)m_indices[curFace*4+3];
            int vv[4] = {i0,i1,i2,i3};

            // Find corner of curKey in this face
            int corner = -1;
            for (int c = 0; c < 4; c++) {
                EdgeKey k = mkKey(vv[c], vv[(c+1)%4]);
                if (k.a == curKey.a && k.b == curKey.b) { corner = c; break; }
            }
            if (corner < 0) break;

            int va = vv[corner], vb = vv[(corner+1)%4];
            int oppCorner = (corner + 2) % 4;
            int oa = vv[oppCorner], ob = vv[(oppCorner+1)%4];
            result.push_back({curFace, va, vb, oa, ob});

            // Cross opposite edge to next face
            EdgeKey oppKey = mkKey(oa, ob);
            auto it = edgeFaces.find(oppKey);
            if (it == edgeFaces.end()) break;
            int nextFace = -1;
            for (int fi : it->second) {
                if (fi != curFace) { nextFace = fi; break; }
            }
            if (nextFace < 0) break;

            // Check for loop closure
            if (nextFace == startFace) break;

            curFace = nextFace;
            curKey = oppKey;
        }
        return result;
    };

    // Start walking from both faces that share the start edge
    auto& startFaces = edgeFaces[startKey];
    if (startFaces.empty()) return false;

    // Collect all visited faces to avoid duplication
    std::vector<bool> visited(faceCount, false);

    for (int sf : startFaces) {
        auto segment = walk(sf, startKey, faceCount * 2);
        for (auto& le : segment) {
            if (!visited[le.face]) {
                loop.push_back(le);
                visited[le.face] = true;
            }
        }
    }

    if (loop.empty()) return false;

    // Now split: insert midpoints and rebuild faces
    int newVertBase = (int)m_verts.size();
    std::vector<Vertex> newVerts;

    struct MidInfo { int edgeMid; int oppMid; };
    std::vector<MidInfo> mids(loop.size());

    auto addMidpoint = [&](int a, int b) -> int {
        const Vertex& va = m_verts[a];
        const Vertex& vb = m_verts[b];
        Vertex m;
        m.px = (va.px + vb.px) * 0.5f;
        m.py = (va.py + vb.py) * 0.5f;
        m.pz = (va.pz + vb.pz) * 0.5f;
        m.tu = (va.tu + vb.tu) * 0.5f;
        m.tv = (va.tv + vb.tv) * 0.5f;
        newVerts.push_back(m);
        return newVertBase + (int)newVerts.size() - 1;
    };

    for (size_t i = 0; i < loop.size(); i++) {
        mids[i].edgeMid = addMidpoint(loop[i].e0, loop[i].e1);
        mids[i].oppMid = addMidpoint(loop[i].o0, loop[i].o1);
    }

    // Build new index buffer
    std::vector<unsigned int> newIdx;
    std::vector<bool> faceInLoop(faceCount, false);
    for (auto& le : loop) faceInLoop[le.face] = true;

    // Keep unmodified faces
    for (int f = 0; f < faceCount; f++) {
        if (!faceInLoop[f]) {
            newIdx.push_back(m_indices[f*4+0]);
            newIdx.push_back(m_indices[f*4+1]);
            newIdx.push_back(m_indices[f*4+2]);
            newIdx.push_back(m_indices[f*4+3]);
        }
    }

    // Replace loop faces with two quads each
    for (size_t i = 0; i < loop.size(); i++) {
        int face = loop[i].face;
        int idx[4] = {
            (int)m_indices[face*4+0], (int)m_indices[face*4+1],
            (int)m_indices[face*4+2], (int)m_indices[face*4+3]
        };
        int v0 = loop[i].e0, v1 = loop[i].e1;
        int em = mids[i].edgeMid;
        int om = mids[i].oppMid;

        // Find positions of v0, v1 in the quad winding
        int p0 = -1, p1 = -1;
        for (int j = 0; j < 4; j++) {
            if (idx[j] == v0) p0 = j;
            if (idx[j] == v1) p1 = j;
        }

        // The vertices before v0 and after v1 in the quad
        int leftOfV0 = idx[(p0 + 3) % 4];
        int rightOfV1 = idx[(p1 + 1) % 4];

        // Quad 1: v0 → edgeMid → oppMid → left (the left side of the cut)
        newIdx.push_back(v0);
        newIdx.push_back(em);
        newIdx.push_back(om);
        newIdx.push_back(leftOfV0);

        // Quad 2: edgeMid → v1 → right → oppMid (the right side of the cut)
        newIdx.push_back(em);
        newIdx.push_back(v1);
        newIdx.push_back(rightOfV1);
        newIdx.push_back(om);
    }

    m_verts.insert(m_verts.end(), newVerts.begin(), newVerts.end());
    m_indices = newIdx;
    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
    return true;
}

void MeshData::inset(int faceIdx, float amount) {
    if (faceIdx < 0 || faceIdx >= faceCount()) return;
    if (amount <= 0.0f) return;

    int i0 = (int)m_indices[faceIdx*4+0];
    int i1 = (int)m_indices[faceIdx*4+1];
    int i2 = (int)m_indices[faceIdx*4+2];
    int i3 = (int)m_indices[faceIdx*4+3];

    const Vertex& v0 = m_verts[i0];
    const Vertex& v1 = m_verts[i1];
    const Vertex& v2 = m_verts[i2];
    const Vertex& v3 = m_verts[i3];

    // Face center
    Vertex c;
    c.px = (v0.px + v1.px + v2.px + v3.px) * 0.25f;
    c.py = (v0.py + v1.py + v2.py + v3.py) * 0.25f;
    c.pz = (v0.pz + v1.pz + v2.pz + v3.pz) * 0.25f;
    c.tu = (v0.tu + v1.tu + v2.tu + v3.tu) * 0.25f;
    c.tv = (v0.tv + v1.tv + v2.tv + v3.tv) * 0.25f;

    auto insetVert = [&](const Vertex& v) -> int {
        Vertex in;
        float t = amount;
        in.px = v.px + (c.px - v.px) * t;
        in.py = v.py + (c.py - v.py) * t;
        in.pz = v.pz + (c.pz - v.pz) * t;
        in.tu = v.tu + (c.tu - v.tu) * t;
        in.tv = v.tv + (c.tv - v.tv) * t;
        return addVertex(in);
    };

    int i0in = insetVert(v0);
    int i1in = insetVert(v1);
    int i2in = insetVert(v2);
    int i3in = insetVert(v3);

    // Replace original face with inner face
    m_indices[faceIdx*4+0] = i0in;
    m_indices[faceIdx*4+1] = i1in;
    m_indices[faceIdx*4+2] = i2in;
    m_indices[faceIdx*4+3] = i3in;

    // Side quads (outer → inner)
    addFace(i0, i1, i1in, i0in);
    addFace(i1, i2, i2in, i1in);
    addFace(i2, i3, i3in, i2in);
    addFace(i3, i0, i0in, i3in);

    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
}

void MeshData::mirror(int axis) {
    int base = (int)m_verts.size();

    // Duplicate vertices with mirrored position
    for (int i = 0; i < base; i++) {
        Vertex v = m_verts[i];
        if (axis == 0) v.px = -v.px;
        else if (axis == 1) v.py = -v.py;
        else v.pz = -v.pz;
        addVertex(v);
    }

    // Duplicate faces with reversed winding for correct normals
    int baseFaces = (int)m_indices.size() / 4;
    for (int i = 0; i < baseFaces; i++) {
        int a = (int)m_indices[i * 4 + 0] + base;
        int b = (int)m_indices[i * 4 + 1] + base;
        int c = (int)m_indices[i * 4 + 2] + base;
        int d = (int)m_indices[i * 4 + 3] + base;
        addFace(a, d, c, b); // reversed winding
    }

    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
}

bool MeshData::weldSelected() {
    int keep = firstSelected();
    if (keep < 0) return false;

    std::vector<int> toWeld;
    for (int i = 0; i < (int)m_verts.size(); i++) {
        if (m_sel[i] && i != keep) toWeld.push_back(i);
    }
    if (toWeld.empty()) return false;

    // Replace all occurrences of toWeld vertices with 'keep'
    for (auto& idx : m_indices) {
        for (int w : toWeld) {
            if ((int)idx == w) { idx = keep; break; }
        }
    }

    // Remove degenerate faces (all 4 indices identical)
    std::vector<unsigned int> newIdx;
    for (size_t i = 0; i < m_indices.size(); i += 4) {
        unsigned int a = m_indices[i], b = m_indices[i + 1];
        unsigned int c = m_indices[i + 2], d = m_indices[i + 3];
        if (a == b && a == c && a == d) continue;
        newIdx.push_back(a); newIdx.push_back(b);
        newIdx.push_back(c); newIdx.push_back(d);
    }
    m_indices = newIdx;

    // Remove welded vertices (descending to keep indices valid)
    std::sort(toWeld.begin(), toWeld.end(), std::greater<int>());
    for (int w : toWeld) {
        for (auto& idx : m_indices) {
            if ((int)idx > w) idx--;
        }
        m_verts.erase(m_verts.begin() + w);
    }

    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
    return true;
}

bool MeshData::bevelEdge(int v0, int v1, float t) {
    if (v0 == v1 || t <= 0.0f || t >= 0.5f) return false;
    int faceCount = (int)m_indices.size() / 4;
    if (faceCount == 0) return false;

    struct EdgeKey { int a, b; };
    auto mkKey = [](int a, int b) { if (a < b) return EdgeKey{a,b}; return EdgeKey{b,a}; };
    auto hash = [](const EdgeKey& k) { return (unsigned)k.a * 73856093u ^ (unsigned)k.b * 19349663u; };
    auto eq = [](const EdgeKey& a, const EdgeKey& b) { return a.a == b.a && a.b == b.b; };
    std::unordered_map<EdgeKey, std::vector<int>, decltype(hash), decltype(eq)> edgeFaces(0, hash, eq);

    for (int f = 0; f < faceCount; f++) {
        int vv[4] = {
            (int)m_indices[f*4+0], (int)m_indices[f*4+1],
            (int)m_indices[f*4+2], (int)m_indices[f*4+3]
        };
        for (int c = 0; c < 4; c++)
            edgeFaces[mkKey(vv[c], vv[(c+1)%4])].push_back(f);
    }

    EdgeKey key = mkKey(v0, v1);
    auto it = edgeFaces.find(key);
    if (it == edgeFaces.end() || it->second.size() < 2) return false;

    // Two faces share this edge
    int fi[2] = { it->second[0], it->second[1] };
    int corner[2];
    int fwd[2];
    int prev[2], next[2];

    for (int k = 0; k < 2; k++) {
        int vv[4] = {
            (int)m_indices[fi[k]*4+0], (int)m_indices[fi[k]*4+1],
            (int)m_indices[fi[k]*4+2], (int)m_indices[fi[k]*4+3]
        };
        corner[k] = -1;
        for (int c = 0; c < 4; c++) {
            if ((vv[c] == v0 && vv[(c+1)%4] == v1) || (vv[c] == v1 && vv[(c+1)%4] == v0)) {
                corner[k] = c; break;
            }
        }
        if (corner[k] < 0) return false;
        fwd[k] = (vv[corner[k]] == v0 && vv[(corner[k]+1)%4] == v1);
        prev[k] = vv[(corner[k]+3)%4];
        next[k] = vv[(corner[k]+2)%4];
    }

    auto lerp = [&](int src, int target, float factor) -> int {
        Vertex nv;
        nv.px = m_verts[src].px + (m_verts[target].px - m_verts[src].px) * factor;
        nv.py = m_verts[src].py + (m_verts[target].py - m_verts[src].py) * factor;
        nv.pz = m_verts[src].pz + (m_verts[target].pz - m_verts[src].pz) * factor;
        nv.tu = m_verts[src].tu + (m_verts[target].tu - m_verts[src].tu) * factor;
        nv.tv = m_verts[src].tv + (m_verts[target].tv - m_verts[src].tv) * factor;
        return addVertex(nv);
    };

    // Face 0: v0 slides to prev[0], v1 slides to next[0]
    int v0off0 = lerp(v0, prev[0], t);
    int v1off0 = lerp(v1, next[0], t);
    m_indices[fi[0]*4+0] = v0off0;
    m_indices[fi[0]*4+1] = v1off0;

    // Face 1: v0 slides to next[1], v1 slides to prev[1] (opposite winding)
    int v0off1 = lerp(v0, next[1], t);
    int v1off1 = lerp(v1, prev[1], t);
    m_indices[fi[1]*4+0] = v0off1;
    m_indices[fi[1]*4+1] = v1off1;

    // Bevel face: connect the 4 offset vertices
    addFace(v0off0, v1off0, v1off1, v0off1);

    m_selectedFace = -1;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
    return true;
}

bool MeshData::bridge(int loop1V0, int loop1V1, int loop2V0, int loop2V1) {
    int faceCount = (int)m_indices.size() / 4;
    if (faceCount == 0) return false;
    if (loop1V0 == loop2V0 && loop1V1 == loop2V1) return false;

    struct EdgeKey { int a, b; };
    auto mkKey = [](int v0, int v1) {
        if (v0 < v1) return EdgeKey{v0, v1};
        return EdgeKey{v1, v0};
    };
    auto hash = [](const EdgeKey& k) { return (unsigned)k.a * 73856093u ^ (unsigned)k.b * 19349663u; };
    auto eq = [](const EdgeKey& a, const EdgeKey& b) { return a.a == b.a && a.b == b.b; };
    std::unordered_map<EdgeKey, std::vector<int>, decltype(hash), decltype(eq)> edgeFaces(0, hash, eq);

    for (int f = 0; f < faceCount; f++) {
        int i0 = (int)m_indices[f*4+0], i1 = (int)m_indices[f*4+1];
        int i2 = (int)m_indices[f*4+2], i3 = (int)m_indices[f*4+3];
        int vv[4] = {i0,i1,i2,i3};
        for (int c = 0; c < 4; c++) {
            EdgeKey k = mkKey(vv[c], vv[(c+1)%4]);
            edgeFaces[k].push_back(f);
        }
    }

    EdgeKey key1 = mkKey(loop1V0, loop1V1);
    EdgeKey key2 = mkKey(loop2V0, loop2V1);
    if (edgeFaces.find(key1) == edgeFaces.end()) return false;
    if (edgeFaces.find(key2) == edgeFaces.end()) return false;
    if (edgeFaces[key1].size() != 1) return false;
    if (edgeFaces[key2].size() != 1) return false;

    std::unordered_map<int, std::vector<int>> boundAdj;
    for (auto& [key, faces] : edgeFaces) {
        if (faces.size() == 1) {
            boundAdj[key.a].push_back(key.b);
            boundAdj[key.b].push_back(key.a);
        }
    }

    auto walkLoop = [&](int seedA, int seedB) -> std::vector<std::pair<int,int>> {
        std::vector<std::pair<int,int>> loop;
        int cur = seedA, next = seedB;
        int startA = seedA, startB = seedB;
        while (true) {
            loop.push_back({cur, next});
            auto& neighbors = boundAdj[next];
            int nxt = -1;
            for (int nb : neighbors) {
                if (nb != cur) { nxt = nb; break; }
            }
            if (nxt < 0) break;
            cur = next;
            next = nxt;
            if (cur == startA && next == startB) break;
            if (loop.size() > 100000) break;
        }
        return loop;
    };

    auto A = walkLoop(loop1V0, loop1V1);
    auto B = walkLoop(loop2V0, loop2V1);

    if (A.size() != B.size() || A.size() < 2) return false;

    int n = (int)A.size();

    auto signedArea = [&](const std::vector<std::pair<int,int>>& loop) -> float {
        float area = 0;
        for (auto& e : loop) {
            const Vertex& va = m_verts[e.first];
            const Vertex& vb = m_verts[e.second];
            area += va.px * vb.pz - vb.px * va.pz;
        }
        return area;
    };

    float areaA = signedArea(A);
    float areaB = signedArea(B);

    std::vector<unsigned int> newIdx = m_indices;

    if (areaA * areaB >= 0) {
        for (int i = 0; i < n; i++) {
            int ai0 = A[i].first, ai1 = A[i].second;
            int bi0 = B[i].first, bi1 = B[i].second;
            newIdx.push_back(ai0);
            newIdx.push_back(ai1);
            newIdx.push_back(bi1);
            newIdx.push_back(bi0);
        }
    } else {
        for (int i = 0; i < n; i++) {
            int ai0 = A[i].first, ai1 = A[i].second;
            int j = (n - i) % n;
            int bi0 = B[j].first, bi1 = B[j].second;
            newIdx.push_back(ai0);
            newIdx.push_back(ai1);
            newIdx.push_back(bi1);
            newIdx.push_back(bi0);
        }
    }

    m_indices = newIdx;
    m_sel.resize(m_verts.size(), false);
    recomputeNormals();
    printf("bridge: Loops combined (%d bridge quads)\n", n);
    return true;
}

void MeshData::recomputeNormals() {
    for (auto& v : m_verts) { v.nx = v.ny = v.nz = 0; }
    for (size_t i = 0; i < m_indices.size(); i += 4) {
        int i0 = m_indices[i+0], i1 = m_indices[i+1], i2 = m_indices[i+2], i3 = m_indices[i+3];
        auto faceNormal = [&](int a, int b, int c) {
            const auto& va = m_verts[a], & vb = m_verts[b], & vc = m_verts[c];
            float ex = vb.px - va.px, ey = vb.py - va.py, ez = vb.pz - va.pz;
            float fx = vc.px - va.px, fy = vc.py - va.py, fz = vc.pz - va.pz;
            float nx = ey*fz - ez*fy;
            float ny = ez*fx - ex*fz;
            float nz = ex*fy - ey*fx;
            float nl = std::sqrt(nx*nx + ny*ny + nz*nz);
            if (nl > 0) { nx /= nl; ny /= nl; nz /= nl; }
            m_verts[a].nx += nx; m_verts[a].ny += ny; m_verts[a].nz += nz;
            m_verts[b].nx += nx; m_verts[b].ny += ny; m_verts[b].nz += nz;
            m_verts[c].nx += nx; m_verts[c].ny += ny; m_verts[c].nz += nz;
        };
        faceNormal(i0, i1, i2);
        if (i2 != i3) faceNormal(i0, i2, i3);
    }
    for (auto& v : m_verts) {
        float l = std::sqrt(v.nx*v.nx + v.ny*v.ny + v.nz*v.nz);
        if (l > 0) { v.nx /= l; v.ny /= l; v.nz /= l; }
    }
}
