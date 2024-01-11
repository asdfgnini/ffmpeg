#pragma once

extern "C" {
#include <libavcodec/avcodec.h>
}
#include <SDL.h>
#include <queue>

class PacketQueue
{
public:

	PacketQueue();
	bool InQueue(const AVPacket* packet);
	bool OutQueue(AVPacket* packet, bool block);
public:
	uint32_t Get_nb_packets();
	bool Get_Queuesize();
private:
	uint32_t nb_packets;
	uint32_t size;
	SDL_mutex* mutex;
	SDL_cond* cond;
	std::queue<AVPacket> queue;
};

