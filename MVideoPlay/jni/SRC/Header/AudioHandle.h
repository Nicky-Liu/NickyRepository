/*
 * AudioHandle.h
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */

#ifndef AUDIOHANDLE_H_
#define AUDIOHANDLE_H_
#define SDL_AUDIO_BUFFER_SIZE 1152
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#include"SDL.h"
#include <android/log.h>
#include"Header/AudioHandle.h"
#include"Header/DecodeInfo.h"

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

#ifndef  LOG_TAG
#define  LOG_TAG    "log_Test"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

typedef void (*ACallBack)(void*, Uint8*, int);

void audioHandleInit(DecodeInfo *pDCInfo);
void audioCallback(void *userdata, Uint8 *stream, int len);
int audioDecodeFrame(AVCodecContext *aCodecCtx, uint8_t *audio_buf,
		int buf_size);
int AudioResampling(AVCodecContext * audio_dec_ctx,
		AVFrame * pAudioDecodeFrame, int out_sample_fmt, int out_channels,
		int out_sample_rate, uint8_t* out_buf);

static ACallBack AC = audioCallback;
static DecodeInfo *pDCInfo;

#endif /* AUDIOHANDLE_H_ */
