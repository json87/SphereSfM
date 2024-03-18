/************************************************************************/
// project : base

// author : jiangsan, Wuhan University, LIESMARS

// e-mail: jiangsan870211@whu.edu.cn

// date : 2016-7-8

/************************************************************************/

#include "pose_provider.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "base/pose.h"
#include "util/math.h"
#include "util/misc.h"

namespace colmap {

namespace fs = boost::filesystem;

const Eigen::Matrix3d rot_pho2cv =
    Eigen::AngleAxisd(M_PI, Eigen::Vector3d::UnitX()).toRotationMatrix();

PoseProvider::PoseProvider(const bool is_cv) : is_cv_(is_cv) {}

PoseProvider::PoseProvider(const std::string& pose_path, const bool is_cv)
    : is_cv_(is_cv) {
  ReadPose(pose_path);
}

void PoseProvider::ReadPose(const std::string& path) {
  if (!fs::exists(fs::path(path))) return;

  std::ifstream file;
  file.open(path.c_str());
  if (!file.is_open()) {
    std::cout << StringPrintf("Fail to open file: %s", path.c_str())
              << std::endl;
    return;
  }

  std::string line, item;

  // Parse pose file format.
  std::vector<std::string> formatStrs;
  bool bHasFormatLine = false;

  std::string seps;
  seps.append(" ");  // space
  seps.append("	");  // tab
  seps.append(",");  // comma
  while (std::getline(file, line)) {
    StringTrim(&line);
    if (line.empty()) continue;

    // Check pose file format line.
    std::stringstream line_stream(line);
    std::getline(line_stream, item, '=');
    if (!boost::equals(item, "#F")) {
      std::cout << StringPrintf("Invalidate coordinate file: %s", path.c_str())
                << std::endl;
      return;
    }

    std::getline(line_stream, item, '=');
    formatStrs = StringSplit(item, seps);

    bHasFormatLine = true;

    break;
  }
  if (!bHasFormatLine) {
    std::cout << StringPrintf("Coordinate file has not format line.")
              << std::endl;
    return;
  }

  int itemNum = formatStrs.size();
  if (itemNum < 4) {
    std::cout
        << StringPrintf(
               "Each pose should have at least 4 values for label and xyz.")
        << std::endl;
    return;
  }

  // Loops over all pose items.
  std::string name;
  PoseItem poseItem;

  double omega, phi, kappa;
  omega = phi = kappa = 0.0;

  bool isRMatrix = false;
  double R[9];
  memset(R, 0.0, 9 * sizeof(double));

  std::vector<std::string> strs;
  while (std::getline(file, line)) {
    boost::trim(line);
    // Skip empty and comment lines.
    if (line.empty() || line[0] == '#') continue;

    strs.clear();
    strs = StringSplit(line, seps);
    // assert(strs.size() == formatStrs.size());

    for (int i = 0; i < itemNum; i++) {
      if (strcmp(formatStrs[i].data(), "N") == 0) {
        name = strs[i];  // image name.
      } else if (strcmp(formatStrs[i].data(), "X") == 0) {
        poseItem.t_vec_(1) = atof(strs[i].data());  // longitude.
      } else if (strcmp(formatStrs[i].data(), "Y") == 0) {
        poseItem.t_vec_(0) = atof(strs[i].data());  // latitude.
      } else if (strcmp(formatStrs[i].data(), "Z") == 0) {
        poseItem.t_vec_(2) = atof(strs[i].data());  // altitude.
      } else if (strcmp(formatStrs[i].data(), "W") == 0) {
        omega = atof(strs[i].data());  // omega.
      } else if (strcmp(formatStrs[i].data(), "P") == 0) {
        phi = atof(strs[i].data());  // phi.
      } else if (strcmp(formatStrs[i].data(), "K") == 0) {
        kappa = atof(strs[i].data());  // kappa.
      } else if (strcmp(formatStrs[i].data(), "R") == 0) {
        // Rotation matrix
        for (int j = 0; j < 9; ++j) {
          R[j] = atof(strs[i + j].data());
        }
        isRMatrix = true;
      }
    }

    Eigen::Matrix3d R_Ei =
        isRMatrix ? Eigen::Map<Eigen::Matrix3d>(R, 3, 3)
                  : EulerAnglesToRotationMatrix(DegToRad(omega), DegToRad(phi),
                                                DegToRad(kappa));
    R_Ei = is_cv_ ? R_Ei : rot_pho2cv * R_Ei;
    poseItem.q_vec_ = NormalizeQuaternion(RotationMatrixToQuaternion(R_Ei));

    pose_map_[name] = poseItem;
  }
  file.close();
}

}  // namespace colmap
