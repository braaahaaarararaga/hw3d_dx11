#include "AnimationSkeleton.h"

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
}
