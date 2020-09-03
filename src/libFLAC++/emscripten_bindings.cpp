///////////////////////////////////////////////////////////////////////////////
// NAME:            emscripten_bindings.cpp
//
// AUTHOR:          Ethan D. Twardy <edtwardy@mtu.edu>
//
// DESCRIPTION:     Bindings for Emscripten
//
// CREATED:         09/01/2020
//
// LAST EDITED:     09/03/2020
////

#include <emscripten/bind.h>

#include "FLAC++/decoder.h"

using namespace emscripten;

class StreamDecoder : public FLAC::Decoder::Stream {
public:
  StreamDecoder() = default;
  // virtual ::FLAC__StreamDecoderWriteStatus writeCallback(const ::FLAC__Frame
  //     *frame, const FLAC__int32 * const buffer[]) = 0;

  virtual int readCallback(val byteBuffer) = 0;
  virtual ::FLAC__StreamDecoderWriteStatus writeCallback() = 0;

  virtual void errorCallback(::FLAC__StreamDecoderErrorStatus status) = 0;

protected:
  virtual ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[],
      size_t *bytes) final override {
    const int bytesRead = readCallback(val(typed_memory_view(*bytes, buffer)));
    if (0 == bytesRead) {
      return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    } else if (0 > bytesRead) {
      return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    } else {
      *bytes = (size_t)bytesRead;
      return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
    }
  }

  virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame
      *frame, const FLAC__int32 * const buffer[]) final override {
    return writeCallback();
  }

  virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) final
    override {
    errorCallback(status);
  }
};

class StreamDecoderWrapper : public wrapper<StreamDecoder> {
public:
  EMSCRIPTEN_WRAPPER(StreamDecoderWrapper);
  int readCallback(val buffer) {
    return call<int>("readCallback", buffer);
  }

  ::FLAC__StreamDecoderWriteStatus writeCallback() {
    return call<::FLAC__StreamDecoderWriteStatus>("writeCallback");
  }

  void errorCallback(::FLAC__StreamDecoderErrorStatus status) {
    return call<void>("error_callback", status);
  }
};

EMSCRIPTEN_BINDINGS(flac) {
  class_<StreamDecoder>("StreamDecoder")
    .function("readCallback", &StreamDecoder::readCallback, pure_virtual())
    .function("writeCallback", &StreamDecoder::writeCallback, pure_virtual())
    .function("errorCallback", &StreamDecoder::errorCallback, pure_virtual())
    .allow_subclass<StreamDecoderWrapper>("StreamDecoderWrapper");
}

///////////////////////////////////////////////////////////////////////////////
