#pragma once



#define FF_REFRESH_EVENT (SDL_USEREVENT)
#define FF_QUIT_EVENT (SDL_USEREVENT + 1)


//ffmpeg
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixfmt.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>

}

//SDL
#include <SDL.h>
#include <SDL_thread.h>

//C++
#include <iostream>
#include <string>
#include <cstdio>
#include <thread>
#include <queue>
#include <mutex>

//user
#include "PacketQueue.h"
#include "FrameQueue.h"



using namespace std;

class Process
{
public:
	Process();
	~Process();

public:
	bool Open(string FilePath);//打开流媒体文件,打开对应的视频和音频解码器
	void Close();//关闭流媒体文件解析和关闭对应的解析器
	void video_display(Process* media);//视频渲染线程函数
	double synchronize(AVFrame* srcFrame, double pts);


public:
	AVFormatContext* Get_format();
	uint32_t Get_VideoIndex();
	uint32_t Get_AudioIndex();

	PacketQueue& GetVideo_queue();
	PacketQueue& GetAudio_queue();

	FrameQueue& GetVideo_Frame_queue();


	AVCodecContext* Get_video_codecContext();
	AVCodecContext* Get_audio_codecContext();

	uint32_t Get_frame_nb_frames();
	double Get_video_frame_timer();

	AVFrame* Get_video_frame();
	AVFrame* Get_video_displayFrame();

	SDL_Renderer* Get_renderer();
	SDL_Texture* Get_bmp();
	SDL_Rect& Get_rect();
private:

	AVFormatContext* m_pAVFormatCtx = NULL;//流文件解析上下文
	AVCodecContext* m_pVidDecodeCtx = NULL;//视频解码器上下文
	uint32_t m_nVidStreamIndex = -1;	   //视频流索引值
	AVCodecContext* m_pAudDecodeCtx = NULL;//音频解码器上下文
	uint32_t m_nAudStreamIndex = -1;	   //音频流索引值

	//video
	PacketQueue video_queue; //视频解码包队列

	FrameQueue frameq;          // 保存解码后的原始帧数据
	AVFrame* frame;
	AVFrame* displayFrame;

	double frame_timer;         // Sync fields
	double frame_last_pts;
	double frame_last_delay;
	double video_clock;

	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* bmp;
	SDL_Rect rect;
	
	//audio
	PacketQueue audio_queue; //音频解码包队列





};
//SDL线程全局函数
int decode_thread(void* data);//分包
int decode(void* arg);//解码视频包

// 延迟delay ms后刷新video帧
void schedule_refresh(Process* media, int delay);
uint32_t sdl_refresh_timer_cb(uint32_t interval, void* opaque);
void video_refresh_timer(void* userdata);

