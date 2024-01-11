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
	bool Open(string FilePath);//����ý���ļ�,�򿪶�Ӧ����Ƶ����Ƶ������
	void Close();//�ر���ý���ļ������͹رն�Ӧ�Ľ�����
	void video_display(Process* media);//��Ƶ��Ⱦ�̺߳���
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

	AVFormatContext* m_pAVFormatCtx = NULL;//���ļ�����������
	AVCodecContext* m_pVidDecodeCtx = NULL;//��Ƶ������������
	uint32_t m_nVidStreamIndex = -1;	   //��Ƶ������ֵ
	AVCodecContext* m_pAudDecodeCtx = NULL;//��Ƶ������������
	uint32_t m_nAudStreamIndex = -1;	   //��Ƶ������ֵ

	//video
	PacketQueue video_queue; //��Ƶ���������

	FrameQueue frameq;          // ���������ԭʼ֡����
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
	PacketQueue audio_queue; //��Ƶ���������





};
//SDL�߳�ȫ�ֺ���
int decode_thread(void* data);//�ְ�
int decode(void* arg);//������Ƶ��

// �ӳ�delay ms��ˢ��video֡
void schedule_refresh(Process* media, int delay);
uint32_t sdl_refresh_timer_cb(uint32_t interval, void* opaque);
void video_refresh_timer(void* userdata);

