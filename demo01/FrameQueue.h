#pragma once

extern "C" {
#include <libavcodec\avcodec.h>
}

#include <SDL.h>
#include <queue>



class FrameQueue
{
public:
	FrameQueue();
	bool InQueue(const AVFrame* frame);
	bool OutQueue(AVFrame **frame);
	static const int capacity = 30;
	uint32_t Get_nb_frames();
private:

	uint32_t nb_frames;
	SDL_mutex* mutex;
	SDL_cond* cond;
	std::queue<AVFrame*> queue;
};

