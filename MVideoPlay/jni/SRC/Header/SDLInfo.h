/*
 * SDLInfo.h
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */

#ifndef SDLINFO_H_
#define SDLINFO_H_
#include "SDL.h"
#include "Header/DecodeInfo.h"
typedef void (*ACallBack)(void*, Uint8*, int);

class SdlInfo
{
public:
	SdlInfo(ACallBack fp);
	~SdlInfo();
	int SdlInfoInit(DecodeInfo *pdecodeInfo);
	void startAudioHandle();
	SDL_Window *getSdlWindow();
	SDL_Renderer *getSdlRender();
	SDL_Texture *getSdlTexture();
	SDL_Rect *getSdlRect();
	struct SwsContext *getSdlConvertCtx();

private:
	SDL_Window *mWindow;
	SDL_Renderer *mRender;
	SDL_Texture *mTexture;
	SDL_Rect mRect;
	struct SwsContext *mImageConvertCtx;
	SDL_AudioSpec mWantedSpec;
	ACallBack audioCallBack;
};
#endif
