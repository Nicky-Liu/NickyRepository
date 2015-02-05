#include <stdio.h>
#include <android/log.h>
#include <iostream>
using namespace std;

#define SDL_AUDIO_BUFFER_SIZE 1152
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000

#ifndef  LOG_TAG
#define  LOG_TAG    "log_Test"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "SDL.h"
#include "SDL_thread.h"
}

#define NB_SAMPLE 1152//这里如果是播放aac要改成1024
//全局变量---------------------
static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

//AudioResampling(pCodecCtx, pFrame,AV_SAMPLE_FMT_S16, 2, 44100);
int AudioResampling(AVCodecContext * audio_dec_ctx,
		AVFrame * pAudioDecodeFrame, int out_sample_fmt, int out_channels,
		int out_sample_rate)
{
	SwrContext * swr_ctx = NULL;
	int data_size = 0;
	int ret = 0;
	int64_t src_ch_layout = audio_dec_ctx->channel_layout;
	int64_t dst_ch_layout = AV_CH_LAYOUT_STEREO;
	int dst_nb_channels = 0;
	int dst_linesize = 0;
	int src_nb_samples = 0;
	int dst_nb_samples = 0;
	int max_dst_nb_samples = 0;
	uint8_t **dst_data = NULL;
	int resampled_data_size = 0;

	swr_ctx = swr_alloc();
	if (!swr_ctx)
	{
		LOGI("swr_alloc error \n");
		return -1;
	}

	src_ch_layout
			= (audio_dec_ctx->channels == av_get_channel_layout_nb_channels(
					audio_dec_ctx->channel_layout)) ? audio_dec_ctx->channel_layout
					: av_get_default_channel_layout(audio_dec_ctx->channels);

	if (out_channels == 1)
	{
		dst_ch_layout = AV_CH_LAYOUT_MONO;
		LOGI("dst_ch_layout: AV_CH_LAYOUT_MONO\n");
	}
	else if (out_channels == 2)
	{
		dst_ch_layout = AV_CH_LAYOUT_STEREO;
		LOGI("dst_ch_layout: AV_CH_LAYOUT_STEREO\n");
	}
	else
	{
		dst_ch_layout = AV_CH_LAYOUT_SURROUND;
		LOGI("dst_ch_layout: AV_CH_LAYOUT_SURROUND\n");
	}

	if (src_ch_layout <= 0)
	{
		LOGI("src_ch_layout error \n");
		return -1;
	}

	src_nb_samples = pAudioDecodeFrame->nb_samples;
	if (src_nb_samples <= 0)
	{
		LOGI("src_nb_samples error \n");
		return -1;
	}

	av_opt_set_int(swr_ctx, "in_channel_layout", src_ch_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", audio_dec_ctx->sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", audio_dec_ctx->sample_fmt,
			0);

	av_opt_set_int(swr_ctx, "out_channel_layout", dst_ch_layout, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", out_sample_rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt",
			(AVSampleFormat) out_sample_fmt, 0);

	if ((ret = swr_init(swr_ctx)) < 0)
	{
		LOGI("Failed to initialize the resampling context\n");
		return -1;
	}

	max_dst_nb_samples = dst_nb_samples = av_rescale_rnd(src_nb_samples,
			out_sample_rate, audio_dec_ctx->sample_rate, AV_ROUND_UP);
	if (max_dst_nb_samples <= 0)
	{
		LOGI("av_rescale_rnd error \n");
		return -1;
	}

	dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
	ret
			= av_samples_alloc_array_and_samples(&dst_data, &dst_linesize,
					dst_nb_channels, dst_nb_samples,
					(AVSampleFormat) out_sample_fmt, 0);
	if (ret < 0)
	{
		LOGI("av_samples_alloc_array_and_samples error \n");
		return -1;
	}

	dst_nb_samples = av_rescale_rnd(swr_get_delay(swr_ctx,
			audio_dec_ctx->sample_rate) + src_nb_samples, out_sample_rate,
			audio_dec_ctx->sample_rate, AV_ROUND_UP);
	if (dst_nb_samples <= 0)
	{
		LOGI("av_rescale_rnd error \n");
		return -1;
	}
	if (dst_nb_samples > max_dst_nb_samples)
	{
		av_free(dst_data[0]);
		ret = av_samples_alloc(dst_data, &dst_linesize, dst_nb_channels,
				dst_nb_samples, (AVSampleFormat) out_sample_fmt, 1);
		max_dst_nb_samples = dst_nb_samples;
	}

	if (swr_ctx)
	{
		ret = swr_convert(swr_ctx, dst_data, dst_nb_samples,
				(const uint8_t **) pAudioDecodeFrame->data,
				pAudioDecodeFrame->nb_samples);
		if (ret < 0)
		{
			LOGI("swr_convert error \n");
			return -1;
		}

		resampled_data_size = av_samples_get_buffer_size(&dst_linesize,
				dst_nb_channels, ret, (AVSampleFormat) out_sample_fmt, 1);
		if (resampled_data_size < 0)
		{
			LOGI("av_samples_get_buffer_size error \n");
			return -1;
		}
	}
	else
	{
		LOGI("swr_ctx null error \n");
		return -1;
	}

	if (!audio_chunk)
	{
		audio_chunk = (uint8_t *) av_malloc(resampled_data_size
				* sizeof(uint8_t));
	}
	memcpy(audio_chunk, dst_data[0], resampled_data_size);

	if (dst_data)
	{
		av_freep(&dst_data[0]);
	}
	av_freep(&dst_data);
	dst_data = NULL;

	if (swr_ctx)
	{
		swr_free(&swr_ctx);
	}
	return resampled_data_size;
}
int myResample(AVFrame *pFrame)
{
	int out_nb_samples = av_rescale_rnd(pFrame->nb_samples, 44100,
			pFrame->sample_rate, AV_ROUND_UP);
	int out_buffer_size = av_samples_get_buffer_size(pFrame->linesize,
			pFrame->channels, pFrame->nb_samples, AV_SAMPLE_FMT_S16, 1);
	uint8_t *out_buffer = new uint8_t[out_buffer_size];
	SwrContext *au_convert_ctx = swr_alloc();
	au_convert_ctx = swr_alloc_set_opts(au_convert_ctx, AV_CH_LAYOUT_STEREO,
			AV_SAMPLE_FMT_S16, 44100, pFrame->channel_layout,
			(AVSampleFormat) pFrame->format, pFrame->sample_rate, 0, NULL);
	swr_init(au_convert_ctx);
	swr_convert(au_convert_ctx, &out_buffer, out_nb_samples,
			(const uint8_t **) pFrame->data, pFrame->nb_samples);

	audio_len = out_buffer_size;
	audio_pos = out_buffer;

	delete out_buffer;
	if (au_convert_ctx)
	{
		swr_free(&au_convert_ctx);
	}
}
void fill_audio(void *udata, Uint8 *stream, int len)
{
	/*  Only  play  if  we  have  data  left  */
	if (audio_len == 0)
		return;
	/*  Mix  as  much  data  as  possible  */
	len = (len > audio_len ? audio_len : len);
	SDL_memset(stream, 0, len);// make sure this is silence.
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	//memcpy(stream, audio_pos, len);
	audio_pos += len;
	audio_len -= len;
}

int decode_audio(char* no_use)
{
	AVFormatContext *pFormatCtx;
	int i, audioStream;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;

	char url[300] =
	{ 0 };
	strcpy(url, no_use);
	av_register_all();

	pFormatCtx = avformat_alloc_context();
	//有参数avdic
	if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0)
	{
		LOGI("Couldn't open file.\n");
		return -1;
	}

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		LOGI("Couldn't find stream information.\n");
		return -1;
	}
	// Dump valid information onto standard error
	av_dump_format(pFormatCtx, 0, url, false);

	// Find the first audio stream
	audioStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		//原为codec_type==CODEC_TYPE_AUDIO
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audioStream = i;
			break;
		}

	if (audioStream == -1)
	{
		LOGI("Didn't find a audio stream.\n");
		return -1;
	}

	// Get a pointer to the codec context for the audio stream
	pCodecCtx = pFormatCtx->streams[audioStream]->codec;

	// Find the decoder for the audio stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		LOGI("Codec not found.\n");
		return -1;
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
	{
		LOGI("Could not open codec.\n");
		return -1;
	}

	//把结构体改为指针
	AVPacket *packet = (AVPacket *) malloc(sizeof(AVPacket));
	av_init_packet(packet);

	//音频和视频解码更加统一！
	AVFrame *pFrame;
	pFrame = av_frame_alloc();

	//初始化
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		LOGI("Could not initialize SDL - %s\n", SDL_GetError());
		exit(1);
	}

	//结构体，包含PCM数据的相关信息
	SDL_AudioSpec wanted_spec;
	wanted_spec.freq = pCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = pCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = NB_SAMPLE;
	wanted_spec.callback = fill_audio;
	wanted_spec.userdata = pCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, NULL) < 0)//步骤（2）打开音频设备
	{
		LOGI("can't open audio.\n");
		return 0;
	}
	LOGI("pCodecCtx->channels=%d \n", pCodecCtx->channels);
	LOGI("pCodecCtx->sample_rate=%d \n", pCodecCtx->sample_rate);
	LOGI("pCodecCtx->sample_rate=%d \n", pCodecCtx->sample_rate);

	uint32_t decodeRetValue;
	int got_picture;
	int index = 0;

	while (av_read_frame(pFormatCtx, packet) >= 0)
	{
		if (packet->stream_index == audioStream)
		{
			decodeRetValue = avcodec_decode_audio4(pCodecCtx, pFrame,
					&got_picture, packet);
			LOGI("decodeRetValue =%d",decodeRetValue);
			LOGI("packet->size =%d",packet->size);
			if (decodeRetValue < 0) // if error len = -1
			{
				LOGI("Error in decoding audio frame.\n");
				exit(0);
			}
			if (got_picture > 0)
			{
				audio_len = AudioResampling(pCodecCtx, pFrame,
						AV_SAMPLE_FMT_S16, 2, 44100);
			}
			audio_pos = audio_chunk;
			SDL_PauseAudio(0);
			while (audio_len > 0)//等待直到音频数据播放完毕!
				SDL_Delay(1);
		}
		av_free_packet(packet);

	}
	av_free_packet(packet);
	SDL_CloseAudio();//关闭音频设备
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

int main(int argc, char* argv[])
{
	char filename[] = "/sdcard/kuvision/mm.mp3";
	if (decode_audio(filename) == 0)
		LOGI("Decode audio successfully.\n");

	return 0;
}
