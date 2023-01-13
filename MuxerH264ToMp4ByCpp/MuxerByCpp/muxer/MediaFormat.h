/*
 * @Description: 媒体格式
 * @Author: xujin
 * @Email: xujin_wuhan@163.com
 * @Date: 2022-11-07 21:40:37
 * @LastEditTime: 2022-11-25 10:47:42
 * @LastEditors: xujin
 * @Reference: 
 */
#ifndef _MEDIA_FORMAT_H_
#define _MEDIA_FORMAT_H_

namespace XJMediaUtils {

//视频像素格式
typedef enum {
    PIXEL_FORMAT_TYPE_YUV420P,
    PIXEL_FORMAT_TYPE_YUVJ420P
} PixelFormatType;

// 视频编码格式
typedef enum {
    VIDEO_ENCODE_TYPE_H264,
    VIDEO_ENCODE_TYPE_H265,
} VideoEncodeType;

// 音频采样率
typedef enum {
    SAMPLE_RATE_44100_HZ,
    SAMPLE_RATE_48000_HZ,
} SampleRateType;

// 音频采样格式
typedef enum {
    SAMPLE_FORMAT_TYPE_FLTP,        // 32bit
} SampleFormatType;

// 音频采样精度（位深）


// 音频编码格式
typedef enum {
    AUDIO_ENCODE_TYPE_AAC,
    AUDIO_ENCODE_TYPE_MP3,
} AudioEncodeType;

#pragma pack(push, 1)
typedef struct AdtsHeader {
	unsigned int syncword:12;//同步字0xfff，说明一个ADTS帧的开始
	unsigned char ID:1;//ID比较奇怪,标准文档中是这么说的”MPEG identifier, set to ‘1’. See ISO/IEC 11172-3″,但我写0了,也可以
	unsigned char layer:2;//一般设置为0
	unsigned char protection_absent:1;//是否误码校验

	unsigned char profile:2;//表示使用哪个级别的AAC，如01 Low Complexity(LC)--- AACLC
	unsigned char sampling_frequency_index:4;//表示使用的采样率下标0x3 48k ,0x4 44.1k, 0x5 32k
	unsigned char private_bit:1;//一般设置为0
	unsigned char channel_configuration:3;// 表示声道数

	unsigned char original_copy:1;//一般设置为0
	unsigned char home:1;//一般设置为0
    
	unsigned char copyright_identification_bit:1;//一般设置为0
	unsigned char copyright_identification_start:1;//一般设置为0
	unsigned int frame_length:13;// 一个ADTS帧的长度包括ADTS头和raw data block
	unsigned int adts_buffer_fullness:11;// 0x7FF 说明是码率可变的码流
	unsigned char number_of_raw_data_blocks_in_frame:2;//表示ADTS帧中有number_of_raw_data_blocks_in_frame + 1个AAC原始帧.
} AdtsHeader;
#pragma pack(pop)

/**
 * @breif: 解析ADTS头信息
 * @param[out] header 解析得到ADTS头信息
 * @param[in] data 待解析的数据
 * @param[in] size 待解析的数据大小
 * @return 成功返回0, 失败返回-1
 */
int ParseAdtsHeaderFromData(AdtsHeader &header, const char *data, int size);

/**
 * @breif: 生成ADTS头信息
 * @param[out] adts_header 生成的adts头信息
 * @param[in] aac_size aac的body长度（不包括头）
 * @param[in] sample_rate 采样率
 * @param[in] channel 通道
 * @param[in] profile 编码等级
 * @return 成功返回0, 失败返回-1
 */
int GetAdtsHeader(char *adts_header, int adts_header_size, int aac_size, int sample_rate, int channel, int profile);

}

#endif