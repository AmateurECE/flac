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

void StreamDecoder::vorbisCommentMetadataGenerator(val object,
  const ::FLAC__StreamMetadata* metadata)
{
  val comments = val::object();
  const auto* info = &(metadata->data.vorbis_comment);
  comments.set("vendor_string",
    val(std::string((const char*)info->vendor_string.entry)));
  val tags = val::object();
  for (FLAC__uint32 i = 0; i < info->num_comments; i++) {
    std::string comment{(const char*)info->comments[i].entry};
    auto delimiter = comment.find("=");
    tags.set(val(comment.substr(0, delimiter)),
      val(comment.substr(delimiter + 1)));
  }

  comments.set("comments", tags);
  object.set("data", comments);
}

void StreamDecoder::seekTableMetadataGenerator(val object,
  const ::FLAC__StreamMetadata* metadata)
{
  val seekTableContainer = val::object();
  val seekTable = val::array();
  const auto* info = &(metadata->data.seek_table);
  for (unsigned i = 0; i < info->num_points; i++) {
    val seekPoint = val::object();
    seekPoint.set("sample_number",
      val(typed_memory_view(sizeof(FLAC__uint64) / sizeof(uint32_t),
        (uint32_t*)&(info->points[i].sample_number))));
    seekPoint.set("stream_offset",
      val(typed_memory_view(sizeof(FLAC__uint64) / sizeof(uint32_t),
        (uint32_t*)&(info->points[i].stream_offset))));
    seekPoint.set("frame_samples", info->points[i].frame_samples);
    seekTable.call<void>("push", seekPoint);
  }

  seekTableContainer.set("points", seekTable);
  object.set("data", seekTableContainer);
}

void StreamDecoder::pictureMetadataGenerator(val object,
  const ::FLAC__StreamMetadata* metadata)
{
  val picture = val::object();
  const auto* info = &(metadata->data.picture);
  picture.set("type", info->type);
  picture.set("mime_type", val(std::string((const char*)info->mime_type)));
  picture.set("description", val(std::string((const char*)info->description)));
  static_assert(sizeof(uint32_t) == sizeof(FLAC__uint32),
    "FLAC__uint32 and uint32_t are not the same length!");
  picture.set("width", val((uint32_t)info->width));
  picture.set("height", val((uint32_t)info->height));
  picture.set("depth", val((uint32_t)info->depth));
  picture.set("colors", val((uint32_t)info->colors));
  picture.set("data", val(typed_memory_view(info->data_length,
        (uint8_t*)info->data)));

  object.set("data", picture);
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
      {FLAC__METADATA_TYPE_SEEKTABLE, &seekTableMetadataGenerator},
      {FLAC__METADATA_TYPE_VORBIS_COMMENT, &vorbisCommentMetadataGenerator},
      // TODO: FLAC__METADATA_TYPE_CUESHEET
      {FLAC__METADATA_TYPE_PICTURE, &pictureMetadataGenerator},
    };

  const auto result = generators.find(metadata->type);
  if (result != generators.end()) {
    result->second(metadataObject, metadata);
  }
  metadataCallback(metadataObject);
}

///////////////////////////////////////////////////////////////////////////////
