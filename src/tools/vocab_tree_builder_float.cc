// COLMAP - Structure-from-Motion and Multi-View Stereo.
// Copyright (C) 2017  Johannes L. Schoenberger <jsch at inf.ethz.ch>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//
// Revised 2020 - 7 - 17: San Jiang <jiangsan@cug.edu.cn>
//  Make this script adapt to the revision of the master COLMAP.

#include "base/database.h"
#include "optim/random_sampler.h"
#include "retrieval/visual_index.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/option_manager.h"

using namespace colmap;

const static int kDescDim = 128;
typedef retrieval::VisualIndex<float, kDescDim> VisualIndexType;

// Read descriptors from binary file.
Eigen::MatrixXf ReadDescriptors(const std::string& path) {
  std::fstream file(path, std::ios::in | std::ios::binary);
  CHECK(file.is_open());

  int num_descriptors = 0;
  int descriptor_dim = 0;
  file.read(reinterpret_cast<char*>(&num_descriptors), sizeof(int));
  file.read(reinterpret_cast<char*>(&descriptor_dim), sizeof(int));

  CHECK_EQ(descriptor_dim, kDescDim);

  Eigen::MatrixXf descriptors(num_descriptors, descriptor_dim);
  Eigen::VectorXf descriptor_row(descriptor_dim);
  for (int i = 0; i < num_descriptors; ++i) {
    file.read(reinterpret_cast<char*>(descriptor_row.data()),
              descriptor_dim * sizeof(float));
    descriptors.row(i) = descriptor_row;
  }

  return descriptors;
}

// Load descriptors for all images in database into one matrix.
Eigen::MatrixXf LoadDescriptors(const std::string& database_path,
                                const std::string& descriptor_path) {
  Database database(database_path);
  const std::vector<Image> images = database.ReadAllImages();

  int all_num_descriptors = 0;
  int all_descriptor_dim = -1;
  for (const auto& image : images) {
	const std::string image_name_ex = StringReplace(image.Name(), "/", "-");
    const auto descriptors =
        ReadDescriptors(JoinPaths(descriptor_path, /*image.Name()*/image_name_ex + ".bin"));
    all_num_descriptors += descriptors.rows();
    if (all_descriptor_dim == -1) {
      all_descriptor_dim = descriptors.cols();
    } else {
      CHECK_EQ(all_descriptor_dim, descriptors.cols());
    }
  }

  Eigen::MatrixXf all_descriptors(all_num_descriptors, all_descriptor_dim);
  int row = 0;
  for (const auto& image : images) {
    const std::string image_name_ex = StringReplace(image.Name(), "/", "-");
    const auto descriptors =
        ReadDescriptors(JoinPaths(descriptor_path, /*image.Name()*/image_name_ex + ".bin"));
    for (Eigen::MatrixXf::Index i = 0; i < descriptors.rows(); ++i) {
      all_descriptors.row(row) = descriptors.row(i);
      row += 1;
    }
  }

  return all_descriptors;
}

int main(int argc, char** argv) {
  InitializeGlog(argv);

  std::string descriptor_path;
  std::string vocab_tree_path;
  VisualIndexType::BuildOptions build_options;

  OptionManager options;
  options.AddDatabaseOptions();
  options.AddRequiredOption("descriptor_path", &descriptor_path);
  options.AddRequiredOption("vocab_tree_path", &vocab_tree_path);
  options.AddDefaultOption("num_visual_words", &build_options.num_visual_words);
  options.AddDefaultOption("branching", &build_options.branching);
  options.AddDefaultOption("num_iterations", &build_options.num_iterations);
  options.Parse(argc, argv);

  VisualIndexType visual_index;

  std::cout << "Loading descriptors..." << std::endl;
  const auto descriptors =
      LoadDescriptors(*options.database_path, descriptor_path);
  std::cout << "  => Loaded a total of " << descriptors.rows() << " descriptors"
            << std::endl;

  std::cout << "Building index for visual words..." << std::endl;
  visual_index.Build(build_options, descriptors);
  std::cout << " => Quantized descriptor space using "
            << visual_index.NumVisualWords() << " visual words" << std::endl;

  std::cout << "Saving index to file..." << std::endl;
  visual_index.Write(vocab_tree_path);

  return EXIT_SUCCESS;
}
