#include "RasterizerState.h"

namespace Nxna
{
namespace Graphics
{
	bool RasterizerState::m_isInitialized = false;
	RasterizerState RasterizerState::m_cullClockwise;
	RasterizerState RasterizerState::m_cullCounterClockwise;
	RasterizerState RasterizerState::m_cullNone;

	void RasterizerState::init()
	{
		m_cullClockwise.TheCullMode = CullMode_CullClockwiseFace;
		m_cullCounterClockwise.TheCullMode = CullMode_CullCounterClockwiseFace;
		m_cullNone.TheCullMode = CullMode_None;

		m_isInitialized = true;
	}
}
}