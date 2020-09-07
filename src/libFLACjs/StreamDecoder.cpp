///////////////////////////////////////////////////////////////////////////////
// NAME:            StreamDecoder.cpp
//
// AUTHOR:          Ethan D. Twardy <edtwardy@mtu.edu>
//
// DESCRIPTION:     Implementation of the StreamDecoder class.
//
// CREATED:         09/07/2020
//
// LAST EDITED:     09/07/2020
////

#include "StreamDecoder.h"

#include <map>

using namespace emscripten;

::FLAC__StreamDecoderReadStatus
StreamDecoder::read_callback(FLAC__byte buffer[], size_t *bytes)
{
  val result = readCallback(val(typed_memory_view(*bytes, buffer)));
  *bytes = result["bytesRead"].as<size_t>();
  const auto status = result["status"].as<::FLAC__StreamDecoderReadStatus>();
  return status;
}

::FLAC__StreamDecoderWriteStatus
StreamDecoder::write_callback(const ::FLAC__Frame* frame,
  const FLAC__int32* const buffer[])
{
  val lpcmBuffer = val::array();
  uint32_t blocksize = frame->header.blocksize;
  for (uint32_t i = 0; i < frame->header.channels; i++) {
    lpcmBuffer.call<void>("push", val(typed_memory_view(blocksize,
          buffer[i])));
  }
  return writeCallback(lpcmBuffer);
}

void StreamDecoder::error_callback(::FLAC__StreamDecoderErrorStatus status)
{
  errorCallback(status);
}

void StreamDecoder::streamInfoMetadataGenerator(val object,
  const ::FLAC__StreamMetadata* metadata)
{
  val streamInfo = val::object();
  const auto* info = &(metadata->data.stream_info);
  streamInfo.set("min_blocksize", info->min_blocksize);
  streamInfo.set("max_blocksize", info->max_blocksize);
  streamInfo.set("min_framesize", info->min_framesize);
  streamInfo.set("max_framesize", info->max_framesize);
  streamInfo.set("sample_rate", info->sample_rate);
  streamInfo.set("channels", info->channels);
  streamInfo.set("bits_per_sample", info->bits_per_sample);
  streamInfo.set("total_samples",
    val(typed_memory_view(sizeof(FLAC__uint64) / sizeof(uint32_t),
        (uint32_t*)&(info->total_samples))));
  static_assert(sizeof(FLAC__byte) == sizeof(uint8_t),
    "FLAC__byte is not the same length as uint8_t!");
  static const constexpr size_t md5sumLength = 16;
  streamInfo.set("md5sum",
    val(typed_memory_view(md5sumLength, (uint8_t*)&(info->md5sum))));

  object.set("data", streamInfo);
}

void StreamDecoder::metadata_callback(const ::FLAC__StreamMetadata* metadata)
{
  val metadataObject = val::object();
  metadataObject.set("type", metadata->type);
  metadataObject.set("is_last", (bool)metadata->is_last);

  static const std::map<::FLAC__MetadataType,
    metadataGenerator> generators =
    {
      {FLAC__METADATA_TYPE_STREAMINFO, &streamInfoMetadataGenerator},
      // TODO: FLAC__METADATA_TYPE_PADDING
      // TODO: FLAC__METADATA_TYPE_APPLICATION
      // TODO: FLAC__METADATA_TYPE_SEEKTABLE
      // TODO: FLAC__METADATA_TYPE_VORBIS_COMMENT
      // TODO: FLAC__METADATA_TYPE_CUESHEET
      // TODO: FLAC__METADATA_TYPE_PICTURE
    };

  const auto result = generators.find(metadata->type);
  if (result != generators.end()) {
    result->second(metadataObject, metadata);
  }
  metadataCallback(metadataObject);
}

///////////////////////////////////////////////////////////////////////////////
