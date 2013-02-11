#ifndef GRAPHICS_INDEXBUFFER_H
#define GRAPHICS_INDEXBUFFER_H

#include "VertexBuffer.h"

namespace Nxna
{
namespace Graphics
{
	NXNA_ENUM(IndexElementSize)
		SixteenBits = 2,
		ThirtyTwoBits = 4
	END_NXNA_ENUM(IndexElementSize)

	class GraphicsDevice;

	class IndexBuffer
	{
	protected:
		IndexElementSize m_elementSize;
		int m_indexCount;

	public:

		virtual ~IndexBuffer() { }
		virtual void SetData(void* indices, int indexCount) = 0;

		int GetIndexCount() const { return m_indexCount; }
		IndexElementSize GetElementSize() const { return m_elementSize; }
	};
}
}

#endif // GRAPHICS_INDEXBUFFER_H
