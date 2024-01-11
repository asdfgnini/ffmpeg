#include <iostream>
using namespace std;

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

}

int main()
{
	unsigned codecVer = avcodec_version();
	int ver_major, ver_minor, ver_micro;
	ver_major = (codecVer >> 16) & 0xff;
	ver_minor = (codecVer >> 8) & 0xff;
	ver_micro = (codecVer) & 0xff;
	cout << "FFmpeg version is: " << av_version_info() << " ,avcodec version is: " << avcodec_version()\
		<< "=" << ver_major << "." << ver_minor <<"." << ver_micro;




	return 0;	
}