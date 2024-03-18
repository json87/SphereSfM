/************************************************************************/
// project : base

// author : jiangsan, school of computer science, CUG.

// e-mail: jiangsan@cug.edu.cn

// date : 2022-7-2

/************************************************************************/

#include "base/sphere_camera.h"

#include <iomanip>

#include "util/math.h"
#include "util/misc.h"
#include "util/threading.h"

namespace colmap {

double ImagePlaneToCameraPlaneError(const size_t width, const size_t height,
                                    double image_error) {
  return image_error / std::max(width, height);
}

double CameraPlaneToImagePlaneError(const size_t width, const size_t height,
                                    double camera_error) {
  return camera_error * std::max(width, height);
}

double ImagePlaneToSpherePlaneError(const size_t width, const size_t height,
                                    double image_error) {
  return 2 * M_PI * ImagePlaneToCameraPlaneError(width, height, image_error);
}

double SpherePlaneToImagePlaneError(const size_t width, const size_t height,
                                    double sphere_error) {
  return CameraPlaneToImagePlaneError(width, height, sphere_error / (2 * M_PI));
}

Eigen::Vector3d NormalizedPointToBearingVector(
    const Eigen::Vector2d& normalized_point) {
  const double lon = normalized_point(0) * 2 * M_PI;
  const double lat = -normalized_point(1) * 2 * M_PI;
  return {std::cos(lat) * std::sin(lon), -std::sin(lat),
          std::cos(lat) * std::cos(lon)};
}

std::vector<Eigen::Vector3d> NormalizedPointsToBearingVectors(
    const std::vector<Eigen::Vector2d>& normalized_points) {
  std::vector<Eigen::Vector3d> bearing_vectors;
  bearing_vectors.resize(normalized_points.size());
  for (size_t i = 0; i < normalized_points.size(); ++i) {
    bearing_vectors[i] = NormalizedPointToBearingVector(normalized_points[i]);
  }
  return bearing_vectors;
}

Eigen::Vector2d BearingVectorToNormalizedPoint(
    const Eigen::Vector3d& bearing_vector) {
  const double lon = std::atan2(bearing_vector(0), bearing_vector(2));
  const double lat = std::atan2(
      -bearing_vector(1), std::hypot(bearing_vector(0), bearing_vector(2)));
  return {lon / (2 * M_PI), -lat / (2 * M_PI)};
}

std::vector<Eigen::Vector2d> BearingVectorsToNormalizedPoints(
    const std::vector<Eigen::Vector3d>& bearing_vectors) {
  std::vector<Eigen::Vector2d> normalized_points;
  normalized_points.resize(bearing_vectors.size());
  for (size_t i = 0; i < bearing_vectors.size(); ++i) {
    normalized_points[i] = BearingVectorToNormalizedPoint(bearing_vectors[i]);
  }
  return normalized_points;
}

Eigen::Vector2d NormalizedPointToLonLat(
    const Eigen::Vector2d& normalized_point) {
  const double lon = normalized_point(0) * 2 * M_PI;
  const double lat = -normalized_point(1) * 2 * M_PI;
  return {RadToDeg(lon), RadToDeg(lat)};
}

std::vector<Eigen::Vector2d> NormalizedPointsToLonLats(
    const std::vector<Eigen::Vector2d>& normalized_points) {
  std::vector<Eigen::Vector2d> lonlats;
  lonlats.resize(normalized_points.size());
  for (size_t i = 0; i < normalized_points.size(); ++i) {
    lonlats[i] = NormalizedPointToLonLat(normalized_points[i]);
  }
  return lonlats;
}

}  // namespace colmap