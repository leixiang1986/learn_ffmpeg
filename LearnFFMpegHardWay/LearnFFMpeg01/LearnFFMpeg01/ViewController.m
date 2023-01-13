//
//  ViewController.m
//  LearnFFMpeg01
//
//  Created by LeiXiang on 2022/12/17.
//

#import "ViewController.h"
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include "AVInfoHelper.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  // Do any additional setup after loading the view.
  int ret = 0;
  NSLog(@"ffmpeg version:%s",av_version_info());
  NSString *videoPath = [[NSBundle mainBundle] pathForResource:@"origin" ofType:@"mp4"];
  const char *url = [videoPath cStringUsingEncoding:NSUTF8StringEncoding];
  AVFormatContext *pFormatCtx = avformat_alloc_context();
  ret = avformat_open_input(&pFormatCtx, url, NULL, NULL);
  if (ret < 0) {
    NSLog(@"打开文件失败:%d",ret);
    return;
  }
  NSLog(@"视频origin的format:%s,duration:%lld",pFormatCtx->iformat->long_name,pFormatCtx->duration);
  [AVInfoHelper printInfoWithFormaterCtx:pFormatCtx des:@"origin视频信息"];
  
  ret = avformat_find_stream_info(pFormatCtx, NULL);
  if (ret < 0) {
    NSLog(@"avformat_find_stream_info 失败:%d",ret);
    return;
  }
  
  for (int i = 0; i < pFormatCtx->nb_streams; i++)
  {
    AVCodecParameters *codecParameters = pFormatCtx->streams[i]->codecpar;
    [AVInfoHelper printInfoWithCodecParameters:codecParameters des:@"编解码参数信息AVCodecParameters-"];
//    if (codecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
//      AVCodec *pAudioCodec = avcodec_find_decoder(codecParameters->codec_id);
//      AVCodecContext *pAudioCodecCtx = avcodec_alloc_context3(pAudioCodec);
//    } else if (codecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
//      AVCodec *pVideoCodec = avcodec_find_decoder(codecParameters->codec_id);
//      AVCodecContext *pVideoCodecCtx = avcodec_alloc_context3(pVideoCodec);
//
//    }
    
    AVCodec *pCodec = avcodec_find_decoder(codecParameters->codec_id);
    AVCodecContext *pCodecCtx = avcodec_alloc_context3(pCodec);
    avcodec_parameters_to_context(pCodecCtx, codecParameters);
    avcodec_open2(pCodecCtx, pCodec, NULL);
    
    AVPacket *pPacket = av_packet_alloc();
    AVFrame *pFrame = av_frame_alloc();
    
    while (av_read_frame(pFormatCtx, pPacket) >= 0) {
      ret = avcodec_send_packet(pCodecCtx, pPacket);
      if (ret < 0) {
        
        NSLog(@"avcodec_send_packet 错误:%d--%s",ret,av_err2str(ret));
        return;
      } else {
        NSLog(@"====正常处理");
      }
      
      
      
      
      
    }
    
    
  }
  
  
  
}


//- (void)logging(const char *fmt, ...)
//{
//    va_list args;
//    fprintf( stderr, "LOG: " );
//    va_start( args, fmt );
//    vfprintf( stderr, fmt, args );
//    va_end( args );
//    fprintf( stderr, "\n" );
//}

- (int)decode_packet:(AVPacket *)pPacket codecContext:(AVCodecContext *)pCodecContext frame:( AVFrame *)pFrame
{
  // Supply raw packet data as input to a decoder
  // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
  int response = avcodec_send_packet(pCodecContext, pPacket);

  if (response < 0) {
    logging("Error while sending a packet to the decoder: %s", av_err2str(response));
    return response;
  }

  while (response >= 0)
  {
    // Return decoded output data (into a frame) from a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
    response = avcodec_receive_frame(pCodecContext, pFrame);
    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
      break;
    } else if (response < 0) {
      logging("Error while receiving a frame from the decoder: %s", av_err2str(response));
      return response;
    }

    if (response >= 0) {
      logging(
          "Frame %d (type=%c, size=%d bytes, format=%d) pts %d key_frame %d [DTS %d]",
          pCodecContext->frame_number,
          av_get_picture_type_char(pFrame->pict_type),
          pFrame->pkt_size,
          pFrame->format,
          pFrame->pts,
          pFrame->key_frame,
          pFrame->coded_picture_number
      );

      char frame_filename[1024];
      snprintf(frame_filename, sizeof(frame_filename), "%s-%d.pgm", "frame", pCodecContext->frame_number);
      // Check if the frame is a planar YUV 4:2:0, 12bpp
      // That is the format of the provided .mp4 file
      // RGB formats will definitely not give a gray image
      // Other YUV image may do so, but untested, so give a warning
      if (pFrame->format != AV_PIX_FMT_YUV420P)
      {
        logging("Warning: the generated file may not be a grayscale image, but could e.g. be just the R component if the video format is RGB");
      }
      // save a grayscale frame into a .pgm file
      [self save_gray_frame:pFrame->data[0] wrap:pFrame->linesize[0] xsize:pFrame->width ysize:pFrame->height filename:frame_filename];
//      save_gray_frame(pFrame->data[0], pFrame->linesize[0], pFrame->width, pFrame->height, frame_filename);
    }
  }
  return 0;
}

- (void)save_gray_frame:(unsigned char *)buf wrap:(int) wrap xsize:(int)xsize ysize:(int)ysize filename:(char *)filename
{
    FILE *f;
    int i;
    f = fopen(filename,"w");
    // writing the minimal required header for a pgm file format
    // portable graymap format -> https://en.wikipedia.org/wiki/Netpbm_format#PGM_example
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

void logging(const char *fmt, ...)
{
  va_list args;
  fprintf( stderr, "LOG: " );
  va_start( args, fmt );
  vfprintf( stderr, fmt, args );
  va_end( args );
  fprintf( stderr, "\n" );
}





@end
