//
//  MP4FileWriter.h
//  FFmpegForIOS
//
//  Created by LeiXiang on 2022/11/28.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>

NS_ASSUME_NONNULL_BEGIN

@interface MP4FileWriter : NSObject
@property (nonatomic, copy, readonly) NSString *filepath;

- (instancetype)initWithMp4Filepath:(NSString *)filepath;

- (BOOL)writeAudioSampleBuffer:(CMSampleBufferRef)sampleBuffer error:(NSError * __autoreleasing  _Nullable *)error;
- (BOOL)writeVideoSampleBuffer:(CMSampleBufferRef)sampleBuffer error:(NSError * __autoreleasing  _Nullable *)error;

@end

NS_ASSUME_NONNULL_END
