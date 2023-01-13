//
//  AVInfoHelper.h
//  LearnFFMpeg01
//
//  Created by LeiXiang on 2022/12/17.
//

#import <Foundation/Foundation.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>

NS_ASSUME_NONNULL_BEGIN




@interface AVInfoHelper : NSObject

/// 打印formatContext信息
/// - Parameters:
///   - formaterCtx: AVFormatContext
///   - des: 描述信息
+ (void)printInfoWithFormaterCtx:(const AVFormatContext *)formaterCtx des:(NSString *)des;


/// 打印编解码器的参数信息
/// - Parameters:
///   - codecParameters: 编解码参数信息
///   - des: 描述信息
+ (void)printInfoWithCodecParameters:(const AVCodecParameters *)codecParameters des:(NSString *)des;

@end

NS_ASSUME_NONNULL_END
