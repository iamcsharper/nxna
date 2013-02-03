#include "OggMediaPlayer.h"
#include "Song.h"
#include "../Content/FileStream.h"
#include "../Audio/OggVorbis/OggVorbisDecoder.h"

#ifndef DISABLE_OPENAL
#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif
#endif

namespace Nxna
{
namespace Media
{
	unsigned int OggMediaPlayer::m_source = 0;
	unsigned int OggMediaPlayer::m_buffers[];

	bool OggMediaPlayer::m_repeat = false;
	float OggMediaPlayer::m_volume = 1.0f;

	bool OggMediaPlayer::Play(Song* song)
	{
		Audio::OggVorbisDecoder* decoder = static_cast<Audio::OggVorbisDecoder*>(song->m_handle);
		if (decoder != nullptr)
			decoder->Rewind();
		else
		{
			FILE* fp = fopen(song->m_path.c_str(), "rb");
			
			if (fp == nullptr)
			{
				// we were unable to find the ogg file associated with this song :(
				// in this case we'll just ignore the song.
				return false;
			}
			
			Content::FileStream* file = new Content::FileStream(fp);
			decoder = new Audio::OggVorbisDecoder(file, false);

			song->m_handle = decoder;
		}

		if (m_source == 0)
		{
			alGenSources(1, &m_source);

			alSourcei(m_source, AL_SOURCE_RELATIVE, AL_TRUE);
			alSourcei(m_source, AL_LOOPING, AL_FALSE);
			alSource3f(m_source, AL_POSITION, 0, 0, 0);
			alSource3f(m_source, AL_VELOCITY, 0, 0, 0);
			alSourcef(m_source, AL_GAIN, 1.0f);

			alGenBuffers(3, m_buffers);
		}

		alSourceStop(m_source);
		alSourcei(m_source, AL_BUFFER, 0);

		stream(m_buffers[0], song->m_handle);
		stream(m_buffers[1], song->m_handle);
		stream(m_buffers[2], song->m_handle);

		alSourceQueueBuffers(m_source, 3, m_buffers);

		alSourcePlay(m_source);

		return true;
	}

	void OggMediaPlayer::Stop()
	{
#ifndef DISABLE_OPENAL
		alSourceStop(m_source);
#endif
	}

	void OggMediaPlayer::SetVolume(float volume)
	{
		m_volume = volume;
		
#ifndef DISABLE_OPENAL
		alSourcef(m_source, AL_GAIN, volume);
#endif
	}

	void OggMediaPlayer::Tick(void* handle)
	{
#ifndef NXNA_DISABLE_OGG
#ifndef DISABLE_OPENAL
		int processed;
		alGetSourcei(m_source, AL_BUFFERS_PROCESSED, &processed);

		int state;
		alGetSourcei(m_source, AL_SOURCE_STATE, &state);
			
		// TODO: it's possible for the buffers to not get updated quick enough, which will cause
		// the source to stop playing. Make it pick up where it left off instead!
		//if (state != AL_PLAYING)
		//	throw Exception("zomg");

		while(processed--)
		{
			ALuint albuffer;
			alSourceUnqueueBuffers(m_source, 1, &albuffer);

			stream(albuffer, handle);

			alSourceQueueBuffers(m_source, 1, &albuffer);
		}
#endif
#endif
	}

	void OggMediaPlayer::stream(unsigned int alBuffer, void* handle)
	{
#ifndef NXNA_DISABLE_OGG
#ifndef DISABLE_OPENAL
		const int BUFFER_SIZE = 176400; // 1 second
		static byte buffer[BUFFER_SIZE];

		Audio::OggVorbisDecoder* decoder = static_cast<Audio::OggVorbisDecoder*>(handle);
		int read = decoder->Read(buffer, BUFFER_SIZE);
		alBufferData(alBuffer, decoder->NumChannels() == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, buffer, read, decoder->SampleRate());

		if (read == 0 && m_repeat)
			decoder->Rewind();
#endif
#endif
	}
}
}
