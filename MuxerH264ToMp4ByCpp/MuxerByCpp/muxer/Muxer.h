/*
 * @Description: 封装器，可以用于实现音视频录制
 * @Author: xujin
 * @Email: xujin_wuhan@163.com
 * @Date: 2022-11-07 21:29:33
 * @LastEditTime: 2022-11-14 14:37:34
 * @LastEditors: xujin
 * @Reference: 
 */

#ifndef _MEDIA_KIT_H_
#define _MEDIA_KIT_H_

#include <string>
#include "MediaFormat.h"
#include <memory>
#include <map>

using namespace XJMediaUtils;

struct AVFormatContext;
struct AVStream;

namespace XJMediaUtils {

typedef struct VideoMuxerParam {
    VideoEncodeType         codecType;
    int                     width;
    int                     height;
    int                     fps;
    PixelFormatType         pixelFormat;
    int                     sps_pps_size;
    std::unique_ptr<char[]> sps_pps;

    VideoMuxerParam() {
        codecType = VIDEO_ENCODE_TYPE_H264;
        width = 1920;
        height = 1080;
        fps = 15;
        pixelFormat = PIXEL_FORMAT_TYPE_YUV420P;
    }
} VideoMuxerParam;

typedef struct AudioMuxerParam {
    SampleRateType          sampleRate;         // 采样率
    SampleFormatType        sampleFormat;       // 采样格式
    int                     sampleSize;         // 采样精度（采样位数）
    int                     channels;           // 通道数
    AudioEncodeType         codecType;          // 编码类型
} AudioMuxerParam;

class Muxer {
public:
    Muxer();

    virtual ~Muxer();
    /*
     * @breif: 创建一个视频文件（例如mp4格式）
     * @param[in] videoFile 视频文件名, 根据后缀名判断封装格式
     * @return {*}
     */
    int CreateFile(const std::string &videoFile);

    /**
     * @breif: 添加视频轨
     * @param[in] param 视频轨参数
     * @return 成功返回0，失败返回-1
     */
    int AddVideoStream(const VideoMuxerParam &param);

    /**
     * @breif: 添加音频轨
     * @param[in] param 音频轨参数
     * @return 成功返回0，失败返回-1
     */
    int AddAudioStream(const AudioMuxerParam &param);

    /**
     * @breif: 写视频帧
     * @param[in] data 编码后的视频帧数据，例如NALU
     * @param[in] dataSize 视频帧数据大小
     * @return 成功返回0，失败返回-1
     */
    int WriteVideoFrame(const char *data, int dataSize);

    /**
     * @breif: 写音频帧
     * @param[in] data 编码后的音频帧数据，例如NALU
     * @param[in] dataSize 音频帧数据大小
     * @return 成功返回0，失败返回-1
     */
    int WriteAudioFrame(const char *data, int dataSize);

    /**
     * @brief: 获取一帧视频播放时长(转换为毫秒)
     * @return 成功返回帧长，失败返回-1
     */
    int GetDurationMsPerVideoFrame() const;

    /**
     * @brief: 获取一音频播放时长(转换为毫秒)
     * @return 成功返回帧长，失败返回-1
     */
    int GetDurationMsPerAudioFrame() const;

    /**
     * @brief: 获取当前视频流的时间戳(转换为毫秒)
     * @return 成功返回时间戳，失败返回-1
     */
    int GetCurrentVideoStreamDtsMs() const;

    /**
     * @brief: 获取当前音频流的时间戳(转换为毫秒)
     * @return 成功返回时间戳，失败返回-1
     */
    int GetCurrentAudioStreamDtsMs() const;

    /**
     * @brief: 写封装文件头信息
     * @return 成功返回0，失败返回-1
     */
    int WriteHeader();

private:    

private:
    AVFormatContext             *m_outFmtCtx = nullptr;
    AVStream                    *m_videoStream = nullptr;
    AVStream                    *m_audioStream = nullptr;
    int                         m_videoStreamIndex = -1;
    int                         m_audioStreamIndex = -1;
    unsigned int                m_videoFrameIndex = 0;
    unsigned int                m_audioPts = 0;
    unsigned int                m_videoPts = 0;
    unsigned int                m_audioDts = 0;
    unsigned int                m_videoDts = 0;
    int                         m_audioFrameDuration = 0;   // 音频帧每帧播放时长，用时间刻度表示
    int                         m_videoFrameDuration = 0;   // 视频帧每帧播放时长，用时间刻度表示
    char                        m_sps[512] = {0};
    char                        m_pps[512] = {0};
    int                         m_sps_size = 0;
    int                         m_pps_size = 0;
    bool                        m_is_write_header = false;  // 是否写了输出封装文件的头信息
    std::map<int, int>          m_aac_sample_frequency_index_map;
    std::string                 m_mux_format;
};

}

#endif
