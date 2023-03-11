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
#include "soloud_mp3stream.h"
#include "soloud_wav.h"
#include "soloud_bus.h"
#include "bass_reimpl.h"

SoLoud::Soloud soloud;
#define min(a, b) (a) < (b) ? (a) : (b)

extern "C" {

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
	if (mem) {
		auto *w = new SoLoud::MP3Stream;
		w->loadMem(file, length, true, false);
		res->duration = w->getLength();
		res->handle2 = (void *)w;
	} else {
		auto *w = new SoLoud::WavStream;
		w->load(file);
		res->duration = w->getLength();
		res->handle2 = (void *)w;
	}
	auto *b = new SoLoud::Bus;
	b->setVisualizationEnable(true);
	soloud.play(*b);
	res->handle = (void *)b;
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
