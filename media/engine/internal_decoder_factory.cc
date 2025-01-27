/*
 *  Copyright (c) 2016 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "media/engine/internal_decoder_factory.h"

#include "absl/strings/match.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_codec.h"
#include "media/base/codec.h"
#include "media/base/media_constants.h"
#include "modules/video_coding/codecs/av1/libaom_av1_decoder.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "modules/video_coding/codecs/h265/include/h265.h"
#include "modules/video_coding/codecs/vp8/include/vp8.h"
#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"

namespace webrtc {

std::vector<SdpVideoFormat> InternalDecoderFactory::GetSupportedFormats()
    const {
  std::vector<SdpVideoFormat> formats;
#ifdef WEBRTC_USE_H265
  formats.push_back(SdpVideoFormat(cricket::kH265CodecName));
#endif
  // formats.push_back(SdpVideoFormat(cricket::kVp8CodecName));
  // for (const SdpVideoFormat& format : SupportedVP9DecoderCodecs())
  //  formats.push_back(format);
  // for (const SdpVideoFormat& h264_format : SupportedH264Codecs())
  //  formats.push_back(h264_format);
  // if (kIsLibaomAv1DecoderSupported)
  //   formats.push_back(SdpVideoFormat(cricket::kAv1CodecName));
  return formats;
}

VideoDecoderFactory::CodecSupport InternalDecoderFactory::QueryCodecSupport(
    const SdpVideoFormat& format,
    bool reference_scaling) const {
  // Query for supported formats and check if the specified format is supported.
  // Return unsupported if an invalid combination of format and
  // reference_scaling is specified.
  if (reference_scaling) {
    VideoCodecType codec = PayloadStringToCodecType(format.name);
    if (codec != kVideoCodecVP9 && codec != kVideoCodecAV1) {
      return {/*is_supported=*/false, /*is_power_efficient=*/false};
    }
  }

  CodecSupport codec_support;
  codec_support.is_supported = format.IsCodecInList(GetSupportedFormats());
  return codec_support;
}

std::unique_ptr<VideoDecoder> InternalDecoderFactory::CreateVideoDecoder(
    const SdpVideoFormat& format) {
  if (!format.IsCodecInList(GetSupportedFormats())) {
    RTC_LOG(LS_WARNING) << "Trying to create decoder for unsupported format. "
                        << format.ToString();
    return nullptr;
  }

  if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName))
    return VP8Decoder::Create();
  if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    return VP9Decoder::Create();
  if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
    return H264Decoder::Create();
  if (absl::EqualsIgnoreCase(format.name, cricket::kH265CodecName))
    return H265Decoder::Create();
  if (kIsLibaomAv1DecoderSupported &&
      absl::EqualsIgnoreCase(format.name, cricket::kAv1CodecName))
    return CreateLibaomAv1Decoder();

  RTC_NOTREACHED();
  return nullptr;
}

}  // namespace webrtc
