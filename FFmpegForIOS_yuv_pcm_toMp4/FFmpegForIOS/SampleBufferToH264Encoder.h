//
//  VideoEncoder.h
//  VideoToolBox
//
//  Created by xiaomage on 2016/11/3.
//  Copyright © 2016年 seemygo. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <VideoToolbox/VideoToolbox.h>

typedef struct LXData {
  void *data;
  int size;
}LXData_t;

@interface SampleBufferToH264Encoder : NSObject
@property (nonatomic, copy)void(^callback)(NSData *data);

- (void)encodeSampleBuffer:(CMSampleBufferRef)sampleBuffer;
- (void)endEncode;

@end
