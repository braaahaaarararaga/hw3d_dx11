#pragma once

#include <string>
#include <vector>
#include <DirectXMath.h>

static constexpr size_t MAX_BONES = 256;

struct BoneData
{
	std::string name;
	int index;
	DirectX::XMMATRIX inverse_transform;
};
struct BoneNode
{
	DirectX::XMMATRIX local_transform;
	int parent_index;
};

