#include "Process.h"

Process::Process()
{

    window = nullptr;
    bmp = nullptr;
    renderer = nullptr;

    frame = nullptr;
    displayFrame = nullptr;


    frame_timer = 0.0;
    frame_last_delay = 0.0;
    frame_last_pts = 0.0;
    video_clock = 0.0;

    rect.x = 0;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0;
}

Process::~Process()
{
    av_frame_free(&frame);
    av_free(displayFrame->data[0]);
    av_frame_free(&displayFrame);
    //关闭流媒体文件解析
    if (m_pAVFormatCtx != nullptr)
    {
        avformat_close_input(&m_pAVFormatCtx);
        m_pAVFormatCtx = nullptr;
    }

    //关闭视频解码器
    if (m_pVidDecodeCtx != nullptr)
    {
        avcodec_close(m_pVidDecodeCtx);
        avcodec_free_context(&m_pVidDecodeCtx);
        m_pVidDecodeCtx = nullptr;
    }
    //关闭音频解码器
    if (m_pAudDecodeCtx != nullptr)
    {
        avcodec_close(m_pAudDecodeCtx);
        avcodec_free_context(&m_pAudDecodeCtx);
        m_pAudDecodeCtx = nullptr;
    }
}

static double r2d(AVRational r)
{
    return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}
bool Process::Open(string FilePath)
{
    bool re_value = true;
    int res = 0;
    cout << "Demux: " << FilePath << endl;
    AVDictionary* format_ops = nullptr;
    av_dict_set(&format_ops, "probesize", "32", 0);

    re_value = avformat_open_input(&m_pAVFormatCtx, FilePath.c_str(), NULL, &format_ops);
    if (re_value)//return 0 success
    {
        char err_buffer[1024] = { 0 };
        av_strerror(re_value, err_buffer, sizeof(err_buffer));
        cout << "Demux: " << "open " << FilePath << "fail: " << err_buffer;
        Close();
        return false;
    }
    else
    {
       cout << "Demux: " << "success to avformat_open_input" << endl;
    }

    res = avformat_find_stream_info(m_pAVFormatCtx, NULL);
    if (res < 0)
    {
        char errBuf[1024] = { 0 };
        av_strerror(res, errBuf, sizeof(errBuf));
        cout << "Demux: " << "open " << FilePath << " failed! :" << errBuf;
        Close();
        return false;
    }
    else if(res == AVERROR_EOF)
    {
        cout << "Demux " << "reached to file end" << endl;;
    }
    else
    {
        cout << "Demux: " << "success to avformat_find_stream_info" <<endl;
    }
    //打印音视频流信息
    cout << "================打印音视频流信息========================"<< endl;
    cout << "Demux: " << "duration: " << m_pAVFormatCtx->duration << endl;//直接获取获取不到，采用探测
    int64_t totalMs = m_pAVFormatCtx->duration / (AV_TIME_BASE / 1000);
    cout << "Demux: " << "totalMs: " << totalMs << "ms" << endl;
    cout << "Demux: " << "nb_stream: " << m_pAVFormatCtx->nb_streams << endl;
    for (unsigned int i = 0; i < m_pAVFormatCtx->nb_streams; i++)
    {
        cout << "Demux: " << "stream codec type: " << m_pAVFormatCtx->streams[i]->codecpar->codec_type << endl;
    }
    cout << "Demux: " << "iformat name: " << m_pAVFormatCtx->iformat->name << endl;
    cout << "Demux: " << "iformat long name: " << m_pAVFormatCtx->iformat->long_name << endl;
    cout << "======================================================" << endl;
    m_nVidStreamIndex = av_find_best_stream(m_pAVFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (m_nVidStreamIndex == -1)
    {
        cout << "Demux: " << "fail to find videostream";
        Close();
        return false;
    }
    else
    {
        cout << "Demux: " << "success to find videostream: " << "videoindex: " << m_nVidStreamIndex << endl;
    }
    //打印视频流相关信息
    cout << "================打印视频流信息========================" << endl;
    cout << "Demux: " << "codec_id = " << m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar->codec_id << endl;
    cout << "Demux: " << "format = " << m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar->format << endl;
    cout << "Demux: " << "width=" << m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar->width << endl;
    cout << "Demux: " << "height=" << m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar->height << endl;

    cout << "Demux: " << "video fps = " << r2d(m_pAVFormatCtx->streams[m_nVidStreamIndex]->avg_frame_rate) << endl;

    cout << "======================================================" << endl;

    m_nAudStreamIndex = av_find_best_stream(m_pAVFormatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (m_nAudStreamIndex == -1)
    {
        cout << "Demux: " << "fail to find audiostream";
        Close();
        return false;
    }
    else
    {
        cout << "Demux: " << "success to find audiostream: " << "audioindex: " << m_nAudStreamIndex << endl;
    }
    //打印音频流相关信息
    cout << "================打印音频流信息========================" << endl;
    cout << "Demux: " << "codec_id = " << m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->codec_id << endl;
    cout << "Demux: " << "format = " << m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->format << endl;
    cout << "Demux: " << "sample_rate = " << m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->sample_rate << endl;
    // AVSampleFormat;
    cout << "Demux: " << "channels = " << m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->ch_layout.nb_channels << endl;
    // 一帧数据 单通道样本数
    cout << "Demux: " << "frame_size = " << m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->frame_size << endl;
    cout << "======================================================" << endl;;


//创建视频解码器并且打开
    const AVCodec* video_codec = nullptr;
    //获取avcodec
    video_codec = avcodec_find_decoder(m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar->codec_id);
    if (video_codec == nullptr)
    {
        cout << "Demux: " << "can not find video codec" << endl;
        Close();
        return false;
    }
    m_pVidDecodeCtx = avcodec_alloc_context3(video_codec);
    if (m_pVidDecodeCtx == nullptr)
    {
        cout << "Demux: " << "fail to avcodec_alloc_context3" << endl;
        Close();
        return false;
    }
    res = avcodec_parameters_to_context(m_pVidDecodeCtx, m_pAVFormatCtx->streams[m_nVidStreamIndex]->codecpar);
    if (res != 0)
    {
        cout << "Demux: " << "fail to avcodec_parameters_to_context" << endl;
        Close();
        return false;
    }
    res = avcodec_open2(m_pVidDecodeCtx, NULL, NULL);
    if (res != 0)
    {
        cout << "Demux: " << "fail to avcodec_open2" << endl;
        Close();
        return false;
    }
    else
    {
        cout << "success to open VideoDecoder" << endl;
    }

//创建音频解码器并且打开
    const AVCodec* audio_codec = nullptr;
    //获取avcodec
    audio_codec = avcodec_find_decoder(m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar->codec_id);
    if (audio_codec == nullptr)
    {
        cout << "Demux: " << "can not find video codec" << "line: " <<  __LINE__ << endl;
        Close();
        return false;
    }
    m_pAudDecodeCtx = avcodec_alloc_context3(audio_codec);
    if (m_pAudDecodeCtx == nullptr)
    {
        cout << "Demux: " << "fail to avcodec_alloc_context3" << "line: " << __LINE__ << endl;
        Close();
        return false;
    }
    res = avcodec_parameters_to_context(m_pAudDecodeCtx, m_pAVFormatCtx->streams[m_nAudStreamIndex]->codecpar);
    if (res != 0)
    {
        cout << "Demux: " << "fail to avcodec_parameters_to_context" << "line: " << __LINE__ << endl;
        Close();
        return false;
    }
    res = avcodec_open2(m_pAudDecodeCtx, NULL, NULL);
    if (res != 0)
    {
        cout << "Demux: " << "fail to avcodec_open2" << "line: " << __LINE__ << endl;
        Close();
        return false;
    }
    else
    {
        cout << "success to open AudioDecoder" << endl;
    }

    return true;
}

void Process::Close()
{
	//关闭流媒体文件解析
	if (m_pAVFormatCtx != nullptr)
	{
		avformat_close_input(&m_pAVFormatCtx);
		m_pAVFormatCtx = nullptr;
	}

	//关闭视频解码器
	if (m_pVidDecodeCtx != nullptr)
	{
		avcodec_close(m_pVidDecodeCtx);
		avcodec_free_context(&m_pVidDecodeCtx);
		m_pVidDecodeCtx = nullptr;
	}
	//关闭音频解码器
	if (m_pAudDecodeCtx != nullptr)
	{
		avcodec_close(m_pAudDecodeCtx);
		avcodec_free_context(&m_pAudDecodeCtx);
		m_pAudDecodeCtx = nullptr;
	}
}

void Process::video_display(Process* media)
{
    int width = 800;
    int height = 600;
    // 创建sdl窗口
    window = SDL_CreateWindow("FFmpeg Decode", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        width, height, SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, 0);
    bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
        width, height);

    rect.x = 0;
    rect.y = 0;
    rect.w = width;
    rect.h = height;

    frame = av_frame_alloc();
    displayFrame = av_frame_alloc();

    displayFrame->format = AV_PIX_FMT_YUV420P;
    displayFrame->width = width;
    displayFrame->height = height;

    int numBytes = av_image_get_buffer_size((AVPixelFormat)displayFrame->format, displayFrame->width, displayFrame->height,1);
    uint8_t* buffer = (uint8_t*)av_malloc(numBytes * sizeof(uint8_t));

    av_image_fill_arrays(displayFrame->data, displayFrame->linesize, buffer, (AVPixelFormat)displayFrame->format, displayFrame->width, displayFrame->height,1);
    SDL_CreateThread(decode, "", this);

    schedule_refresh(media, 40); // start display

}

double Process::synchronize(AVFrame* srcFrame, double pts)
{
    double frame_delay;

    if (pts != 0)
    {
        video_clock = pts; // Get pts,then set video clock to it
    }
    else
    {
        pts = video_clock; // Don't get pts,set it to video clock
    }
    //根据时间基，计算每一帧的间隔时间
    frame_delay = av_q2d(m_pAVFormatCtx->streams[m_nVidStreamIndex]->time_base);
    //解码后的帧要延时的时间
    frame_delay += srcFrame->repeat_pict * (frame_delay * 0.5);
    video_clock += frame_delay;//得到video_clock,实际上也是预测的下一帧视频的时间

    return pts;
}

AVFormatContext* Process::Get_format()
{
    // TODO: 在此处插入 return 语句
    return m_pAVFormatCtx;
}

uint32_t Process::Get_VideoIndex()
{
    return m_nVidStreamIndex;
}

uint32_t Process::Get_AudioIndex()
{
    return m_nAudStreamIndex;
}

PacketQueue& Process::GetVideo_queue()
{
    // TODO: 在此处插入 return 语句
    return video_queue;
}

PacketQueue& Process::GetAudio_queue()
{
    // TODO: 在此处插入 return 语句
    return audio_queue;
}

FrameQueue& Process::GetVideo_Frame_queue()
{
    // TODO: 在此处插入 return 语句
    return frameq;
}



AVCodecContext* Process::Get_video_codecContext()
{
    return m_pVidDecodeCtx;
}

AVCodecContext* Process::Get_audio_codecContext()
{
    return m_pAudDecodeCtx;
}

uint32_t Process::Get_frame_nb_frames()
{
    return frameq.Get_nb_frames();
}

double Process::Get_video_frame_timer()
{
    return frame_timer;
}

AVFrame* Process::Get_video_frame()
{
    return frame;
}

AVFrame* Process::Get_video_displayFrame()
{
    return displayFrame;
}

SDL_Renderer* Process::Get_renderer()
{
    return renderer;
}

SDL_Texture* Process::Get_bmp()
{
    return bmp;
}

SDL_Rect& Process::Get_rect()
{
    // TODO: 在此处插入 return 语句
    return rect;
}

int decode_thread(void* data)
{
    Process* media = (Process*)data;

    AVFormatContext* format = media->Get_format();

    AVPacket* packet = av_packet_alloc();

    while (true)
    {
        int ret = av_read_frame(format,packet);
        if (ret < 0)
        {
            if (ret == AVERROR_EOF)
            {
                cout << "count: " << media->GetVideo_queue().Get_nb_packets() << endl;
                break;
            }

            if (format->pb->error == 0)//NO error,wait for user input
            {
                SDL_Delay(100);
                continue;
            }
            else
            {
                cout << "[decode_thread]: read packet fail " << " line: " << __LINE__ << endl;
                break;
            }
        }
        if (packet->stream_index == media->Get_VideoIndex())
        {
            media->GetVideo_queue().InQueue(packet);
            av_packet_unref(packet);
        }
        else if (packet->stream_index == media->Get_AudioIndex())
        {
            media->GetAudio_queue().InQueue(packet);
            av_packet_unref(packet);
        }
        else
        {
            av_packet_unref(packet);
        }

    }

    av_packet_free(&packet);

    return 0;
}

int decode(void* arg)
{
    Process* video = (Process*)arg;
    AVFrame* frame = av_frame_alloc();

    AVPacket packet;
    double pts;
    while (true)
    {
        video->GetVideo_queue().OutQueue(&packet,true);

        int ret = avcodec_send_packet(video->Get_video_codecContext(), &packet);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
        {
            continue;
        }
        ret = avcodec_receive_frame(video->Get_video_codecContext(), frame);
        if (ret < 0 && ret != AVERROR_EOF)
        {
            continue;
        }
        if ((pts = (double)frame->best_effort_timestamp) == AV_NOPTS_VALUE)
        {
            pts = 0;
        }
        pts *= av_q2d(video->Get_video_codecContext()->time_base);//时间基换算，单位为秒
        pts = video->synchronize(frame, pts);

        frame->opaque = &pts;

        if (video->Get_frame_nb_frames() >= FrameQueue::capacity)
        {
            SDL_Delay(500 * 2);
        }

        video->GetVideo_Frame_queue().InQueue(frame);

        av_frame_unref(frame);
    }

    av_frame_free(&frame);

    return 0;
}

void schedule_refresh(Process* media, int delay)
{
    SDL_AddTimer(delay, sdl_refresh_timer_cb, media);

}

uint32_t sdl_refresh_timer_cb(uint32_t interval, void* opaque)
{

    SDL_Event event;
    event.type = FF_REFRESH_EVENT;
    event.user.data1 = opaque;
    SDL_PushEvent(&event);
    return 0; /* 0 means stop timer */
}

void video_refresh_timer(void* userdata)
{
    Process* video = (Process*)userdata;

    if (video->GetVideo_queue().Get_Queuesize())
    {
        schedule_refresh(video, 1);
    }
    else
    {
        AVFrame* frame = video->Get_video_frame();
        video->GetVideo_Frame_queue().OutQueue(&frame);

        double actual_delay = video->Get_video_frame_timer() - static_cast<double>(av_gettime()) / 1000000.0;
        if (actual_delay <= 0.010)
        {
            actual_delay = 0.010;
        }
        schedule_refresh(video, static_cast<int>(actual_delay * 1000 + 0.5));

        SwsContext* sws_ctx = sws_getContext(video->Get_video_codecContext()->width, video->Get_video_codecContext()->height, video->Get_video_codecContext()->pix_fmt,
            video->Get_video_displayFrame()->width, video->Get_video_displayFrame()->height, (AVPixelFormat)video->Get_video_displayFrame()->format, SWS_BILINEAR, nullptr, nullptr, nullptr);
        
        sws_scale(sws_ctx, (uint8_t const* const*)video->Get_video_frame()->data, video->Get_video_frame()->linesize, 0,
            video->Get_video_codecContext()->height, video->Get_video_displayFrame()->data, video->Get_video_displayFrame()->linesize);
    
        // Display the image to screen
        SDL_UpdateTexture(video->Get_bmp(), &(video->Get_rect()), video->Get_video_displayFrame()->data[0], video->Get_video_displayFrame()->linesize[0]);
        SDL_RenderClear(video->Get_renderer());
        SDL_RenderCopy(video->Get_renderer(), video->Get_bmp(), &video->Get_rect(), &video->Get_rect());
        SDL_RenderPresent(video->Get_renderer());

        sws_freeContext(sws_ctx);
        av_frame_unref(video->Get_video_frame());
    }

}