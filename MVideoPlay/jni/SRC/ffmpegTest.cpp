#include <jni.h>
#include <stdio.h>
#include <String>
#include <stdlib.h>
#include <android/log.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

//#include"Header/Conversion.h"

#ifndef  LOG_TAG
#define  LOG_TAG    "log_Test"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

//指定要注册的类
static const char *classPathName = "com/example/studyffmpeg/FfmpegCls";


/*
 * ffmpeg 的操作
 * */
JNIEXPORT void JNICALL ffmpegTest(JNIEnv* env, jobject thiz)
{
	av_register_all();
	avcodec_register_all();
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame;
	AVFrame *pFrameRGB;
	int numBytes;
	uint8_t *buffer;
	AVPacket packet;

	if (avformat_open_input(&pFormatCtx, "/sdcard/kuvision/720.wmv", NULL, NULL)
			< 0)
	{
		LOGI("打开失败！");
		return;
	}
	else
	{
		LOGI("打开成功！");
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		LOGI("获取流信息失败！");
		return;
	}

	// Find the first video stream
	int i, videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	if (videoStream == -1)
	{
		LOGI("获取流信息失败！");
		return;
	}
	//获取解码上下文
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		LOGI("Codec not found");
		return;
	}
	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		LOGI("Could not open codec");
		return;
	}

	pFrame = avcodec_alloc_frame();
	pFrameRGB = avcodec_alloc_frame();
	if (pFrameRGB == NULL)
	{
		return;
	}

	numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width,
			pCodecCtx->height);
	LOGI("numunsigned chars is ：%d",numBytes);

	buffer = (uint8_t *) av_malloc(numBytes * sizeof(uint8_t));
	avpicture_fill((AVPicture *) pFrameRGB, buffer, PIX_FMT_RGB24,
			pCodecCtx->width, pCodecCtx->height);

	//读取数据
	int frameFinished;
	i = 0;
	//从流里面读取一帧
	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		if (packet.stream_index == videoStream)
		{
			int result;
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
			if (frameFinished)
			{
				unsigned char *rgbBuf = new unsigned char[pCodecCtx->width
						* pCodecCtx->height * 3];

				LOGI("pCodecCtx->width is ：%d",pCodecCtx->width);
				LOGI("pCodecCtx->height is ：%d",pCodecCtx->height);
				LOGI("pCodecCtx->pix_fmt is ：%d",pCodecCtx->pix_fmt);

				struct SwsContext *img_convert_ctx = NULL;
				img_convert_ctx = sws_getContext(pCodecCtx->width,
						pCodecCtx->height, pCodecCtx->pix_fmt,
						pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24,
						SWS_POINT, NULL, NULL, NULL);

				result = sws_scale(img_convert_ctx,
						(const uint8_t* const *) pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height,
						(uint8_t**) &rgbBuf, pFrameRGB->linesize);
				sws_freeContext(img_convert_ctx);

				if (++i <= 3)
				{
					RGB2BMP(rgbBuf, pCodecCtx->width, pCodecCtx->height, i);
					delete[] rgbBuf;
					//解码前3帧
				}
				else
				{
					break;
				}
			}
		}
		av_free_packet(&packet);
	}
	av_free(buffer);
	av_free(pFrameRGB);
	av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	return;
}

/**
 * 方法对应表
 */
static JNINativeMethod methodsTab[] =
{
{ "ffmpegTest", "()V", (void*) ffmpegTest } };

//初始化函数
jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	JNIEnv* env = NULL;
	jclass clazz;

	//获取JNI环境对象
	if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
	{
		LOGE("ERROR: GetEnv failed\n");
		return JNI_ERR;
	}

	//注册本地方法,Load目标类
	clazz = env->FindClass(classPathName);
	if (clazz == NULL)
	{
		LOGE("Native registration unable to find class '%s'", classPathName);
		return JNI_ERR;
	}

	//注册本地方法
	if (env->RegisterNatives(clazz, methodsTab, sizeof(methodsTab)
			/ sizeof(methodsTab[0])) < 0)
	{
		LOGE("ERROR: MediaPlayer native registration failed\n");
		return JNI_ERR;
	}

	//返回版本号
	return JNI_VERSION_1_4;
}
