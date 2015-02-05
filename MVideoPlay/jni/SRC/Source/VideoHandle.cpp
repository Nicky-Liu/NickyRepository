/*
 * VideoHandle.cpp
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */
#include"Header/VideoHandle.h"

VideoHandle::VideoHandle(DecodeInfo *myDInfo, SdlInfo *pSDLInfo)
{
	this->mDecodeInfo = myDInfo;
	this->mSDLInfo = pSDLInfo;
}
VideoHandle::~VideoHandle()
{

}
void VideoHandle::startVideoHandle()
{
	this->videoDecodeFrame();
}

int VideoHandle::videoDisplay()
{
	sws_scale(mSDLInfo->getSdlConvertCtx(),
			(const uint8_t* const *) mDecodeInfo->getSrcFrame()->data,
			mDecodeInfo->getSrcFrame()->linesize, 0,
			mDecodeInfo->getVideoCodecCtx()->height,
			mDecodeInfo->getFrameYUV()->data,
			mDecodeInfo->getFrameYUV()->linesize);

	SDL_UpdateYUVTexture(mSDLInfo->getSdlTexture(), mSDLInfo->getSdlRect(),
			mDecodeInfo->getFrameYUV()->data[0],
			mDecodeInfo->getFrameYUV()->linesize[0],
			mDecodeInfo->getFrameYUV()->data[1],
			mDecodeInfo->getFrameYUV()->linesize[1],
			mDecodeInfo->getFrameYUV()->data[2],
			mDecodeInfo->getFrameYUV()->linesize[2]);

	SDL_RenderClear(mSDLInfo->getSdlRender());
	SDL_RenderCopy(mSDLInfo->getSdlRender(), mSDLInfo->getSdlTexture(),
			mSDLInfo->getSdlRect(), mSDLInfo->getSdlRect());
	SDL_RenderPresent(mSDLInfo->getSdlRender());
	return 1;
}

int VideoHandle::videoDecodeFrame()
{
	AVFormatContext *pFormatCtx = mDecodeInfo->getFromatContext();
	AVCodecContext * VideoCodeContex = mDecodeInfo->getVideoCodecCtx();
	int len1;
	int frameFinished;

	AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
	av_init_packet(pkt);

	AVFrame *pFrame = (AVFrame*) av_malloc(sizeof(AVFrame));
	memset(pFrame, 0, sizeof(AVFrame));
	for (;;)
	{
		LOGI("get video pkt");
		if (mDecodeInfo->mVideoQue.packetQueueGet(pkt, 1) < 0)
		{
			break;
		}

		len1 = avcodec_decode_video2(VideoCodeContex, pFrame, &frameFinished,
				pkt);

		if (len1 >= 0)
		{
			if (frameFinished)
			{
				mDecodeInfo->setSrcFrame(pFrame);
				if (videoDisplay() < 0)
				{
					av_free(pFrame);
				}
			}
		}
		av_free_packet(pkt);
	}

	return 0;
}
