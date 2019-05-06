#include "nuscenes2bag/RadarDirectoryConverter.hpp"
#include "nuscenes2bag/PclRadarObject.hpp"

using namespace sensor_msgs;
using namespace std;
using namespace nuscenes2bag;

template <>
std::optional<RadarObjects> processSingleFileFun(
    const std::pair<std::filesystem::path, ExtractedFileNameInfo> &fileInfo) {

  pcl::PointCloud<PclRadarObject>::Ptr cloud(new pcl::PointCloud<PclRadarObject>);
  const auto fileName = fileInfo.first.string();

  if (pcl::io::loadPCDFile<PclRadarObject>(fileName, *cloud) ==
      -1) //* load the file
  {
    std::string error = "Could not read ";
    error += fileName;
    cout << error << endl;
    // PCL_ERROR(error);
    return std::nullopt;
  }

  RadarObjects radarObjects;
  radarObjects.header.stamp = stampUs2RosTime(fileInfo.second.stampUs);

  for(const auto& pclRadarObject: *cloud) {
      RadarObject obj;
      obj.pose.x = pclRadarObject.x;
      obj.pose.y = pclRadarObject.y;
      obj.pose.z = pclRadarObject.z;
      obj.dyn_prop = pclRadarObject.dyn_prop;
      obj.rcs = pclRadarObject.rcs;
      obj.vx = pclRadarObject.vx;
      obj.vy = pclRadarObject.vy;
      obj.vx_comp = pclRadarObject.vx_comp;
      obj.vy_comp = pclRadarObject.vy_comp;
      obj.is_quality_valid = pclRadarObject.is_quality_valid;
      obj.ambig_state = pclRadarObject.ambig_state;
      obj.x_rms = pclRadarObject.x_rms;
      obj.y_rms = pclRadarObject.y_rms;
      obj.invalid_state = pclRadarObject.invalid_state;
      obj.pdh0 = pclRadarObject.pdh0;
      obj.vx_rms = pclRadarObject.vx_rms;
      obj.vy_rms = pclRadarObject.vy_rms;
      radarObjects.objects.push_back(obj);

  }


  return radarObjects;
}

template <>
class MsgDirectoryConverter<RadarObjects>
    : public SampleSetDirectoryConverter {};