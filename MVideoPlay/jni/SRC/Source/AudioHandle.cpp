/*
 * AudioHandle.cpp
 *
 *  Created on: 2014-8-18
 *      Author: Administrator
 */
#include"Header/AudioHandle.h"

void audioHandleInit(DecodeInfo *myDCInfo)
{
	pDCInfo = myDCInfo;
}

void audioCallback(void *userdata, Uint8 *stream, int len)
{
	AVCodecContext *aCodecCtx = (AVCodecContext*) userdata;
	int len1, audio_size;

	static uint8_t audio_buf[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while (len > 0)
	{
		if (audio_buf_index >= audio_buf_size)
		{
			audio_size = audioDecodeFrame(aCodecCtx, audio_buf,
					sizeof(audio_buf));
			if (audio_size < 0)
			{
				LOGI("error, output silence\n");
				audio_buf_size = SDL_AUDIO_BUFFER_SIZE;
				memset(audio_buf, 0, audio_buf_size);
			}
			else
			{
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if (len1 > len)
		{
			len1 = len;
		}
		memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
}

int audioDecodeFrame(AVCodecContext *aCodecCtx, uint8_t *audio_buf,
		int buf_size)
{
	static AVPacket *pkt = (AVPacket *) av_malloc(sizeof(AVPacket));
	av_init_packet(pkt);
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;

	int len1, data_size, ret = 0;

	static AVFrame *pFrame;
	pFrame = av_frame_alloc();

	for (;;)
	{
		while (audio_pkt_size > 0)
		{
			data_size = buf_size;
			len1 = avcodec_decode_audio4(aCodecCtx, pFrame, &ret, pkt);
			if (len1 < 0)
			{
				LOGI("error:%s\n", SDL_GetError());
				audio_pkt_size = 0;
				break;
			}
			data_size = AudioResampling(aCodecCtx, pFrame, AV_SAMPLE_FMT_S16,
					2, 44100, audio_buf);
			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			if (data_size <= 0)//No data yet, get more frames
				continue;
			return data_size;
		}
		if (pkt->data)
		{
			av_free_packet(pkt);
		}
		LOGI("get audio pkt");
		if (pDCInfo->mAudioQue.packetQueueGet(pkt, 1) < 0)
		{
			LOGI("error, can't get packet from the queue");
			return -1;
		}
		LOGI("pkt->size=%d",pkt->size);
		audio_pkt_data = pkt->data;
		audio_pkt_size = pkt->size;
	}
}

int AudioResampling(AVCodecContext * audio_dec_ctx,
		AVFrame * pAudioDecodeFrame, int out_sample_fmt, int out_channels,
		int out_sample_rate, uint8_t* out_buf)
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
		//LOGI("dst_ch_layout: AV_CH_LAYOUT_MONO\n");
	}
	else if (out_channels == 2)
	{
		dst_ch_layout = AV_CH_LAYOUT_STEREO;
		//LOGI("dst_ch_layout: AV_CH_LAYOUT_STEREO\n");
	}
	else
	{
		dst_ch_layout = AV_CH_LAYOUT_SURROUND;
		//LOGI("dst_ch_layout: AV_CH_LAYOUT_SURROUND\n");
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

	memcpy(out_buf, dst_data[0], resampled_data_size);

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
