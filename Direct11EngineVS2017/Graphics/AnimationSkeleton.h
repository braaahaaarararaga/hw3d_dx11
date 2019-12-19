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

template <typename T>
struct KeyFrame
{
	T value;
	float timestamp;
	
	bool operator==(const KeyFrame& rhs)
		return value == rhs.value;
	bool operator!=(const KeyFrame& rhs)
		return value != rhs.value;
	bool operator<(const KeyFrame& rhs)
		return timestamp < rhs.timestamp;
	bool operator<=(const KeyFrame& rhs)
		return timestamp <= rhs.timestamp;
	bool operator>(const KeyFrame& rhs)
		return timestamp > rhs.timestamp;
	bool operator>=(const KeyFrame& rhs)
		return timestamp >= rhs.timestamp;
		
};

using PosKeyFrame = KeyFrame<DirectX::XMFLOAT3>; // Value is position in bone space
using RotKeyFrame = KeyFrame<DirectX::XMFLOAT4>; // Value is rotation in bone space as a quaternion

class AnimationChannel
{
public:
	// Returns bone space transform of this bone in the pose at the given timestamp
	DirectX::XMMATRIX GetSample(float timestamp) const;
public:
	std::vector<PosKeyFrame> position_keyframes;
	std::vector<RotKeyFrame> rotation_keyframes;
	int node_index;
private:
	void ResetCache() const;
private:
	mutable size_t last_pos_index = 0;
	mutable size_t last_rot_index = 0;
	mutable float last_timestamp = 0.0f;
};

struct MeshAnimation
{
	std::string name;
	std::vector<AnimationChannel> channels;
	float duration;

	// Returns bone space transforms for each node in the skeleton at the given timestamp
	std::vector<DirectX::XMMATRIX> GetSample(float timestamp, const std::vector<BoneNode>& original_skeleton) const;
};