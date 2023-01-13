//
//  Log.h
//  MuxerByCpp
//
//  Created by LeiXiang on 2022/11/30.
//

#ifndef Log_h
#define Log_h

#include <stdio.h>
extern "C" {
#include <libavutil/error.h>
}


#define ERR(format, ... ) \
do{ \
printf("Error:" format "\n",##__VA_ARGS__);\
}while(0)

#define DBG(format, ...) \
do{ \
printf("Debug:" format "\n",##__VA_ARGS__);\
}while(0)

#define FFMPEG_ERROR(e) av_err2str(AVERROR(e))

#endif /* Log_h */
