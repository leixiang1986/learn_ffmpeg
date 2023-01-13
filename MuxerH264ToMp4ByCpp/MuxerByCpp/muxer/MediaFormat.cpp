/*
 * @Description: 
 * @Author: xujin
 * @Email: xujin_wuhan@163.com
 * @Date: 2022-11-24 11:54:07
 * @LastEditTime: 2022-11-25 10:47:29
 * @LastEditors: xujin
 * @Reference: 
 */
#include "MediaFormat.h"
#include "Log.h"
#include <stdint.h>

struct  AVCodecContext;

namespace XJMediaUtils {

int ParseAdtsHeaderFromData(AdtsHeader &header, const char *data, int size) {
    if (!data || size < sizeof(AdtsHeader)) {
        ERR("Failed to parse adts header");
        return -1;
    }

    header.syncword = data[0] << 4 | ((data[1] & 0xf0) >> 4);
    header.ID = (data[1] & 0x8) >> 3;
    header.layer = (data[1] & 0x6) >> 1;
    header.protection_absent = data[1] & 0x1;
    header.profile = (data[2] & 0xc0) >> 6;
    header.sampling_frequency_index = (data[2] & 0x3c) >> 2;
    header.private_bit = (data[2] & 0x2)>> 1;
    header.channel_configuration = ((data[2] & 0x01) << 2) | ((data[3] & 0xc0) >> 6);
    header.original_copy = (data[3] & 0x20) >> 5;
    header.home = (data[3] & 0x10) >> 4;
    header.copyright_identification_bit = (data[3] & 0x08) >> 3;
    header.copyright_identification_start = (data[3] & 0x04) >> 2;
    header.frame_length = ((data[3] & 0x3) << 11) | (data[4] << 3) | ((data[5] &0xe0) >> 5);
    header.adts_buffer_fullness = ((data[5] & 0x1f) << 6) | ((data[6] & 0xfc) >> 2);
    header.number_of_raw_data_blocks_in_frame = data[7] & 0x3;

    return 0;
}

int GetAdtsHeader(char *adts_header, int adts_header_size, int aac_size, int sample_rate, int channel, int profile) {
    if (!adts_header || adts_header_size < 7) {
        ERR("Failed, invalid parameters");
        return -1;
    }

    uint8_t freq_idx = 0;    //0: 96000 Hz  3: 48000 Hz 4: 44100 Hz
    switch (sample_rate) {
        case 96000: freq_idx = 0; break;
        case 88200: freq_idx = 1; break;
        case 64000: freq_idx = 2; break;
        case 48000: freq_idx = 3; break;
        case 44100: freq_idx = 4; break;
        case 32000: freq_idx = 5; break;
        case 24000: freq_idx = 6; break;
        case 22050: freq_idx = 7; break;
        case 16000: freq_idx = 8; break;
        case 12000: freq_idx = 9; break;
        case 11025: freq_idx = 10; break;
        case 8000: freq_idx = 11; break;
        case 7350: freq_idx = 12; break;
        default: freq_idx = 4; break;
    }
    uint32_t frame_size = aac_size + 7;
    adts_header[0] = 0xFF;
    adts_header[1] = 0xF1;
    adts_header[2] = (profile << 6) + (freq_idx << 2) + (channel >> 2);
    adts_header[3] = (((channel & 3) << 6) + (frame_size  >> 11));
    adts_header[4] = ((frame_size & 0x7FF) >> 3);
    adts_header[5] = (((frame_size & 7) << 5) + 0x1F);
    adts_header[6] = 0xFC;

    return 0;
}

}