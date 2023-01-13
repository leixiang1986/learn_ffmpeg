//
//  MP4Writer.h
//  FFmpegForIOS
//
//  Created by LeiXiang on 2022/11/25.
//

#ifndef MP4Writer_h
#define MP4Writer_h

#include <stdio.h>

#define YUV_WIDTH 720
#define YUV_HEIGHT 576
#define YUV_FPS  25

#define VIDEO_BIT_RATE (500*1024)

#define PCM_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
#define PCM_SAMPLE_RATE 44100
#define PCM_CHANNELS 2

#define AUDIO_BIT_RATE 128*1024

#define AUDIO_TIME_BASE 1000000
#define VIDEO_TIME_BASE 1000000


class MP4Writer {
public:
  MP4Writer();
  ~MP4Writer();
  
  int writerWriteYUV(void *data, int len);
  int writerWriteH264(void *data, int len);
  int writerWritePCM(void *data, int len);
private:
  
};

int test(int argc, const char **argv);



#endif /* MP4Writer_h */
