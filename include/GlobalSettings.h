#pragma once
#include <nlohmann/json.hpp>
using json = nlohmann::json;

class GlobalSettings {
public:
    static GlobalSettings& getInstance();

    void SetBool(const std::string& key, bool value);
    void SetFloat(const std::string& key, float value);
    void SetInt(const std::string& key, int value);

    bool GetBool(const std::string& key) const;
    float GetFloat(const std::string& key) const;
    int GetInt(const std::string& key) const;

    // 新增 JSON 加载/保存接口
    bool LoadFromFile(const std::string& filename);
    bool SaveToFile(const std::string& filename) const;

private:
    GlobalSettings() = default;

    std::unordered_map<std::string, bool> boolSettings;
    std::unordered_map<std::string, float> floatSettings;
    std::unordered_map<std::string, int> intSettings;
};