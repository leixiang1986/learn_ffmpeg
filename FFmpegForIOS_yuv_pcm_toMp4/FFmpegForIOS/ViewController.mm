//
//  ViewController.m
//  FFmpegForIOS
//
//  Created by LeiXiang on 2022/11/23.
//

#import "ViewController.h"

#include "MP4Writer.h"
#import "MyCaptureSession.h"

#define YUV_WIDTH 720
#define YUV_HEIGHT 576
#define YUV_FPS  25

#define VIDEO_BIT_RATE 500*1024

#define PCM_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define PCM_SAMPLE_RATE 44100
#define PCM_CHANNELS 2

#define AUDIO_BIT_RATE 128*1024

#define AUDIO_TIME_BASE 1000000
#define VIDEO_TIME_BASE 1000000


@interface ViewController ()<MycaptureSessionDelegate>

@property (nonatomic, strong) MyCaptureSession *session;
@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.
  AVCaptureVideoPreviewLayer *layer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:self.session.session];
  layer.bounds = [UIScreen.mainScreen bounds];
  layer.backgroundColor = [UIColor cyanColor].CGColor;
  [self.view.layer addSublayer:layer];
  [self.session start];
}



- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
  NSString *yuvPath = [[NSBundle mainBundle] pathForResource:@"output" ofType:@"yuv"];
  NSString *pcmPath = [[NSBundle mainBundle] pathForResource:@"out" ofType:@"pcm"];
  NSDateFormatter *dateFormater = [[NSDateFormatter alloc] init];
  [dateFormater setDateFormat:@"MMdd_HHmmss"];
  NSString *time = [dateFormater stringFromDate:[NSDate date]];
  NSString *outPutPath = [[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) lastObject] stringByAppendingPathComponent:[NSString stringWithFormat:@"%@.mp4",time]];
  NSLog(@"yuv:%@,pcm:%@,output:%@",yuvPath,pcmPath,outPutPath);
  const char *paths[3] = {};
  paths[0] = [yuvPath cStringUsingEncoding:NSUTF8StringEncoding];
  paths[1] = [pcmPath cStringUsingEncoding:NSUTF8StringEncoding];
  paths[2] = [outPutPath cStringUsingEncoding:(NSStringEncoding)NSUTF8StringEncoding];
  const char **parameters = paths;
  test(3, parameters);

}


- (MyCaptureSession *)session {
  if (!_session) {
    _session = [[MyCaptureSession alloc] initCaptureWithSessionPreset:(CaptureSessionPreset1280x720)];
    _session.delegate = self;
  }
  return _session;
}

- (void)audioWithSampleBuffer:(CMSampleBufferRef)sampleBuffer {
//  NSLog(@"音频回调:duration:%f--pts:%f",CMTimeGetSeconds( CMSampleBufferGetDuration(sampleBuffer)),CMTimeGetSeconds(CMSampleBufferGetPresentationTimeStamp(sampleBuffer)));
}

- (void)videoWithSampleBuffer:(CMSampleBufferRef)sampleBuffer {
//  NSLog(@"视频回调:duration:%f--pts:%f",CMTimeGetSeconds(CMSampleBufferGetDuration(sampleBuffer)),CMTimeGetSeconds(CMSampleBufferGetPresentationTimeStamp(sampleBuffer)));
}




@end
