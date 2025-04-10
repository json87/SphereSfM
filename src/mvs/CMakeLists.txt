# Copyright (c) 2023, ETH Zurich and UNC Chapel Hill.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#
#     * Neither the name of ETH Zurich and UNC Chapel Hill nor the names of
#       its contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Author: Johannes L. Schoenberger (jsch-at-demuc-dot-de)

set(FOLDER_NAME "mvs")

COLMAP_ADD_SOURCES(
    consistency_graph.h consistency_graph.cc
    depth_map.h depth_map.cc
    fusion.h fusion.cc
    image.h image.cc
    mat.h mat.cc
    meshing.h meshing.cc
    model.h model.cc
    normal_map.h normal_map.cc
    workspace.h workspace.cc
)

COLMAP_ADD_TEST(consistency_graph_test consistency_graph_test.cc)
COLMAP_ADD_TEST(depth_map_test depth_map_test.cc)
COLMAP_ADD_TEST(mat_test mat_test.cc)
COLMAP_ADD_TEST(normal_map_test normal_map_test.cc)

if(CUDA_ENABLED)
    COLMAP_ADD_CUDA_SOURCES(
        gpu_mat_prng.h gpu_mat_prng.cu
        gpu_mat_ref_image.h gpu_mat_ref_image.cu
        patch_match.h patch_match.cc
        patch_match_cuda.h patch_match_cuda.cu
    )

    COLMAP_ADD_CUDA_TEST(gpu_mat_test gpu_mat_test.cu)
endif()
