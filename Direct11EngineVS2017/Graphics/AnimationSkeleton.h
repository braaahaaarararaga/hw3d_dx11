#pragma once

#include <string>
#include <vector>
#include <DirectXMath.h>
#include "ConstantBuffer.h"

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

struct CB_Bones
{
	DirectX::XMMATRIX bone_Transforms[MAX_BONES];
};

template <typename T>
struct KeyFrame
{
	T value;
	float timestamp;
	
	bool operator==(const KeyFrame& rhs)
	{
		return value == rhs.value;
	}
	bool operator!=(const KeyFrame& rhs)
	{
		return value != rhs.value;
	}
	bool operator<(const KeyFrame& rhs)
	{
		return timestamp < rhs.timestamp;
	}
	bool operator<=(const KeyFrame& rhs)
	{
		return timestamp <= rhs.timestamp;
	}
	bool operator>(const KeyFrame& rhs)
	{
		return timestamp > rhs.timestamp;
	}
	bool operator>=(const KeyFrame& rhs)
	{
		return timestamp >= rhs.timestamp;
	}
		
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

class MeshAnimator
{
public:
	MeshAnimator() = default;
	MeshAnimator(std::vector<BoneNode> original_skeleton, std::vector<BoneData> bones, std::vector<MeshAnimation> animations, ConstantBuffer<CB_Bones>* cbuf_bone)
		:
		m_OriginalSkeleton(std::move(original_skeleton)),
		m_Bones(std::move(bones)),
		m_Animations(std::move(animations)),
		m_cbufBones(cbuf_bone)
	{}

	void Bind(ID3D11DeviceContext* deviceContext);

	size_t GetNumAnimations() const { return m_Animations.size(); }
	const MeshAnimation& GetAnimation(size_t index) const { return m_Animations[index]; }
	const MeshAnimation* GetAnimationByName(const std::string& name) const;

	size_t GetCurrentAnimationIndex() const { return m_iCurrentAnimation; }
	void SetCurrentAnimationIndex(size_t index) { m_iCurrentAnimation = index; }
	const MeshAnimation& GetCurrentAnimation() const { return GetAnimation(GetCurrentAnimationIndex()); }

	float GetTimestamp() const { return m_flTimestamp; }
	void SetTimestamp(float timestamp) { m_flTimestamp = timestamp; }
private:
	void GetPoseOffsetTransforms(DirectX::XMMATRIX* out, const MeshAnimation& animation, float timestamp) const;

private:
	std::vector<BoneData> m_Bones;
	std::vector<BoneNode> m_OriginalSkeleton;
	std::vector<MeshAnimation> m_Animations;

	DirectX::XMMATRIX bone_Transforms[MAX_BONES];
	ConstantBuffer<CB_Bones>* m_cbufBones;
	size_t m_iCurrentAnimation = 0;
	float m_flTimestamp = 0.0f;
	float m_preSampleTimestamp = -1.0f;
};

class AnimationComponent
{
public:
	AnimationComponent(MeshAnimator* animator)
		:
		m_pAnimator(animator)
	{}

	void AddChannel(const std::string& anim_name, size_t channel_idx)
	{
		m_Channels.emplace_back(anim_name, channel_idx);
	}

	const AnimationChannel* GetChannel(const std::string& name)
	{
		for (const AnimationNameAndChannel& channel : m_Channels)
		{
			if (channel.name == name)
			{
				const MeshAnimation& curr_anim = m_pAnimator->GetCurrentAnimation();
				return &curr_anim.channels[channel.index];
			}
		}
		return nullptr;
	}

	const AnimationChannel* GetCurrentChannel()
	{
		const MeshAnimation& curr_anim = m_pAnimator->GetCurrentAnimation();
		return GetChannel(curr_anim.name);
	}

	DirectX::XMMATRIX GetSample()
	{
		const AnimationChannel* channel = GetCurrentChannel();
		float timestamp = m_pAnimator->GetTimestamp();
		return channel->GetSample(timestamp);
	}

	MeshAnimator* GetAnimator()
	{
		return m_pAnimator;
	}

private:
	struct AnimationNameAndChannel
	{
		AnimationNameAndChannel(const std::string& name, size_t channel_idx)
			:
			name(name),
			index(channel_idx)
		{}

		std::string name;
		size_t index;
	};

	std::vector<AnimationNameAndChannel> m_Channels;
	MeshAnimator* m_pAnimator;
};