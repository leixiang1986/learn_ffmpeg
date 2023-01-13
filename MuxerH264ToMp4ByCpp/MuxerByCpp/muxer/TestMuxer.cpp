/*
 * @Description: 将H264 + AAC 封装为mp4
 * @Author: xujin
 * @Email: xujin_wuhan@163.com
 * @Date: 2022-11-10 12:45:46
 * @LastEditTime: 2022-11-14 14:34:03
 * @LastEditors: xujin
 * @Reference: 
 */
#include "Muxer.h"
#include "Log.h"
#include <memory>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include "TestMuxer.h"

#define     VIDEO_FRAME_SIZE        (1920 * 1080 * 3 / 2)
#define     SAMPLE_RATE             48000
#define     SAMPLE_SIZE             1024
#define     CHANNEL_SIZE            2
#define     SAMPLE_BITS             32 // AV_SAMPLE_FMT_FLTP
#define     AUDIO_FRAME_SIZE        (SAMPLE_SIZE * CHANNEL_SIZE * SAMPLE_BITS)

using namespace XJMediaUtils;

static int startcode_size = 0;

static inline int startCode3(char* buf) {
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 1) {
        startcode_size = 3;
        return 1;
    }
    return 0;
}

static inline int startCode4(char* buf) {
    if(buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1) {
        startcode_size = 4;
        return 1;
    }
    
    return 0;
}

static char* findNextStartCode(char* buf, int len) {
    int i;
  DBG("findNextStartCode 1buf:%p",buf);
    if(len < 3) {
        return nullptr;
    }

    for(i = 0; i < len-3; ++i) {
        if(startCode3(buf) || startCode4(buf)) {
          DBG("findNextStartCode 2buf:%p",buf);
            return buf;
        }
        
        ++buf;
    }

    if(startCode3(buf)) {
      DBG("findNextStartCode 3buf:%p",buf);
        return buf;
    }
  DBG("findNextStartCode 4buf:NULL");
    return nullptr;
}

static int getFrameFromH264File(int fd, char* frame, int size) {
    int rSize, frameSize;
    char* nextStartCode;

    if(fd < 0)
        return fd;

    rSize = read(fd, frame, size);
    if(!startCode3(frame) && !startCode4(frame))
        return -1;
    
    nextStartCode = findNextStartCode(frame+3, rSize-3);
    if(!nextStartCode) {
        // lseek(fd, 0, SEEK_SET);
        return -1;
        frameSize = rSize;
    } else {
        frameSize = (nextStartCode-frame);
        lseek(fd, frameSize - rSize, SEEK_CUR);
    }

    return frameSize;
}

int GetSpsPpsData(char *buf, int buf_size, char *sps_pps, int &sps_pps_size) {
    if (!buf || !sps_pps || buf_size < 0) {
        ERR("Failed to GetSpsPpsData, paramter error");
        return -1;
    }
    
    char *data = buf;

    if(!startCode3(buf) && !startCode4(buf)) {
        ERR("Failed, buf is invalid");
        return -1;
    }

    int offset = 0;
    char *nextStartCode = nullptr;
    bool is_get_sps = false;
    bool is_get_pps = false;
    while (nextStartCode = findNextStartCode(data + 3, buf_size - 3)) {
        int frame_size = nextStartCode - data;
        DBG("nalType:%d", data[startcode_size] & 0x1f);
        // sps
        if ((data[startcode_size] & 0x1f) == 7)  {
            DBG("===== sps =====");
            memcpy(sps_pps + offset, data, frame_size);
            offset += frame_size;
            sps_pps_size += frame_size;
            is_get_sps = true;
        }
        // pps
        if ((data[startcode_size] & 0x1f) == 8) {
            DBG("===== pps =====");
            memcpy(sps_pps + offset, data, frame_size);
            offset += frame_size;
            sps_pps_size += frame_size;
            is_get_pps = true;
        }
                
        data = nextStartCode;
        if (!data) {
            break;
        }

        if (is_get_sps && is_get_pps) {
            break;
        }
    }

    return 0;
}

// ./TestMuxer ./test.h264 ./test.aac new.mp4
int test(int argc, const char **argv) {
    if (argc < 4) {
        ERR("Usage:<%s> <input h264 file> <input aac file> <output mp4 file>", argv[0]);
        return -1;
    }

    const char *input_h264_filename = argv[1];
    const char *input_aac_filename = argv[2];
    const char *output_mp4_filename = argv[3];
    
    std::ifstream h264_file(input_aac_filename);
    if (!h264_file.is_open()) {
        ERR("Failed to open %s", input_h264_filename);
        return -1;
    }
    std::ifstream aac_file(input_aac_filename);
    if (!aac_file.is_open()) {
        ERR("Failed to open %s", input_aac_filename);
        return -1;
    }

    bool video_finish = false;
    bool audio_finish = false;    

    int h264_fd = ::open(input_h264_filename, O_RDONLY);
    if (h264_fd < 0) {
        ERR("Failed to open %s", input_h264_filename);
        return -1;
    }
    std::unique_ptr<char[]> buf(new char[VIDEO_FRAME_SIZE]{0}); // 这里按照YUV420P的方式分配了单帧内存，足够存储编码的单帧H264数据
    int ret = ::read(h264_fd, buf.get(), VIDEO_FRAME_SIZE);
    if (ret <= 0) {
        ERR("Failed to read");
        return -1;
    }
    char sps_pps[1024] = {0};
    int sps_pps_size = 0;
    DBG("read:%d", ret);
    GetSpsPpsData(buf.get(), VIDEO_FRAME_SIZE, sps_pps, sps_pps_size);
    lseek(h264_fd, 0, SEEK_SET);
    DBG("sps_pps_size:%d", sps_pps_size);

    std::unique_ptr<char[]> audio_frame_buf(new char[AUDIO_FRAME_SIZE]{0});
    if (!audio_frame_buf) {
        ERR("Failed to get memory");
        return -1;
    }

    Muxer muxer;
    ret = muxer.CreateFile(output_mp4_filename);
    if (ret < 0) {
        ERR("Failed to CreateFile %s", output_mp4_filename);
        return -1;
    }

    VideoMuxerParam video_muxer_param;
    video_muxer_param.codecType = VIDEO_ENCODE_TYPE_H264;
    video_muxer_param.fps = 15;
    video_muxer_param.width = 1920;
    video_muxer_param.height = 1080;
    video_muxer_param.pixelFormat = PIXEL_FORMAT_TYPE_YUV420P;
    std::unique_ptr<char[]> sps_pps_ptr(new char[sps_pps_size]);
    memcpy(sps_pps_ptr.get(), sps_pps, sps_pps_size);
    video_muxer_param.sps_pps = std::move(sps_pps_ptr);
    video_muxer_param.sps_pps_size = sps_pps_size;
    ret = muxer.AddVideoStream(video_muxer_param);
    if (ret < 0) {
        ERR("Failed to AddVideoStream, ret:%d", ret);
        return -1;
    }

    AudioMuxerParam audio_muxer_param;
    audio_muxer_param.channels = 2;
    audio_muxer_param.codecType = AUDIO_ENCODE_TYPE_AAC;
    audio_muxer_param.sampleFormat = SAMPLE_FORMAT_TYPE_FLTP;
    audio_muxer_param.sampleRate = SAMPLE_RATE_48000_HZ;
    audio_muxer_param.sampleSize = 32;
    ret = muxer.AddAudioStream(audio_muxer_param);
    if (ret < 0) {
        ERR("Failed to AddAudioStream, ret:%d", ret);
        return -1;
    }

    if (muxer.WriteHeader() < 0) {
        ERR("Failed to WriteHeader");
        return -1;
    }

    int video_dts = muxer.GetCurrentVideoStreamDtsMs();
    int audio_dts = muxer.GetCurrentAudioStreamDtsMs();

    int video_frame_count = 0;
    while (1) {
        if (audio_finish && video_finish) {
            break;
        }

        DBG("video_dts:%d(ms), audio_dts:%d(ms)", video_dts, audio_dts);

        if (!video_finish && video_dts < audio_dts) {
            DBG("====== 写一帧视频 =======");
            ret = getFrameFromH264File(h264_fd, buf.get(), VIDEO_FRAME_SIZE);
            if (ret > 0) {
                muxer.WriteVideoFrame(buf.get(), ret);
                memset(buf.get(), 0, VIDEO_FRAME_SIZE);
            } else {
                video_finish = true;
                DBG("video_finish");
            }
            video_dts = muxer.GetCurrentVideoStreamDtsMs();
        } else if (!audio_finish) {
            DBG("====== 写一帧音频 =======");
            memset(audio_frame_buf.get(), 0, AUDIO_FRAME_SIZE);
            aac_file.read(audio_frame_buf.get(), sizeof(AdtsHeader));
            if (aac_file.gcount() != sizeof(AdtsHeader)) {
                audio_finish = true;
                DBG("audio_finish");
                continue;
            }
            AdtsHeader header;
            ParseAdtsHeaderFromData(header, audio_frame_buf.get(), aac_file.gcount());
            // DBG("frame_size:%d", header.frame_length);
            aac_file.read(audio_frame_buf.get() + sizeof(AdtsHeader), header.frame_length - sizeof(AdtsHeader));
            muxer.WriteAudioFrame(audio_frame_buf.get(), header.frame_length);
            audio_dts = muxer.GetCurrentAudioStreamDtsMs();
        }

        // 有一方已经结束了
        if (audio_finish && !video_finish) {
            DBG("====== 写一帧视频 =======");
            ret = getFrameFromH264File(h264_fd, buf.get(), VIDEO_FRAME_SIZE);
            if (ret > 0) {
                muxer.WriteVideoFrame(buf.get(), ret);
                memset(buf.get(), 0, VIDEO_FRAME_SIZE);
            } else {
                video_finish = true;
                DBG("video_finish");
            }
            video_dts = muxer.GetCurrentVideoStreamDtsMs();
        }
        if (!audio_finish && video_finish) {
            DBG("====== 写一帧音频 =======");
            memset(audio_frame_buf.get(), 0, AUDIO_FRAME_SIZE);
            aac_file.read(audio_frame_buf.get(), sizeof(AdtsHeader));
            if (aac_file.gcount() != sizeof(AdtsHeader)) {
                audio_finish = true;
                DBG("audio_finish");
                continue;
            }
            AdtsHeader header;
            ParseAdtsHeaderFromData(header, audio_frame_buf.get(), aac_file.gcount());
            // DBG("frame_size:%d", header.frame_length);
            aac_file.read(audio_frame_buf.get() + sizeof(AdtsHeader), header.frame_length - sizeof(AdtsHeader));
            muxer.WriteAudioFrame(audio_frame_buf.get(), header.frame_length);
            audio_dts = muxer.GetCurrentAudioStreamDtsMs();
        }
    }

    return 0;
}
