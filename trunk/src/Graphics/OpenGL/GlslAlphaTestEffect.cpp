#include <cstdio>
#include "GlslAlphaTestEffect.h"
#include "OpenGLDevice.h"

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	const char* alphaTestVertexSource = 
		"uniform highp mat4 ModelViewProjection;\n"

		"in vec4 position : POSITION0;\n"
		"in vec2 texCoords : TEXCOORD0;\n"
		"#if defined VERTEXCOLORENABLED\n"
		"in vec4 color : COLOR0;\n"
		"out vec4 o_color;\n"
		"#endif\n"
		"out vec2 o_diffuseCoords;\n"

		"void main()\n"
		"{\n"
		"	gl_Position = ModelViewProjection * position;\n"
		"	o_diffuseCoords = texCoords;\n"
		"#if defined VERTEXCOLORENABLED\n"
		"	o_color = color;\n"
		"#endif\n"
		"}";

	const char* alphaTestFragmentSource =
		"uniform highp vec4 AlphaTest;\n"
		"uniform sampler2D Diffuse;\n"
		"in HIGHP vec2 o_diffuseCoords;\n"
		"#if defined VERTEXCOLORENABLED\n"
		"in HIGHP vec4 o_color;\n"
		"#endif\n"
		"#if __VERSION__ >= 130\n"
		"out vec4 outputColor;\n"
		"#endif\n"

		"void clip(HIGHP float a) "
		"{ "
		"	if (a < 0.0) discard; "
		"} "

		"void main()\n"
		"{\n"
		"	HIGHP vec4 finalColor;\n"
		"#if __VERSION__ < 130\n"
		"	finalColor = texture2D(Diffuse, o_diffuseCoords);\n"
		"#else\n"
		"	finalColor = texture(Diffuse, o_diffuseCoords);\n"
		"#endif\n"
		"#if defined VERTEXCOLORENABLED\n"
		"	finalColor *= o_color;\n"
		"#endif\n"

		"#if defined LESSGREATER\n"
		"	clip((finalColor.a < AlphaTest.x) ? AlphaTest.z : AlphaTest.w); \n"
		"#else\n"
		"	clip((abs(finalColor.a - AlphaTest.x) < AlphaTest.y) ? AlphaTest.z : AlphaTest.w);\n"
		"#endif\n"
		
		"#if __VERSION__ < 130\n"
		"	gl_FragColor = finalColor;\n"
		"#else\n"
		"	outputColor = finalColor;\n"
        "#endif\n"

		"}";


	GlslAlphaTestEffect::GlslAlphaTestEffect(OpenGlDevice* device)
		: GlslEffect(device)
	{
		m_finalTransformDirty = true;
		m_isVertexColorEnabled = false;
		m_alpha = 0;
		m_referenceAlpha = 0;
		m_compareFunction = CompareFunction::Greater;

		Matrix::GetIdentity(m_world);
		Matrix::GetIdentity(m_view);
		Matrix::GetIdentity(m_projection);

		std::string vertexResult, fragResult;
		ProcessSource(alphaTestVertexSource, alphaTestFragmentSource, vertexResult, fragResult);

		char buffer[100];
		sprintf(buffer, "#version %d\n", device->GetGlslVersion());
		std::string versionString = buffer;

		const char* colorAndLess[] = { "#define VERTEXCOLORENABLED\n", "#define LESSGREATER\n"};
		const char* color[] = { "#define VERTEXCOLORENABLED\n" };
		const char* less[] = { "#define LESSGREATER\n" };

		CreateProgram(vertexResult, fragResult, colorAndLess, 2);
		CreateProgram(vertexResult, fragResult, color, 1);
		CreateProgram(vertexResult, fragResult, less, 1);
		CreateProgram(vertexResult, fragResult, nullptr, 0);
	}

	void GlslAlphaTestEffect::SetTexture(Texture2D* texture)
	{
		GetParameter("Diffuse")->SetValue(texture);
	}

	void GlslAlphaTestEffect::Apply()
	{
		Matrix worldView;
		Matrix worldViewProjection;

		Matrix::Multiply(m_world, m_view, worldView);
		Matrix::Multiply(worldView, m_projection, worldViewProjection);

		GetParameter("ModelViewProjection")->SetValue(worldViewProjection.C);

		// calculate the AlphaTest parameter
		const float threshold = 0.5f / 255.0f;
		Vector4 alphaTest;
		if (m_compareFunction == CompareFunction::Less)
		{
			alphaTest.X = m_referenceAlpha - threshold;
			alphaTest.Z = 1.0f;
			alphaTest.W = -1.0f;
		}
		else if (m_compareFunction == CompareFunction::LessEqual)
		{
			alphaTest.X = m_referenceAlpha + threshold;
			alphaTest.Z = 1.0f;
			alphaTest.W = -1.0f;
		}
		else if (m_compareFunction == CompareFunction::GreaterEqual)
		{
			alphaTest.X = m_referenceAlpha - threshold;
			alphaTest.Z = -1.0f;
			alphaTest.W = 1.0f;
		}
		else if (m_compareFunction == CompareFunction::Greater)
		{
			alphaTest.X = m_referenceAlpha + threshold;
			alphaTest.Z = -1.0f;
			alphaTest.W = 1.0f;
		}
		else if (m_compareFunction == CompareFunction::Equal)
		{
			alphaTest.X = m_referenceAlpha;
			alphaTest.Y = threshold;
			alphaTest.Z = 1.0f;
			alphaTest.W = -1.0f;
		}
		else if (m_compareFunction == CompareFunction::NotEqual)
		{
			alphaTest.X = m_referenceAlpha;
			alphaTest.Y = threshold;
			alphaTest.Z = -1.0f;
			alphaTest.W = 1.0f;
		}
		else if (m_compareFunction == CompareFunction::Never)
		{
			alphaTest.Z = -1.0f;
			alphaTest.W = -1.0f;
		}
		else if (m_compareFunction == CompareFunction::Always)
		{
			alphaTest.Z = 1.0f;
			alphaTest.W = -1.0f;
		}

		GetParameter("AlphaTest")->SetValue(alphaTest);

		// figure out which program to use
		if (m_isVertexColorEnabled)
		{
			if (m_compareFunction == CompareFunction::Equal ||
				m_compareFunction == CompareFunction::NotEqual)
				ApplyProgram(1);
			else
				ApplyProgram(0);
		}
		else
		{
			if (m_compareFunction == CompareFunction::Equal ||
				m_compareFunction == CompareFunction::NotEqual)
				ApplyProgram(3);
			else
				ApplyProgram(2);
		}
	}
}
}
}