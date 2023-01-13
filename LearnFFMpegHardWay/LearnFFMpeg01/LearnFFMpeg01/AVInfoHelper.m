//
//  AVInfoHelper.m
//  LearnFFMpeg01
//
//  Created by LeiXiang on 2022/12/17.
//

#import "AVInfoHelper.h"

@implementation AVInfoHelper
/// 打印formatContext信息
/// - Parameters:
///   - formaterCtx: AVFormatContext
///   - des: 描述信息
+ (void)printInfoWithFormaterCtx:(const AVFormatContext *)formaterCtx des:(NSString *)des {
  if (formaterCtx == NULL) {
    return;
  }
  NSLog(@"%@ AVFormatContext info:format:%s,duration:%lld us,bit_rate:%lld",des,formaterCtx->iformat->long_name,formaterCtx->duration,formaterCtx->bit_rate);
  
}

/// 打印编解码器的参数信息
/// - Parameters:
///   - codecParameters: 编解码参数信息
///   - type: 0 video,1 audio
///   - des: 描述信息
+ (void)printInfoWithCodecParameters:(const AVCodecParameters *)codecParameters
                                
                                 des:(NSString *)des {
  if (codecParameters == NULL) return;
  
  NSMutableString *info = [[NSMutableString alloc] initWithFormat:@"%@ ",des];
  
  if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
    //视频信息
    [info appendFormat:@"视频信息 width=%d,height=%d,aspect->num=%d,aspect->den=%d",codecParameters->width,codecParameters->height,codecParameters->sample_aspect_ratio.num,codecParameters->sample_aspect_ratio.den];
  } else if (codecParameters->codec_type  == AVMEDIA_TYPE_AUDIO) {
    //音频信息
    [info appendFormat:@"音频信息 sample_rate=%d,initial_padding=%d,frame_size=%d,channels=%d,channel_layout=%lld",codecParameters->sample_rate,codecParameters->initial_padding,codecParameters->frame_size,codecParameters->channels,codecParameters->channel_layout];
  }
  
  [info appendFormat:@",bit_rate=%lld,bits_per_raw_sample=%d,bits_per_coded_sample=%d",codecParameters->bit_rate,codecParameters->bits_per_raw_sample,codecParameters->bits_per_coded_sample];
  NSLog(@"%@",info);
  
}

@end
