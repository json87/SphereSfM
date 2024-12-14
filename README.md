# SphereSfM

SphereSfM aims to achieve sparse reconstruction of spherical images in the ERP (EquiRectangular Projection) format based on increamental Structure from Motion. 
Recently, Spherical images are gaining more and more attention due to the popularity of spherical cameras, such as Insta360 and Ricoh theta, and they have been exploited
for 3D modeling in complex urban scenes, especially for near-ground buildings and streets. This project provides a software package for image orientation, which can be
integrated into a SfM-MVS workflow.

![SphereSfM](https://github.com/json87/SphereSfM/blob/main/doc/spheresfm.jpg)

![sphere image](https://github.com/json87/SphereSfM/blob/main/doc/sphere%20image.jpg)

## Build

The software package has been integrated into the well-known SfM engine [ColMap](https://github.com/colmap/colmap). Thus, you can build the code according to the [document](https://colmap.github.io/).

## Dataset

Three datasets that are recorded from different scenes are provided, including campus parterre ([google driver](https://drive.google.com/file/d/1KB1uk9wEUvEGVnFOwcrw4r_KxUk711eb/view?usp=drive_link), [baidu disk](https://pan.baidu.com/s/1C259Ygf_lJHd5iT-gmJWGA?pwd=5cqb)), campus building ([google driver](https://drive.google.com/file/d/17HfwXxuU-Q-tzZtlsroGa-ZibepAT0-a/view?usp=drive_link), [baidu disk](https://pan.baidu.com/s/1r_41WPs4R1wV2ow1rmgabw?pwd=olxy)), and urban street ([google driver](https://drive.google.com/file/d/1Tmm7_7153ybi1mhzGUe2L8j_r1ho-UJf/view?usp=drive_link), [baidu disk](https://pan.baidu.com/s/1YcNiCH7oWSA4EW_x5epAsQ?pwd=sis5)). 

The illustration of the SfM reconstruction of urban street can be found [here](https://www.dropbox.com/s/pq8zv8hrljmbp6q/SphereSfM%2002.webm?dl=0).

## Usage

There are two ways for data processing, i.e., GUI and command-line modes.
The main steps of the command-line mode are described in the following 5 steps, which are also illustrated by the GUI setting.

### Step 1 - create a database

Create a database in the project directory [step1](https://github.com/json87/SphereSfM/blob/main/doc/step1.jpg).

```sh
colmap database_creator --database_path ./colmap/database.db
```

### Step 2 - feature extraction

Extract features for all images. In this step, camera model, camera parameters, camera mask, and POS data can be provided [step2](https://github.com/json87/SphereSfM/blob/main/doc/step2.jpg).

```sh
colmap feature_extractor 
--database_path ./colmap/database.db 
--image_path ./images 
--ImageReader.camera_model SPHERE 
--ImageReader.camera_params "1,3520,1760" 
--ImageReader.single_camera 1 
--ImageReader.camera_mask_path ./camera_mask.png 
--ImageReader.pose_path ./POS.txt
```

### Step 3 - feature matching

Feature matching for all image pairs. When POS data is provides, spatial matching mode is prefered. Otherwise, the vocabulary tree-based mode can accelerate feature matching [step3](https://github.com/json87/SphereSfM/blob/main/doc/step3.jpg).

```sh
colmap spatial_matcher 
--database_path ./colmap/database.db 
--SiftMatching.max_error 4 
--SiftMatching.min_num_inliers 50 
--SpatialMatching.is_gps 0 
--SpatialMatching.max_distance 50
```

### Step 4 - mapper

Sparse reconstruction based on increamental SfM. In this step, camera parameters are kept fixed [step4](https://github.com/json87/SphereSfM/blob/main/doc/step4.jpg).

```sh
colmap mapper 
--database_path ./colmap/database.db 
--image_path ./images 
--output_path ./colmap/sparse 
--Mapper.ba_refine_focal_length 0 
--Mapper.ba_refine_principal_point 0 
--Mapper.ba_refine_extra_params 0 
--Mapper.sphere_camera 1
```

### Step 5 - show model

Show reconstructed models by using ColMap GUI.

```sh
colmap gui --database_path ./colmap/database.db --image_path ./images --import_path ./colmap/sparse/0
```

### Step 6 - cubic conversion

Convert SfM reconstruction to cubic format for the further dense matching, which can be executed in ColMap.

```sh
colmap sphere_cubic_reprojecer --image_path ./images --input_path ./colmap/sparse/0/ --output_path ./colmap/sparse-cubic
```
![cubic map](https://github.com/json87/SphereSfM/blob/main/doc/cubic%20map.png)

```sh
colmap image_undistorter --image_path ./colmap/sparse-cubic --input_path ./colmap/sparse-cubic/sparse --output_path ./colmap/dense --output_type COLMAP
colmap patch_match_stereo --workspace_path ./colmap/dense --PatchMatchStereo.geom_consistency false
colmap stereo_fusion --workspace_path ./colmap/dense --output_path ./colmap/dense/fused.ply --input_type photometric
```
![dense point](https://github.com/json87/SphereSfM/blob/main/doc/dense%20point.png)

## Reference

Please refer to the following papers for the technique details.

```
@article{
  title = {3D reconstruction of spherical images: a review of techniques, applications, and prospects},
  author = {Jiang, San and You, Kan and Li, Yaxin and Weng, Duojie and Chen, Wu},
  journal = {Geo-spatial Information Science},
  pages = {1--30},
  year = {2024},
  publisher = {Taylor & Francis}
}

@article{
  title = {3D reconstruction of spherical images based on incremental structure from motion},
  author = {San Jiang, Kan You, Wu Chen, Duojie Weng and Yaxin Li},
  journal = {International Journal of Remote Sensing},
  volume = {45},
  number = {8},
  pages = {2596--2621},
  year = {2024},
  publisher = {Taylor & Francis}
}

@article{
  title = {Reliable Feature Matching for Spherical Images via Local Geometric Rectification and Learned Descriptor},
  author = {Jiang, San and Liu, Junhuan and Li, Yaxin and Weng, Duojie and Chen, Wu},
  journal = {Remote Sensing},
  volume = {15},
  number = {20},
  pages = {4954},
  year = {2023},
  publisher = {MDPI}
}
```
