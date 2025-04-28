#include "GlobalSettings.h"
#include <fstream>

GlobalSettings& GlobalSettings::getInstance() {
    static GlobalSettings instance;
    return instance;
}

void GlobalSettings::SetBool(const std::string& key, bool value) {
    boolSettings[key] = value;
}
void GlobalSettings::SetFloat(const std::string& key, float value) {
    floatSettings[key] = value;
}
void GlobalSettings::SetInt(const std::string& key, int value) {
    intSettings[key] = value;
}

bool GlobalSettings::GetBool(const std::string& key) const {
    auto it = boolSettings.find(key);
    return it != boolSettings.end() ? it->second : false;
}
float GlobalSettings::GetFloat(const std::string& key) const {
    auto it = floatSettings.find(key);
    return it != floatSettings.end() ? it->second : 0.0f;
}
int GlobalSettings::GetInt(const std::string& key) const {
    auto it = intSettings.find(key);
    return it != intSettings.end() ? it->second : 0;
}

bool GlobalSettings::LoadFromFile(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) return false;

    json j;
    inFile >> j;

    if (j.contains("bool")) {
        for (auto& [k, v] : j["bool"].items()) {
            boolSettings[k] = v.get<bool>();
        }
    }
    if (j.contains("float")) {
        for (auto& [k, v] : j["float"].items()) {
            floatSettings[k] = v.get<float>();
        }
    }
    if (j.contains("int")) {
        for (auto& [k, v] : j["int"].items()) {
            intSettings[k] = v.get<int>();
        }
    }

    return true;
}

bool GlobalSettings::SaveToFile(const std::string& filename) const {
    json j;
    j["bool"] = boolSettings;
    j["float"] = floatSettings;
    j["int"] = intSettings;

    std::ofstream outFile(filename);
    if (!outFile.is_open()) return false;

    outFile << j.dump(4); // 美化输出（4 空格缩进）
    return true;
}
