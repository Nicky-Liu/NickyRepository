#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"

#include <android/log.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#ifndef  LOG_TAG
#define  LOG_TAG    "log_Test"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#define SCREEN_WIDTH 2560
#define SCREEN_HEIGH 1600

//全局变量
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

/*
 回调函数
 */
void fill_audio(void *udata, Uint8 *stream, int len)
{
	if (audio_len == 0)
		return;
	len = (len > audio_len ? audio_len : len);
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
}

int main(int argc, char* argv[])
{
	av_register_all();
	avcodec_register_all();
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx;//视频解码上下文
	AVCodecContext *aCodecCtx;//音频解码上下文
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVFrame *pFrameRGB;
	int i, numBytes;
	uint8_t *buffer;
	AVPacket *packet;
	SDL_AudioSpec wanted_spec;

	int videoStream = -1;
	int audioStream = -1;
	if (avformat_open_input(&pFormatCtx, "/sdcard/kuvision/720.wmv", NULL, NULL)
			< 0)
	{
		LOGI("打开失败！");
		return 1;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		LOGI("获取流信息失败！");
		return 2;
	}

	LOGI("流信息：nb_streams = %d",pFormatCtx->nb_streams);
	// Find the first video stream

	for (i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO
				&& videoStream < 0)
		{
			videoStream = i;
			//break;
		}
		else if (pFormatCtx->streams[i]->codec->codec_type
				== AVMEDIA_TYPE_AUDIO && audioStream < 0)
		{
			audioStream = i;
		}
	}
	if (videoStream == -1 || audioStream == -1)
	{
		LOGI("获取流信息失败！");
		return 3;
	}
	//获取解码上下文
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		LOGI("Codec not found");
		return 4;
	}
	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		LOGI("Could not open codec");
		return 5;
	}

	aCodecCtx = pFormatCtx->streams[audioStream]->codec;
	//设置音频信息
	wanted_spec.freq = aCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = aCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = 1024;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = aCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL) < 0)
	{
		LOGI("ERROR: %s",SDL_GetError());
		return -1;
	}

	AVCodec *aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if (!aCodec)
	{
		LOGI("ERROR: Unsupported codec!\n");
		return -1;
	}
	avcodec_open2(aCodecCtx, aCodec, NULL);

	//把结构体改为指针
	packet = (AVPacket *) malloc(sizeof(AVPacket));
	av_init_packet(packet);

	pFrame = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL)
	{
		return 6;
	}

	numBytes = avpicture_get_size(AV_PIX_FMT_YUV420P, SCREEN_WIDTH,
			SCREEN_HEIGH);
	LOGI("numBytes chars is ：%d",numBytes);

	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *) pFrameRGB, buffer, AV_PIX_FMT_YUV420P,
			SCREEN_WIDTH, SCREEN_HEIGH);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{
		LOGI("Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window *screen;
	screen = SDL_CreateWindow("Hello World!", 0, 0, SCREEN_WIDTH, SCREEN_HEIGH,
			SDL_WINDOW_SHOWN);
	if (!screen)
	{
		LOGI("SDL: could not set video mode - exiting\n");
		exit(1);
	}
	SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	SDL_Texture *tex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
			SDL_TEXTUREACCESS_STATIC, SCREEN_WIDTH, SCREEN_HEIGH);

	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = SCREEN_WIDTH;
	rect.h = SCREEN_HEIGH;

	struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width,
			pCodecCtx->height, pCodecCtx->pix_fmt, SCREEN_WIDTH, SCREEN_HEIGH,
			PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	//读取数据
	int frameFinished;
	//从流里面读取一帧
	uint32_t ret, len = 0;
	int got_picture;
	int index = 0;
	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == videoStream)
		{
			LOGI("+++++解码视频！++++");
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, packet);
			if (frameFinished)
			{
				//将yuv数据转换成RGB格式
				sws_scale(img_convert_ctx,
						(const uint8_t* const *) pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height,
						pFrameRGB->data, pFrameRGB->linesize);

				////iPitch 计算yuv一行数据占的字节数
				SDL_UpdateTexture(tex, &rect, pFrameRGB->data[0],
						pFrameRGB->linesize[0]);
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, tex, &rect, &rect);
				SDL_RenderPresent(renderer);
			}
		}
		else if (packet->stream_index == audioStream)
		{
			LOGI("+++++解码音频！++++");
			ret
					= avcodec_decode_audio4(aCodecCtx, pFrame, &got_picture,
							packet);
			if (ret < 0)
			{
				LOGI("Error in decoding audio frame.\n");
				return -1;
			}

			audio_chunk = (Uint8*) pFrame->data[0];
			audio_len = pFrame->linesize[0];
			audio_pos = audio_chunk;
			SDL_PauseAudio(1);
			//等待直到音频数据播放完毕!
			while (audio_len > 0)
				SDL_Delay(1);
		}
		else
		{
			av_free_packet(packet);
		}
	}
	av_free(buffer);
	av_free(pFrameRGB);
	av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return 7;
}
