#ifndef CONTENT_XNBREADER_H
#define CONTENT_XNBREADER_H

#include <string>
#include "../NxnaConfig.h"

namespace Nxna
{
namespace Content
{
	class Stream;
	class ContentManager;

	NXNA_ENUM(TargetPlatform)
		Windows,
		WinPhone,
		XBox360
	END_NXNA_ENUM(TargetPlatform)

	class XnbReader
	{
		Stream* m_stream;
		std::string m_name;
		std::string m_fullPath;
		ContentManager* m_content;

		TargetPlatform m_target;
		bool m_isHighDef;
		bool m_isCompressed;
		int m_compressedSize;
		int m_uncompressedSize;

	public:
		XnbReader(Stream* stream, const char* name, const char* fullPath, ContentManager* contentManager);
		~XnbReader();

		TargetPlatform GetTargetPlatform();
		bool IsHighDef();
		bool IsCompressed();

		Stream* GetStream() { return m_stream; }
		ContentManager* GetContentManager() { return m_content; }
		const char* GetName() { return m_name.c_str(); }
		const std::string& GetFullPath() { return m_fullPath; }
		int ReadTypeID();

		std::string ReadString();

	private:
		void readHeader();
		int read7BitEncodedInt();
		
		void skipString();
	};
}
}

#endif // CONTENT_XNBREADER_H