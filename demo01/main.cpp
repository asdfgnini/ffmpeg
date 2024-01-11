

#include <iostream>
using namespace std;

#include "Process.h"



bool quit = false;

int main(int argc,char* agrv[])
{
	Process test;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);

	//bool res = test.Open("C:\\Users\\Administrator\\Desktop\\vs2022\\ffmpeg\\demo01\\test.mp4");
	bool res = test.Open("F:\\QML\\dove_640x360.mp4");
	if (res)
	{
		SDL_CreateThread(decode_thread, "", &test); // 创建视频解码线程，读取packet到队列中缓存
	}
	else
	{
		cout << "[main]: media Open fail" << " line: " << __LINE__ << endl;
		exit(-1);
	}
	//视频渲染线程
	test.video_display(&test);

	SDL_Event event;
	while (true) // SDL event loop
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
		case FF_QUIT_EVENT:
		case SDL_QUIT:
			quit = 1;
			SDL_Quit();

			return 0;
			break;

		case FF_REFRESH_EVENT:
			video_refresh_timer(&test);
			break;

		default:
			break;
		}
	}

	return 0;
}