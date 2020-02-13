#pragma once

class IPipeline
{
public:
	virtual ~IPipeline() = default;
public:
	virtual void Render(Graphics* gfx, Mesh& mesh) = 0;
};

class ShadowMapPipeline : public IPipeline
{
public:
	void Render(Graphics* gfx, Mesh& mesh) override;
};

class General3DPipeline : public IPipeline
{
public:
	void Render(Graphics* gfx, Mesh& mesh) override;
};

class NoLight3DPipeline : public IPipeline
{
public:
	void Render(Graphics* gfx, Mesh& mesh) override;
};