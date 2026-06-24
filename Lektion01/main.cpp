#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <iostream>

// Diese Funktion wird aufgerufen, wenn das Fenster in der Größe verändert wird
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    // 1. Initialisiere GLFW
    if (!glfwInit()) {
        std::cerr << "Fehler beim Starten von GLFW" << std::endl;
        return -1;
    }

    // 2. Fenster erstellen
    GLFWwindow* window = glfwCreateWindow(800, 600, "NeHe Lektion 01 - Das Fenster", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    // Den Kontext im aktuellen Thread aktivieren
    glfwMakeContextCurrent(window);
    // Callback für Größenänderungen registrieren
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 3. OpenGL Grundeinstellungen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Hintergrundfarbe: Schwarz

    // 4. Die Hauptschleife (Main Loop)
    while (!glfwWindowShouldClose(window)) {
        // Den Bildschirm mit der Hintergrundfarbe löschen
        glClear(GL_COLOR_BUFFER_BIT);

        // Hier würde später der Zeichen-Code stehen

        // Puffer tauschen (Double Buffering)
        glfwSwapBuffers(window);
        // Eingaben (Tastatur/Maus) verarbeiten
        glfwPollEvents();

        // ESC-Taste zum Beenden prüfen
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }

    // 5. Aufräumen
    glfwTerminate();
    return 0;
}
