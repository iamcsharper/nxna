#ifndef GRAPHICS_GLSLEFFECT_H
#define GRAPHICS_GLSLEFFECT_H

#include <vector>
#include <string>
#include "../../NxnaConfig.h"
#include "../Effect.h"
#include "../IEffectPimpl.h"
#include "../VertexDeclaration.h"

NXNA_DISABLE_OVERRIDE_WARNING

namespace Nxna
{
namespace Graphics
{
	class SamplerStateCollection;

namespace OpenGl
{
	class OpenGlDevice;

	struct GlslAttribute
	{
		std::string Name;
		int Handle;
		int GlHandle;
		int Index;
		Nxna::Graphics::VertexElementUsage Usage;
	};

	class GlslEffect : public Pvt::IEffectPimpl
	{
		friend class OpenGlDevice;

		struct GlslUniform
		{
			EffectParameter* Param;
			unsigned int Program;
			unsigned int Uniform;
		};

		struct GlslProgram
		{
			unsigned int Program;
			std::vector<GlslAttribute> Attributes;
			std::vector<GlslUniform> Uniforms;
		};

		NXNA_ENUM(CacheType)
			CacheType_Float,
			CacheType_Int
		END_NXNA_ENUM(CacheType)

		struct ParamCache
		{
			CacheType CachedValueType;
			int CachedValueSize;
			int CachedValue[4 * 4];
		};

		struct strcmpop
		{
			bool operator()(const char* a, const char* b) const
			{
				return strcmp(a, b) < 0;
			}
		};
		
		typedef std::map<const char*, EffectParameter*, strcmpop> ParamMap;
		ParamMap m_parameters;

		std::vector<GlslProgram> m_programs;
		std::vector<GlslAttribute> m_attributes;
		std::vector<EffectParameter*> m_parameterList;
		OpenGlDevice* m_device;
		
	protected:
		std::vector<EffectParameter*> m_textureParams;

	private:
		static const int MAX_ATTRIB_SIZE = 256;
		static char m_attribNameBuffer[MAX_ATTRIB_SIZE];
		static int m_boundProgramIndex;

	public:
		GlslEffect(OpenGlDevice* device, Effect* parent, const char* vertexSource, const char* fragmentSource);
		GlslEffect(OpenGlDevice* device, Effect* parent);
		virtual ~GlslEffect();

		virtual EffectParameter* GetParameter(const char* name) override
		{
			ParamMap::iterator itr = m_parameters.find(name);
			if (itr != m_parameters.end())
			{
				return (*itr).second;
			}
			
			return nullptr;
		}

		virtual EffectParameter* GetParameter(int index) override
		{
			return m_parameterList[index];
		}

		virtual int GetNumParameters() override
		{
			return m_parameterList.size();
		}

		virtual void AddConstantBuffer(bool vertex, bool pixel, int sizeInBytes, int numParameters) override;
		virtual EffectParameter* AddParameter(const char* name, EffectParameterType type, int numElements, int constantBufferIndex, int constantBufferConstantIndex, int constantBufferOffset) override;

		const GlslAttribute* GetAttribute(VertexElementUsage usage, int index);

		void ProcessSource(const char* vertexSource, const char* fragmentSource, std::string& vertexResult, std::string& fragResult);
		virtual EffectTechnique* CreateProgram(const char* name, bool hidden, const byte* vertexSource, int vertexSourceLength, const byte* fragSource, int fragSourceLength) override;
		virtual void AddAttributeToProgram(int programIndex, const char* name, EffectParameterType type, int numElements, Semantic semantic, int usageIndex) override;

		virtual int ScoreProfile(ShaderProfile profile) override;

		void ApplyProgram(int programIndex);

		void ApplySamplerStates(SamplerStateCollection* samplerStates);

		std::vector<EffectParameter*>& GetTextureParams() { return m_textureParams; }

	protected:
		virtual void Apply(int techniqueIndex) override;

	private:
		int compile(const char* source[], int numSource, bool vertex);
		void processSource(std::string& source, bool vertex);
		void loadUniformInfo(GlslProgram& program);
		void loadAttributeInfo(GlslProgram& program);
		std::string extractAttribInfo(const char* vertexShaderSource);
		void replaceAll(std::string& original, const std::string& toRemove, const std::string& toPut);

		static const char* lastIndexNotSpace(const char* str, int startIndex)
        {
            for (int i = startIndex; i >= 0; i--)
            {
                if (str[i] != ' ')
                    return &str[i];
            }

            // not found :(
            return nullptr;
        }

        static const char* firstIndexNotSpace(const char* str)
        {
            for (size_t i = 0; i < strlen(str); i++)
            {
                if (str[i] != ' ')
                    return &str[i];
            }

            // not found :(
            return nullptr;
        }

		static const char* lastIndexOf(const char* str, char chr, int start)
		{
			for (int i = start; i >= 0; i--)
			{
				if (str[i] == chr)
					return &str[i];
			}

			return nullptr;
		}

		static const char* strchrAny(const char* str, const char* list, int length)
		{
			for (size_t i = 0; i < strlen(str); i++)
			{
				for (int j = 0; j < length; j++)
				{
					if (str[i] == list[j])
						return &str[i];
				}
			}

			return nullptr;
		}

	};
}
}
}

NXNA_ENABLE_OVERRIDE_WARNING

#endif // GRAPHICS_GLSLEFFECT_H