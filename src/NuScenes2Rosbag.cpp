#include "nuscenes2rosbag/NuScenes2Rosbag.hpp"
#include "nuscenes2rosbag/SampleQueue.hpp"
#include "nuscenes2rosbag/utils.hpp"
#include "nuscenes2rosbag/ImageDirectoryConverter.hpp"
#include "nuscenes2rosbag/MyProcessor.hpp"

#include <boost/asio.hpp>


#include <iostream>

#include <std_msgs/Int32.h>
#include <std_msgs/String.h>
#include <thread>
#include <array>

namespace fs = std::filesystem;

NuScenes2Rosbag::NuScenes2Rosbag() {}


std::optional<FileSystemSampleSet>
NuScenes2Rosbag::extractSampleSetDescriptorInDirectory(
    const std::filesystem::path &inDirectoryPath) {
  const std::string &dirName = inDirectoryPath.filename();

  struct InfoPreset {
    const char* name;
    const SampleSetType sampleType;
  };

  static std::array<InfoPreset, 3> presets {
    "CAM",   SampleSetType::CAMERA,
    "RADAR", SampleSetType::RADAR,
    "LIDAR", SampleSetType::LIDAR,
  };

  for(auto& preset : presets) {
    if(string_icontains(dirName, preset.name)) {
      return std::optional<FileSystemSampleSet>{
          {{dirName, preset.sampleType}, inDirectoryPath}};
    }
  }

  std::cout << "Skipping " << inDirectoryPath << std::endl;
  return std::nullopt;
}

std::vector<FileSystemSampleSet> NuScenes2Rosbag::getSampleSetsInDirectory(
    const std::filesystem::path &inDatasetPath) {
  std::vector<FileSystemSampleSet> sets;

  for (auto &dir : fs::directory_iterator(inDatasetPath)) {
    if (dir.is_directory()) {
      auto sampleSetOpt = extractSampleSetDescriptorInDirectory(dir.path());
      if (sampleSetOpt.has_value()) {
        sets.push_back(sampleSetOpt.value());
      }
    }
  }

  return sets;
}

std::vector<FileSystemSampleSet> NuScenes2Rosbag::filterChosenSampleSets(
    const std::vector<FileSystemSampleSet> &sampleSets) {
  std::vector<FileSystemSampleSet> filteredSets;
  std::copy_if(sampleSets.begin(), sampleSets.end(),
               std::back_inserter(filteredSets), [](auto &fileSystemSampleSet) {
                 return fileSystemSampleSet.descriptor.setType ==
                        SampleSetType::CAMERA;
               });

  return filteredSets;
}

// SampleSetDirectoryConverter&& NuScenes2Rosbag::prepareConverter() {

// }

void NuScenes2Rosbag::processSampleSets(
  const std::vector<FileSystemSampleSet>& sampleSets, 
  const std::filesystem::path &outputRosbagPath) {

  std::vector<std::pair<TopicInfo, TypeErasedQueue>> typeErasedQueueList;
  std::vector<std::unique_ptr<SampleSetDirectoryConverter>> sampleSetConverters;

  // Launch the pool with four threads.
  boost::asio::thread_pool pool(6);

  for (const auto &sampleSet : sampleSets) {

    auto queueProducerConsumerPair =
        SampleQueueFactory<sensor_msgs::Image>::makeQueue();
    typeErasedQueueList.emplace_back(
        TopicInfo(topicNameForSampleSetType(
          sampleSet.descriptor.directoryName, sampleSet.descriptor.setType)),
        TypeErasedQueue(queueProducerConsumerPair.second));

    if (sampleSet.descriptor.setType == SampleSetType::CAMERA) {
      // check if value is really moved here
      sampleSetConverters.push_back(std::make_unique<MsgDirectoryConverter<sensor_msgs::Image>>(
          std::move(queueProducerConsumerPair.first), sampleSet.directoryPath));
    } else {
      throw std::runtime_error("Not supported SampleSetType");
    }
  }
  SampleSetDirectoryConverter* converter = sampleSetConverters.back().get();
  boost::asio::post(pool, [converter]() { converter->process(); });

  fs::remove(outputRosbagPath);

  MyProcessor processor(outputRosbagPath.filename());

  while (true) {
    bool atLeastOneQueueIsStillOpen = false;
    for (auto &[topicInfo, queue] : typeErasedQueueList) {
      if (!queue.isClosed() || (queue.size() > 0)) {
        atLeastOneQueueIsStillOpen = true;
        queue.process(topicInfo, processor);
      }
    }
    if (!atLeastOneQueueIsStillOpen) {
      std::cout << "completed all queue" << std::endl;
      break;
    }
  }

  pool.join();
}

void NuScenes2Rosbag::convertDirectory(
    const std::filesystem::path &inDatasetPath,
    const std::filesystem::path &outputRosbagPath) {

  auto availableSampleSets = getSampleSetsInDirectory(inDatasetPath);
  std::cout << "Found " << availableSampleSets.size()
            << " valid sample directory" << std::endl;

  auto chosenSets = filterChosenSampleSets(availableSampleSets);
  std::cout << "Chosen " << chosenSets.size() << " sample directory"
            << std::endl;

  processSampleSets(chosenSets, outputRosbagPath);
}