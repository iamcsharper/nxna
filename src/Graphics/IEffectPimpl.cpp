#include "IEffectPimpl.h"
#include "Effect.h"
#include "../Exception.h"

namespace Nxna
{
namespace Graphics
{
namespace Pvt
{
	EffectParameter* IEffectPimpl::CreateParameter(Effect* parent, EffectParameterType type, int numElements, void* handle, const char* name)
	{
		return new EffectParameter(parent, type, numElements, handle, name);
	}

	int* IEffectPimpl::GetRawValue(EffectParameter* parameter)
	{
		return parameter->m_value;
	}

	EffectTechnique* IEffectPimpl::CreateTechnique(const char* name, bool hidden)
	{
		return m_parent->CreateTechnique(name, hidden);
	}
}
}
}