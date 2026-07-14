#include <fstream>
#include <iostream>

class Settings {
public:
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    float volume = 0.8f;

    bool loadFromFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            auto eq = line.find('=');
            if (eq == std::string::npos) continue;

            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            // trim whitespace on key/value here if needed

            if (key == "windowWidth") windowWidth = std::stoi(value);
            else if (key == "windowHeight") windowHeight = std::stoi(value);
            else if (key == "fullscreen") fullscreen = (value == "true" || value == "1");
            else if (key == "volume") volume = std::stof(value);
        }
        return true;
    }

    void saveToFile(const std::string& path) {
        std::ofstream file(path);
        file << "windowWidth=" << windowWidth << "\n";
        file << "windowHeight=" << windowHeight << "\n";
        file << "fullscreen=" << (fullscreen ? "true" : "false") << "\n";
        file << "volume=" << volume << "\n";
    }
};
