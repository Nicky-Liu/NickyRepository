/*
 * PktQueue.h
 *
 *  Created on: 2014-8-13
 *      Author: Administrator
 */

#ifndef PKTQUEUE_H_
#define PKTQUEUE_H_
extern "C"
{
#include "libavformat/avformat.h"
#include <SDL.h>
}

class PktQueue
{
private:
	AVPacketList *mFirstPkt, *mLastPkt;
	int mSize;
	int mAbortRequest;
	SDL_mutex *mMutex;
	SDL_cond *mCond;
public:
	PktQueue();
	~PktQueue();
	int packetQueuePut(AVPacket *pkt);
	int packetQueueGet(AVPacket *pkt, int block);
	void packetQueueEnd();
	void packetQueueAbort();
	int getQueueSize();
};

#endif /* PKTQUEUE_H_ */
