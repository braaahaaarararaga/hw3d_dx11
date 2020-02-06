#pragma once
#include <unordered_map>



class AttributeInfo
{
public:
	enum class VertexAttribute
	{
		Invalid = -1,
		VertexId,
		Position,
		Color,
		Normal,
		UV,
		Tangent,
		Bitangent,
		BoneId,
		BoneWeight,
		InstanceData,
		TotalAttributes
	};

	static VertexAttribute SemanticToAttribute(const std::string& semantic)
	{
		static std::unordered_map<std::string, VertexAttribute> s_mapConvert;
		static bool initialized = false;
		if (!initialized)
		{
			initialized = true;
			s_mapConvert["SV_VertexID"] = VertexAttribute::VertexId;
			s_mapConvert["POSITION"] = VertexAttribute::Position;
			s_mapConvert["COLOR"] = VertexAttribute::Color;
			s_mapConvert["NORMAL"] = VertexAttribute::Normal;
			s_mapConvert["TEXCOORD"] = VertexAttribute::UV;
			s_mapConvert["TANGENT"] = VertexAttribute::Tangent;
			s_mapConvert["BITANGENT"] = VertexAttribute::Bitangent;
			s_mapConvert["BONEID"] = VertexAttribute::BoneId;
			s_mapConvert["BONEWEIGHT"] = VertexAttribute::BoneWeight;
			s_mapConvert["INSTANCE_DATA"] = VertexAttribute::InstanceData;
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
			s_mapConvert[VertexAttribute::VertexId] = "SV_VertexID";
			s_mapConvert[VertexAttribute::Position] = "POSITION";
			s_mapConvert[VertexAttribute::Color] = "COLOR";
			s_mapConvert[VertexAttribute::Normal] = "NORMAL";
			s_mapConvert[VertexAttribute::UV] = "TEXCOORD";
			s_mapConvert[VertexAttribute::Tangent] = "TANGENT";
			s_mapConvert[VertexAttribute::Bitangent] = "BITANGENT";
			s_mapConvert[VertexAttribute::BoneId] = "BONEID";
			s_mapConvert[VertexAttribute::BoneWeight] = "BONEWEIGHT";
			s_mapConvert[VertexAttribute::InstanceData] = "INSTANCE_DATA";
		}

		auto it = s_mapConvert.find(attribute);
		if (it == s_mapConvert.end())
		{
			return "";
		}

		return it->second;
	}
};
