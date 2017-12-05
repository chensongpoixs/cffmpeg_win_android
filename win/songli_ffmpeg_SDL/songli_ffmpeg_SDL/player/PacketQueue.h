
#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H
#include <queue>
extern "C"{
#include <libavcodec\avcodec.h>
#include "sdl\SDL.h"
#include "sdl\SDL_thread.h"
}

struct PacketQueue
{
	std::queue<AVPacket> queue;

	Uint32    nb_packets;
	Uint32    size;
	SDL_mutex *mutex;   //互斥锁
	SDL_cond  *cond;    //条件变量的使用

	PacketQueue();
	bool enQueue(const AVPacket *packet);
	bool deQueue(AVPacket *packet, bool block);
};

#endif