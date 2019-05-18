#include <filesystem>
#include <iostream>
#include <map>
#include <set>

#include <nlohmann/json.hpp>

#include "nuscenes2bag/MetaData.hpp"
#include "nuscenes2bag/MetaDataProvider.hpp"
#include "nuscenes2bag/ToDebugString.hpp"

class MetaDataReader : public MetaDataProvider {
public:
  void loadFromDirectory(const std::filesystem::path &directoryPath);


  std::vector<Token> getAllSceneTokens() const override;
  std::optional<SceneInfo> getSceneInfo(const Token &sceneToken) const override;
  std::vector<SampleDataInfo>
  getSceneSampleData(const Token &sceneToken) const override;
  std::vector<EgoPoseInfo>
  getEgoPoseInfo(const Token &sceneToken) const override;
  CalibratedSensorInfo
  getCalibratedSensorInfo(const Token &calibratedSensorToken) const override;
  std::vector<CalibratedSensorInfo>
  getSceneCalibratedSensorInfo(const Token &sceneToken) const override;

private:
  static nlohmann::json slurpJsonFile(const std::filesystem::path &filePath);
  static std::vector<SceneInfo>
  loadScenesFromFile(const std::filesystem::path &filePath);
  static std::map<Token, std::vector<SampleInfo>>
  loadSampleInfos(const std::filesystem::path &filePath);
  static std::map<Token, std::vector<SampleDataInfo>>
  loadSampleDataInfos(const std::filesystem::path &filePath);
  static std::map<Token, std::vector<EgoPoseInfo>> loadEgoPoseInfos(
      const std::filesystem::path &filePath,
      std::map<Token, Token> sample2SampleData);
  static std::map<Token, CalibratedSensorInfo>
  loadCalibratedSensorInfo(const std::filesystem::path &filePath);

  std::vector<SceneInfo> scenes;
  std::map<Token, std::vector<SampleInfo>> scene2Samples;
  std::map<Token, std::vector<SampleDataInfo>> sample2SampleData;
  std::map<Token, std::vector<EgoPoseInfo>> scene2EgoPose;
  std::map<Token, CalibratedSensorInfo> calibratedSensorToken2CalibratedSensorInfo;
  std::map<Token, std::set<CalibratedSensorInfo>> scene2CalibratedSensorInfo;
  bool loadFromDirectoryCalled = false;
};
