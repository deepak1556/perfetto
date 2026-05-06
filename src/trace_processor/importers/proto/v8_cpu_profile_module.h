/*
 * Copyright (C) 2026 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef SRC_TRACE_PROCESSOR_IMPORTERS_PROTO_V8_CPU_PROFILE_MODULE_H_
#define SRC_TRACE_PROCESSOR_IMPORTERS_PROTO_V8_CPU_PROFILE_MODULE_H_

#include <cstdint>
#include <optional>
#include <vector>

#include "perfetto/ext/base/flat_hash_map.h"
#include "perfetto/protozero/field.h"
#include "perfetto/trace_processor/ref_counted.h"
#include "perfetto/trace_processor/trace_blob_view.h"
#include "src/trace_processor/importers/proto/proto_importer_module.h"
#include "src/trace_processor/storage/trace_storage.h"
#include "src/trace_processor/tables/v8_tables_py.h"

namespace perfetto::trace_processor {

class TraceProcessorContext;
class PacketSequenceStateGeneration;

// Module that handles V8 CPU profile session packets and exposes helpers to
// parse the V8-specific extension fields that ride along the generic
// Frame / StreamingProfilePacket messages emitted by V8 CPU profiler.
class V8CpuProfileModule : public ProtoImporterModule {
 public:
  V8CpuProfileModule(ProtoImporterModuleContext* module_context,
                     TraceProcessorContext* context);
  ~V8CpuProfileModule() override;

  ModuleResult TokenizePacket(
      const protos::pbzero::TracePacket_Decoder& decoder,
      TraceBlobView* packet,
      int64_t packet_timestamp,
      RefPtr<PacketSequenceStateGeneration> state,
      uint32_t field_id) override;

  void ParseTracePacketData(const protos::pbzero::TracePacket_Decoder& decoder,
                            int64_t ts,
                            const TracePacketData& data,
                            uint32_t field_id) override;

  struct V8SampleExtensions {
    std::vector<int32_t> sample_kind;
    std::vector<uint32_t> leaf_line;
    std::vector<uint32_t> leaf_column;
  };

  static void OnFrameInterned(TraceProcessorContext* context,
                              FrameId frame_id,
                              const uint8_t* frame_bytes,
                              size_t frame_size);
  static V8SampleExtensions ParseStreamingProfileExtensions(
      const uint8_t* packet_bytes,
      size_t packet_size);
  static void OnSampleInserted(TraceProcessorContext* context,
                               tables::CpuProfileStackSampleTable::Id sample_id,
                               const V8SampleExtensions& exts,
                               size_t index);

 private:
  TraceProcessorContext* const context_;
  base::FlatHashMap<int64_t, tables::V8CpuProfileSessionTable::Id>
      open_sessions_;
};

}  // namespace perfetto::trace_processor

#endif  // SRC_TRACE_PROCESSOR_IMPORTERS_PROTO_V8_CPU_PROFILE_MODULE_H_
