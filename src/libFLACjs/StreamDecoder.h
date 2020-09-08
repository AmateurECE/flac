///////////////////////////////////////////////////////////////////////////////
// NAME:            StreamDecoder.h
//
// AUTHOR:          Ethan D. Twardy <edtwardy@mtu.edu>
//
// DESCRIPTION:     Definition for the StreamDecoder class, a wrapper around
//                  FLAC::Decoder::Stream.
//
// CREATED:         09/07/2020
//
// LAST EDITED:     09/08/2020
////

#include <emscripten/bind.h>
#include "FLAC++/decoder.h"

class StreamDecoder : public FLAC::Decoder::Stream {
public:
  StreamDecoder() = default;

  using val = emscripten::val;
  virtual val readCallback(val byteBuffer) = 0;
  // TODO: Somehow pass the FLAC__Frame to the callee?
  virtual ::FLAC__StreamDecoderWriteStatus writeCallback(val lpcmBuffer) = 0;

  virtual void errorCallback(::FLAC__StreamDecoderErrorStatus status) = 0;
  virtual void metadataCallback(val) = 0;

protected:
  virtual ::FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[],
    size_t *bytes) final override;
  virtual ::FLAC__StreamDecoderWriteStatus write_callback(const ::FLAC__Frame
    *frame, const FLAC__int32 * const buffer[]) final override;
  virtual void error_callback(::FLAC__StreamDecoderErrorStatus status) final
    override;
  virtual void metadata_callback(const ::FLAC__StreamMetadata* metadata) final
    override;

private:
  typedef void(*metadataGenerator)(val, const ::FLAC__StreamMetadata*);

  static void streamInfoMetadataGenerator(val, const ::FLAC__StreamMetadata*);
  static void vorbisCommentMetadataGenerator(val,
    const ::FLAC__StreamMetadata*);
  static void seekTableMetadataGenerator(val, const ::FLAC__StreamMetadata*);
  static void pictureMetadataGenerator(val, const ::FLAC__StreamMetadata*);
};

class StreamDecoderImpl : public emscripten::wrapper<StreamDecoder> {
public:
  EMSCRIPTEN_WRAPPER(StreamDecoderImpl);
  using val = emscripten::val;
  val readCallback(val buffer) {
    return call<val>("readCallback", buffer);
  }

  ::FLAC__StreamDecoderWriteStatus writeCallback(val lpcmBuffer) {
    return call<::FLAC__StreamDecoderWriteStatus>("writeCallback",
      lpcmBuffer);
  }

  void errorCallback(::FLAC__StreamDecoderErrorStatus status) {
    return call<void>("error_callback", status);
  }

  void metadataCallback(val data) {
    return call<void>("metadataCallback", data);
  }
};

///////////////////////////////////////////////////////////////////////////////
