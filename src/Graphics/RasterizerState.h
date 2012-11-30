#ifndef NXNA_GRAPHICS_RASTERIZERSTATE_H
#define NXNA_GRAPHICS_RASTERIZERSTATE_H

namespace Nxna
{
namespace Graphics
{
	enum CullMode
	{
		CullMode_CullClockwiseFace,
		CullMode_CullCounterClockwiseFace,
		CullMode_None
	};

	class RasterizerState
	{
		friend class GraphicsDevice;

		static RasterizerState m_cullClockwise;
		static RasterizerState m_cullCounterClockwise;
		static RasterizerState m_cullNone;

		void* m_handle;

	public:

		RasterizerState()
		{
			TheCullMode = CullMode_CullCounterClockwiseFace;

			m_handle = nullptr;
		}

		CullMode TheCullMode;

		static const RasterizerState* GetCullClockwise()
		{
			if (m_isInitialized == false)
				init();

			return &m_cullClockwise;
		}

		static const RasterizerState* GetCullCounterClockwise()
		{
			if (m_isInitialized == false)
				init();

			return &m_cullCounterClockwise;
		}

		static const RasterizerState* GetCullNone()
		{
			if (m_isInitialized == false)
				init();

			return &m_cullNone;
		}

	private:
		static bool m_isInitialized;
		static void init();
	};
}
}

#endif // NXNA_GRAPHICS_RASTERIZERSTATE_H
