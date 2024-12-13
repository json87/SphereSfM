/************************************************************************/
// project : colmap

// author : jiangsan, Shenzhen University.

// e-mail: jiangsan@szu.edu.cn

// date : 2023-5-19

/************************************************************************/

#include "exe/sphere.h"

#include "base/reconstruction.h"
#include "base/sphere_camera.h"
#include "util/misc.h"
#include "util/option_manager.h"

namespace colmap {

int RunSphereCubicReprojecter(int argc, char** argv) {
  std::string input_path;
  std::string output_path;
  std::string image_ids = "0,1,2,3,4,5";
  int image_size = 0;
  double field_of_view = 45.0;

  OptionManager options;
  options.AddImageOptions();
  options.AddRequiredOption("input_path", &input_path);
  options.AddRequiredOption("output_path", &output_path);
  options.AddDefaultOption("image_ids", &image_ids);
  options.AddDefaultOption("image_size", &image_size);
  options.AddDefaultOption("field_of_view", &field_of_view);
  options.Parse(argc, argv);

  CreateDirIfNotExists(output_path);

  Reconstruction reconstruction;
  reconstruction.Read(input_path);
  reconstruction.ExportPerspectiveCubic(output_path, *options.image_path,
                                        CSVToVector<int>(image_ids), image_size,
                                        field_of_view);

  return EXIT_SUCCESS;
}

}  // namespace colmap
