///////////////////////////////////////////////////////////////////////////////
// NAME:            emscripten_bindings.cpp
//
// AUTHOR:          Ethan D. Twardy <edtwardy@mtu.edu>
//
// DESCRIPTION:     Bindings for Emscripten
//
// CREATED:         09/01/2020
//
// LAST EDITED:     09/08/2020
////

#include <emscripten/bind.h>

#include "FLAC++/decoder.h"
#include "StreamDecoder.h"

using namespace emscripten;

// TODO: Create directory libFLACjs
// This directory will link against libFLAC++, and the makefile will produce
// the asm.js and wasm targets using emscripten. The docs should follow the
// same format as the other two libFLAC targets. Ideally, this would be a
// seamless subproject extension of the repository, so that the developers
// would actually have motivation to integrate it.

EMSCRIPTEN_BINDINGS(flac) {
  //
  // Bindings for enumerations
  //
  enum_<FLAC__MetadataType>("MetadataType")
    .value("STREAMINFO", FLAC__METADATA_TYPE_STREAMINFO)
    .value("PADDING", FLAC__METADATA_TYPE_PADDING)
    .value("APPLICATION", FLAC__METADATA_TYPE_APPLICATION)
    .value("SEEKTABLE", FLAC__METADATA_TYPE_SEEKTABLE)
    .value("VORBIS_COMMENT", FLAC__METADATA_TYPE_VORBIS_COMMENT)
    .value("CUESHEET", FLAC__METADATA_TYPE_CUESHEET)
    .value("PICTURE", FLAC__METADATA_TYPE_PICTURE)
    .value("UNDEFINED", FLAC__METADATA_TYPE_UNDEFINED)
    ;

  constant("MAX_METADATA_TYPE", FLAC__MAX_METADATA_TYPE);

  enum_<FLAC__StreamMetadata_Picture_Type>("StreamMetadata_Picture_Type")
    .value("OTHER", FLAC__STREAM_METADATA_PICTURE_TYPE_OTHER)
    .value("FILE_ICON_STANDARD",
      FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON_STANDARD)
    .value("FILE_ICON", FLAC__STREAM_METADATA_PICTURE_TYPE_FILE_ICON)
    .value("FRONT_COVER", FLAC__STREAM_METADATA_PICTURE_TYPE_FRONT_COVER)
    .value("BACK_COVER", FLAC__STREAM_METADATA_PICTURE_TYPE_BACK_COVER)
    .value("LEAFLET_PAGE", FLAC__STREAM_METADATA_PICTURE_TYPE_LEAFLET_PAGE)
    .value("MEDIA", FLAC__STREAM_METADATA_PICTURE_TYPE_MEDIA)
    .value("LEAD_ARTIST", FLAC__STREAM_METADATA_PICTURE_TYPE_LEAD_ARTIST)
    .value("ARTIST", FLAC__STREAM_METADATA_PICTURE_TYPE_ARTIST)
    .value("CONDUCTOR", FLAC__STREAM_METADATA_PICTURE_TYPE_CONDUCTOR)
    .value("BAND", FLAC__STREAM_METADATA_PICTURE_TYPE_BAND)
    .value("COMPOSER", FLAC__STREAM_METADATA_PICTURE_TYPE_COMPOSER)
    .value("LYRICIST", FLAC__STREAM_METADATA_PICTURE_TYPE_LYRICIST)
    .value("RECORDING_LOCATION",
      FLAC__STREAM_METADATA_PICTURE_TYPE_RECORDING_LOCATION)
    .value("DURING_RECORDING",
      FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_RECORDING)
    .value("DURING_PERFORMANCE",
      FLAC__STREAM_METADATA_PICTURE_TYPE_DURING_PERFORMANCE)
    .value("VIDEO_SCREEN_CAPTURE",
      FLAC__STREAM_METADATA_PICTURE_TYPE_VIDEO_SCREEN_CAPTURE)
    .value("FISH", FLAC__STREAM_METADATA_PICTURE_TYPE_FISH)
    .value("ILLUSTRATION", FLAC__STREAM_METADATA_PICTURE_TYPE_ILLUSTRATION)
    .value("BAND_LOGOTYPE", FLAC__STREAM_METADATA_PICTURE_TYPE_BAND_LOGOTYPE)
    .value("PUBLISHER_LOGOTYPE",
      FLAC__STREAM_METADATA_PICTURE_TYPE_PUBLISHER_LOGOTYPE)
    ;

  //
  // StreamDecoder Status enumerations
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
    // TODO: FLAC::Decoder::Stream::set_ogg_serial_number
    .function("set_md5_checking", &FLAC::Decoder::Stream::set_md5_checking)
    .function("set_metadata_respond",
      &FLAC::Decoder::Stream::set_metadata_respond)
    // TODO: FLAC::Decoder::Stream::set_metadata_respond_application
    .function("set_metadata_respond_all",
      &FLAC::Decoder::Stream::set_metadata_respond_all)
    .function("set_metadata_ignore",
      &FLAC::Decoder::Stream::set_metadata_ignore)
    // TODO: FLAC::Decoder::Stream::set_metadata_ignore_application
    .function("set_metadata_ignore_all",
      &FLAC::Decoder::Stream::set_metadata_ignore_all)
    // TODO: FLAC::Decoder::Stream::get_state
    .function("get_md5_checking", &FLAC::Decoder::Stream::get_md5_checking)
    .function("get_total_samples", &FLAC::Decoder::Stream::get_total_samples)
    .function("get_channels", &FLAC::Decoder::Stream::get_channels)
    // TODO: FLAC::Decoder::Stream::get_channel_assignment
    .function("get_bits_per_sample",
      &FLAC::Decoder::Stream::get_bits_per_sample)
    .function("get_sample_rate", &FLAC::Decoder::Stream::get_sample_rate)
    .function("get_blocksize", &FLAC::Decoder::Stream::get_blocksize)
    // TODO: FLAC::Decoder::Stream::get_decode_position
    .function("init", &FLAC::Decoder::Stream::init)
    // TODO: FLAC::Decoder::Stream::init_ogg
    .function("finish", &FLAC::Decoder::Stream::finish)
    .function("flush", &FLAC::Decoder::Stream::flush)
    .function("reset", &FLAC::Decoder::Stream::reset)
    .function("process_single", &FLAC::Decoder::Stream::process_single)
    .function("process_until_end_of_metadata",
      &FLAC::Decoder::Stream::process_until_end_of_metadata)
    .function("process_until_end_of_stream",
      &FLAC::Decoder::Stream::process_until_end_of_stream)
    // TODO: FLAC::Decoder::Stream::skip_single_frame
    // TODO: FLAC::Decoder::Stream::seek_absolute
    .function("is_valid", &FLAC::Decoder::Stream::is_valid)
    ;

  class_<StreamDecoder, base<FLAC::Decoder::Stream>>("StreamDecoder")
    .function("readCallback", &StreamDecoder::readCallback, pure_virtual())
    .function("writeCallback", &StreamDecoder::writeCallback, pure_virtual())
    .function("errorCallback", &StreamDecoder::errorCallback, pure_virtual())
    .function("metadataCallback", &StreamDecoder::metadataCallback,
      pure_virtual())
    .allow_subclass<StreamDecoderImpl>("StreamDecoderImpl")
    ;
}

///////////////////////////////////////////////////////////////////////////////
