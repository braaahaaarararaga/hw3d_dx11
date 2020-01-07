#include "AnimationSkeleton.h"
#include <algorithm>
template<typename T>
static void GetPrevAndNextKeyFrame(const std::vector<KeyFrame<T>>& keyframes, float timestamp, size_t start_at, size_t* pPrevKeyFrame, size_t* pNextKeyFrame)
{
	*pPrevKeyFrame = start_at;
	for (size_t i = start_at + 1; i < keyframes.size(); i++)
	{
		*pNextKeyFrame = i;
		if (keyframes[*pNextKeyFrame].timestamp > timestamp)
		{
			break;
		}
		*pPrevKeyFrame = *pNextKeyFrame;
	}
}

static DirectX::XMFLOAT3 InterpolatePosKeyFrames(const PosKeyFrame& prev_key, const PosKeyFrame& next_key, float timestamp)
{
	const float range = next_key.timestamp - prev_key.timestamp;
	const float pct = (timestamp - prev_key.timestamp) / range;
	DirectX::XMFLOAT3 ret = { next_key.value.x * pct ,next_key.value.y * pct ,next_key.value.z * pct };
	ret.x += prev_key.value.x * (1 - pct);
	ret.y += prev_key.value.y * (1 - pct);
	ret.z += prev_key.value.z * (1 - pct);
	return ret;
}

static DirectX::XMFLOAT3 GetPosKeyFrameAt(const std::vector<PosKeyFrame>& keyframes, size_t* last_pos_index, float timestamp)
{
	if (keyframes.size() == 1)
		return keyframes[0].value;

	// clamp timestamp
	if (timestamp <= keyframes.front().timestamp)
		return keyframes.front().value;
	if (timestamp >= keyframes.back().timestamp)
		return keyframes.back().value;

	size_t prev_key_frame;
	size_t next_key_frame;
	GetPrevAndNextKeyFrame(keyframes, timestamp, *last_pos_index, &prev_key_frame, &next_key_frame);
	*last_pos_index = prev_key_frame;
	return InterpolatePosKeyFrames(keyframes[prev_key_frame], keyframes[next_key_frame], timestamp);
}

static DirectX::XMFLOAT4 InterpolateRotKeyFrames(const RotKeyFrame& prev_key, const RotKeyFrame& next_key, float timestamp)
{
	const float range = next_key.timestamp - prev_key.timestamp;
	const float pct = (timestamp - prev_key.timestamp) / range;
	DirectX::XMFLOAT4 ret;
	DirectX::XMStoreFloat4( &ret, DirectX::XMQuaternionSlerp(DirectX::XMLoadFloat4(&prev_key.value), DirectX::XMLoadFloat4(&next_key.value), pct));
	return ret;
}

static DirectX::XMFLOAT4 GetRotKeyFrameAt(const std::vector<RotKeyFrame>& keyframes, size_t* last_rot_index, float timestamp)
{
	if (keyframes.size() == 1)
		return keyframes[0].value;
	size_t prev_key_frame;
	size_t next_key_frame;
	GetPrevAndNextKeyFrame(keyframes, timestamp, *last_rot_index, &prev_key_frame, &next_key_frame);
	*last_rot_index = prev_key_frame;
	return InterpolateRotKeyFrames(keyframes[prev_key_frame], keyframes[next_key_frame], timestamp);
}

DirectX::XMMATRIX AnimationChannel::GetSample(float timestamp) const
{
	if (timestamp < last_timestamp)
	{
		ResetCache();
	}
	last_timestamp = timestamp;

	// No channels case
	if (position_keyframes.empty() || rotation_keyframes.empty())
	{
		assert(position_keyframes.empty() && rotation_keyframes.empty(), "only one keyframe type provided");
		return DirectX::XMMatrixIdentity();
	}

	DirectX::XMFLOAT3 interp_pos = GetPosKeyFrameAt(position_keyframes, &last_pos_index, timestamp);
	DirectX::XMFLOAT4 interp_rot = GetRotKeyFrameAt(rotation_keyframes, &last_rot_index, timestamp);

	return DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&interp_rot)) *
		DirectX::XMMatrixTranslation(interp_pos.x, interp_pos.y, interp_pos.z);
}

void AnimationChannel::ResetCache() const
{
	last_pos_index = 0;
	last_rot_index = 0;
}

std::vector<DirectX::XMMATRIX> MeshAnimation::GetSample(float timestamp, const std::vector<BoneNode>& original_skeleton) const
{
	std::vector<DirectX::XMMATRIX> new_transforms;
	new_transforms.reserve(original_skeleton.size());
	std::transform(original_skeleton.begin(), original_skeleton.end(), std::back_inserter(new_transforms), [](const BoneNode& node)
	{
		return node.local_transform;
	});

	for (const AnimationChannel& channel : channels)
	{
		new_transforms[channel.node_index] = channel.GetSample(timestamp);
	}

	return new_transforms;
}

void MeshAnimator::Bind(ID3D11DeviceContext * deviceContext)
{
	const MeshAnimation& animation = GetAnimation(GetCurrentAnimationIndex());

	// TODO: Share cbuf between all animators

	GetPoseOffsetTransforms(m_cbufBones->data.bone_Transforms, animation, GetTimestamp());

	m_cbufBones->ApplyChanges();
	deviceContext->VSSetConstantBuffers(1, 1, m_cbufBones->GetAddressOf());
}

const MeshAnimation * MeshAnimator::GetAnimationByName(const std::string & name) const
{
	for (const MeshAnimation& animation : m_Animations)
	{
		if (animation.name == name)
			return &animation;
	}
	return nullptr;
}

void MeshAnimator::GetPoseOffsetTransforms(DirectX::XMMATRIX * out, const MeshAnimation & animation, float timestamp) const
{
	assert(m_Bones.size() <= MAX_BONES, "bone num out of limit");
	
	std::vector<DirectX::XMMATRIX> transforms = animation.GetSample(timestamp, m_OriginalSkeleton);

	for (size_t i = 1; i < transforms.size(); i++)
	{
		transforms[i] = transforms[i] * transforms[m_OriginalSkeleton[i].parent_index];
	}
	for (size_t i = 0; i < m_Bones.size(); i++)
	{
		out[i] = m_Bones[i].inverse_transform * transforms[m_Bones[i].index];
	}
}
