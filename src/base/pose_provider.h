/************************************************************************/
// project : base

// author : jiangsan, Wuhan University, LIESMARS

// e-mail: jiangsan870211@whu.edu.cn

// date : 2016-7-8

/************************************************************************/

#ifndef COLMAP_SRC_BASE_POSE_PROVIDER_H_
#define COLMAP_SRC_BASE_POSE_PROVIDER_H_

#include <Eigen/Core>
#include <unordered_map>
#include <unordered_set>

namespace colmap {

/// Pose information structure.
struct PoseItem {
  Eigen::Vector4d q_vec_;
  Eigen::Vector3d t_vec_;
};

// Pose provider is used to access pose information from pose file.
// The default pose file format is formated as [N, Y, X, Z, W, P, K],
// which stands for name, latitude, longitude, altitude, omega, phi
// and kappa; OR [N, Y, X, Z, R] with R stands for the rotation matrix.
class PoseProvider {
 public:
  PoseProvider(const bool is_cv = false);
  PoseProvider(const std::string& pose_path, const bool is_cv = false);
  ~PoseProvider(){};

 public:
  void ReadPose(const std::string& path);

  // Access all pose items.
  inline const std::unordered_map<std::string, PoseItem>& Poses() const;
  inline std::unordered_map<std::string, PoseItem>& Poses();

  // Access specified pose item.
  inline const PoseItem& Pose(const std::string& name) const;
  inline PoseItem& Pose(const std::string& name);

  // Check existence of specified pose.
  inline bool ExistsPose(const std::string& name) const;

 private:
  // Whether or not the input POS is in the CV format.
  bool is_cv_;

  // Pose information is indexed by image file name.
  std::unordered_map<std::string, PoseItem> pose_map_;
};

////////////////////////////////////////////////////////////////////////////////
// Implementation
////////////////////////////////////////////////////////////////////////////////

inline const std::unordered_map<std::string, PoseItem>& PoseProvider::Poses()
    const {
  return pose_map_;
}
inline std::unordered_map<std::string, PoseItem>& PoseProvider::Poses() {
  return pose_map_;
}

inline const PoseItem& PoseProvider::Pose(const std::string& name) const {
  return pose_map_.at(name);
}
inline PoseItem& PoseProvider::Pose(const std::string& name) {
  return pose_map_.at(name);
}

inline bool PoseProvider::ExistsPose(const std::string& name) const {
  return pose_map_.count(name) > 0;
}

}  // namespace colmap

#endif  // COLMAP_SRC_BASE_POSE_PROVIDER_H_
