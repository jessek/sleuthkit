// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "tsk/tsk_tools_i.h"
#include "mem_img.h"

#ifndef VSTYPE
#error Define VSTYPE as a valid value of TSK_VS_TYPE_ENUM.
#endif

static TSK_WALK_RET_ENUM part_act(TSK_VS_INFO *vs, const TSK_VS_PART_INFO *part,
                                  void *ptr) {
  return TSK_WALK_CONT;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  std::unique_ptr<TSK_IMG_INFO, decltype(&tsk_img_close)> img{
    mem_open(data, size);
    tsk_img_close
  };

  if (!img) {
    return 0;
  }

  std::unique_ptr<TSK_VS_INFO, decltype(&tsk_vs_close)> vs{
    tsk_vs_open(img.get(), 0, VSTYPE);
    tsk_vs_close
  };

  if (vs) {
    tsk_vs_part_walk(vs, 0, vs->part_count - 1, TSK_VS_PART_FLAG_ALL, part_act,
                     nullptr);
  }

  return 0;
}
