#ifdef __cplusplus
extern "C" {
#endif

// BASS_ChannelGetData flags
#define BASS_DATA_AVAILABLE	0			// query how much data is buffered
#define BASS_DATA_NOREMOVE	0x10000000	// flag: don't remove data from recording buffer
#define BASS_DATA_FIXED		0x20000000	// unused
#define BASS_DATA_FLOAT		0x40000000	// flag: return floating-point sample data
#define BASS_DATA_FFT256	0x80000000	// 256 sample FFT
#define BASS_DATA_FFT512	0x80000001	// 512 FFT
#define BASS_DATA_FFT1024	0x80000002	// 1024 FFT
#define BASS_DATA_FFT2048	0x80000003	// 2048 FFT
#define BASS_DATA_FFT4096	0x80000004	// 4096 FFT
#define BASS_DATA_FFT8192	0x80000005	// 8192 FFT
#define BASS_DATA_FFT16384	0x80000006	// 16384 FFT
#define BASS_DATA_FFT32768	0x80000007	// 32768 FFT
#define BASS_DATA_FFT_INDIVIDUAL 0x10	// FFT flag: FFT for each channel, else all combined
#define BASS_DATA_FFT_NOWINDOW	0x20	// FFT flag: no Hanning window
#define BASS_DATA_FFT_REMOVEDC	0x40	// FFT flag: pre-remove DC bias
#define BASS_DATA_FFT_COMPLEX	0x80	// FFT flag: return complex data
#define BASS_DATA_FFT_NYQUIST	0x100	// FFT flag: return extra Nyquist value

#define BASS_ATTRIB_FREQ			1
#define BASS_ATTRIB_VOL				2
#define BASS_ATTRIB_PAN				3
#define BASS_ATTRIB_EAXMIX			4
#define BASS_ATTRIB_NOBUFFER		5
#define BASS_ATTRIB_VBR				6
#define BASS_ATTRIB_CPU				7
#define BASS_ATTRIB_SRC				8
#define BASS_ATTRIB_NET_RESUME		9
#define BASS_ATTRIB_SCANINFO		10
#define BASS_ATTRIB_NORAMP			11
#define BASS_ATTRIB_BITRATE			12
#define BASS_ATTRIB_BUFFER			13
#define BASS_ATTRIB_GRANULE			14
#define BASS_ATTRIB_USER			15
#define BASS_ATTRIB_TAIL			16
#define BASS_ATTRIB_PUSH_LIMIT		17
#define BASS_ATTRIB_DOWNLOADPROC	18
#define BASS_ATTRIB_VOLDSP			19
#define BASS_ATTRIB_VOLDSP_PRIORITY	20
#define BASS_ATTRIB_MUSIC_AMPLIFY	0x100
#define BASS_ATTRIB_MUSIC_PANSEP	0x101
#define BASS_ATTRIB_MUSIC_PSCALER	0x102
#define BASS_ATTRIB_MUSIC_BPM		0x103
#define BASS_ATTRIB_MUSIC_SPEED		0x104
#define BASS_ATTRIB_MUSIC_VOL_GLOBAL 0x105
#define BASS_ATTRIB_MUSIC_ACTIVE	0x106
#define BASS_ATTRIB_MUSIC_VOL_CHAN	0x200 // + channel #
#define BASS_ATTRIB_MUSIC_VOL_INST	0x300 // + instrument #

#define BASS_ACTIVE_STOPPED			0
#define BASS_ACTIVE_PLAYING			1
#define BASS_ACTIVE_STALLED			2
#define BASS_ACTIVE_PAUSED			3
#define BASS_ACTIVE_PAUSED_DEVICE	4

#define BASS_CONFIG_BUFFER			0
#define BASS_CONFIG_UPDATEPERIOD	1
#define BASS_CONFIG_GVOL_SAMPLE		4
#define BASS_CONFIG_GVOL_STREAM		5
#define BASS_CONFIG_GVOL_MUSIC		6
#define BASS_CONFIG_CURVE_VOL		7
#define BASS_CONFIG_CURVE_PAN		8
#define BASS_CONFIG_FLOATDSP		9
#define BASS_CONFIG_3DALGORITHM		10
#define BASS_CONFIG_NET_TIMEOUT		11
#define BASS_CONFIG_NET_BUFFER		12
#define BASS_CONFIG_PAUSE_NOPLAY	13
#define BASS_CONFIG_NET_PREBUF		15
#define BASS_CONFIG_NET_PASSIVE		18
#define BASS_CONFIG_REC_BUFFER		19
#define BASS_CONFIG_NET_PLAYLIST	21
#define BASS_CONFIG_MUSIC_VIRTUAL	22
#define BASS_CONFIG_VERIFY			23
#define BASS_CONFIG_UPDATETHREADS	24
#define BASS_CONFIG_DEV_BUFFER		27
#define BASS_CONFIG_REC_LOOPBACK	28
#define BASS_CONFIG_VISTA_TRUEPOS	30
#define BASS_CONFIG_IOS_SESSION		34
#define BASS_CONFIG_IOS_MIXAUDIO	34
#define BASS_CONFIG_DEV_DEFAULT		36
#define BASS_CONFIG_NET_READTIMEOUT	37
#define BASS_CONFIG_VISTA_SPEAKERS	38
#define BASS_CONFIG_IOS_SPEAKER		39
#define BASS_CONFIG_MF_DISABLE		40
#define BASS_CONFIG_HANDLES			41
#define BASS_CONFIG_UNICODE			42
#define BASS_CONFIG_SRC				43
#define BASS_CONFIG_SRC_SAMPLE		44
#define BASS_CONFIG_ASYNCFILE_BUFFER 45
#define BASS_CONFIG_OGG_PRESCAN		47
#define BASS_CONFIG_MF_VIDEO		48
#define BASS_CONFIG_AIRPLAY			49
#define BASS_CONFIG_DEV_NONSTOP		50
#define BASS_CONFIG_IOS_NOCATEGORY	51
#define BASS_CONFIG_VERIFY_NET		52
#define BASS_CONFIG_DEV_PERIOD		53
#define BASS_CONFIG_FLOAT			54
#define BASS_CONFIG_NET_SEEK		56
#define BASS_CONFIG_AM_DISABLE		58
#define BASS_CONFIG_NET_PLAYLIST_DEPTH	59
#define BASS_CONFIG_NET_PREBUF_WAIT	60
#define BASS_CONFIG_ANDROID_SESSIONID	62
#define BASS_CONFIG_WASAPI_PERSIST	65
#define BASS_CONFIG_REC_WASAPI		66
#define BASS_CONFIG_ANDROID_AAUDIO	67
#define BASS_CONFIG_SAMPLE_ONEHANDLE	69
#define BASS_CONFIG_NET_META		71
#define BASS_CONFIG_NET_RESTRATE	72
#define BASS_CONFIG_REC_DEFAULT		73
#define BASS_CONFIG_NORAMP			74

#define BASS_CTYPE_SAMPLE		1
#define BASS_CTYPE_RECORD		2
#define BASS_CTYPE_STREAM		0x10000
#define BASS_CTYPE_STREAM_VORBIS	0x10002
#define BASS_CTYPE_STREAM_OGG	0x10002
#define BASS_CTYPE_STREAM_MP1	0x10003
#define BASS_CTYPE_STREAM_MP2	0x10004
#define BASS_CTYPE_STREAM_MP3	0x10005
#define BASS_CTYPE_STREAM_AIFF	0x10006
#define BASS_CTYPE_STREAM_CA	0x10007
#define BASS_CTYPE_STREAM_MF	0x10008
#define BASS_CTYPE_STREAM_AM	0x10009
#define BASS_CTYPE_STREAM_SAMPLE	0x1000a
#define BASS_CTYPE_STREAM_DUMMY		0x18000
#define BASS_CTYPE_STREAM_DEVICE	0x18001
#define BASS_CTYPE_STREAM_WAV	0x40000 // WAVE flag (LOWORD=codec)
#define BASS_CTYPE_STREAM_WAV_PCM	0x50001
#define BASS_CTYPE_STREAM_WAV_FLOAT	0x50003
#define BASS_CTYPE_MUSIC_MOD	0x20000
#define BASS_CTYPE_MUSIC_MTM	0x20001
#define BASS_CTYPE_MUSIC_S3M	0x20002
#define BASS_CTYPE_MUSIC_XM		0x20003
#define BASS_CTYPE_MUSIC_IT		0x20004
#define BASS_CTYPE_MUSIC_MO3	0x00100

typedef struct {
	uint32_t freq;
	float volume;
	float pan;
	uint32_t flags;
	uint32_t length;
	uint32_t max;
	uint32_t origres;
	uint32_t chans;
	uint32_t mingap;
	uint32_t mode3d;
	float mindist;
	float maxdist;
	uint32_t iangle;
	uint32_t oangle;
	float outvol;
	uint32_t vam;
	uint32_t priority;
} BASS_SAMPLE;

typedef struct {
    uint32_t freq;
    uint32_t chans;
    uint32_t flags;
    uint32_t ctype;
    uint32_t origres;
    void *plugin;
    void *sample;
    char *filename;
} BASS_CHANNELINFO;

typedef struct {
	uint32_t tick;
	uint32_t duration;
	void *handle;
	void *handle2;
	uint32_t play_handle;
	BASS_SAMPLE info;
	BASS_CHANNELINFO info2;
} BASS_internal_sample;

const void *BASS_GetConfigPtr(uint32_t option);
uint32_t BASS_GetVersion();
int BASS_Init(int device, uint32_t freq, uint32_t flags, int win, void *clsid);
uint32_t BASS_GetDevice();
int BASS_SetDevice(uint32_t device);
int BASS_SetConfig(uint32_t option, uint32_t value);
BASS_internal_sample *BASS_SampleLoad(uint32_t mem, void *file, uint64_t offset, uint32_t length, uint32_t max, uint32_t flags);
BASS_internal_sample *BASS_StreamCreateFile(uint32_t mem, void *file, uint64_t offset, uint64_t length, uint32_t flags);
uint32_t BASS_SampleGetInfo(BASS_internal_sample *handle, BASS_SAMPLE *info);
uint32_t BASS_ChannelGetInfo(BASS_internal_sample *handle, BASS_CHANNELINFO *info);
uint32_t BASS_SampleGetChannel(BASS_internal_sample *handle, uint32_t flags);
uint32_t BASS_ChannelSetAttribute(BASS_internal_sample *handle, uint32_t attrib, float value);
uint32_t BASS_ChannelPlay(BASS_internal_sample *handle, uint32_t restart);
uint32_t BASS_ChannelIsActive(BASS_internal_sample *handle);
uint32_t BASS_ChannelGetData(BASS_internal_sample *handle, void *buffer, uint32_t length);
uint32_t BASS_ChannelGetLength(BASS_internal_sample *handle, uint32_t mode);
double BASS_ChannelBytes2Seconds(BASS_internal_sample *handle, uint64_t pos);
uint64_t BASS_ChannelGetPosition(BASS_internal_sample *handle, uint32_t mode);
uint32_t BASS_ChannelStop(BASS_internal_sample *handle);
int BASS_ChannelPause(BASS_internal_sample *handle);

int Song_GetTotalDuration(const char *file);

#ifdef __cplusplus
}
#endif
