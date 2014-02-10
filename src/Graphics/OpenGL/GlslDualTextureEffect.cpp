#include <cstdio>
#include "GlslDualTextureEffect.h"
#include "OpenGLDevice.h"

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	
#include "ShaderSource/DualTextureEffect.vert.inc"
#include "ShaderSource/DualTextureEffect.frag.inc"

	GlslDualTextureEffect::GlslDualTextureEffect(OpenGlDevice* device, GlslEffect* glslEffect)
		: Pvt::DualTextureEffectPimpl(glslEffect), m_glslEffect(glslEffect)
	{
		m_isVertexColorEnabled = false;

		std::string vertexResult, fragResult;
		glslEffect->ProcessSource(DualTextureEffect_vert, DualTextureEffect_frag, vertexResult, fragResult);

		const char* color[] = { "#define VERTEXCOLORENABLED\n" };

		glslEffect->CreateProgram("ColorEnabled", true, (const byte*)vertexResult.c_str(), vertexResult.length(), (const byte*)fragResult.c_str(), fragResult.length());
		glslEffect->CreateProgram("ColorDisabled", true, (const byte*)vertexResult.c_str(), vertexResult.length(), (const byte*)fragResult.c_str(), fragResult.length());
		
		// create the dummy technique
		glslEffect->CreateProgram("default", false, nullptr, 0, nullptr, 0);

		// HACK: sometimes OpenGL reports the sampler uniforms in a different
		// order than they exist in the shader source, which breaks this effect.
		// Go back and make sure the order is correct.
		std::vector<EffectParameter*>& textureParams = glslEffect->GetTextureParams();
		if (textureParams[0]->Name != "Diffuse")
		{
			EffectParameter* dummy = textureParams[0];
			textureParams[0] = textureParams[1];
			textureParams[1] = dummy;
		}
	}

	void GlslDualTextureEffect::Apply(int programIndex)
	{
		Matrix worldView;
		Matrix worldViewProjection;

		Matrix::Multiply(m_world, m_view, worldView);
		Matrix::Multiply(worldView, m_projection, worldViewProjection);

		m_glslEffect->GetParameter("ModelViewProjection")->SetValue(worldViewProjection.C);

		m_glslEffect->ApplyProgram(programIndex);
	}
}
}
}