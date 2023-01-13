/*
 * @Description: 封装器，可以用于实现音视频录制
 * @Author: xujin
 * @Email: xujin_wuhan@163.com
 * @Date: 2022-11-07 21:29:33
 * @LastEditTime: 2022-11-14 16:46:47
 * @LastEditors: xujin
 * @Reference: 
 */
#include "Muxer.h"
#include <stdexcept>
#include <algorithm>
#include "Log.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
}

using namespace XJMediaUtils;

namespace  XJMediaUtils {

// TODO 支持一段视频录制中，可以暂停的方式录制多段
Muxer::Muxer() {
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(96000, 0));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(88200, 0x1));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(64000, 0x2));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(48000, 0x3));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(44100, 0x4));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(32000, 0x5));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(24000, 0x6));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(22050, 0x7));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(16000, 0x8));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(12000, 0x9));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(11025, 0xa));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(8000, 0xb));
    m_aac_sample_frequency_index_map.insert(std::pair<int,int>(7350, 0xc));
}

Muxer::~Muxer() {
    if (m_outFmtCtx) {
        // if (m_is_write_header) {
            av_write_trailer(m_outFmtCtx);
        // }
        avio_close(m_outFmtCtx->pb);
        avformat_free_context(m_outFmtCtx);
        m_outFmtCtx = nullptr;
    }
}

int Muxer::CreateFile(const std::string &videoFile) {
    size_t pos = videoFile.find_last_of('.');
    if (pos == videoFile.npos) {
        // throw std::runtime_error("Failed, videoFilename is invalid"); // 找不到就找不到，打印error就可以了，屏蔽使用异常的处理方式
        ERR("Failed, videoFilename is invalid");
        return -1;
    }

    m_mux_format = videoFile.substr(pos + 1);
    std::transform(m_mux_format.begin(), m_mux_format.end(), m_mux_format.begin(), ::tolower);
    DBG("封装格式为:%s", m_mux_format.c_str());

    int ret = avformat_alloc_output_context2(&m_outFmtCtx, nullptr, m_mux_format.c_str(), videoFile.c_str());
    if (ret < 0 || !m_outFmtCtx) {
        ERR("Failed to avformat_alloc_output_context2"); // TODO 将FFmpeg的错误日志打印出来
        return -1;
    }

    if (!(m_outFmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&(m_outFmtCtx->pb), videoFile.c_str(), AVIO_FLAG_READ_WRITE);
        if (ret < 0) {
            ERR("Failed to avio_open");
            return -1;
        }
    }

    return 0;
}

int Muxer::AddVideoStream(const VideoMuxerParam &param) {
    if (!m_outFmtCtx) {
        ERR("Failed, m_outFmtCtx is nullptr");
        return -1;
    }

    m_videoStream = avformat_new_stream(m_outFmtCtx, nullptr);
    if (!m_videoStream) {
        ERR("Failed to avformat_new_stream");
        return -1;
    }
    m_videoStreamIndex = m_videoStream->index;
    m_videoStream->r_frame_rate = AVRational{param.fps, 1};

    if (m_mux_format == "mp4") {
        // 发现mp4的情况下，time_base.den 必须大于等于10000 （最大可以达到int的最大值 2147483647）
        // 手动设置为1/9000，被FFmpeg内部强制改为了1/18000
        // 设置为1/1000的时候被FFmpeg强行指定为了1/16000
        m_videoStream->time_base = AVRational{1, 90000};  // 设置为常见的时间基就可以了
    } else if (m_mux_format == "flv") {
        m_videoStream->time_base = AVRational{1, 1000};
    }

    m_videoStream->codecpar->width = param.width;
    m_videoStream->codecpar->height = param.height;
    // 例如25fps，timebase为90000，则pts每次递增 1000 / 25 / (1 / 90000) = 3600
    m_videoFrameDuration = 1.0 / (1.0 * m_videoStream->r_frame_rate.num / m_videoStream->r_frame_rate.den) 
                            * m_videoStream->time_base.den;

    AVCodecParameters *codecParam = m_videoStream->codecpar;
    codecParam->codec_type = AVMEDIA_TYPE_VIDEO;
    switch (param.codecType) {
        case VIDEO_ENCODE_TYPE_H264: {
            codecParam->codec_id = AV_CODEC_ID_H264;
            break;
        }
        case VIDEO_ENCODE_TYPE_H265: {
            codecParam->codec_id = AV_CODEC_ID_H265;
            break;
        }
        default: {
            ERR("Failed to set codec id");
            break;
        }
    }
    switch (param.pixelFormat) {
        case PIXEL_FORMAT_TYPE_YUV420P: {
            codecParam->format = AV_PIX_FMT_YUV420P;
            break;
        }
        case PIXEL_FORMAT_TYPE_YUVJ420P: {
            codecParam->format = AV_PIX_FMT_YUVJ420P;
            break;
        }
        default: {
            ERR("Failed to set pixel format");
            break;
        }
    }

    if (m_outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER) {
        m_outFmtCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER; // lace global headers in extradata instead of every keyframe
    }
    codecParam->extradata = (uint8_t *)av_malloc(param.sps_pps_size + AV_INPUT_BUFFER_PADDING_SIZE);
    memcpy(m_videoStream->codecpar->extradata, param.sps_pps.get(), param.sps_pps_size);
    m_videoStream->codecpar->extradata_size = param.sps_pps_size;
    m_sps_size = param.sps_pps_size; // test
    m_pps_size = param.sps_pps_size; // test

    return 0;
}

int Muxer::AddAudioStream(const AudioMuxerParam &param) {
    if (!m_outFmtCtx) {
        ERR("Failed, m_outFmtCts is nullptr");
        return -1;
    }

    m_audioStream = avformat_new_stream(m_outFmtCtx, nullptr);
    if (!m_audioStream) {
        ERR("Failed to avformat_new_stream");
        return -1;
    }

    

    m_audioStreamIndex = m_audioStream->index;
    AVCodecParameters *codecParam = m_audioStream->codecpar;
    codecParam->codec_type = AVMEDIA_TYPE_AUDIO;
    switch (param.sampleRate) {
        case SAMPLE_RATE_44100_HZ: {
            codecParam->sample_rate = 44100;
            break;
        }
        case SAMPLE_RATE_48000_HZ: {
            codecParam->sample_rate = 48000;
            break;
        }
        default: {
            ERR("Failed to support sample rate");
            break;
        }
    }
    switch (param.sampleFormat) {
        case SAMPLE_FORMAT_TYPE_FLTP: {
            codecParam->format = AV_SAMPLE_FMT_FLTP;
            break;
        }
        default: {
            ERR("Failed to support sample format");
            break;
        }
    }

    if (m_mux_format == "mp4") {
        // TODO 调试过程中发现，当封装格式为MP4时，音频的时间基得为采样率，例如(1/48000)
        m_audioStream->time_base = AVRational{1, codecParam->sample_rate};
    } else if (m_mux_format == "flv") {
        m_audioStream->time_base = AVRational{1, 1000};
    }

    codecParam->channels = param.channels;
    codecParam->channel_layout = av_get_default_channel_layout(codecParam->channels);
    codecParam->bits_per_raw_sample = param.sampleSize;
    switch (param.codecType) {
        case AUDIO_ENCODE_TYPE_MP3: {
            codecParam->codec_id = AV_CODEC_ID_MP3;
            break;
        }
        case AUDIO_ENCODE_TYPE_AAC: {
            codecParam->codec_id = AV_CODEC_ID_AAC;
            // 将时间基从bq转换为cq
            // `a * bq / cq`
            // av_rescale_q()
            // AAC规定一帧有1024个采样点，如果的采样率是44100，那么一帧播放1024 / 44100 * 1000 = 23ms
            // m_audioFrameDuration = 1024;
            m_audioFrameDuration = 1024 * (1.0 / codecParam->sample_rate) / (1.0 * m_audioStream->time_base.num / m_audioStream->time_base.den);
            codecParam->frame_size = 1024 * codecParam->bits_per_raw_sample * codecParam->channels;
            unsigned int sample_frequency_index = m_aac_sample_frequency_index_map.find(48000)->second;
            unsigned int object_type = 2; // AAC LC by default
            unsigned int channel = codecParam->channels;
            // 根据AdtsHeader获取ASC(MPEG-4 Audio Specific Configuration)
            // TODO 将获取 ASC 数据封装起来
            unsigned char asc[2] = {0};
            asc[0] = (object_type << 3) | (sample_frequency_index >> 1);
            asc[1] = ((sample_frequency_index & 0x1) << 7) | (channel << 3);
            codecParam->extradata = (uint8_t *)av_malloc(sizeof(asc) + AV_INPUT_BUFFER_PADDING_SIZE);
            memcpy(codecParam->extradata, asc, sizeof(asc));
            codecParam->extradata_size = sizeof(asc);
            break;
        }
        default: {
            ERR("Failed to support encode type");
            break;
        }
    }

    

    

    return 0;
}

int Muxer::WriteVideoFrame(const char *data, int dataSize) {
    if (!data || dataSize < 0 || !m_outFmtCtx) {
        ERR("Failed, paramter error");
        return -1;
    }

    // 判断startcode是否为 00 00 01
    int startCodeSize = 0;
    auto is_startcode3 = [&startCodeSize](char *buf)->int {
        if (buf[0] == 0 && buf[1] == 0 && buf[2] == 1) {
            startCodeSize = 3;
            return 1;
        }
        return 0;
    };
    // 判断startcode是否为 00 00 00 01
    auto is_startcode4 = [&startCodeSize](char *buf)->int {
        if (buf[0] == 0 && buf[1] == 0 && buf[2] == 0 && buf[3] == 1) {
            startCodeSize = 4;
            return 1;
        }
        return 0;
    };

    is_startcode3(const_cast<char*>(data));
    is_startcode4(const_cast<char*>(data));

    // 保证第一帧为IDR帧，避免起播出现漫长的黑名等待
    int nalType = data[startCodeSize] & 0x1f;
    int isKeyFrame = 0;
    // DBG("nalType:%d, starCodeSize:%d", nalType, startCodeSize);
    if (nalType == 6) { // SEI 不写入到MP4中
        return 0;
    }
    if (nalType == 5) {
        isKeyFrame = 1;
    }
    // SPS
    if (nalType == 7) {
        if (m_sps_size <= 0) {
            memcpy(m_sps, data, dataSize);
            m_sps_size = dataSize;
            DBG("sps_size:%d", m_sps_size);
        }
        return 0;
    }
    // PPS
    if (nalType == 8) {
        if (m_pps_size <= 0) {
            memcpy(m_pps, data, dataSize);
            m_pps_size = dataSize;
            DBG("sps_size:%d", m_pps_size);
        }
        return 0;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.stream_index = m_videoStreamIndex;
    pkt.data = (uint8_t*)data;
    pkt.size = dataSize;
    pkt.duration = m_videoFrameDuration;
    pkt.pts = m_videoPts;
    pkt.dts = m_videoPts; // 没有B的情况下可以这么干
    pkt.pos = -1;
    pkt.flags = isKeyFrame;
    m_videoPts += m_videoFrameDuration;
    m_videoDts += m_videoFrameDuration;
    int ret = av_write_frame(m_outFmtCtx, &pkt);
    if (ret < 0) {
        ERR("Failed to av_write_frame, ret:%d", ret);
        FFMPEG_ERROR(ret);
        return -1;
    }

    return 0;
}

int Muxer::WriteAudioFrame(const char *data, int dataSize) {
    if (!data || dataSize < 0 || !m_outFmtCtx) {
        ERR("Failed, paramter error");
        return -1;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.stream_index = m_audioStreamIndex;
    pkt.data = (uint8_t*)data + sizeof(AdtsHeader);
    pkt.size = dataSize - sizeof(AdtsHeader);
    pkt.duration = m_audioFrameDuration;
    // DBG("audio_frame_duration:%d", pkt.duration);
    pkt.pts = m_audioPts;
    pkt.dts = m_audioPts;
    m_audioPts += m_audioFrameDuration;
    m_audioDts += m_audioFrameDuration;
    int ret = av_write_frame(m_outFmtCtx, &pkt);
    if (ret < 0) {
        ERR("Failed to av_write_frame, ret:%d", ret);
        return -1;
    }

    return 0;
}

int Muxer::GetDurationMsPerVideoFrame() const {
    return m_videoFrameDuration * av_q2d(m_videoStream->time_base);
}

int Muxer::GetDurationMsPerAudioFrame() const {
    return m_audioFrameDuration * av_q2d(m_audioStream->time_base);
}

int Muxer::GetCurrentVideoStreamDtsMs() const {
    return m_videoDts * av_q2d(m_videoStream->time_base) * 1000;
}

int Muxer::GetCurrentAudioStreamDtsMs() const {
    return m_audioDts * av_q2d(m_audioStream->time_base) * 1000;
}

int Muxer::WriteHeader() {
    // 录制成 fmp4 [windows的mediaplayer不支持fmp4的进度条显示，老版本的mediainfo工具也没办法分析出duration来], 普通mp4则均正常
    AVDictionary *opt = nullptr;
    av_dict_set(&opt, "movflags", "frag_keyframe+empty_moov", 0);
    if (avformat_write_header(m_outFmtCtx, &opt) < 0) {
        ERR("Failed to avformat_write_header");
        return -1;
    }
    return 0;
}

} // namespace  XJMediaUtils
