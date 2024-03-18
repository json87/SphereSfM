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

#include "base/database_cache.h"
#include "controllers/incremental_mapper.h"
#include "util/logging.h"
#include "util/math.h"
#include "util/misc.h"
#include "util/option_manager.h"

using namespace colmap;

int main(int argc, char** argv) {
  InitializeGlog(argv);

  std::string output_path;
  std::string postfix = "";

  OptionManager options;
  options.AddDatabaseOptions();
  options.AddRequiredOption("output_path", &output_path);
  options.AddDefaultOption("postfix", &postfix);
  options.AddMapperOptions();
  options.Parse(argc, argv);

  // Load database.
  DatabaseCache database_cache;

  Database database(*options.database_path);
  const size_t min_num_matches =
      static_cast<size_t>(options.mapper->min_num_matches);

  database_cache.Load(database, min_num_matches,
                      options.mapper->ignore_watermarks,
                      options.mapper->image_names);

  // Read image information.
  struct ImageInfo {
    point2D_t feature_num;
    point2D_t observation_num;
    point2D_t correspondence_num;
  };

  std::map<image_t, ImageInfo> image_infos;
  for (const auto& image : database_cache.Images()) {
    ImageInfo item;
    item.feature_num = image.second.NumPoints2D();
    item.observation_num = image.second.NumObservations();
    item.correspondence_num = image.second.NumCorrespondences();
    image_infos.emplace(image.first, item);
  }

  std::vector<double> match_ratios;
  for (const auto& image_info : image_infos) {
    match_ratios.push_back(
        static_cast<double>(image_info.second.observation_num) /
        static_cast<double>(image_info.second.feature_num));
  }

  const double mean_ratio = Mean(match_ratios);
  const double stddev_ratio = StdDev(match_ratios);

  // Write statistic results.
  std::string statistic_result_path;
  if (postfix.empty()) {
    statistic_result_path = output_path + "statistic_match.txt";
  } else {
    statistic_result_path =
        output_path + StringPrintf("statistic_match-%s.txt", postfix.c_str());
  }

  std::ofstream write(statistic_result_path);
  CHECK(write.is_open()) << StringPrintf("Fail to open file: %s",
                                         statistic_result_path.c_str());

  write << StringPrintf("%lf %lf", mean_ratio, stddev_ratio) << std::endl;
  for (const auto& image_info : image_infos) {
    image_t image_id = image_info.first;
    double feature_num = image_info.second.feature_num;
    write << StringPrintf(
                 "%d %d %d %.3f", image_info.first,
                 image_info.second.feature_num,
                 image_info.second.observation_num,
                 static_cast<double>(image_info.second.observation_num) /
                     static_cast<double>(image_info.second.feature_num))
          << std::endl;
  }

  write.close();

  return EXIT_SUCCESS;
}
