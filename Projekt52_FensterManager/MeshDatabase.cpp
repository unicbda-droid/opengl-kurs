#include "MeshDatabase.h"
#include <cstdio>

bool MeshDatabase::loadFile(const std::string& name, const std::string& filepath) {
    MeshData mesh;
    if (!MeshData::loadOBJ(filepath, mesh)) return false;
    m_entries[name] = mesh;
    printf("DB: '%s' aus %s geladen (%d verts)\n", name.c_str(), filepath.c_str(), mesh.vertexCount());
    return true;
}

bool MeshDatabase::add(const std::string& name, const MeshData& mesh) {
    m_entries[name] = mesh;
    return true;
}

MeshData* MeshDatabase::find(const std::string& name) {
    auto it = m_entries.find(name);
    if (it != m_entries.end()) return &it->second;
    return nullptr;
}

std::vector<std::string> MeshDatabase::names() const {
    std::vector<std::string> out;
    for (auto& p : m_entries) out.push_back(p.first);
    return out;
}

void MeshDatabase::clear() { m_entries.clear(); }
