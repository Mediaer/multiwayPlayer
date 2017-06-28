#define LOG_TAG "cmcc_player"
#include <stdint.h>
#include "CMCC_MediaProcessorImpl.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <utils/Log.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/DataSource.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/MediaDefs.h>
#include <media/stagefright/NuMediaExtractor.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/Surface.h>
#include <ui/DisplayInfo.h>
#include <cutils/properties.h>
//#include <CTC_MediaControl.h>
#define kReadSize (254*1024)
#define MAX_INSTANCE 10




#define DEBUG_VIDEO_INFO
#ifdef DEBUG_VIDEO_INFO

#define MAX_WAY atoi(argv[1])

#ifdef __cplusplus
extern "C"
{
#endif

#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"

#ifdef __cplusplus
};
#endif


/**
 * Get the base information for video and audio.
 *
 * @param filename the name of video to be parsered
 * @param vpamr pointer to the struct of video information defined in MediaCodec.h
 * @param apamr pointer to the struct of audio information defined in MediaCodec.h
 * @return whether get the base information of the video or not
 */
static int get_info_av(const char *filename, VIDEO_PARA_T* vpamr, AUDIO_PARA_T* apamr)
{
	ALOGD("TS_PARSER : Beginning to do get_info_av()......");

	AVFormatContext *pFormatCtx;
    int             i, videoindex;
	int             audioindex;
	double          fps;
    AVCodecContext  *pCodecCtx, *aCodecCtx;
    AVCodec         *pCodec, *aCodec;
	AVStream        *st_video, *st_audio;
	const char      *vfmt_name, *afmt_name;

	av_register_all();
    avformat_network_init();

	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx, filename, NULL, NULL) != 0) {
        ALOGD("TS_PARSER : Couldn't open input stream.\n");
        return -1;
    }

	if(avformat_find_stream_info(pFormatCtx, NULL)<0){
        ALOGD("TS_PARSER : Couldn't find stream information.\n");
        return -1;
    }

	videoindex = -1;
	audioindex = -1;
	for (i=0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
		} else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
		    audioindex = i;
		}
	}

	if (videoindex == -1) {
		ALOGD("TS_PARSER : Didn't find a video stream.\n");
		return -1;
	} else if (audioindex == -1) {
	    ALOGD("TS_PARSER : Didn't find a audio stream.");
		return -1;
	}

	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

	aCodecCtx = pFormatCtx->streams[audioindex]->codec;
	aCodec = avcodec_find_encoder(aCodecCtx->codec_id);

    if(pCodec == NULL){
        ALOGD("TS_PARSER : Video Codec not found.\n");
        //return -1;
    } else if (aCodec == NULL) {
        ALOGD("TS_PARSER : Audio Codec not found.\n");
        //return -1;
    }

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		ALOGD("TS_PARSER : Could not open video codec ! \n");
		//return -1;
	} else if (avcodec_open2(aCodecCtx, aCodec, NULL) < 0) {
		ALOGD("TS_PARSER : Could not open audio codec ! \n");
		//return -1;
	}

	av_dump_format(pFormatCtx, 0, filename, 0);

    st_video = pFormatCtx->streams[videoindex];
	st_audio = pFormatCtx->streams[audioindex];

    vfmt_name = st_video->codec->codec->name;
	fps = av_q2d(st_video->avg_frame_rate);
    ALOGD("codec= %s, width= %d, height= %d, pid = %d, fps = %f,",
               vfmt_name, st_video->codec->coded_width,
               st_video->codec->coded_height, st_video->id, fps);

    /* pass video information */
	if (strcmp(vfmt_name, "h264") == 0) {
        vpamr->vFmt = VIDEO_FORMAT_H264;
	} else if (strcmp(vfmt_name, "hevc") == 0) {
	    vpamr->vFmt = VIDEO_FORMAT_H265;
	}

	vpamr->pid = st_video->id;
	vpamr->nVideoWidth = st_video->codec->coded_width;
	vpamr->nVideoHeight = st_video->codec->coded_height;
	vpamr->nFrameRate = fps;
	vpamr->nExtraSize = 0;
	vpamr->pExtraData = NULL;

    /* pass audio information */
    afmt_name = avcodec_get_name(aCodecCtx->codec_id);
	ALOGD("codec = %s, sample_rate = %d, channels = %d, pid = %d, extradata_size = %d, block_align = %d, bit_rate = %d",
               afmt_name, st_audio->codec->sample_rate,st_audio->codec->channels, st_audio->id,
               st_audio->codec->extradata_size, st_audio->codec->block_align, st_audio->codec->bit_rate/1000);

    if (strcmp(afmt_name, "ac3") == 0) {
		apamr->aFmt = AUDIO_FORMAT_AC3;
    } else if (strcmp(afmt_name, "aac") == 0) {
        apamr->aFmt = AUDIO_FORMAT_AAC;
    }

	apamr->pid = st_audio->id;
	apamr->nChannels = st_audio->codec->channels;
	apamr->nSampleRate = st_audio->codec->sample_rate;
	apamr->block_align = st_audio->codec->block_align;
	apamr->bit_per_sample = (st_audio->codec->bit_rate) /1000;
	apamr->nExtraSize = st_audio->codec->extradata_size;
	apamr->pExtraData = NULL;

	ALOGD("TS_PARSER : Stopping to do get_info_av()......\n");

    /* free memory for format and codec */
    avcodec_close(pCodecCtx);
	avcodec_close(aCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}
#endif



unsigned char* mBuf[MAX_INSTANCE];// = (unsigned char*) malloc(sizeof(unsigned char) * kReadSize);
//VIDEO_PARA_T vidPara = {256, 640, 480, 24, VFORMAT_H264, 0};
//VIDEO_PARA_T vidPara = {256, VIDEO_FORMAT_H264, 640, 480, 24, 0, NULL};
//AUDIO_PARA_T audPara = {156,AUDIO_FORMAT_AAC,2,48000,512,32,0,NULL};
#if 0
//VIDEO_PARA_T vidPara = {4113, VIDEO_FORMAT_H264, 720, 576, 25, 0, NULL};
//AUDIO_PARA_T audPara = {4352, AUDIO_FORMAT_AAC,2,48000,512,32,0,NULL};
#define FILE_NAME  "/sdcard/demo_video/test%d.ts"
#else
//VIDEO_PARA_T vidPara = {256, VIDEO_FORMAT_H264, 720, 574, 23.98,  0,  NULL};
//AUDIO_PARA_T audPara = {257, AUDIO_FORMAT_AAC, 2, 48000, 512, 318, 0, NULL};
#define FILE_NAME "/sdcard/demo_video/Live%d.ts"
#endif




char filename[MAX_INSTANCE][256];
int active_bitmask = 0x0;
pthread_t tid[MAX_INSTANCE];

int x = 100, y = 100, w = 640, h = 360;
int last_active_index = 0;
CMCC_MediaProcessor* player[MAX_INSTANCE];
int mSourceFD[MAX_INSTANCE];
int my_idx = 0;
int idx[MAX_INSTANCE];
int cur_index  = 0 , new_index = 0;



void* doWrite(void *arg)
{
  int audio = 0,switchFlag = 0,count = 0;
    int i = *(static_cast<int*>(arg));
 struct timeval start_time , end_time;
 if (i == 0) {
 gettimeofday(&start_time ,NULL);
 ALOGD("instance : %d----first_start_time.tv_sec :%ld",
			i,  start_time.tv_sec);
		}
    while (true)
	{
         if (i == 0) {
			gettimeofday(&end_time ,NULL);
		}

        int ret = read(mSourceFD[i], mBuf[i], kReadSize);
        if (ret == 0) {
           ALOGD("EOF, rewind");
             lseek(mSourceFD[i], 0, SEEK_SET);
             continue;
            //break;
        }
        player[i]->WriteData(PLAYER_STREAMTYPE_TS, mBuf[i], ret, 0L);
        usleep(25 * 1000);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    ALOGD("start cmcc_test");

#ifdef DEBUG_VIDEO_INFO
    int ret;

    if (argc < 3) {
        printf("Usage : %s [ways of video] [video_0] [video_1] ... [video_n]\n", argv[0]);
		return -1;
    } else if ( (argc - MAX_WAY) != 2) {
        printf("Videos to be played are not equal to %d ways!\n", MAX_WAY);
		return -1;
    }

    VIDEO_PARA_T *vidPara;
	vidPara = (VIDEO_PARA_T *)malloc(sizeof(VIDEO_PARA_T) * MAX_WAY);
    AUDIO_PARA_T *audPara;
	audPara = (AUDIO_PARA_T *)malloc(sizeof(AUDIO_PARA_T) * MAX_WAY);

	struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int k = 0; k < MAX_WAY; k++) {
        ret = get_info_av(argv[k+2], vidPara+k, audPara+k);
		if (ret != 0) {
			ALOGD("TS_PARSER: Parser video(%s) information failed!", argv[k+2]);
            return -1;
		}
    }
    gettimeofday(&end, NULL);
    long time_flow = 1000000 * ( end.tv_sec - start.tv_sec ) + end.tv_usec - start.tv_usec;
	ALOGD("TS_PARSER : Time using of get_info_av() : %f", time_flow / 1000000.0);
    ALOGD("TS_PARSER : End get_info_av()....");
    ALOGD("TS_PARSER : Begin to create threads for player....");
#endif

    for (int i = 0; i < MAX_WAY; i++) {
       // player[i] = GetMediaControl(i);
        player[i] = new CMCC_MediaProcessorImpl();
        player[i]->InitVideo(vidPara+i);
        player[i]->InitAudio(audPara+i);
        void *ptr = NULL;
        player[i]->SetSurfaceTexture(ptr);
      //  player[i]->SetVideoWindow((i%3)*640, (i/3)*360, 640, 360);
        player[i]->StartPlay();

      //  sprintf(filename[i], "/sdcard/cmcc_test_video/test_h264_aac.ts", i);

#ifdef DEBUG_VIDEO_INFO
        sprintf(filename[i], argv[i+2], i);
#endif

        ALOGD("filename: %s",filename[i]);
        mSourceFD[i] = open(filename[i], O_RDONLY);
        ALOGD("open file %s result: %s",filename[i], strerror(errno));

        mBuf[i] = (unsigned char*) malloc(sizeof(unsigned char) * kReadSize);
        idx[i] = i;
   //     pthread_create(&(tid[i]), NULL, doWrite, (int *)(&idx[i]));

        pthread_create(&(tid[i]), NULL, doWrite, &idx[i]);
     //   usleep(i * 1000);
    }

    ALOGD("[%s:%d]", __FUNCTION__, __LINE__);
    while (getchar() != 'q')
		sleep(10*1000);

#ifdef DEBUG_VIDEO_INFO
	/* free video and audio infomation buffers */
	free(vidPara);
	free(audPara);
	vidPara = NULL;
	audPara = NULL;
#endif

    int i = 0;
    while (i < MAX_INSTANCE) {
      free(mBuf[i]);
      mBuf[i] = NULL;
      close(mSourceFD[i]);
      mSourceFD[i] = NULL;
	  i++;
    }
    return 0;
}


