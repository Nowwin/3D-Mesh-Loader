#include "GraphicsApp.hpp"

int main() {
    
    GraphicsApp app(640, 480, -2.0f, 0.0f);

    app.Initialize();
    app.MainLoop();
    app.CleanUp();

    return 0;
}
