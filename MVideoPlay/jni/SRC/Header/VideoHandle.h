/*
 * VideoHandle.h
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */

#ifndef VIDEOHANDLE_H_
#define VIDEOHANDLE_H_
#include "SDL.h"
#include"Header/DecodeInfo.h"
#include"Header/SDLInfo.h"

class VideoHandle
{
public:
	VideoHandle(DecodeInfo *myDInfo, SdlInfo *SDLInfo);
	~VideoHandle();
	void startVideoHandle();
	int videoDecodeFrame();
	int videoDisplay();
private:
	DecodeInfo *mDecodeInfo;
	SdlInfo *mSDLInfo;
};

#endif /* VIDEOHANDLE_H_ */
