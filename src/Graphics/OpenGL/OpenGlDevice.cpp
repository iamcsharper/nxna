#include <cassert>
#include "OpenGL.h"
#include "../VertexDeclaration.h"
#include "OpenGLDevice.h"
#include "GlslBasicEffect.h"
#include "GlslSpriteEffect.h"
#include "GlslDualTextureEffect.h"
#include "GlslAlphaTestEffect.h"
#include "GlTexture2D.h"
#include "GlVertexBuffer.h"
#include "GlIndexBuffer.h"

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	OpenGlDevice::OpenGlDevice()
	{
		m_instance = this;
		m_vertexPointersNeedSetup = true;
		m_declaration = nullptr;
		m_effect = nullptr;
	}

	void OpenGlDevice::OnContextCreated()
	{
#ifndef USING_OPENGLES
		glewInit();
		glGetError();

		if (glGenVertexArrays != nullptr)
		{
			GLuint vao;
		//	glGenVertexArrays(1, &vao);
		//	glBindVertexArray(vao);
		}
#endif

		glEnable(GL_BLEND);

		setClearColor(Color::GetCornflowerBlue());
		setClearDepth(1.0f);

		GlException::ThrowIfError();

#ifndef USING_OPENGLES
        const GLubyte* version = glGetString(GL_VERSION);
		const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
        
		m_version = (version[0] - '0') * 100 + (version[2] - '0') * 10;
		m_glslVersion = (glslVersion[0] - '0') * 100 + (glslVersion[2] - '0') * 10;
#else
        m_version = 200;
        m_glslVersion = 100;
#endif
	}

	void OpenGlDevice::UpdateScreenSize(int screenWidth, int screenHeight)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
	}

	CullMode OpenGlDevice::GetRasterizerState()
	{
		if (glIsEnabled(GL_CULL_FACE) == GL_FALSE)
            return CullMode_None;

        int param;
        glGetIntegerv(GL_FRONT_FACE, &param);

        // OpenGL stores what NOT to cull, so we need to reverse it
        if (param == GL_CW)
            return CullMode_CullCounterClockwiseFace;
        else
            return CullMode_CullClockwiseFace;
	}

	void OpenGlDevice::SetRasterizerState(const RasterizerState* state)
	{
		assert(state != nullptr);

		if (state->TheCullMode == CullMode_None)
            glDisable(GL_CULL_FACE);
		else
		{
			glEnable(GL_CULL_FACE);

            if (state->TheCullMode == CullMode_CullClockwiseFace)
                glFrontFace(GL_CCW);
            else
                glFrontFace(GL_CW);
        }
	}

	DepthStencilState OpenGlDevice::GetDepthStencilState()
	{
		DepthStencilState result;

		int func;
		glGetIntegerv(GL_DEPTH_FUNC, &func);
		result.DepthBufferFunction = convertCompareFunction(func);
		result.DepthBufferEnable = glIsEnabled(GL_DEPTH_TEST) > 0;

		int mask;
		glGetIntegerv(GL_DEPTH_WRITEMASK, &mask);
		result.DepthBufferWriteEnable = mask > 0;

		result.StencilEnable = glIsEnabled(GL_STENCIL_TEST) > 0;

		int ref;
		glGetIntegerv(GL_STENCIL_REF, &ref);
		result.ReferenceStencil = ref;

		glGetIntegerv(GL_STENCIL_FUNC, &func);
		result.StencilFunction = convertCompareFunction(func);

		int pass;
		glGetIntegerv(GL_STENCIL_PASS_DEPTH_PASS, &pass);
		result.StencilPass = convertStencilOperation(pass);

		return result;
	}

	void OpenGlDevice::SetDepthStencilState(const DepthStencilState* state)
	{
		// depth buffer stuff
		if (state->DepthBufferEnable)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);

		glDepthFunc(convertCompareFunction(state->DepthBufferFunction));
		glDepthMask(state->DepthBufferWriteEnable ? GL_TRUE : GL_FALSE);
		
		// stencil buffer stuff
		if (state->StencilEnable)
			glEnable(GL_STENCIL_TEST);
		else
			glDisable(GL_STENCIL_TEST);

		glStencilFunc(convertCompareFunction(state->StencilFunction), state->ReferenceStencil, 0xffffffff);

		glStencilOp(GL_KEEP, convertStencilOperation(state->StencilPass), convertStencilOperation(state->StencilPass));
	}

	void OpenGlDevice::SetIndices(const IndexBuffer* indices)
	{
		m_indices = static_cast<const GlIndexBuffer*>(indices);
		m_indices->Bind();
	}

	void OpenGlDevice::Clear(const Color& c)
	{
		if (c != m_clearColor)
		{
			setClearColor(c);
		}

		int mask;
		glGetIntegerv(GL_DEPTH_WRITEMASK, &mask);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDepthMask((GLboolean)mask);

        GlException::ThrowIfError(); 
	}

	void OpenGlDevice::Clear(ClearOptions options, const Color& c, float depth, int stencil)
	{
		if (c != m_clearColor)
			setClearColor(c);

		if (depth != m_clearDepth)
			setClearDepth(depth);

		if (stencil != m_clearStencil)
			setClearStencil(stencil);

		int depthMask = 0;
		int gloptions = 0;
		if ((options & ClearOptions_DepthBuffer) == ClearOptions_DepthBuffer)
		{
			gloptions |= GL_DEPTH_BUFFER_BIT;

			// OpenGL seems to need depth buffer writing enabled for glClear() to have an effect
			glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMask);
			glDepthMask(GL_TRUE);
		}
		if ((options & ClearOptions_Stencil) == ClearOptions_Stencil)
			gloptions |= GL_STENCIL_BUFFER_BIT;
		if ((options & ClearOptions_Target) == ClearOptions_Target)
			gloptions |= GL_COLOR_BUFFER_BIT;

		if (gloptions != 0)
			glClear(gloptions);

		if ((options & ClearOptions_DepthBuffer) == ClearOptions_DepthBuffer)
		{
			// restore the original depth mask state
			glDepthMask((GLboolean)depthMask);
		}
	}

	Viewport OpenGlDevice::GetViewport()
	{
		int vp[4];
		glGetIntegerv(GL_VIEWPORT, vp);

		Viewport v;
		v.X = vp[0];
		v.Y = vp[1];
		v.Width = vp[2];
		v.Height = vp[3];

		return v;
	}

	void OpenGlDevice::SetViewport(const Viewport& viewport)
	{
		// OpenGL stores the bottom-left corner, but XNA
		// stores the upper-left corner, so we have to convert.
		int y = m_screenHeight - (viewport.Height + viewport.Y);

		glViewport(viewport.X, y, viewport.Width, viewport.Height);
        
        GlException::ThrowIfError(); 
	}

	void OpenGlDevice::DrawIndexedPrimitives(PrimitiveType primitiveType, int baseVertex, int minVertexIndex, int numVertices, int startIndex, int primitiveCount)
	{
		assert(m_indices != nullptr);
		assert(startIndex + primitiveCount < m_indices->GetIndexCount());

#ifndef NDEBUG
		for (int i = startIndex; i < primitiveCount + startIndex; i++)
		{
			int index = m_indices->GetIndex(i);

			assert(index < m_vertices->GetVertexCount());
            assert(index >= 0);
		}
#endif

		if (m_vertexPointersNeedSetup)
			setupVertexBufferPointers(nullptr);

		IndexElementSize elementSize = m_indices->GetElementSize();

		int size = GL_UNSIGNED_SHORT;
		if (elementSize == IndexElementSize_ThirtyTwoBits)
			size = GL_UNSIGNED_INT;

		GLenum glPrimitiveType;
		if (primitiveType == PrimitiveType_TriangleStrip)
			glPrimitiveType = GL_TRIANGLE_STRIP;
		else
			glPrimitiveType = GL_TRIANGLES;

		glDrawElements(glPrimitiveType, primitiveCount * 3, size, (void*)(startIndex * (int)elementSize));

		GlException::ThrowIfError();
	}

	void OpenGlDevice::DrawPrimitives(PrimitiveType primitiveType, int startVertex, int primitiveCount) {}
	void OpenGlDevice::DrawUserIndexedPrimitives(PrimitiveType primitiveType, void* data, int numVertices, int* indices, int primitiveCount, const VertexDeclaration* vertexDeclaration) {}
	void OpenGlDevice::DrawUserIndexedPrimitives(PrimitiveType primitiveType, void* data, int numVertices, short* indices, int primitiveCount, const VertexDeclaration* vertexDeclaration)
	{
#ifndef NDEBUG
		for (int i = 0; i < primitiveCount * 3; i++)
		{
			int index = indices[i];
            
			assert(index < numVertices);
            assert(index >= 0);
		}
#endif
        
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		GlException::ThrowIfError();

		m_indices = nullptr;
		m_vertices = nullptr;
		m_declaration = vertexDeclaration;
		setupVertexBufferPointers(data);

		GLenum glPrimitiveType;
		if (primitiveType == PrimitiveType_TriangleStrip)
			glPrimitiveType = GL_TRIANGLE_STRIP;
		else
			glPrimitiveType = GL_TRIANGLES;

		glDrawElements(glPrimitiveType, primitiveCount * 3, GL_UNSIGNED_SHORT, indices);
        
        GlException::ThrowIfError();  
	}

	void OpenGlDevice::DrawUserPrimitives(PrimitiveType primitiveType, float* data, int primitiveCount, const VertexDeclaration* vertexDeclaration) {}

	void OpenGlDevice::SetVertexBuffer(const VertexBuffer* vertexBuffer)
	{
		if (vertexBuffer != nullptr)
		{
			dynamic_cast<const GlVertexBuffer*>(vertexBuffer)->Bind();

			m_vertices = vertexBuffer;
			m_declaration = vertexBuffer->GetDeclaration();
		}
		else
		{
			m_vertices = nullptr;
			m_declaration = nullptr;
		}

		m_vertexPointersNeedSetup = true;
	}

	void OpenGlDevice::SetBlendState(const BlendState* blendState)
	{
		assert(blendState != nullptr);
        GlException::ThrowIfError(); 
        
		int colorSrc = convertBlendMode(blendState->ColorSourceBlend);
        int colorDest = convertBlendMode(blendState->ColorDestinationBlend);
        int alphaSrc = convertBlendMode(blendState->AlphaSourceBlend);
        int alphaDest = convertBlendMode(blendState->AlphaDestinationBlend);

        glBlendFuncSeparate(colorSrc, colorDest, alphaSrc, alphaDest);
        
        GlException::ThrowIfError();
        
        glBlendEquationSeparate(convertBlendFunc(blendState->ColorBlendFunction),
            convertBlendFunc(blendState->AlphaBlendFunction));

		GlException::ThrowIfError();
	}

	void OpenGlDevice::Present() {}

	Texture2D* OpenGlDevice::CreateTexture(int width, int height)
	{
		return new GlTexture2D(this, width, height, SurfaceFormat_Color);
	}

	Texture2D* OpenGlDevice::CreateTexture(int width, int height, SurfaceFormat format)
	{
		return new GlTexture2D(this, width, height, format);
	}

	BasicEffect* OpenGlDevice::CreateBasicEffect()
	{
		try
		{
			return new GlslBasicEffect(this);
		}
		catch(Exception& ex)
		{
			throw Exception("Error while creating Basic Effect: " + ex.GetMessage());
		}
	}

	SpriteEffect* OpenGlDevice::CreateSpriteEffect()
	{
		try
		{
			return new GlslSpriteEffect(this);
		}
		catch(Exception& ex)
		{
			throw Exception("Error while creating Sprite Effect: " + ex.GetMessage());
		}
	}

	DualTextureEffect* OpenGlDevice::CreateDualTextureEffect()
	{
		try
		{
			return new GlslDualTextureEffect(this);
		}
		catch(Exception& ex)
		{
			throw Exception("Error while creating Dual Texture Effect: " + ex.GetMessage());
		}
	}

	AlphaTestEffect* OpenGlDevice::CreateAlphaTestEffect()
	{
		try
		{
			return new GlslAlphaTestEffect(this);
		}
		catch(Exception& ex)
		{
			throw Exception("Error while creating Alpha Test Effect: " + ex.GetMessage());
		}
	}

	VertexBuffer* OpenGlDevice::CreateVertexBuffer(const VertexDeclaration* vertexDeclaration, int vertexCount, BufferUsage usage)
	{
		return new GlVertexBuffer(this, vertexDeclaration, vertexCount, usage);
	}

	DynamicVertexBuffer* OpenGlDevice::CreateDynamicVertexBuffer(const VertexDeclaration* vertexDeclaration, int vertexCount, BufferUsage usage)
	{
		return new GlDynamicVertexBuffer(this, vertexDeclaration, vertexCount, usage);
	}

	IndexBuffer* OpenGlDevice::CreateIndexBuffer(IndexElementSize elementSize)
	{
		return new GlIndexBuffer(elementSize);
	}

	void OpenGlDevice::setClearColor(const Color& c)
	{
		m_clearColor = c;
		glClearColor(m_clearColor.R / 255.0f, m_clearColor.G / 255.0f, m_clearColor.B / 255.0f, m_clearColor.A / 255.0f);
	}

	void OpenGlDevice::setClearDepth(float depth)
	{
		m_clearDepth = depth;
#ifdef USING_OPENGLES
		glClearDepthf(depth);
#else
		glClearDepth(depth);
#endif
	}

	void OpenGlDevice::setClearStencil(int stencil)
	{
		m_clearStencil = stencil;
		glClearStencil(stencil);
	}

	void OpenGlDevice::setupVertexBufferPointers(void* verts)
	{
		assert(m_declaration != nullptr);
		assert(m_effect != nullptr);


		for (int i = 0; i < m_declaration->GetNumElements(); i++)
		{
			const GlslAttribute* attrib = m_effect->GetAttribute(m_declaration->GetElements()[i].ElementUsage, m_declaration->GetElements()[i].UsageIndex);
			if (attrib == nullptr)
				continue;

			glEnableVertexAttribArray(attrib->GlHandle);
			glVertexAttribPointer(attrib->GlHandle, (int)m_declaration->GetElements()[i].ElementFormat, GL_FLOAT, 0, 
				m_declaration->GetStride(), (byte*)verts + m_declaration->GetElements()[i].Offset);
		}

		m_vertexPointersNeedSetup = false;

		GlException::ThrowIfError();
	}

	int OpenGlDevice::convertCompareFunction(CompareFunction func)
	{
		if (func == CompareFunction_Never)
			return GL_NEVER;
		if (func == CompareFunction_Less)
			return GL_LESS;
		if (func == CompareFunction_Equal)
			return GL_EQUAL;
		if (func == CompareFunction_LessEqual)
			return GL_LEQUAL;
		if (func == CompareFunction_Greater)
			return GL_GREATER;
		if (func == CompareFunction_NotEqual)
			return GL_NOTEQUAL;
		if (func == CompareFunction_GreaterEqual)
			return GL_GEQUAL;
		if (func == CompareFunction_Always)
			return GL_ALWAYS;

		return GL_ALWAYS;
	}

	CompareFunction OpenGlDevice::convertCompareFunction(int func)
	{
		if (func == GL_NEVER)
			return CompareFunction_Never;
		if (func == GL_LESS)
			return CompareFunction_Less;
		if (func == GL_EQUAL)
			return CompareFunction_Equal;
		if (func == GL_LEQUAL)
			return CompareFunction_LessEqual;
		if (func == GL_GREATER)
			return CompareFunction_Greater;
		if (func == GL_NOTEQUAL)
			return CompareFunction_NotEqual;
		if (func == GL_GEQUAL)
			return CompareFunction_GreaterEqual;
		if (func == GL_ALWAYS)
			return CompareFunction_Always;

		return CompareFunction_Always;
	}

	int OpenGlDevice::convertStencilOperation(StencilOperation op)
	{
		if (op == StencilOperation_Keep)
			return GL_KEEP;
		if (op == StencilOperation_Zero)
			return GL_ZERO;
		if (op == StencilOperation_Replace)
			return GL_REPLACE;
		if (op == StencilOperation_IncrementSaturation)
			return GL_INCR;
		if (op == StencilOperation_Increment)
			return GL_INCR_WRAP;
		if (op == StencilOperation_DecrementSaturation)
			return GL_DECR;
		if (op == StencilOperation_Decrement)
			return GL_DECR_WRAP;
		if (op == StencilOperation_Invert)
			return GL_INVERT;

		return GL_KEEP;
	}

	StencilOperation OpenGlDevice::convertStencilOperation(int op)
	{
		if (op == GL_KEEP)
			return StencilOperation_Keep;
		if (op == GL_ZERO)
			return StencilOperation_Zero;
		if (op == GL_REPLACE)
			return StencilOperation_Replace;
		if (op == GL_INCR)
			return StencilOperation_IncrementSaturation;
		if (op == GL_INCR_WRAP)
			return StencilOperation_Increment;
		if (op == GL_DECR)
			return StencilOperation_DecrementSaturation;
		if (op == GL_DECR_WRAP)
			return StencilOperation_Decrement;
		if (op == GL_INVERT)
			return StencilOperation_Invert;

		return StencilOperation_Keep;
	}

	int OpenGlDevice::convertBlendMode(Blend mode)
	{
		if (mode == Blend_One)
            return GL_ONE;
        if (mode == Blend_Zero)
            return GL_ZERO;
        if (mode == Blend_SourceAlpha)
            return GL_SRC_ALPHA;
        if (mode == Blend_DestinationAlpha)
            return GL_DST_ALPHA;
        if (mode == Blend_InverseSourceAlpha)
            return GL_ONE_MINUS_SRC_ALPHA;
        if (mode == Blend_InverseDestinationAlpha)
            return GL_ONE_MINUS_DST_ALPHA;

        return GL_ZERO;
	}

	// these are not available in OpenGL ES
#ifndef GL_MIN
#define GL_MIN 0x8007
#endif
#ifndef GL_MAX
#define GL_MAX 0x8008
#endif

	int OpenGlDevice::convertBlendFunc(BlendFunction func)
	{
		if (func == BlendFunction_Add)
            return GL_FUNC_ADD;
        else if (func == BlendFunction_Subtract)
            return GL_FUNC_SUBTRACT;
        else if (func == BlendFunction_ReverseSubtract)
            return GL_FUNC_REVERSE_SUBTRACT;
        else if (func == BlendFunction_Min)
            return GL_MIN;
        else
            return GL_MAX;
	}

	void GlException::ThrowIfError()
	{
		int err = glGetError();
		if (err != 0)
			throw GlException();
	}
}
}
}
