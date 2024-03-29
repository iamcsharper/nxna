#include <cstring>
#include <cassert>
#include <sstream>
#include "OpenGL.h"
#include "GlslEffect.h"
#include "GlTexture2D.h"
#include "OpenGLDevice.h"
#include "../SamplerStateCollection.h"
#include "GlslSource.h"

namespace Nxna
{
namespace Graphics
{
namespace OpenGl
{
	char GlslEffect::m_attribNameBuffer[];
	int GlslEffect::m_boundProgramIndex = -1;

	GlslEffect::GlslEffect(OpenGlDevice* device, Effect* parent, const char* vertexSource, const char* fragmentSource)
		: Pvt::IEffectPimpl(parent)
	{
		m_device = device;

		std::string vertexResult, fragResult;
		ProcessSource(vertexSource, fragmentSource, vertexResult, fragResult);

		CreateProgram("default", false, (const byte*)vertexResult.c_str(), vertexResult.length(), (const byte*)fragResult.c_str(), fragResult.length());
	}

	GlslEffect::GlslEffect(OpenGlDevice* device, Effect* parent)
		: Pvt::IEffectPimpl(parent)
	{
		assert(device != nullptr);
		m_device = device;
	}

	GlslEffect::~GlslEffect()
	{
		// delete all the parameters
		for (std::vector<EffectParameter*>::size_type i = 0; i < m_parameterList.size(); i++)
		{
			EffectParameter* p = m_parameterList[i];
			delete p;
		}

		// delete all the programs
		for (std::vector<EffectParameter*>::size_type i = 0; i < m_programs.size(); i++)
		{
			glDeleteProgram(m_programs[i].Program);
		}
	}

	void GlslEffect::ProcessSource(const char* vertexSource, const char* fragmentSource, std::string& vertexResult, std::string& fragResult)
	{
		assert(vertexSource != nullptr);
		assert(fragmentSource != nullptr);

		vertexResult = vertexSource;
		fragResult = fragmentSource;

		processSource(vertexResult, true);
		processSource(fragResult, false);
	}

	EffectTechnique* GlslEffect::CreateProgram(const char* name, bool hidden, const byte* vertexSource, int vertexSourceLength, const byte* fragSource, int fragSourceLength)
	{
		if (hidden != false || vertexSource != nullptr || fragSource != nullptr)
		{
			glGetError();

			GlslSource source((const char*)vertexSource, (const char*)fragSource, m_device->GetGlslVersion());

			// HACK: We *should* be able to send each piece as a separate element in an array,
			// but to get this to work with AMD hardware we have to concatenate beforehand.
			// Otherwise we get shader compiler errors.
			char buffer[256];
#ifdef NXNA_PLATFORM_WIN32
			_snprintf_s(buffer, 256, "#define %s \n", name);
#else
			snprintf(buffer, 256, "#define %s \n", name);
#endif
			const char* defines[] = { buffer };

			GlslProgram program;
			program.Program = source.Build(defines, 1);

			loadUniformInfo(program);
			loadAttributeInfo(program);

			m_programs.push_back(program);
		}

		return CreateTechnique(name, hidden);
	}

	void GlslEffect::AddAttributeToProgram(int programIndex, const char* name, EffectParameterType type, int numElements, Semantic semantic, int usageIndex)
	{
		GlslAttribute attrib;
		attrib.Name = name;
		attrib.Index = usageIndex;
		attrib.GlHandle = glGetAttribLocation(m_programs[programIndex].Program, name);

		if (attrib.GlHandle < 0)
			throw GraphicsException("Unable to find OpenGL shader attribute");
		
		switch(semantic)
		{
		case Semantic::Position:
			attrib.Usage = VertexElementUsage::Position;
			break;
		case Semantic::TexCoord:
			attrib.Usage = VertexElementUsage::TextureCoordinate;
			break;
		case Semantic::Color:
			attrib.Usage = VertexElementUsage::Color;
		}

		m_programs[programIndex].Attributes.push_back(attrib);
	}

	int GlslEffect::ScoreProfile(ShaderProfile profile)
	{
		if (profile == ShaderProfile::GLSL_130)
			return 1;

		return 0;
	}

	/*void GlslEffect::SetParameter(EffectParameter* param, Texture2D* texture)
	{
		GlTexture2D* glTexture = static_cast<GlTexture2D*>(texture);

		// look for the index of the texture
		int texindex = 0;
		for (int i = 0; i < m_textureParams.size(); i++)
		{
			if (m_textureParams[i] == param)
			{
				texindex = i;
				break;
			}
		}

		glActiveTexture(GL_TEXTURE0 + texindex);
		glBindTexture(GL_TEXTURE_2D, glTexture->GetGlTexture());

		int index = (int)reinterpret_cast<intptr_t>(param->GetHandle());
		ParamCache& cache = m_cache[index];
		cache.CachedValueType = CacheType_Int;
		cache.CachedValueSize = 1;
		cache.CachedValue[0] = texindex;
	}

	void GlslEffect::SetParameter(EffectParameter* param, const Vector4& value)
	{
		int index = (int)reinterpret_cast<intptr_t>(param->GetHandle());

		ParamCache& cache = m_cache[index];
		cache.CachedValueType = CacheType_Float;
		cache.CachedValueSize = 4;
		memcpy(&cache.CachedValue, &value, sizeof(float) * 4);
	}

	void GlslEffect::SetParameter(EffectParameter* param, float matrix4x4[])
	{
		int index = (int)reinterpret_cast<intptr_t>(param->GetHandle());

		ParamCache& cache = m_cache[index];
		cache.CachedValueType = CacheType_Float;
		cache.CachedValueSize = 16;
		memcpy(&cache.CachedValue, matrix4x4, sizeof(float) * 16);
	}

	void GlslEffect::SetParameter(EffectParameter* param, const Matrix& matrix)
	{
		int index = (int)reinterpret_cast<intptr_t>(param->GetHandle());

		ParamCache& cache = m_cache[index];
		cache.CachedValueType = CacheType_Float;
		cache.CachedValueSize = 16;
		memcpy(&cache.CachedValue, matrix.C, sizeof(float) * 16);
	}*/

	void GlslEffect::ApplyProgram(int programIndex)
	{
		// bind the effect
		glUseProgram(m_programs[programIndex].Program);
		m_device->SetCurrentEffect(this);
		m_boundProgramIndex = programIndex;

		int usedTextureUnits = 0;

		// go through the cached values and send them to OpenGL
		for (std::vector<GlslUniform>::iterator itr = m_programs[programIndex].Uniforms.begin();
			itr != m_programs[programIndex].Uniforms.end(); ++itr)
		{
			EffectParameter* param = (*itr).Param;
			EffectParameterType type = param->GetType();
			int numElements = param->GetNumElements();

			if (type == EffectParameterType::Int32)
			{
				glUniform1iv((*itr).Uniform, numElements, GetRawValue((*itr).Param));
			}
			else if (type == EffectParameterType::Single)
			{
				if (numElements == 1)
					glUniform1fv((*itr).Uniform, 1, (float*)GetRawValue((*itr).Param));
				else if (numElements == 2)
					glUniform2fv((*itr).Uniform, 1, (float*)GetRawValue((*itr).Param));
				else if (numElements == 3)
					glUniform3fv((*itr).Uniform, 1, (float*)GetRawValue((*itr).Param));
				else if (numElements == 4)
					glUniform4fv((*itr).Uniform, 1, (float*)GetRawValue((*itr).Param));
				else if (numElements == 16)
					glUniformMatrix4fv((*itr).Uniform, 1, GL_FALSE, (float*)GetRawValue(param));
			}
			else if (type == EffectParameterType::Texture2D)
			{
				if ((*itr).Param->GetValueTexture2D() != nullptr)
				{
					// look for the index of the texture
					int texindex = 0;
					for (std::vector<EffectParameter*>::size_type i = 0; i < m_textureParams.size(); i++)
					{
						if (m_textureParams[i] == (*itr).Param)
						{
							texindex = i;
							break;
						}
					}

					glActiveTexture(GL_TEXTURE0 + usedTextureUnits);
					glBindTexture(GL_TEXTURE_2D, static_cast<GlTexture2D*>(param->GetValueTexture2D()->GetPimpl())->GetGlTexture());
					glUniform1i((*itr).Uniform, usedTextureUnits);

					usedTextureUnits++;
				}
			}

			GlException::ThrowIfError(__FILE__, __LINE__);
		}
	}

	void GlslEffect::ApplySamplerStates(SamplerStateCollection* samplerStates)
	{
		int usedTextureUnits = 0;

		for (std::vector<GlslUniform>::iterator itr = m_programs[m_boundProgramIndex].Uniforms.begin();
			itr != m_programs[m_boundProgramIndex].Uniforms.end(); ++itr)
		{
			EffectParameterType type = (*itr).Param->GetType();

			if (type == EffectParameterType::Texture2D)
			{
				GlTexture2D* glTex = static_cast<GlTexture2D*>((*itr).Param->GetValueTexture2D()->GetPimpl());
				
				if (glTex != nullptr)
				{
					// look for the index of the texture
					int texindex = 0;
					for (std::vector<EffectParameter*>::size_type i = 0; i < m_textureParams.size(); i++)
					{
						if (m_textureParams[i] == (*itr).Param)
						{
							texindex = i;
							break;
						}
					}

					glActiveTexture(GL_TEXTURE0 + usedTextureUnits);
					glBindTexture(GL_TEXTURE_2D, glTex->GetGlTexture());
					
					glTex->SetSamplerState(samplerStates->Get(usedTextureUnits));

					usedTextureUnits++;
				}
			}
		}
	}

	void GlslEffect::AddConstantBuffer(bool vertex, bool pixel, int sizeInBytes, int numParameters)
	{
		// nothing... not using cbuffers (yet)
	}

	EffectParameter* GlslEffect::AddParameter(const char* name, EffectParameterType type, int numElements, int constantBufferIndex, int constantBufferConstantIndex, int constantBufferOffset)
	{
		EffectParameter* param = CreateParameter(m_parent, type, numElements, nullptr, name);

		m_parameters.insert(ParamMap::value_type(param->Name.c_str(), param));

		m_parameterList.push_back(param);

		return param;
	}

	const GlslAttribute* GlslEffect::GetAttribute(VertexElementUsage usage, int index)
	{
		for (std::vector<GlslAttribute>::iterator itr = m_programs[m_boundProgramIndex].Attributes.begin();
			itr != m_programs[m_boundProgramIndex].Attributes.end(); ++itr)
		{
			if ((*itr).Index == index &&
				(*itr).Usage == usage)
				return &(*itr);
		}

		return nullptr;
	}

	void GlslEffect::Apply(int techniqueIndex)
	{
		// which program do we use?
		assert(m_programs.empty() == false);

		ApplyProgram(techniqueIndex);
	}

	int GlslEffect::compile(const char* source[], int numSource, bool vertex)
	{
		int shader;
		if (vertex)
		{
			shader = glCreateShader(GL_VERTEX_SHADER);
		}
		else
		{
			shader = glCreateShader(GL_FRAGMENT_SHADER);
		}
        if (shader == 0)
            throw GraphicsException("Unable to create shader");

		glShaderSource(shader, numSource, source, nullptr);

		glCompileShader(shader);

		int logLength;
		char logBuffer[256];
		glGetShaderInfoLog(shader, 255, &logLength, logBuffer);
		logBuffer[255] = 0;

		int result;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

		if (result != GL_TRUE)
			throw GraphicsException(std::string("Unable to compile shader: ") + logBuffer);

		return shader;
	}

	void GlslEffect::processSource(std::string& source, bool vertex)
	{
		// extract all the attrib info
		if (vertex)
			source = extractAttribInfo(source.c_str());
	}

	void GlslEffect::loadUniformInfo(GlslProgram& program)
	{
		int numUniforms;
		glGetProgramiv(program.Program, GL_ACTIVE_UNIFORMS, &numUniforms);

		for (int i = 0; i < numUniforms; i++)
		{
			GLenum type;
			int size;
			char nameBuffer[256];
			glGetActiveUniform(program.Program, i, 255, nullptr, &size, &type, nameBuffer);
			nameBuffer[255] = 0;

			int location = glGetUniformLocation(program.Program, nameBuffer);

			EffectParameter* param = GetParameter(nameBuffer);

			if (param == nullptr)
			{
				EffectParameterType pType;
				if (type == GL_BOOL || type == GL_BOOL_VEC2 || type == GL_BOOL_VEC3 || type == GL_BOOL_VEC4)
					pType = EffectParameterType::Bool;
				else if (type == GL_INT || type == GL_INT_VEC2 || type == GL_INT_VEC3 || type == GL_INT_VEC4)
					pType = EffectParameterType::Int32;
				else if (type == GL_FLOAT || type == GL_FLOAT_VEC2 || type == GL_FLOAT_VEC3 || type == GL_FLOAT_VEC4 || type == GL_FLOAT_MAT4)
					pType = EffectParameterType::Single;
#ifndef USING_OPENGLES
				else if (type == GL_SAMPLER_1D)
					pType = EffectParameterType::Texture1D;
#endif
				else if (type == GL_SAMPLER_2D)
					pType = EffectParameterType::Texture2D;
#ifndef USING_OPENGLES
				else if (type == GL_SAMPLER_3D)
					pType = EffectParameterType::Texture3D;
#endif
				else if (type == GL_SAMPLER_CUBE)
					pType = EffectParameterType::TextureCube;
				else
					pType = EffectParameterType::Void;
				// what about EffectParameterType_String? does GLSL even support that?

				int numElements = 1;
				if (type == GL_BOOL ||
					type == GL_INT ||
					type == GL_FLOAT ||
#ifndef USING_OPENGLES
					type == GL_SAMPLER_1D || 
#endif
					type == GL_SAMPLER_2D || 
#ifndef USING_OPENGLES
					type == GL_SAMPLER_3D ||
#endif
					type == GL_SAMPLER_CUBE)
					numElements = 1;
				else if (type == GL_BOOL_VEC2 ||
					type == GL_INT_VEC2 ||
					type == GL_FLOAT_VEC2)
					numElements = 2;
				else if (type == GL_BOOL_VEC3 ||
					type == GL_INT_VEC3 ||
					type == GL_FLOAT_VEC3)
					numElements = 3;
				else if (type == GL_BOOL_VEC4 ||
					type == GL_INT_VEC4 ||
					type == GL_FLOAT_VEC4)
					numElements = 4;
				else if (type == GL_FLOAT_MAT4)
					numElements = 16;

				// add the parameter, with the handle being the index of this parameter
				param = this->AddParameter(nameBuffer, pType, numElements, 0, 0, 0);

				// if this is a texture param then add it to the list of
				// texture parameters
				if (pType == EffectParameterType::Texture ||
					pType == EffectParameterType::Texture1D ||
					pType == EffectParameterType::Texture2D ||
					pType == EffectParameterType::Texture3D ||
					pType == EffectParameterType::TextureCube)
				{
					m_textureParams.push_back(param);
				}
			}

			GlslUniform uniform;
			uniform.Param = param;
			uniform.Program = program.Program;
			uniform.Uniform = location;

			program.Uniforms.push_back(uniform);
		}
	}

	void GlslEffect::loadAttributeInfo(GlslProgram& program)
	{
		int numAttributes;
		glGetProgramiv(program.Program, GL_ACTIVE_ATTRIBUTES, &numAttributes);

		for (int i = 0; i < numAttributes; i++)
		{
			int length, size;
			GLenum type;
			glGetActiveAttrib(program.Program, i, MAX_ATTRIB_SIZE, &length, &size, &type, m_attribNameBuffer);

			for (std::vector<GlslAttribute>::size_type j = 0; j < m_attributes.size(); j++)
			{
				if (strcmp(m_attributes[j].Name.c_str(), m_attribNameBuffer) == 0)
				{
					GlslAttribute attr = m_attributes[j];
					attr.GlHandle = glGetAttribLocation(program.Program, m_attribNameBuffer);

					if (attr.GlHandle < 0)
						throw GraphicsException("Unable to find OpenGL handle of attribute");

					program.Attributes.push_back(attr);

					break;
				}
			}
		}
	}

	std::string GlslEffect::extractAttribInfo(const char* vertexShaderSource)
	{
		std::string result;

		char nameBuffer[256];
		char usageBuffer[256];
		char numbers[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

		const char* cursor = vertexShaderSource;
		for (;;)
		{
			const char* colon = strchr(cursor, ':');
			if (colon == nullptr)
			{
				result += cursor;
				break;
			}

			const char* nameEnd = lastIndexNotSpace(cursor, (int)(colon - cursor - 1));
			const char* nameStart = lastIndexOf(cursor, ' ', (int)(nameEnd - cursor - 1)) + 1;
#if defined NXNA_PLATFORM_WIN32
			strncpy_s(nameBuffer, nameStart, nameEnd - nameStart + 1);
#else
			strncpy(nameBuffer, nameStart, nameEnd - nameStart + 1);
#endif
			nameBuffer[nameEnd - nameStart + 1] = 0;

			GlslAttribute attrib;
			attrib.Name = nameBuffer;
			attrib.Usage = VertexElementUsage::Position;
			attrib.Index = 0;
			attrib.GlHandle = 0;

			const char* semicolon = strchr(colon, ';');
			const char* usageStart = firstIndexNotSpace(colon + 1);
#if defined NXNA_PLATFORM_WIN32
			strncpy_s(usageBuffer, usageStart, semicolon - usageStart);
#else
			strncpy(usageBuffer, usageStart, semicolon - usageStart);
#endif
			usageBuffer[semicolon - usageStart] = 0;

			const char* firstNumber = strchrAny(usageBuffer, numbers, 10);
			if (firstNumber != nullptr)
			{
				attrib.Index = firstNumber[0] - '0';
			}

			if (strncmp(usageBuffer, "POSITION", 8) == 0)
				attrib.Usage = VertexElementUsage::Position;
			else if (strncmp(usageBuffer, "TEXCOORD", 8) == 0)
				attrib.Usage = VertexElementUsage::TextureCoordinate;
			else if (strncmp(usageBuffer, "COLOR", 5) == 0)
				attrib.Usage = VertexElementUsage::Color;
			else if (strncmp(usageBuffer, "NORMAL", 6) == 0)
				attrib.Usage = VertexElementUsage::Normal;

			result.append(cursor, colon - cursor);

			cursor = semicolon;

			m_attributes.push_back(attrib);
		}

		return result;
	}

	void GlslEffect::replaceAll(std::string& original, const std::string& toRemove, const std::string& toPut)
	{
		std::string::size_type offset = 0;
		while(offset < original.length())
		{
			std::string::size_type pos = original.find(toRemove, offset);
			if (pos == std::string::npos) break;
			if (pos == 0 || (pos > 0 && (original[pos - 1] == ' ' || original[pos - 1] == '\t' || original[pos - 1] == '\n' 
				|| original[pos - 1] == '(')))
			{
				original.erase(pos, toRemove.size());
				original.insert(pos, toPut);
			}
			
			offset += toPut.size();
		}
	}
}
}
}
