#pragma once
#include "MeshData.h"
#include <string>
#include <vector>
#include <map>

class MeshDatabase {
public:
    bool loadFile(const std::string& name, const std::string& filepath);
    bool add(const std::string& name, const MeshData& mesh);
    MeshData* find(const std::string& name);
    std::vector<std::string> names() const;
    void clear();
    int count() const { return (int)m_entries.size(); }

private:
    std::map<std::string, MeshData> m_entries;
};
