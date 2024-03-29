#ifndef NXNA_MEDIA_OGGMEDIAPLAYER_H
#define NXNA_MEDIA_OGGMEDIAPLAYER_H

namespace Nxna
{
namespace Media
{
	class Song;

	class OggMediaPlayer
	{
		static unsigned int m_source;
		static unsigned int m_buffers[3];

		static bool m_repeat;
		static float m_volume;

	public:
		static bool Play(Song* song);
		static void Pause();
		static void Resume();
		static void Stop();

		static bool IsRepeating() { return m_repeat; }
		static void IsRepeating(bool repeat) { m_repeat = repeat; }

		static void SetVolume(float volume);
		static float GetVolume() { return m_volume; }

		static void Tick(void* handle);

	private:
		static void stream(unsigned int alBuffer, void* handle);
	};
}
}

#endif // NXNA_MEDIA_OGGMEDIAPLAYER_H