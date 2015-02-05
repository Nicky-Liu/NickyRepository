/*
 * PktQueue.cpp
 *
 *  Created on: 2014-8-13
 *      Author: Administrator
 */

#include "Header/PktQueue.h"

PktQueue::PktQueue()
{
	memset(this, 0, sizeof(PktQueue));
	this->mMutex = SDL_CreateMutex();
	this->mCond = SDL_CreateCond();
}
PktQueue::~PktQueue()
{
	SDL_DestroyMutex(this->mMutex);
	SDL_DestroyCond(this->mCond);
}
int PktQueue::packetQueuePut(AVPacket *pkt)
{
	AVPacketList *pkt1;
	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
	{
		return -1;
	}
	pkt1->pkt = *pkt;
	pkt1->next = NULL;

	SDL_LockMutex(this->mMutex);

	if (!this->mLastPkt)
	{
		this->mFirstPkt = pkt1;
	}
	else
	{
		this->mLastPkt->next = pkt1;
	}
	this->mLastPkt = pkt1;
	this->mSize += pkt1->pkt.size;
	SDL_CondSignal(this->mCond);
	SDL_UnlockMutex(this->mMutex);
	return 0;
}
int PktQueue::packetQueueGet(AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;
	SDL_LockMutex(this->mMutex);
	for (;;)
	{
		if (this->mAbortRequest)
		{
			ret = -1; // 异常
			break;
		}

		pkt1 = this->mFirstPkt;
		if (pkt1)
		{
			this->mFirstPkt = pkt1->next;
			if (!this->mFirstPkt)
			{
				this->mLastPkt = NULL;
			}
			this->mSize -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block)// 阻塞标记，1(阻塞模式)，0(非阻塞模式)
		{
			ret = 0; // 非阻塞模式，没东西直接返回0
			break;
		}
		else
		{
			SDL_CondWait(this->mCond, this->mMutex);
		}
	}
	SDL_UnlockMutex(this->mMutex);
	return ret;
}


void PktQueue::packetQueueEnd()
{
	AVPacketList *pkt, *pkt1;

	SDL_LockMutex(this->mMutex);
	for (pkt = this->mFirstPkt; pkt != NULL; pkt = pkt1)
	{
		pkt1 = pkt->next;
		av_free_packet(&pkt->pkt);
		av_freep(&pkt);
	}
	this->mLastPkt = NULL;
	this->mFirstPkt = NULL;
	this->mSize = 0;
	SDL_UnlockMutex(this->mMutex);
	SDL_DestroyMutex(this->mMutex);
	SDL_DestroyCond(this->mCond);
}

void PktQueue::packetQueueAbort()
{
	SDL_LockMutex(this->mMutex);
	this->mAbortRequest = 1;
	SDL_CondSignal(this->mCond);
	SDL_UnlockMutex(this->mMutex);
}

int PktQueue::getQueueSize()
{
	return this->mSize;
}
