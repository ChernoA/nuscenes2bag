
#pragma once

#include "nuscenes2bag/MsgDirectoryConverter.hpp"

#include "sensor_msgs/PointCloud2.h"
#include <pcl_ros/point_cloud.h>

template <>
std::optional<sensor_msgs::PointCloud2> processSingleFileFun(
    const std::pair<std::filesystem::path, ExtractedFileNameInfo> &fileInfo);