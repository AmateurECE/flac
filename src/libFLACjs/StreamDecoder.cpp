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

void StreamDecoder::metadata_callback(const ::FLAC__StreamMetadata* metadata)
{
  val tagObject = val::object();
  tagObject.set("type", metadata->type);

  switch (metadata->type) {
  case FLAC__METADATA_TYPE_STREAMINFO:
    break;
  case FLAC__METADATA_TYPE_PADDING:
    break;
  case FLAC__METADATA_TYPE_APPLICATION:
    break;
  case FLAC__METADATA_TYPE_SEEKTABLE:
    break;
  case FLAC__METADATA_TYPE_VORBIS_COMMENT:
    break;
  case FLAC__METADATA_TYPE_CUESHEET:
    break;
  case FLAC__METADATA_TYPE_PICTURE:
    break;
  case FLAC__METADATA_TYPE_UNDEFINED:
    break;
  default:
    break;
  }

  metadataCallback(tagObject);
}

///////////////////////////////////////////////////////////////////////////////
