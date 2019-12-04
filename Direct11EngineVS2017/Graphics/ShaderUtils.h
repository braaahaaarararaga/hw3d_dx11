#pragma once
#include <unordered_map>



class AttributeInfo
{
public:
	enum class VertexAttribute
	{
		Invalid = -1,
		Position,
		Color,
		Normal,
		TexCoord,
		Tangent,
		Bitangent,
		BoneName,
		BoneWeight,
		TotalAttributes
	};

	static VertexAttribute SemanticToAttribute(const std::string& semantic)
	{
		static std::unordered_map<std::string, VertexAttribute> s_mapConvert;
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_mapConvert["POSITION"] = VertexAttribute::Position;
			s_mapConvert["COLOR"] = VertexAttribute::Color;
			s_mapConvert["NORMAL"] = VertexAttribute::Normal;
			s_mapConvert["TEXCOORD"] = VertexAttribute::TexCoord;
			s_mapConvert["TANGENT"] = VertexAttribute::Tangent;
			s_mapConvert["BITANGENT"] = VertexAttribute::Bitangent;
		}

		auto it = s_mapConvert.find(semantic);
		if (it == s_mapConvert.end())
		{
			return VertexAttribute::Invalid;
		}

		return it->second;
	}

	static std::string AttributeToSemantic(const VertexAttribute attribute)
	{
		static std::unordered_map<VertexAttribute, std::string> s_mapConvert;
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_mapConvert[VertexAttribute::Position] = "POSITION";
			s_mapConvert[VertexAttribute::Color] = "COLOR";
			s_mapConvert[VertexAttribute::Normal] = "NORMAL";
			s_mapConvert[VertexAttribute::TexCoord] = "TEXCOORD";
			s_mapConvert[VertexAttribute::Tangent] = "TANGENT";
			s_mapConvert[VertexAttribute::Bitangent] = "BITANGENT";
		}

		auto it = s_mapConvert.find(attribute);
		if (it == s_mapConvert.end())
		{
			return "";
		}

		return it->second;
	}
};
