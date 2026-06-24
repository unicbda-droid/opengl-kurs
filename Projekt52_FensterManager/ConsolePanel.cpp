#include "ConsolePanel.h"
#include "ViewportPanel.h"
#define NOMINMAX
#include <windows.h>
#include <sstream>
#include <algorithm>
#include <cstdio>
#include <cctype>

ConsolePanel::ConsolePanel(GLFWwindow* window, ViewportPanel* viewport)
    : Panel("Console", window), m_viewport(viewport) {
    m_output.push_back("Germanische Bibel Engine Console");
    m_output.push_back("Type 'help' for commands");
    m_output.push_back("");
}

void ConsolePanel::print(const std::string& text) {
    m_output.push_back(text);
    if ((int)m_output.size() > m_maxLines) m_output.pop_front();
    m_scrollOffset = 0;
}

void ConsolePanel::render() {
    if (m_rect.w <= 0 || m_rect.h <= 0) return;

    glEnable(GL_SCISSOR_TEST);
    setupViewport();
    glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_font.setWindowSize(m_rect.w, m_rect.h);

    drawOutput();
    drawInputLine();

    glDisable(GL_SCISSOR_TEST);
}

void ConsolePanel::drawOutput() {
    int y = m_rect.h - m_font.charHeight() - 4;
    int lines = (m_rect.h - m_font.charHeight() - 12) / m_font.charHeight();
    int start = (int)m_output.size() - lines - m_scrollOffset;
    if (start < 0) start = 0;
    for (int i = start; i < (int)m_output.size(); i++) {
        m_font.print(4, y, m_output[i], 0.6f, 0.8f, 1.0f);
        y -= m_font.charHeight();
    }
}

void ConsolePanel::drawInputLine() {
    int y = 2;
    std::string prompt = "> " + m_input;
    m_font.print(4, y, prompt, 0.9f, 0.9f, 0.2f);

    if (m_keyboardFocus) {
        // Simple cursor blink
        int cx = m_font.textWidth(prompt) + 4;
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, m_rect.w, 0, m_rect.h, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glColor3f(0.9f, 0.9f, 0.2f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2i(cx, y + 2);
        glVertex2i(cx, y + m_font.charHeight() - 2);
        glEnd();
        glLineWidth(1.0f);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }
}

bool ConsolePanel::onMouseButton(int button, int action, int mods) {
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    if (action == GLFW_PRESS) {
        setKeyboardFocus(contains((int)mx, (int)my));
        return true;
    }
    return false;
}

bool ConsolePanel::onKey(int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return false;

    if (key == GLFW_KEY_ENTER) {
        std::string cmd = m_input;
        m_input.clear();
        m_historyPos = -1;
        if (!cmd.empty()) {
            print("> " + cmd);
            execute(cmd);
            m_history.push_back(cmd);
        }
        return true;
    }

    if (key == GLFW_KEY_BACKSPACE) {
        if (!m_input.empty()) m_input.pop_back();
        return true;
    }

    if (key == GLFW_KEY_UP) {
        if (m_history.empty()) return true;
        if (m_historyPos < (int)m_history.size() - 1) m_historyPos++;
        m_input = m_history[m_history.size() - 1 - m_historyPos];
        return true;
    }

    if (key == GLFW_KEY_DOWN) {
        if (m_historyPos > 0) {
            m_historyPos--;
            m_input = m_history[m_history.size() - 1 - m_historyPos];
        } else {
            m_historyPos = -1;
            m_input.clear();
        }
        return true;
    }

    if (key == GLFW_KEY_TAB) {
        if (m_input.empty()) return false; // pass to viewport for edit mode toggle
        // simple command completion
        static const char* commands[] = {
            "help","save","save_scene","load_scene","list","select","remove",
            "rename","duplicate","merge","subdivide","loopcut","extrude","inset",
            "bevel","bridge","mirror","weld","plane","cube","pos","rot","scale",
            "color","setParent","face_clear","ss","exit","quit",nullptr
        };
        std::string partial;
        size_t sp = m_input.find(' ');
        if (sp == std::string::npos) partial = m_input;
        else partial = m_input.substr(0, sp);
        std::string match;
        for (int ci = 0; commands[ci]; ci++) {
            if (strncmp(commands[ci], partial.c_str(), partial.size()) == 0) {
                if (match.empty()) { match = commands[ci]; }
                else { match.clear(); break; } // multiple matches
            }
        }
        if (!match.empty()) {
            if (sp == std::string::npos) m_input = match + " ";
            else m_input = match + m_input.substr(sp);
        }
        return true;
    }

    return false;
}

bool ConsolePanel::onChar(unsigned int codepoint) {
    if (!m_keyboardFocus) return false;
    if (codepoint >= 32 && codepoint <= 126) {
        m_input += (char)codepoint;
        return true;
    }
    return false;
}

std::vector<std::string> ConsolePanel::splitArgs(const std::string& text) {
    std::vector<std::string> args;
    std::stringstream ss(text);
    std::string tok;
    while (ss >> tok) args.push_back(tok);
    return args;
}

void ConsolePanel::execute(const std::string& cmd) {
    auto args = splitArgs(cmd);
    if (args.empty()) return;

    auto& c = args[0];
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);

    if (c == "help") {
        print("Commands:");
        print("  help              - this list");
        print("  cube              - create cube");
        print("  plane [sx] [sz]   - create plane");
        print("  load <path>       - load OBJ file");
        print("  save <path>       - save selected object as OBJ");
        print("  save_scene <name> - save entire scene");
        print("  load_scene <name> - load entire scene");
        print("  list              - list objects");
        print("  select <id>       - select object");
        print("  remove <id>       - remove object");
        print("  subdivide         - subdivide selected");
        print("  loopcut [v0] [v1] - cut edge loop");
        print("  bevel <v0> <v1> [t] - bevel edge (t=0.0-0.5)");
        print("  bridge <v0> <v1> <w0> <w1> - bridge two boundary edge loops");
        print("  extrude <dist>    - extrude selected");
        print("  inset <amount>    - inset selected (scale face inward)");
        print("  mirror <axis>     - mirror mesh (0=X, 1=Y, 2=Z)");
        print("  weld              - merge selected vertices into first");
        print("  pos <x> <y> <z>   - set position");
        print("  scale <s>         - set scale");
        print("  rot <y> <p> <r>   - set rotation (yaw pitch roll)");
        print("  solid|wire|tex|normal - view mode");
        print("  edit              - toggle edit mode");
        print("  script <path>     - run script file");
        print("  cls               - clear console");
    }
    else if (c == "cls" || c == "clear") {
        m_output.clear();
    }
    else if (c == "cube") {
        int idx = m_viewport->addObject(0, 0, 0);
        m_viewport->selectObjectByIndex(idx);
        char buf[64]; snprintf(buf, sizeof(buf), "Cube #%d created", idx);
        print(buf);
    }
    else if (c == "plane") {
        int sx = 1, sz = 1;
        if (args.size() > 1) sx = std::max(1, atoi(args[1].c_str()));
        if (args.size() > 2) sz = std::max(1, atoi(args[2].c_str()));
        int idx = m_viewport->addObject(0, 0, 0);
        m_viewport->getObject(idx)->mesh() = MeshData::createPlane(sx, sz);
        m_viewport->selectObjectByIndex(idx);
        char buf[64]; snprintf(buf, sizeof(buf), "Plane %dx%d #%d created", sx, sz, idx);
        print(buf);
    }
    else if (c == "load") {
        if (args.size() < 2) { print("Usage: load <path>"); return; }
        m_viewport->loadOBJFile(args[1]);
    }
    else if (c == "save") {
        if (args.size() < 2) { print("Usage: save <path>"); return; }
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        if (obj->mesh().saveOBJ(args[1]))
            print("Saved to " + args[1]);
        else
            print("Failed to save " + args[1]);
    }
    else if (c == "save_scene") {
        if (args.size() < 2) { print("Usage: save_scene <name>"); return; }
        std::string dir = args[1];
        CreateDirectoryA(dir.c_str(), NULL);
        char meta[512] = {0};
        int cnt = m_viewport->objectCount();
        snprintf(meta, sizeof(meta), "%s/scene.txt", dir.c_str());
        FILE* f = fopen(meta, "w");
        if (!f) { print("Cannot create " + dir); return; }
        fprintf(f, "objects %d\n", cnt);
        for (int i = 0; i < cnt; i++) {
            auto* obj = m_viewport->getObject(i);
            float x,y,z, yaw,pitch,roll; float sc;
            obj->getPosition(x,y,z);
            obj->getRotation(yaw,pitch,roll);
            sc = obj->getScale();
            fprintf(f, "%d pos=%.3f %.3f %.3f rot=%.1f %.1f %.1f scale=%.2f parent=%d name=%s\n",
                i, x,y,z, yaw,pitch,roll, sc, obj->getParentIndex(), obj->getName().c_str());
            // save mesh as OBJ
            char objPath[256];
            snprintf(objPath, sizeof(objPath), "%s/obj_%d.obj", dir.c_str(), i);
            obj->mesh().saveOBJ(objPath);
        }
        fclose(f);
        char buf[128]; snprintf(buf, sizeof(buf), "Scene saved: %s (%d objs)", dir.c_str(), cnt);
        print(buf);
    }
    else if (c == "load_scene") {
        if (args.size() < 2) { print("Usage: load_scene <name>"); return; }
        std::string dir = args[1];
        char meta[512];
        snprintf(meta, sizeof(meta), "%s/scene.txt", dir.c_str());
        FILE* f = fopen(meta, "r");
        if (!f) { print("Scene not found: " + dir); return; }
        int cnt = 0;
        fscanf(f, " objects %d\n", &cnt);
        for (int i = 0; i < cnt; i++) {
            float x=0,y=0,z=0, yaw=0,pitch=0,roll=0, sc=1;
            int parent = -1, idx = -1;
            char objName[64] = "";
        fscanf(f, " %d pos=%f %f %f rot=%f %f %f scale=%f parent=%d name=%63s\n",
                &idx, &x,&y,&z, &yaw,&pitch,&roll, &sc, &parent, objName);
            char objPath[256];
            snprintf(objPath, sizeof(objPath), "%s/obj_%d.obj", dir.c_str(), i);
            int newIdx = m_viewport->addObject(x, y, z);
            auto* obj = m_viewport->getObject(newIdx);
            if (obj) {
                if (strlen(objName) > 0) obj->setName(objName);
                MeshData mesh;
                if (MeshData::loadOBJ(objPath, mesh)) {
                    obj->mesh() = mesh;
                }
                obj->setRotation(yaw, pitch, roll);
                obj->setScale(sc);
                if (parent >= 0 && parent < newIdx) obj->setParent(parent);
            }
        }
        fclose(f);
        char buf[128]; snprintf(buf, sizeof(buf), "Scene loaded: %s (%d objs)", dir.c_str(), cnt);
        print(buf);
    }
    else if (c == "list") {
        char buf[128];
        snprintf(buf, sizeof(buf), "Scene: %d objects", m_viewport->objectCount());
        print(buf);
        for (int i = 0; i < m_viewport->objectCount(); i++) {
            auto* obj = m_viewport->getObject(i);
            if (!obj) continue;
            bool sel = obj == m_viewport->getSelectedObject();
            int parent = obj->getParentIndex();
            if (parent >= 0)
                snprintf(buf, sizeof(buf), "  [%d] %s (%d verts) parent=%d%s",
                    i, obj->getName().c_str(), obj->mesh().vertexCount(), parent, sel ? " *" : "");
            else
                snprintf(buf, sizeof(buf), "  [%d] %s (%d verts)%s",
                    i, obj->getName().c_str(), obj->mesh().vertexCount(), sel ? " *" : "");
            print(buf);
        }
    }
    else if (c == "select") {
        if (args.size() < 2) { print("Usage: select <id>"); return; }
        int id = atoi(args[1].c_str());
        m_viewport->selectObjectByIndex(id);
        char buf[64]; snprintf(buf, sizeof(buf), "Selected object #%d", id);
        print(buf);
    }
    else if (c == "remove") {
        if (args.size() < 2) { print("Usage: remove <id>"); return; }
        int id = atoi(args[1].c_str());
        m_viewport->removeObject(id);
        char buf[64]; snprintf(buf, sizeof(buf), "Removed object #%d", id);
        print(buf);
    }
    else if (c == "rename") {
        if (args.size() < 2) { print("Usage: rename <name>"); return; }
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        obj->setName(args[1]);
        print("Renamed to " + args[1]);
    }
    else if (c == "duplicate") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        int idx = m_viewport->addObject(0, 0, 0);
        auto* dup = m_viewport->getObject(idx);
        if (dup) {
            dup->mesh() = obj->mesh();
            dup->setName(obj->getName() + "_copy");
            m_viewport->selectObjectByIndex(idx);
        }
        char buf[64]; snprintf(buf, sizeof(buf), "Duplicated as #%d", idx);
        print(buf);
    }
    else if (c == "merge") {
        auto* sel = m_viewport->getSelectedObject();
        if (!sel) { print("No object selected"); return; }
        // find second selected object (first non-selected)
        int mergeIdx = -1;
        for (int i = 0; i < m_viewport->objectCount(); i++) {
            auto* obj = m_viewport->getObject(i);
            if (obj && obj != sel) { mergeIdx = i; break; }
        }
        if (mergeIdx < 0) { print("Need at least 2 objects"); return; }
        auto* other = m_viewport->getObject(mergeIdx);
        // copy other's vertices and faces into selected
        int voff = sel->mesh().vertexCount();
        for (int vi = 0; vi < other->mesh().vertexCount(); vi++) {
            MeshData::Vertex v = other->mesh().vertex(vi);
            sel->mesh().addVertex(v);
        }
        for (int fi = 0; fi < other->mesh().faceCount(); fi++) {
            int a = other->mesh().faceVertex(fi, 0);
            int b = other->mesh().faceVertex(fi, 1);
            int c = other->mesh().faceVertex(fi, 2);
            int d = other->mesh().faceVertex(fi, 3);
            sel->mesh().addFace(a+voff, b+voff, c+voff, d+voff);
        }
        m_viewport->removeObject(mergeIdx);
        char buf[64]; snprintf(buf, sizeof(buf), "Merged #%d into selected (%d verts, %d faces)",
            mergeIdx, sel->mesh().vertexCount(), sel->mesh().faceCount());
        print(buf);
    }
    else if (c == "subdivide") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        obj->mesh().subdivide();
        char buf[64]; snprintf(buf, sizeof(buf), "Subdivided (%d verts)", obj->mesh().vertexCount());
        print(buf);
    }
    else if (c == "loopcut") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        int v0 = -1, v1 = -1;
        if (args.size() >= 3) {
            v0 = atoi(args[1].c_str());
            v1 = atoi(args[2].c_str());
        } else {
            // use first two selected vertices
            v0 = obj->mesh().firstSelected();
            if (v0 >= 0) {
                for (int i = v0 + 1; i < obj->mesh().vertexCount(); i++) {
                    if (obj->mesh().isSelected(i)) { v1 = i; break; }
                }
            }
        }
        if (v0 < 0 || v1 < 0) { print("Select two vertices or specify: loopcut <v0> <v1>"); return; }
        if (obj->mesh().loopCut(v0, v1)) {
            char buf[64]; snprintf(buf, sizeof(buf), "Loop cut from edge (%d,%d) (%d verts)", v0, v1, obj->mesh().vertexCount());
            print(buf);
        } else {
            print("Loop cut failed (edge not found or empty loop)");
        }
    }
    else if (c == "bevel") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        int v0 = -1, v1 = -1; float t = 0.2f;
        if (args.size() >= 3) {
            v0 = atoi(args[1].c_str());
            v1 = atoi(args[2].c_str());
            if (args.size() >= 4) t = (float)atof(args[3].c_str());
        } else {
            v0 = obj->mesh().firstSelected();
            if (v0 >= 0) {
                for (int i = v0 + 1; i < obj->mesh().vertexCount(); i++) {
                    if (obj->mesh().isSelected(i)) { v1 = i; break; }
                }
            }
        }
        if (v0 < 0 || v1 < 0) { print("Select two vertices or specify: bevel <v0> <v1> [t]"); return; }
        if (obj->mesh().bevelEdge(v0, v1, t)) {
            char buf[64]; snprintf(buf, sizeof(buf), "Bevel edge (%d,%d) t=%.2f (%d verts)", v0, v1, t, obj->mesh().vertexCount());
            print(buf);
        } else {
            print("Bevel failed (need interior edge with 2 adjacent faces)");
        }
    }
    else if (c == "bridge") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        if (args.size() < 5) { print("Usage: bridge <v0> <v1> <w0> <w1>"); return; }
        int v0 = atoi(args[1].c_str());
        int v1 = atoi(args[2].c_str());
        int w0 = atoi(args[3].c_str());
        int w1 = atoi(args[4].c_str());
        if (obj->mesh().bridge(v0, v1, w0, w1)) {
            char buf[64]; snprintf(buf, sizeof(buf), "Bridge created (%d verts, %d faces)",
                obj->mesh().vertexCount(), obj->mesh().faceCount());
            print(buf);
        } else {
            print("Bridge failed (edges must be boundary edges with equal loop sizes)");
        }
    }
    else if (c == "mirror") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        int axis = 0;
        if (args.size() > 1) axis = atoi(args[1].c_str());
        if (axis < 0 || axis > 2) axis = 0;
        obj->mesh().mirror(axis);
        char buf[64]; snprintf(buf, sizeof(buf), "Mirrored on axis %d (%d verts)", axis, obj->mesh().vertexCount());
        print(buf);
    }
    else if (c == "weld") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        if (obj->mesh().weldSelected())
            print("Welded vertices");
        else
            print("Weld failed (select 2+ vertices)");
    }
    else if (c == "inset") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        float amount = 0.2f;
        if (args.size() > 1) amount = (float)atof(args[1].c_str());
        int fc = 0;
        int sf = obj->mesh().selectedFace();
        if (sf >= 0) { obj->mesh().inset(sf, amount); fc = 1; }
        else {
            fc = obj->mesh().faceCount();
            for (int f = fc - 1; f >= 0; f--) obj->mesh().inset(f, amount);
        }
        char buf[64]; snprintf(buf, sizeof(buf), "Inset %d faces by %.2f (%d verts)", fc, amount, obj->mesh().vertexCount());
        print(buf);
    }
    else if (c == "face_clear") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        obj->mesh().clearFaceSelection();
        print("Face selection cleared");
    }
    else if (c == "extrude") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        float dist = 0.3f;
        if (args.size() > 1) dist = (float)atof(args[1].c_str());
        int fc = 0;
        int sf = obj->mesh().selectedFace();
        if (sf >= 0) { obj->mesh().extrude(sf, dist); fc = 1; }
        else {
            fc = obj->mesh().faceCount();
            for (int f = fc - 1; f >= 0; f--) obj->mesh().extrude(f, dist);
        }
        char buf[64]; snprintf(buf, sizeof(buf), "Extruded %d faces by %.2f", fc, dist);
        print(buf);
    }
    else if (c == "pos") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        float x = 0, y = 0, z = 0;
        if (args.size() > 1) x = (float)atof(args[1].c_str());
        if (args.size() > 2) y = (float)atof(args[2].c_str());
        if (args.size() > 3) z = (float)atof(args[3].c_str());
        obj->setPosition(x, y, z);
        char buf[64]; snprintf(buf, sizeof(buf), "Position set to (%.2f, %.2f, %.2f)", x, y, z);
        print(buf);
    }
    else if (c == "scale") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        float s = 1.0f;
        if (args.size() > 1) s = (float)atof(args[1].c_str());
        obj->setScale(s);
        char buf[64]; snprintf(buf, sizeof(buf), "Scale set to %.2f", s);
        print(buf);
    }
    else if (c == "rot") {
        auto* obj = m_viewport->getSelectedObject();
        if (!obj) { print("No object selected"); return; }
        float yaw = 0, pitch = 0, roll = 0;
        if (args.size() > 1) yaw = (float)atof(args[1].c_str());
        if (args.size() > 2) pitch = (float)atof(args[2].c_str());
        if (args.size() > 3) roll = (float)atof(args[3].c_str());
        obj->setRotation(yaw, pitch, roll);
        char buf[64]; snprintf(buf, sizeof(buf), "Rotation set to (%.1f, %.1f, %.1f)", yaw, pitch, roll);
        print(buf);
    }
    else if (c == "solid") {
        m_viewport->setViewMode(ViewMode::Solid);
        print("View mode: Solid");
    }
    else if (c == "wire" || c == "wireframe") {
        m_viewport->setViewMode(ViewMode::Wireframe);
        print("View mode: Wireframe");
    }
    else if (c == "tex" || c == "texture") {
        m_viewport->setViewMode(ViewMode::Texture);
        print("View mode: Texture");
    }
    else if (c == "normal") {
        m_viewport->setViewMode(ViewMode::Normal);
        print("View mode: Normal");
    }
    else if (c == "script") {
        if (args.size() < 2) { print("Usage: script <path>"); return; }
        FILE* f = fopen(args[1].c_str(), "rb");
        if (!f) { print("Cannot open: " + args[1]); return; }
        char line[512];
        int lineNum = 0, execCount = 0;
        char buf[128];
        while (fgets(line, sizeof(line), f)) {
            lineNum++;
            // strip trailing whitespace/newline
            char* e = line + strlen(line) - 1;
            while (e >= line && (*e == ' ' || *e == '\t' || *e == '\r' || *e == '\n')) *e-- = 0;
            if (line[0] == '#' || line[0] == 0) continue;
            snprintf(buf, sizeof(buf), "[%d] %s", lineNum, line);
            print(buf);
            execute(std::string(line));
            execCount++;
        }
        fclose(f);
        snprintf(buf, sizeof(buf), "Script done: %d commands executed from %s", execCount, args[1].c_str());
        print(buf);
    }
    else if (c == "edit") {
        m_viewport->setEditMode(!m_viewport->isEditMode());
        char buf[64]; snprintf(buf, sizeof(buf), "Edit mode: %s",
            m_viewport->isEditMode() ? "ON" : "OFF");
        print(buf);
    }
    else {
        print("Unknown command: " + c);
        print("Type 'help' for available commands");
    }
}
