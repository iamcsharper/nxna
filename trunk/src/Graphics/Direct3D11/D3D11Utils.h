#ifndef NXNA_GRAPHICS_DIRECT3D11_D3DUTILS_H
#define NXNA_GRAPHICS_DIRECT3D11_D3DUTILS_H

#include <D3D11.h>
#include "../BlendState.h"

namespace Nxna
{
namespace Graphics
{
namespace Direct3D11
{
	class D3D11Utils
	{
	public:
		static D3D11_BLEND ConvertBlendMode(Blend blend)
		{
			if (blend == Blend_Zero)
				return D3D11_BLEND_ZERO;
			if (blend == Blend_One)
				return D3D11_BLEND_ONE;
			if (blend == Blend_SourceAlpha)
				return D3D11_BLEND_SRC_ALPHA;
			if (blend == Blend_InverseSourceAlpha)
				return D3D11_BLEND_INV_SRC_ALPHA;
			if (blend == Blend_DestinationAlpha)
				return D3D11_BLEND_DEST_ALPHA;
			if (blend == Blend_InverseDestinationAlpha)
				return D3D11_BLEND_INV_DEST_ALPHA;

			// we should never get here
			return D3D11_BLEND_ZERO;
		}

		static D3D11_BLEND_OP ConvertBlendFunc(BlendFunction func)
		{
			if (func == BlendFunction_Add)
				return D3D11_BLEND_OP_ADD;
			if (func == BlendFunction_Subtract)
				return D3D11_BLEND_OP_SUBTRACT;
			if (func == BlendFunction_ReverseSubtract)
				return D3D11_BLEND_OP_REV_SUBTRACT;
			if (func == BlendFunction_Max)
				return D3D11_BLEND_OP_MAX;
			if (func == BlendFunction_Min)
				return D3D11_BLEND_OP_MIN;

			// we should never get here
			return D3D11_BLEND_OP_MAX;
		
		}

		static D3D11_COMPARISON_FUNC ConvertComparisonFunc(CompareFunction func)
		{
			if (func == CompareFunction_Always)
				return D3D11_COMPARISON_ALWAYS;
			if (func == CompareFunction_Equal)
				return D3D11_COMPARISON_EQUAL;
			if (func == CompareFunction_Greater)
				return D3D11_COMPARISON_GREATER;
			if (func == CompareFunction_GreaterEqual)
				return D3D11_COMPARISON_GREATER_EQUAL;
			if (func == CompareFunction_Less)
				return D3D11_COMPARISON_LESS;
			if (func == CompareFunction_LessEqual)
				return D3D11_COMPARISON_LESS_EQUAL;
			if (func == CompareFunction_Never)
				return D3D11_COMPARISON_NEVER;
			if (func == CompareFunction_NotEqual)
				return D3D11_COMPARISON_NOT_EQUAL;

			// we should never get here
			return D3D11_COMPARISON_NEVER;
		}
	};
}
}
}

#endif // NXNA_GRAPHICS_DIRECT3D11_D3DUTILS_H
