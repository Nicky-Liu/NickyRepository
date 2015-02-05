/*
 * DecodeInfo.cpp
 *
 *  Created on: 2014-8-14
 *      Author: Administrator
 */

#include"Header/DecodeInfo.h"

DecodeInfo::DecodeInfo() :
	mAudioIndex(-1), mVideoIndex(-1)
{
	av_register_all();
	this->mpFormatCtx = avformat_alloc_context();
	this->mpFrame = av_frame_alloc();
	this->mpFrameYUV = av_frame_alloc();
}

DecodeInfo::~DecodeInfo()
{
	av_frame_free(&this->mpFrame);
	av_frame_free(&this->mpFrameYUV);
	avcodec_close(this->mVideoCodecCtx);
	avcodec_close(this->mAudioCodecCtx);
	avformat_close_input(&this->mpFormatCtx);
}

int DecodeInfo::openFileStream(const char *filename)
{
	if (avformat_open_input(&this->mpFormatCtx, filename, NULL, NULL) != 0)
	{
		LOGE("Can't open the file\n");
		return -1;
	}
	LOGI("before avformat_find_stream_info");
	if (avformat_find_stream_info(this->mpFormatCtx, NULL) < 0)
	{
		LOGE("Couldn't find stream information.\n");
		return -1;
	}

	for (int i = 0; i < this->mpFormatCtx->nb_streams; i++)
	{//视音频流的个数
		if (this->mpFormatCtx->streams[i]->codec->codec_type
				== AVMEDIA_TYPE_VIDEO && this->mVideoIndex < 0)
		{
			this->mVideoIndex = i;
		}
		if (this->mpFormatCtx->streams[i]->codec->codec_type
				== AVMEDIA_TYPE_AUDIO && this->mAudioIndex < 0)
			this->mAudioIndex = i;
	}
	return 1;
}

int DecodeInfo::decodeInfoInit()
{
	if (this->mVideoIndex >= 0)
	{
		this->mVideoCodecCtx
				= this->mpFormatCtx->streams[this->mVideoIndex]->codec;
	}
	if (this->mAudioIndex >= 0)
	{
		this->mAudioCodecCtx
				= this->mpFormatCtx->streams[this->mAudioIndex]->codec;
	}

	this->mVideoCodec = avcodec_find_decoder(this->mVideoCodecCtx->codec_id);
	this->mAudioCodec = avcodec_find_decoder(this->mAudioCodecCtx->codec_id);

	if (this->mVideoCodec == NULL || this->mAudioCodec == NULL)
	{
		LOGE("Unsupported codec!\n");
		return -1;
	}
	//Open codec
	if (avcodec_open2(this->mVideoCodecCtx, this->mVideoCodec, NULL) < 0)
	{
		LOGE("Could not open video codec.\n");
		return -1;
	}
	if (avcodec_open2(this->mAudioCodecCtx, this->mAudioCodec, NULL) < 0)
	{
		LOGE("Could not open audio codec.\n");
		return -1;
	}
	this->mNumBytes = avpicture_get_size(PIX_FMT_YUV420P,
			this->mVideoCodecCtx->width, this->mVideoCodecCtx->height);
	this->mOutBuffer = (uint8_t *) av_malloc(this->mNumBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture*) this->mpFrameYUV, this->mOutBuffer,
			PIX_FMT_YUV420P, this->mVideoCodecCtx->width,
			this->mVideoCodecCtx->height);
	return 1;
}

void DecodeInfo::startGetDataThread()
{
	getDataThread = SDL_CreateThread(readData, "decode thread", this);
}
int readData(void *ptr)
{
	DecodeInfo *pDecodeInfo = (DecodeInfo *) ptr;
	AVFormatContext *pFormatCtx = pDecodeInfo->getFromatContext();
	int AudioIndex = pDecodeInfo->getAudioIndex();
	int VideoIndex = pDecodeInfo->getVideoIndex();
	AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
	av_init_packet(pkt);

	if (VideoIndex < 0 && AudioIndex < 0)
	{
		LOGE("index error!");
		return -1;
	}

	for (;;)
	{
		if (pDecodeInfo->mAudioQue.getQueueSize() > MAX_AUDIOQ_SIZE
				|| pDecodeInfo->mVideoQue.getQueueSize() > MAX_VIDEOQ_SIZE)
		{
			//延迟
			SDL_Delay(10);
			continue;
		}

		int ret = av_read_frame(pFormatCtx, pkt);
		if (ret < 0)
		{
			LOGE("get packget error!!");
			break;
		}

		if (pkt->stream_index == AudioIndex)
		{
			LOGI("put Audio pkt");
			pDecodeInfo->mAudioQue.packetQueuePut(pkt);
		}
		else if (pkt->stream_index == VideoIndex)
		{
			LOGI("put video pkt");
			pDecodeInfo->mVideoQue.packetQueuePut(pkt);
		}
		else
		{
			av_free_packet(pkt);
		}
	}
	return 1;
}

AVFormatContext *DecodeInfo::getFromatContext()
{
	return this->mpFormatCtx;
}

AVCodecContext *DecodeInfo::getVideoCodecCtx()
{
	return this->mVideoCodecCtx;
}
AVCodecContext *DecodeInfo::getAudioCodecCtx()
{
	return this->mAudioCodecCtx;
}
AVCodec *DecodeInfo::getVideoCodec()
{
	return this->mVideoCodec;
}
AVCodec *DecodeInfo::getAudioCodec()
{
	this->mAudioCodec;
}
int DecodeInfo::getAudioIndex()
{
	return this->mAudioIndex;
}
int DecodeInfo::getVideoIndex()
{
	return this->mVideoIndex;
}
uint8_t *DecodeInfo::getOutBuffer()
{
	return this->mOutBuffer;
}

AVFrame *DecodeInfo::getSrcFrame()
{
	return this->mpFrame;
}

void DecodeInfo::setSrcFrame(AVFrame *srcFrame)
{
	this->mpFrame = srcFrame;
}

AVFrame *DecodeInfo::getFrameYUV()
{
	return this->mpFrameYUV;
}
