/*
 * SDLInfo.cpp
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */
#include"Header/SDLInfo.h"

SdlInfo::SdlInfo(ACallBack fp) :
	mWindow(NULL), mRender(NULL), mTexture(NULL), mImageConvertCtx(NULL)
{
	this->audioCallBack = fp;
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		LOGI("Could not initialize SDL -%s\n", SDL_GetError());
	}
	this->mRect.h = 420;
	this->mRect.w = 640;
	this->mRect.x = 0;
	this->mRect.y = 0;
}
SdlInfo::~SdlInfo()
{

}
int SdlInfo::SdlInfoInit(DecodeInfo *pdecodeInfo)
{
	this->mWantedSpec.freq = pdecodeInfo->getAudioCodecCtx()->sample_rate;
	this->mWantedSpec.format = AUDIO_S16SYS;
	this->mWantedSpec.channels = pdecodeInfo->getAudioCodecCtx()->channels; //声音的通道数
	this->mWantedSpec.silence = 0; //用来表示静音的值
	this->mWantedSpec.samples = SDL_AUDIO_BUFFER_SIZE; //声音缓冲区的大小
	this->mWantedSpec.callback = audioCallBack;
	this->mWantedSpec.userdata = pdecodeInfo->getAudioCodecCtx();
	if (SDL_OpenAudio(&mWantedSpec, NULL) < 0)
	{
		LOGI("SDL_OpenAudio error: %s\n", SDL_GetError());
		return -1;
	}

	this->mWindow = SDL_CreateWindow("MyPlayer", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, pdecodeInfo->getVideoCodecCtx()->width,
			pdecodeInfo->getVideoCodecCtx()->height, SDL_WINDOW_SHOWN);
	if (!this->mWindow)
	{
		LOGI("ERROR:%s",SDL_GetError() );
		return 1;
	}

	this->mRender = SDL_CreateRenderer(this->mWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (this->mRender == NULL)
	{
		LOGI("ERROR:%s",SDL_GetError() );
		return -1;
	}

	this->mTexture = SDL_CreateTexture(this->mRender, SDL_PIXELFORMAT_YV12,
			SDL_TEXTUREACCESS_STREAMING,
			pdecodeInfo->getVideoCodecCtx()->width,
			pdecodeInfo->getVideoCodecCtx()->height);

	this->mRect.x = 0;
	this->mRect.y = 0;
	this->mRect.w = pdecodeInfo->getVideoCodecCtx()->width;
	this->mRect.h = pdecodeInfo->getVideoCodecCtx()->height;

	this->mImageConvertCtx = sws_getContext(
			pdecodeInfo->getVideoCodecCtx()->width,
			pdecodeInfo->getVideoCodecCtx()->height,
			pdecodeInfo->getVideoCodecCtx()->pix_fmt,
			pdecodeInfo->getVideoCodecCtx()->width,
			pdecodeInfo->getVideoCodecCtx()->height, PIX_FMT_YUV420P,
			SWS_BICUBIC, NULL, NULL, NULL);
	return 1;
}
void SdlInfo::startAudioHandle()
{
	SDL_PauseAudio(0);
}
SDL_Window *SdlInfo::getSdlWindow()
{
	return this->mWindow;
}
SDL_Renderer *SdlInfo::getSdlRender()
{
	return this->mRender;
}
SDL_Texture *SdlInfo::getSdlTexture()
{
	return this->mTexture;
}
SDL_Rect *SdlInfo::getSdlRect()
{
	return &this->mRect;
}
struct SwsContext *SdlInfo::getSdlConvertCtx()
{
	return this->mImageConvertCtx;
}

