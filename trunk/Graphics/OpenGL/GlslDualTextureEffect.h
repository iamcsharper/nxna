#ifndef GRAPHICS_OPENGL_GLSLDUALTEXTUREEFFECT_H
#define GRAPHICS_OPENGL_GLSLDUALTEXTUREEFFECT_H

#include "../DualTextureEffect.h"
#include "GlslEffect.h"

// VC++ gives a stupid warning that doesn't
// apply since we're using pure virtual methods
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4250)
#endif

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	class GlslDualTextureEffect :  public GlslEffect, public DualTextureEffect
	{
		Matrix m_world;
		Matrix m_view;
		Matrix m_projection;
		Matrix m_finalTransform;
		bool m_finalTransformDirty;

		bool m_isVertexColorEnabled;

	public:
		GlslDualTextureEffect(OpenGlDevice* device);

		virtual bool IsVertexColorEnabled() override { return m_isVertexColorEnabled; }
		virtual void IsVertexColorEnabled(bool enabled) override { m_isVertexColorEnabled = enabled; }

		virtual void SetWorld(const Matrix& matrix) override
		{
			m_world = matrix; m_finalTransformDirty = true;
		};

		virtual void SetView(const Matrix& matrix) override
		{
			m_view = matrix; m_finalTransformDirty = true;
		};

		virtual void SetProjection(const Matrix& matrix) override
		{
			m_projection = matrix; m_finalTransformDirty = true;
		};

		virtual void SetTexture(Texture2D* texture) override;
		virtual void SetTexture2(Texture2D* texture) override;

		virtual void Apply() override;
	};
}
}
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // GRAPHICS_OPENGL_GLSLDUALTEXTUREEFFECT_H