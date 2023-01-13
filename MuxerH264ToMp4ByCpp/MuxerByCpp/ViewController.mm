//
//  ViewController.m
//  MuxerByCpp
//
//  Created by LeiXiang on 2022/11/30.
//

#import "ViewController.h"
#import "TestMuxer.h"
#import "LXMuxer.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  
  self.view.backgroundColor = [UIColor cyanColor];
  NSString *pathAAC = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"aac"];
  NSString *pathH264 = [[NSBundle mainBundle] pathForResource:@"test" ofType:@"h264"];
  NSString *pathMp4 = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).lastObject stringByAppendingPathComponent:@"out.mp4"];
  [[NSFileManager defaultManager] removeItemAtPath:pathMp4 error:nil];
  const char *paths[4] = {"",[pathH264 cStringUsingEncoding:NSUTF8StringEncoding],[pathAAC cStringUsingEncoding:NSUTF8StringEncoding],[pathMp4 cStringUsingEncoding:NSUTF8StringEncoding] };
//  test(4, paths);
  tmain(4,paths);
  
}


@end
