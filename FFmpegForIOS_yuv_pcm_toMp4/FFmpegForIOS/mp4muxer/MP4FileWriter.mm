//
//  MP4FileWriter.m
//  FFmpegForIOS
//
//  Created by LeiXiang on 2022/11/28.
//

#import "MP4FileWriter.h"
#import "Muxer.h"
#import "SampleBufferToH264Encoder.h"

@interface MP4FileWriter () {
  Muxer *muxer;
}
@property (nonatomic, copy, readwrite) NSString *filepath;
@property (nonatomic, strong) SampleBufferToH264Encoder *videoEncoder;

@end

@implementation MP4FileWriter

#pragma mark - public methods
- (instancetype)initWithMp4Filepath:(NSString *)filepath {
  self = [super init];
  if (self) {
    _filepath = filepath;
  }
  return self;
}

- (BOOL)writeAudioSampleBuffer:(CMSampleBufferRef)sampleBuffer error:(NSError * __autoreleasing  _Nullable *)error {
  
  
  return YES;
}

- (BOOL)writeVideoSampleBuffer:(CMSampleBufferRef)sampleBuffer error:(NSError * __autoreleasing  _Nullable *)error {
 
  
  return YES;
}

#pragma mark - private methods

/// 硬编码为h264的回调
/// - Parameter h264Data: h264数据
- (void)videoSampleBufferEncodedWithData:(NSData *)h264Data {
  
  
  
}


#pragma mark - setter and getter

- (SampleBufferToH264Encoder *)videoEncoder {
  if (!_videoEncoder) {
    _videoEncoder = [[SampleBufferToH264Encoder alloc] init];
    
  }
  return _videoEncoder;
}

@end
