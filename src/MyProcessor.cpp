#include "nuscenes2rosbag/MyProcessor.hpp"

MyProcessor::MyProcessor(const std::string& bagName) {
  outBag.open(bagName, rosbag::bagmode::Write);
}

MyProcessor::~MyProcessor() {
  outBag.close();
}