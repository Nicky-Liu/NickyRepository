#include "Header/DecodeInfo.h"
#include "Header/PktQueue.h"
#include "Header/SDLInfo.h"
#include "Header/AudioHandle.h"
#include "Header/VideoHandle.h"

#define SDL_AUDIO_BUFFER_SIZE 1152
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

int main(int argc, char **argv)
{
	DecodeInfo myDecodeVideo;
	PktQueue myPktQueue;
	SdlInfo mySDLInfo(AC);

	const char *input_filename = "/sdcard/kuvision/720.wmv";
	//const char *input_filename = "/sdcard/kuvision/ff.mp4";

	if (myDecodeVideo.openFileStream(input_filename) < 0)
	{
		LOGI("error!!");
	}
	if (myDecodeVideo.decodeInfoInit() < 0)
	{
		LOGI("error!!");
	}

	if (mySDLInfo.SdlInfoInit(&myDecodeVideo) < 0)
	{
		LOGI("error!!");
	}
	VideoHandle vdHandle(&myDecodeVideo, &mySDLInfo);
	audioHandleInit(&myDecodeVideo);

	myDecodeVideo.startGetDataThread();
	mySDLInfo.startAudioHandle();
	vdHandle.startVideoHandle();
	return 0;
}
