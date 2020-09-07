///////////////////////////////////////////////////////////////////////////////
// NAME:            emscripten_bindings.cpp
//
// AUTHOR:          Ethan D. Twardy <edtwardy@mtu.edu>
//
// DESCRIPTION:     Bindings for Emscripten
//
// CREATED:         09/01/2020
//
// LAST EDITED:     09/07/2020
////

#include <emscripten/bind.h>
#include <stdint.h>

#include "FLAC++/decoder.h"

using namespace emscripten;

class StreamDecoder : public FLAC::Decoder::Stream {
public:
  StreamDecoder() = default;

  // TODO: Return an Object which has a status code and a bytesRead property?
  virtual val readCallback(val byteBuffer) = 0;
  // TODO: Somehow pass the FLAC__Frame to the callee?
  virtual ::FLAC__StreamDecoderWriteStatus writeCallback(val lpcmBuffer) = 0;

  virtual void errorCallback(::FLAC__StreamDecoderErrorStatus status) = 0;

  // Metadata callbacks
  virtual void streamInfoCallback(::FLAC__StreamMetadata_StreamInfo) {}

protected:
  virtual ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[],
      size_t *bytes) final override {
    val result = readCallback(val(typed_memory_view(*bytes, buffer)));
    *bytes = result["bytesRead"].as<size_t>();
    const auto status = result["status"].as<::FLAC__StreamDecoderReadStatus>();
    return status;
  }

  virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame
      *frame, const FLAC__int32 * const buffer[]) final override {
    val lpcmBuffer = val::array();
    uint32_t blocksize = frame->header.blocksize;
    for (uint32_t i = 0; i < frame->header.channels; i++) {
      lpcmBuffer.call<void>("push", val(typed_memory_view(blocksize,
                  buffer[i])));
    }
    return writeCallback(lpcmBuffer);
  }

  virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) final
    override {
    errorCallback(status);
  }

  virtual void metadata_callback(const ::FLAC__StreamMetadata* metadata) final
    override {
    switch (metadata->type) {
    case FLAC__METADATA_TYPE_STREAMINFO:
      streamInfoCallback(metadata->data.stream_info);
      return;
    default: return;
    }
  }
};

class StreamDecoderImpl : public wrapper<StreamDecoder> {
public:
  EMSCRIPTEN_WRAPPER(StreamDecoderImpl);
  val readCallback(val buffer) {
    return call<val>("readCallback", buffer);
  }

  ::FLAC__StreamDecoderWriteStatus writeCallback(val lpcmBuffer) {
    return call<::FLAC__StreamDecoderWriteStatus>("writeCallback", lpcmBuffer);
  }

  void errorCallback(::FLAC__StreamDecoderErrorStatus status) {
    return call<void>("error_callback", status);
  }

  void streamInfoCallback(::FLAC__StreamMetadata_StreamInfo info) {
    return call<void>("streamInfoCallback", info);
  }
};

// TODO: Create directory libFLACjs
// This directory will link against libFLAC++, and the makefile will produce
// the asm.js and wasm targets using emscripten. The docs should follow the
// same format as the other two libFLAC targets. Ideally, this would be a
// seamless subproject extension of the repository, so that the developers
// would actually have motivation to integrate it.

EMSCRIPTEN_BINDINGS(flac) {
  //
  // Bindings for metadata
  //
  value_object<::FLAC__StreamMetadata_StreamInfo>("StreamInfo")
    .field("min_blocksize", &::FLAC__StreamMetadata_StreamInfo::min_blocksize)
    .field("max_blocksize", &::FLAC__StreamMetadata_StreamInfo::max_blocksize)
    .field("min_framesize", &::FLAC__StreamMetadata_StreamInfo::min_framesize)
    .field("max_framesize", &::FLAC__StreamMetadata_StreamInfo::max_framesize)
    .field("sample_rate", &::FLAC__StreamMetadata_StreamInfo::sample_rate)
    .field("channels", &::FLAC__StreamMetadata_StreamInfo::channels)
    .field("bits_per_sample",
        &::FLAC__StreamMetadata_StreamInfo::bits_per_sample)
    // TODO: field "total_samples"
    // TODO: field "md5sum"
    ;

  //
  // Bindings for enumerations
  //
  enum_<FLAC__StreamDecoderInitStatus>("StreamDecoderInitStatus")
    .value("OK", FLAC__STREAM_DECODER_INIT_STATUS_OK)
    .value("UNSUPPORTED_CONTAINER",
        FLAC__STREAM_DECODER_INIT_STATUS_UNSUPPORTED_CONTAINER)
    .value("INVALID_CALLBACKS",
        FLAC__STREAM_DECODER_INIT_STATUS_INVALID_CALLBACKS)
    .value("MEMORY_ALLOCATION_ERROR",
        FLAC__STREAM_DECODER_INIT_STATUS_MEMORY_ALLOCATION_ERROR)
    .value("ERROR_OPENING_FILE",
        FLAC__STREAM_DECODER_INIT_STATUS_ERROR_OPENING_FILE)
    .value("ALREADY_INITIALIZED",
        FLAC__STREAM_DECODER_INIT_STATUS_ALREADY_INITIALIZED)
    ;

  enum_<FLAC__StreamDecoderReadStatus>("StreamDecoderReadStatus")
    .value("CONTINUE", FLAC__STREAM_DECODER_READ_STATUS_CONTINUE)
    .value("END_OF_STREAM", FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM)
    .value("ABORT", FLAC__STREAM_DECODER_READ_STATUS_ABORT)
    ;

  enum_<FLAC__StreamDecoderWriteStatus>("StreamDecoderWriteStatus")
    .value("CONTINUE", FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE)
    .value("ABORT", FLAC__STREAM_DECODER_WRITE_STATUS_ABORT)
    ;

  enum_<FLAC__StreamDecoderErrorStatus>("StreamDecoderErrorStatus")
    .value("LOST_SYNC", FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
    .value("BAD_HEADER", FLAC__STREAM_DECODER_ERROR_STATUS_BAD_HEADER)
    .value("FRAME_CRC_MISMATCH",
        FLAC__STREAM_DECODER_ERROR_STATUS_FRAME_CRC_MISMATCH)
    .value("UNPARSEABLE_STREAM",
        FLAC__STREAM_DECODER_ERROR_STATUS_UNPARSEABLE_STREAM)
    ;

  //
  // Bindings for Stream Decoder
  //
  class_<FLAC::Decoder::Stream>("Stream")
    .function("process_until_end_of_metadata",
        &FLAC::Decoder::Stream::process_until_end_of_metadata)
    .function("process_single", &FLAC::Decoder::Stream::process_single)
    .function("init", &FLAC::Decoder::Stream::init)
    ;

  const auto streamInfo = [](StreamDecoder& self,
      ::FLAC__StreamMetadata_StreamInfo info) {
                      return self.StreamDecoder::streamInfoCallback(info);
                    };
  class_<StreamDecoder, base<FLAC::Decoder::Stream>>("StreamDecoder")
    .function("readCallback", &StreamDecoder::readCallback, pure_virtual())
    .function("writeCallback", &StreamDecoder::writeCallback, pure_virtual())
    .function("errorCallback", &StreamDecoder::errorCallback, pure_virtual())
    .function("streamInfoCallback", optional_override(streamInfo))
    .allow_subclass<StreamDecoderImpl>("StreamDecoderImpl")
    ;
}

///////////////////////////////////////////////////////////////////////////////
