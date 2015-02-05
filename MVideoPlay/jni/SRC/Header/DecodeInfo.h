/*
 * DecodeInfo.h
 *
 *  Created on: 2014-8-14
 *      Author: Administrator
 */
#ifndef DECODEINFO_H_
#define DECODEINFO_H_
#include "PktQueue.h"
#include "SDL.h"
#include <android/log.h>

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

#define MAX_VIDEOQ_SIZE (5 * 256 * 1024)
#define MAX_AUDIOQ_SIZE (5 * 16 * 1024)
#define SDL_AUDIO_BUFFER_SIZE 1152
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

int readData(void *ptr);
class DecodeInfo
{
public:
	DecodeInfo();
	~DecodeInfo();
	int openFileStream(const char *filename);
	int decodeInfoInit();
	void startGetDataThread();

	AVFormatContext *getFromatContext();
	AVCodecContext *getVideoCodecCtx();
	AVCodecContext *getAudioCodecCtx();
	AVCodec *getVideoCodec();
	AVCodec *getAudioCodec();
	int getAudioIndex();
	int getVideoIndex();
	uint8_t *getOutBuffer();
	PktQueue mVideoQue;
	PktQueue mAudioQue;
	AVFrame *getSrcFrame();
	void setSrcFrame(AVFrame *srcFrame);
	AVFrame *getFrameYUV();

private:
	AVFormatContext *mpFormatCtx;
	AVFrame *mpFrame;
	AVFrame *mpFrameYUV;
	AVCodecContext *mVideoCodecCtx;
	AVCodecContext *mAudioCodecCtx;
	AVCodec *mVideoCodec;
	AVCodec *mAudioCodec;
	int mAudioIndex;
	int mVideoIndex;
	uint8_t *mOutBuffer;
	int mNumBytes;
	SDL_Thread *getDataThread;
};

#endif /* DECODEINFO_H_ */
