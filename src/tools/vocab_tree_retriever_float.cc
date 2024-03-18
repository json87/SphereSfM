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
#include "feature/types.h"
#include "retrieval/visual_index.h"
#include "util/logging.h"
#include "util/misc.h"
#include "util/option_manager.h"
#include "util/timer.h"

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

void IndexImagesInVisualIndex(const std::string& descriptor_path,
                              Database* database,
                              VisualIndexType* visual_index) {
  const std::vector<Image> images = database->ReadAllImages();

  for (size_t i = 0; i < images.size(); ++i) {
    Timer timer;
    timer.Start();

    std::cout << StringPrintf("Indexing image [%d/%d]", i + 1, images.size())
              << std::flush;

    const std::string image_name_ex = StringReplace(images[i].Name(), "/", "-");
    const auto descriptors = ReadDescriptors(JoinPaths(
        descriptor_path, /*images[i].Name()*/ image_name_ex + ".bin"));
    const auto keypoints = FeatureKeypoints(descriptors.rows());

    visual_index->Add(VisualIndexType::IndexOptions(), images[i].ImageId(),
                      keypoints, descriptors);

    std::cout << StringPrintf(" in %.3fs", timer.ElapsedSeconds()) << std::endl;
  }

  // Compute the TF-IDF weights, etc.
  visual_index->Prepare();
}

void QueryImagesInVisualIndex(const std::string& descriptor_path,
                              const int num_images, Database* database,
                              VisualIndexType* visual_index) {
  const std::vector<Image> images = database->ReadAllImages();

  VisualIndexType::QueryOptions query_options;
  query_options.max_num_images = num_images;

  std::unordered_map<image_t, const Image*> image_id_to_image;
  image_id_to_image.reserve(images.size());
  for (const auto& image : images) {
    image_id_to_image.emplace(image.ImageId(), &image);
  }

  for (size_t i = 0; i < images.size(); ++i) {
    Timer timer;
    timer.Start();

    std::cout << StringPrintf("Querying for image %s [%d/%d]",
                              images[i].Name().c_str(), i + 1, images.size())
              << std::flush;

    const std::string image_name_ex = StringReplace(images[i].Name(), "/", "-");
    const auto descriptors = ReadDescriptors(JoinPaths(
        descriptor_path, /*images[i].Name()*/ image_name_ex + ".bin"));

    std::vector<retrieval::ImageScore> image_scores;
    visual_index->Query(query_options, descriptors, &image_scores);

    std::cout << StringPrintf(" in %.3fs", timer.ElapsedSeconds()) << std::endl;
    for (const auto& image_score : image_scores) {
      const auto& image = *image_id_to_image.at(image_score.image_id);
      std::cout << StringPrintf("  image_id=%d, image_name=%s, score=%f",
                                image_score.image_id, image.Name().c_str(),
                                image_score.score)
                << std::endl;
    }
  }
}

int main(int argc, char** argv) {
  InitializeGlog(argv);

  std::string descriptor_path;
  std::string vocab_tree_path;
  int num_images = 100;

  OptionManager options;
  options.AddDatabaseOptions();
  options.AddRequiredOption("descriptor_path", &descriptor_path);
  options.AddRequiredOption("vocab_tree_path", &vocab_tree_path);
  options.AddDefaultOption("num_images", &num_images);
  options.Parse(argc, argv);

  VisualIndexType visual_index;
  visual_index.Read(vocab_tree_path);

  Database database(*options.database_path);

  IndexImagesInVisualIndex(descriptor_path, &database, &visual_index);
  QueryImagesInVisualIndex(descriptor_path, num_images, &database,
                           &visual_index);

  return EXIT_SUCCESS;
}
