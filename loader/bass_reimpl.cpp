/* bass_reimpl.c -- Minimalistic libBASS reimplementation
 *
 * Copyright (C) 2023 Rinnegatamante
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
 
#include <vitasdk.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sndfile.h>
#include <mpg123.h>
#include "soloud.h"
#include "soloud_wavstream.h"
#include "soloud_wav.h"
#include "soloud_bus.h"
#include "bass_reimpl.h"

SoLoud::Soloud soloud;
#define min(a, b) (a) < (b) ? (a) : (b)

extern "C" {

typedef struct {
	uint32_t off_start;
	uint32_t off_end;
	void *buf;
} mp3_buf;

static ssize_t mp3_read(void *io, void *buffer, size_t nbyte) {
	mp3_buf *mus = (mp3_buf *)io;
	
	uint32_t end_read = min((uint32_t)mus->buf + nbyte, mus->off_end);
	uint32_t read_bytes = end_read - (uint32_t)mus->buf;
	if (read_bytes) {
		sceClibMemcpy(buffer, mus->buf, read_bytes);
		mus->buf += read_bytes;
	}
	return read_bytes;
}

static off_t mp3_seek(void *io, off_t offset, int seek_type) {
	mp3_buf *mus = (mp3_buf *)io;
	
	switch (seek_type) {
	case SEEK_SET:	
		mus->buf = mus->off_start + offset;
		break;
	case SEEK_CUR:
		mus->buf += offset;
		break;
	case SEEK_END:
		mus->buf = mus->off_end + offset;
		break;
	}
	return (uint32_t)mus->buf - mus->off_start;
}

static void mp3_close(void *io) {
}
	
// Taken from https://github.com/libsndfile/libsndfile/blob/master/programs/common.c
int sfe_copy_data_fp(SNDFILE *outfile, SNDFILE *infile, int channels, int normalize) {
	static double	data [4096], max ;
	sf_count_t	 frames, readcount, k ;

	frames = 4096 / channels ;
	readcount = frames ;

	sf_command (infile, SFC_CALC_SIGNAL_MAX, &max, sizeof (max)) ;
	if (!isnormal (max)) /* neither zero, subnormal, infinite, nor NaN */
		return 1 ;

	if (!normalize && max < 1.0)
	{	
		while (readcount > 0) {
			readcount = sf_readf_double (infile, data, frames) ;
			sf_writef_double (outfile, data, readcount) ;
		}
	}
	else
	{	
		sf_command (infile, SFC_SET_NORM_DOUBLE, NULL, SF_FALSE) ;
		while (readcount > 0) {
			readcount = sf_readf_double (infile, data, frames) ;
			for (k = 0 ; k < readcount * channels ; k++) {
				data [k] /= max ;
				if (!isfinite (data [k])) /* infinite or NaN */
					return 1 ;
			}
			sf_writef_double (outfile, data, readcount) ;
		}
	}

	return 0;
}

void transpile_to_ogg(void *file, uint32_t length, char *dst_file) {
	mp3_buf tmp;
	tmp.buf = file;
	tmp.off_start = file;
	tmp.off_end = file + length;
	mpg123_handle *mp3_handle = mpg123_new(NULL, NULL);
	mpg123_replace_reader_handle(mp3_handle, mp3_read, mp3_seek, mp3_close);
	mpg123_open_handle(mp3_handle, &tmp);
	mpg123_scan(mp3_handle);
	long srate; int channels, dummy;
	mpg123_getformat(mp3_handle, &srate, &channels, &dummy);
	mpg123_format_none(mp3_handle);
	mpg123_format(mp3_handle, srate, channels, MPG123_ENC_SIGNED_16);
	printf("File has samplerate %d and %d channels\n", srate, channels);
		
	size_t decoded = 0, done = 0;
	int err;
	void *out_buf = malloc(100 * 1024 * 1024);
	void *dst_buf = out_buf;
	do {
		err = mpg123_read(mp3_handle, dst_buf, 100 * 1024 * 1024, &done);
		dst_buf += done;
		decoded += done;
	} while (done && err != MPG123_OK);
	mpg123_close(mp3_handle);
	mpg123_delete(mp3_handle);
		
	FILE *f = fopen("ux0:data/hazard/tmp.wav", "wb");
	char data[9];
	uint32_t four = decoded + 36;
	uint16_t two;
	strcpy(data, "RIFF");
	fwrite(data, 1, 4, f);
	fwrite(&four, 1, 4, f);
	strcpy(data, "WAVEfmt ");
	fwrite(data, 1, 8, f);
	four = 0x10;
	fwrite(&four, 1, 4, f);
	two = 0x01;
	fwrite(&two, 1, 2, f);
	two = channels;
	fwrite(&two, 1, 2, f);
	fwrite(&srate, 1, 4, f);
	four = srate * channels * 2;
	fwrite(&four, 1, 4, f);
	two = channels * 2;
	fwrite(&two, 1, 2, f);
	two = 0x10;
	fwrite(&two, 1, 2, f);
	strcpy(data, "data");
	fwrite(data, 1, 4, f);
	fwrite(&decoded, 1, 4, f);
	fwrite(out_buf, 1, decoded, f);
	fclose(f);
	free(out_buf);
		
	SF_INFO sfinfo;
	SNDFILE *in = sf_open("ux0:data/hazard/tmp.wav", SFM_READ, &sfinfo);
	sfinfo.format = SF_FORMAT_OGG | SF_FORMAT_VORBIS | SF_ENDIAN_FILE;
	SNDFILE *out = sf_open(dst_file, SFM_WRITE, &sfinfo);
	sfe_copy_data_fp(out, in, sfinfo.channels, 0);
	sf_close(in);
	sf_close(out);
	sceIoRemove("ux0:data/hazard/tmp.wav");
}

const void *BASS_GetConfigPtr(uint32_t option) {
	return NULL;
}

uint32_t BASS_GetVersion() {
	return 0x2040000;
}

int BASS_Init(int device, uint32_t freq, uint32_t flags, int win, void *clsid) {
	soloud.init(SoLoud::Soloud::CLIP_ROUNDOFF);
	mpg123_init();
	return 1;
}

uint32_t BASS_GetDevice() {
	return 0;
}

int BASS_SetDevice(uint32_t device) {
	return 1;
}

int BASS_SetConfig(uint32_t option, uint32_t value) {
	switch (option) {
	default:
		//printf("BASS_SetConfig(%u, %u)\n", option, value);
		break;
	}
	return 1;
}

BASS_internal_sample *BASS_SampleLoad(uint32_t mem, void *file, uint64_t offset, uint32_t length, uint32_t max, uint32_t flags) {
	//printf("BASS_SampleLoad(%x, %x, %llu, %u, %u, %u)\n", mem, file, offset, length, max, flags);
	BASS_internal_sample *res = (BASS_internal_sample *)malloc(sizeof(BASS_internal_sample));
	res->play_handle = NULL;
	auto *w = new SoLoud::Wav;
	if (mem) {
		w->loadMem(file, length, true, false);
	} else {
		w->load(file);
	}
	res->handle = (void *)w;
	res->handle2 = NULL;
	w->setSingleInstance(1);
	sceClibMemset(&res->info, 0, sizeof(BASS_SAMPLE));
	res->info.freq = 44100;
	res->info.volume = 1.0f;
	res->info.pan = 0.0f;
	res->info.flags = 0;
	res->info.length = length;
	res->info.max = max;
	res->info.origres = 0;
	res->info.chans = 2;
	res->info.mingap = 0;
	res->info.mode3d = 2;
	res->info2.freq = 44100;
	res->info2.chans = 2;
	res->info2.flags = 0;
	res->info2.ctype = BASS_CTYPE_SAMPLE;
	res->info2.origres = 0;
	res->info2.plugin = NULL;
	res->info2.sample = NULL;
	res->info2.filename = NULL;
	return res;
}

BASS_internal_sample *BASS_StreamCreateFile(uint32_t mem, void *file, uint64_t offset, uint64_t length, uint32_t flags) {
	//printf("BASS_StreamCreateFile(%x, %x, %llu, %llu, %u)\n", mem, file, offset, length, flags);
	BASS_internal_sample *res = (BASS_internal_sample *)malloc(sizeof(BASS_internal_sample));
	res->play_handle = NULL;
	auto *w = new SoLoud::WavStream;
	if (mem) {
		char ogg_name[256];
		sprintf(ogg_name, "ux0:data/hazard/%d.ogg", length);
		FILE *f = fopen(ogg_name, "rb");
		SceIoStat stat;
		if (sceIoGetstat(ogg_name, &stat) < 0) {
			transpile_to_ogg(file, length, ogg_name);
		}
		w->load(ogg_name);
	} else {
		w->load(file);
	}
	auto *b = new SoLoud::Bus;
	b->setVisualizationEnable(true);
	soloud.play(*b);
	res->handle = (void *)b;
	res->duration = w->getLength();
	res->handle2 = (void *)w;
	sceClibMemset(&res->info, 0, sizeof(BASS_SAMPLE));
	res->info.freq = 44100;
	res->info.volume = 1.0f;
	res->info.pan = 0.0f;
	res->info.flags = 0;
	res->info.length = length;
	res->info.max = 0;
	res->info.origres = 0;
	res->info.chans = 2;
	res->info.mingap = 0;
	res->info.mode3d = 2;
	res->info2.freq = 44100;
	res->info2.chans = 2;
	res->info2.flags = 0;
	res->info2.ctype = BASS_CTYPE_STREAM;
	res->info2.origres = 0;
	res->info2.plugin = NULL;
	res->info2.sample = NULL;
	res->info2.filename = NULL;
	return res;
}

uint32_t BASS_SampleGetInfo(BASS_internal_sample *handle, BASS_SAMPLE *info) {
	sceClibMemcpy(info, &handle->info, sizeof(BASS_SAMPLE));
	return 1;
}

uint32_t BASS_ChannelGetInfo(BASS_internal_sample *handle, BASS_CHANNELINFO *info) {
	sceClibMemcpy(info, &handle->info2, sizeof(BASS_CHANNELINFO));
	return 1;
}

uint32_t BASS_SampleGetChannel(BASS_internal_sample *handle, uint32_t flags) {
	//printf("BASS_SampleGetChannel(%x, %u)\n", handle, flags);
	return handle;
}



uint32_t BASS_ChannelSetAttribute(BASS_internal_sample *handle, uint32_t attrib, float value) {
	//printf("BASS_ChannelSetAttribute(%x, %u, %f)\n", handle, attrib, value);
	if (!handle->handle2) {
		auto *w = (SoLoud::Wav *)handle->handle;
		switch (attrib) {
		case BASS_ATTRIB_PAN:
			handle->info.pan = value;
			break;
		case BASS_ATTRIB_VOL:
			handle->info.volume = value;
			w->setVolume(value);
			break;
		default:
			break;
		}
	} else {
		switch (attrib) {
		case BASS_ATTRIB_PAN:
			handle->info.pan = value;
			break;
		case BASS_ATTRIB_VOL:
			handle->info.volume = value;
			break;
		default:
			break;
		}
	}
	return 1;
}

uint32_t BASS_ChannelPlay(BASS_internal_sample *handle, uint32_t restart) {
	if (!handle->handle2) {
		//printf("BASS_ChannelPlay(%x, %x) Wav\n", handle, restart);
		auto *w = (SoLoud::Wav *)handle->handle;
		handle->play_handle = soloud.play(*w, handle->info.volume, handle->info.pan);
		handle->tick = sceKernelGetProcessTimeWide();
	} else {
		//printf("BASS_ChannelPlay(%x, %x) WavStream\n", handle, restart);
		auto *w = (SoLoud::WavStream *)handle->handle2;
		if (handle->play_handle) {
			soloud.setPause(handle->play_handle, 0);
			handle->tick = sceKernelGetProcessTimeWide() - handle->tick;
		} else {
			auto *b = (SoLoud::Bus *)handle->handle;
			handle->play_handle = b->play(*w, handle->info.volume, handle->info.pan);
			handle->tick = sceKernelGetProcessTimeWide();
		}
	}
	return 1;
}

uint32_t BASS_ChannelIsActive(BASS_internal_sample *handle) {
	return (sceKernelGetProcessTimeWide() - handle->tick) / 1000000 < handle->duration ? BASS_ACTIVE_PLAYING : BASS_ACTIVE_STOPPED;
}

// Hardcoded to Beat Hazard 2 requirements and tuned down since it seems FFT output range from Soloud is different from libBASS one
uint32_t BASS_ChannelGetData(BASS_internal_sample *handle, void *buffer, uint32_t length) {
	float *fft;
	float *fbuffer = (float *)buffer;
	auto *b = (SoLoud::Bus *)handle->handle;
	switch (length) {
	case BASS_DATA_FFT2048:
		//printf("GetData BASS_DATA_FFT2048\n");
		fft = b->calcFFT();
		sceClibMemcpy(fbuffer, fft, 256 * sizeof(float));
		for (int i = 0; i < 256; i++) {
			fbuffer[i] /= 32.0f;
		}
		sceClibMemcpy(fbuffer + 256, fbuffer, 256 * sizeof(float));
		//sceClibMemset(fbuffer + 256, 0, 256 * sizeof(float));
		return 0x800;
	default:
		//printf("GetData %x\n", length);
		fft = b->getWave();
		sceClibMemcpy(fbuffer, fft, 256 * sizeof(float));
		sceClibMemcpy(fbuffer + 256, fft, 256 * sizeof(float));
		sceClibMemcpy(fbuffer + 512, fbuffer, 512 * sizeof(float));
		sceClibMemcpy(fbuffer + 1024, fbuffer, 1024 * sizeof(float));
		return 0x2000;
	}
	return -1;
}

uint32_t BASS_ChannelGetLength(BASS_internal_sample *handle, uint32_t mode) {
	return handle->info.length;
}

double BASS_ChannelBytes2Seconds(BASS_internal_sample *handle, uint64_t pos) {
	if (!handle->handle2) {
		//printf("Byte2Seconds %x %llu\n", handle->handle, pos);
		return pos == handle->info.length ? handle->duration : min((float)(sceKernelGetProcessTimeWide() - handle->tick) / 1000000.0f, handle->duration);
	} else {
		//printf("Byte2Seconds %x %llu\n", handle->handle, pos);
		return pos == handle->info.length ? handle->duration : min((float)(sceKernelGetProcessTimeWide() - handle->tick) / 1000000.0f, handle->duration);
	}
}

uint64_t BASS_ChannelGetPosition(BASS_internal_sample *handle, uint32_t mode) {
	if (!handle->handle2)
		return 0;
	return 1;
}

uint32_t BASS_ChannelStop(BASS_internal_sample *handle) {
	if (!handle->handle2) {
		soloud.stop(handle->play_handle);
		auto *w = (SoLoud::WavStream *)handle->handle2;
		delete w;
		free(handle);
	} else {
		auto *b = (SoLoud::Bus *)handle->handle;
		auto *w = (SoLoud::WavStream *)handle->handle2;
		b->stop();
		delete b;
		delete w;
		free(handle);
	}
	return 1;
}

int BASS_ChannelPause(BASS_internal_sample *handle) {
	//printf("BASS_ChannelPause(%x)\n", handle);
	soloud.setPause(handle->play_handle, 1);
	handle->tick = sceKernelGetProcessTimeWide() - handle->tick;
	return 1;
}

int Song_GetTotalDuration(const char *file) {
	SoLoud::WavStream m;
	m.load(file);
	return m.getLength();
}

};
