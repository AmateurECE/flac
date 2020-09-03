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
  // virtual ::FLAC__StreamDecoderReadStatus readCallback(FLAC__byte buffer[],
  //     size_t *bytes) = 0;
  // virtual ::FLAC__StreamDecoderWriteStatus writeCallback(const ::FLAC__Frame
  //     *frame, const FLAC__int32 * const buffer[]) = 0;

  virtual ::FLAC__StreamDecoderReadStatus readCallback() = 0;
  virtual ::FLAC__StreamDecoderWriteStatus writeCallback() = 0;

  virtual void errorCallback(::FLAC__StreamDecoderErrorStatus status) = 0;

protected:
  virtual ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[],
      size_t *bytes) final override {
    return readCallback();
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
  ::FLAC__StreamDecoderReadStatus readCallback() {
    return call<::FLAC__StreamDecoderReadStatus>("readCallback");
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
