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

double PinholeFocalLength(const int height, const double field_of_view) {
  return height / (2 * std::tan(DegToRad(field_of_view)));
}

Camera PinholeCamera(const int width, const int height,
                     const double field_of_view) {
  const double focal_length = PinholeFocalLength(height, field_of_view);
  Camera pinhole_camera;
  pinhole_camera.InitializeWithName("SIMPLE_PINHOLE", focal_length, width,
                                    height);
  return pinhole_camera;
}

Camera SphereCamera(const int width, const int height) {
  Camera sphere_camera;
  sphere_camera.InitializeWithName("SPHERE", 1.0, width, height);
  return sphere_camera;
}

const std::unordered_map<int, Eigen::Matrix3d> GetCubicRotations() {
  // return {Eigen::AngleAxisd(DegToRad(0.0), Eigen::Vector3d::UnitY())
  //            .toRotationMatrix(),  // front
  //        Eigen::AngleAxisd(DegToRad(90.0), Eigen::Vector3d::UnitY())
  //            .toRotationMatrix(),  // right
  //        Eigen::AngleAxisd(DegToRad(180.0), Eigen::Vector3d::UnitY())
  //            .toRotationMatrix(),  // back
  //        Eigen::AngleAxisd(DegToRad(270.0), Eigen::Vector3d::UnitY())
  //            .toRotationMatrix(),  // left
  //        Eigen::AngleAxisd(DegToRad(90.0), Eigen::Vector3d::UnitX())
  //            .toRotationMatrix(),  // top
  //        Eigen::AngleAxisd(DegToRad(-90.0), Eigen::Vector3d::UnitX())
  //            .toRotationMatrix()};  // bottom
  std::unordered_map<int, Eigen::Matrix3d> rots;
  rots[0] = GetTangentPlaneRotation(0.0, 0.0);    // front
  rots[1] = GetTangentPlaneRotation(90.0, 0.0);   // right
  rots[2] = GetTangentPlaneRotation(180.0, 0.0);  // back
  rots[3] = GetTangentPlaneRotation(270.0, 0.0);  // left
  rots[4] = GetTangentPlaneRotation(0.0, 90);     // top
  rots[5] = GetTangentPlaneRotation(0.0, -90.0);  // bottom
  return rots;
}

const Eigen::Matrix3d GetTangentPlaneRotation(const double lon,
                                              const double lat,
                                              const double rot) {
  const Eigen::Matrix3d rotx =
      Eigen::AngleAxisd(DegToRad(lat), Eigen::Vector3d::UnitX())
          .toRotationMatrix();
  const Eigen::Matrix3d roty =
      Eigen::AngleAxisd(DegToRad(lon), Eigen::Vector3d::UnitY())
          .toRotationMatrix();
  const Eigen::Matrix3d rotz =
      Eigen::AngleAxisd(DegToRad(rot), Eigen::Vector3d::UnitZ())
          .toRotationMatrix();
  return roty * rotx * rotz;
}

void SphericalToPatch(const Camera& sphere_camera, const Bitmap& sphere_bitmap,
                      const Eigen::Matrix3d& rotation,
                      const Camera& pinhole_camera, Bitmap& pinhole_bitmap) {
  pinhole_bitmap.Allocate(pinhole_camera.Width(), pinhole_camera.Height(),
                          sphere_bitmap.IsRGB());

  // Calculate the pixel location of the patch center.
  // Bearing vector in pinhole camera.
  const Eigen::Vector3d bearing_pinhole(0.0, 0.0, 1.0);
  // Bearing vector in sphere camera.
  const Eigen::Vector3d bearing_sphere = rotation * bearing_pinhole;
  // Projection to the sphere image.
  const Eigen::Vector2d coord_center = sphere_camera.WorldToImage(
      BearingVectorToNormalizedPoint(bearing_sphere));

  const Eigen::Vector2d offset(
      static_cast<int>(pinhole_bitmap.Width() / 2.0) + 0.5,
      static_cast<int>(pinhole_bitmap.Height() / 2.0) + 0.5);

  // Copy the patch directly from the sphere image.
  BitmapColor<float> color;
  for (size_t y = 0; y < pinhole_bitmap.Height(); ++y) {
    for (size_t x = 0; x < pinhole_bitmap.Width(); ++x) {
      const Eigen::Vector2d coord_pinhole(x + 0.5, y + 0.5);
      Eigen::Vector2d coord_sphere = coord_pinhole - offset + coord_center;

      if (coord_sphere.x() >= sphere_bitmap.Width()) {
        coord_sphere.x() -= sphere_bitmap.Width();
      }
      if (coord_sphere.x() < 0.0) {
        coord_sphere.x() += sphere_bitmap.Width();
      }
      if (coord_sphere.y() >= sphere_bitmap.Height()) {
        coord_sphere.y() -= sphere_bitmap.Height();
      }
      if (coord_sphere.y() < 0.0) {
        coord_sphere.y() += sphere_bitmap.Height();
      }

      if (sphere_bitmap.InterpolateBilinear(coord_sphere.x(), coord_sphere.y(),
                                            &color)) {
        pinhole_bitmap.SetPixel(x, y, color.Cast<uint8_t>());
      }
    }
  }
}

void SphericalToTangent(const Camera& sphere_camera,
                        const Bitmap& sphere_bitmap,
                        const Eigen::Matrix3d& rotation,
                        const Camera& pinhole_camera, Bitmap& pinhole_bitmap) {
  pinhole_bitmap.Allocate(pinhole_camera.Width(), pinhole_camera.Height(),
                          sphere_bitmap.IsRGB());

  BitmapColor<float> color;
  for (size_t y = 0; y < pinhole_bitmap.Height(); ++y) {
    for (size_t x = 0; x < pinhole_bitmap.Width(); ++x) {
      const Eigen::Vector2d coord_pinhole(x + 0.5, y + 0.5);

      // Bearing vector in pinhole camera.
      const Eigen::Vector3d bearing_pinhole =
          pinhole_camera.ImageToWorld(coord_pinhole).homogeneous().normalized();
      // Bearing vector in sphere camera.
      const Eigen::Vector3d bearing_sphere = rotation * bearing_pinhole;
      // Projection to the sphere image.
      const Eigen::Vector2d coord_sphere = sphere_camera.WorldToImage(
          BearingVectorToNormalizedPoint(bearing_sphere));

      if (sphere_bitmap.InterpolateBilinear(coord_sphere.x(), coord_sphere.y(),
                                            &color)) {
        pinhole_bitmap.SetPixel(x, y, color.Cast<uint8_t>());
      }
    }
  }
}

std::vector<std::string> SphericalToPinhole(
    const Camera& sphere_camera, const Bitmap& sphere_bitmap,
    const std::string& sphere_path, const Camera& pinhole_camera,
    const std::string& output_path, const std::vector<int>& image_ids,
    const std::unordered_map<int, Eigen::Matrix3d>& rotations,
    const bool tangent_proj) {
  CHECK_LE(image_ids.size(), rotations.size());

  std::vector<std::string> pinhole_paths(image_ids.size());

  // Function for generating pinhole images.
  auto GeneratePinholeImage = [&](const int image_id) {
    Bitmap pinhole_bitmap;
    const Eigen::Matrix3d rotation = rotations.at(image_id);
    if (tangent_proj) {
      SphericalToTangent(sphere_camera, sphere_bitmap, rotation, pinhole_camera,
                         pinhole_bitmap);
    } else {
      SphericalToPatch(sphere_camera, sphere_bitmap, rotation, pinhole_camera,
                       pinhole_bitmap);
    }

    std::string file_name, file_ext;
    const std::string base_name = GetPathBaseName(sphere_path);
    SplitFileExtension(base_name, &file_name, &file_ext);

    std::ostringstream os;
    os << std::setw(8) << std::setfill('0') << image_id;
    pinhole_paths[image_id] = JoinPaths(
        output_path, StringPrintf("%s_perspective_%s", file_name.c_str(),
                                  (os.str() + file_ext).c_str()));
    pinhole_bitmap.Write(pinhole_paths[image_id]);
  };

  // Determine the number of workers.
  const int kMaxNumThreads = -1;
  const int num_eff_threads = GetEffectiveNumThreads(kMaxNumThreads);
  const int num_eff_workers =
      std::min(static_cast<int>(image_ids.size()), num_eff_threads);

  ThreadPool thread_pool(num_eff_workers);
  for (const auto& image_id : image_ids) {
    thread_pool.AddTask(GeneratePinholeImage, image_id);
  }
  thread_pool.Wait();

  return pinhole_paths;
}

}  // namespace colmap