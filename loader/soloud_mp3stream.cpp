// Taken from https://github.com/jarikomppa/soloud/blob/master/include/soloud_wavstream.h
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "soloud.h"
#include "soloud_mp3stream.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"
#include "soloud_file.h"

namespace SoLoud
{

	size_t drmp3_read_func(void* pUserData, void* pBufferOut, size_t bytesToRead)
	{
		File *fp = (File*)pUserData;
		return fp->read((unsigned char*)pBufferOut, (unsigned int)bytesToRead);
	}

	drmp3_bool32 drmp3_seek_func(void* pUserData, int offset, drmp3_seek_origin origin)
	{
		File *fp = (File*)pUserData;
		if (origin != drmp3_seek_origin_start)
			offset += fp->pos();
		fp->seek(offset);
		return 1;
	}

	MP3StreamInstance::MP3StreamInstance(MP3Stream *aParent)
	{
		mOggFrameSize = 0;
		mParent = aParent;
		mOffset = 0;
		mFile = 0;
		if (aParent->mMemFile)
		{
			MemoryFile *mf = new MemoryFile();
			mFile = mf;
			mf->openMem(aParent->mMemFile->getMemPtr(), aParent->mMemFile->length(), false, false);
		}
		else
		if (aParent->mFilename)
		{
			DiskFile *df = new DiskFile;
			mFile = df;
			df->open(aParent->mFilename);
		}
		else
		if (aParent->mStreamFile)
		{
			mFile = aParent->mStreamFile;
			mFile->seek(0); // stb_vorbis assumes file offset to be at start of ogg
		}
		else
		{
			return;
		}
		
		if (mFile)
		{

				mOgg = new drmp3;
				if (!drmp3_init(mOgg, drmp3_read_func, drmp3_seek_func, (void*)mFile, NULL))
				{
					delete mOgg;
					mOgg = 0;
					if (mFile != mParent->mStreamFile)
						delete mFile;
					mFile = 0;
				}

		}
	}

	MP3StreamInstance::~MP3StreamInstance()
	{

			if (mOgg)
			{
				drmp3_uninit((drmp3*)mOgg);
				delete mOgg;
				mOgg = 0;
			}

	}

	void MP3StreamInstance::getAudio(float *aBuffer, unsigned int aSamples)
	{			
		unsigned int offset = 0;
		float tmp[512 * MAX_CHANNELS];
		if (mFile == NULL)
			return 0;

				unsigned int i, j, k;

				for (i = 0; i < aSamples; i += 512)
				{
					unsigned int blockSize = (aSamples - i) > 512 ? 512 : aSamples - i;
					offset += (unsigned int)drmp3_read_pcm_frames_f32((drmp3*)mOgg, blockSize, tmp);

					for (j = 0; j < blockSize; j++)
					{
						for (k = 0; k < mChannels; k++)
						{
							aBuffer[k * aSamples + i + j] = tmp[j * ((drmp3*)mOgg)->channels + k];
						}
					}
				}
				mOffset += offset;
	}

	result MP3StreamInstance::rewind()
	{

			if (mOgg)
			{
				drmp3_seek_to_pcm_frame(mOgg, 0);
			}
		
		mOffset = 0;

		return 0;
	}

	bool MP3StreamInstance::hasEnded()
	{
		if (mOffset >= mParent->mSampleCount)
		{
			return 1;
		}
		return 0;
	}

	MP3Stream::MP3Stream()
	{
		mFilename = 0;
		mSampleCount = 0;
		mMemFile = 0;
		mStreamFile = 0;
	}
	
	MP3Stream::~MP3Stream()
	{
		stop();
		delete[] mFilename;
		delete mMemFile;
	}
	
#define MAKEDWORD(a,b,c,d) (((d) << 24) | ((c) << 16) | ((b) << 8) | (a))

	result MP3Stream::loadmp3(File * fp)
	{
		fp->seek(0);
		drmp3 decoder;
		if (!drmp3_init(&decoder, drmp3_read_func, drmp3_seek_func, (void*)fp, NULL))
			return FILE_LOAD_FAILED;


		mChannels = decoder.channels;
		if (mChannels > MAX_CHANNELS)
		{
			mChannels = MAX_CHANNELS;
		}

		drmp3_uint64 samples = drmp3_get_pcm_frame_count(&decoder);

		mBaseSamplerate = (float)decoder.sampleRate;
		mSampleCount = (unsigned int)samples;
		drmp3_uninit((drmp3*)&decoder);

		return SO_NO_ERROR;
	}

	result MP3Stream::load(const char *aFilename)
	{
		delete[] mFilename;
		delete mMemFile;
		mMemFile = 0;
		mFilename = 0;
		mSampleCount = 0;
		DiskFile fp;
		int res = fp.open(aFilename);
		if (res != SO_NO_ERROR)
			return res;
		
		int len = (int)strlen(aFilename);
		mFilename = new char[len+1];		
		memcpy(mFilename, aFilename, len);
		mFilename[len] = 0;
		
		res = parse(&fp);

		if (res != SO_NO_ERROR)
		{
			delete[] mFilename;
			mFilename = 0;
			return res;
		}

		return 0;
	}

	result MP3Stream::loadMem(unsigned char *aData, unsigned int aDataLen, bool aCopy, bool aTakeOwnership)
	{
		delete[] mFilename;
		delete mMemFile;
		mStreamFile = 0;
		mMemFile = 0;
		mFilename = 0;
		mSampleCount = 0;

		if (aData == NULL || aDataLen == 0)
			return INVALID_PARAMETER;

		MemoryFile *mf = new MemoryFile();
		int res = mf->openMem(aData, aDataLen, aCopy, aTakeOwnership);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}

		res = parse(mf);

		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}

		mMemFile = mf;

		return 0;
	}

	result MP3Stream::loadToMem(const char *aFilename)
	{
		DiskFile df;
		int res = df.open(aFilename);
		if (res == SO_NO_ERROR)
		{
			res = loadFileToMem(&df);
		}
		return res;
	}

	result MP3Stream::loadFile(File *aFile)
	{
		delete[] mFilename;
		delete mMemFile;
		mStreamFile = 0;
		mMemFile = 0;
		mFilename = 0;
		mSampleCount = 0;

		int res = parse(aFile);

		if (res != SO_NO_ERROR)
		{
			return res;
		}

		mStreamFile = aFile;

		return 0;
	}

	result MP3Stream::loadFileToMem(File *aFile)
	{
		delete[] mFilename;
		delete mMemFile;
		mStreamFile = 0;
		mMemFile = 0;
		mFilename = 0;
		mSampleCount = 0;

		MemoryFile *mf = new MemoryFile();
		int res = mf->openFileToMem(aFile);
		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}

		res = parse(mf);

		if (res != SO_NO_ERROR)
		{
			delete mf;
			return res;
		}

		mMemFile = mf;

		return res;
	}


	result MP3Stream::parse(File *aFile)
	{
		int tag = aFile->read32();
		int res = SO_NO_ERROR;

		if (loadmp3(aFile) == SO_NO_ERROR)
		{
			res = SO_NO_ERROR;
		}
		else
		{
			res = FILE_LOAD_FAILED;
		}
		return res;
	}

	AudioSourceInstance *MP3Stream::createInstance()
	{
		return new MP3StreamInstance(this);
	}

	double MP3Stream::getLength()
	{
		if (mBaseSamplerate == 0)
			return 0;
		return mSampleCount / mBaseSamplerate;
	}
};