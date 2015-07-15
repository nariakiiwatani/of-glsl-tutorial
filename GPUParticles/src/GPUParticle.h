#pragma once

#include "ofShader.h"
#include "DoubleBuffer.h"
#include "ofFbo.h"

class GPUParticleUpdateFactor;
class GPUParticle
{
public:
	void setup(size_t size);
	void update();
	void draw();
	void addUpdater(GPUParticleUpdateFactor* updater);
	ofFbo& getFbo() { return fbo_.front(); }
	void drawDebug();
private:
	ofShader draw_shader_;
	ofVboMesh mesh_;
	DoubleBuffer<ofFbo> fbo_;
	vector<GPUParticleUpdateFactor*> updaters_;
};

class GPUParticleUpdateFactor
{
public:
	GPUParticleUpdateFactor():is_enabled_(true){}
	virtual void update(ofFbo &src, ofFbo &dst)=0;
	void setEnabled(bool enabled) { is_enabled_ = enabled; }
	bool isEnabled() { return is_enabled_;}
	void drawQuad(float x, float y, float w, float h, float s, float t);
private:
	bool is_enabled_;
};

class GPUParticleUpdatePosition : public GPUParticleUpdateFactor
{
public:
	float time_step_;
public:
	GPUParticleUpdatePosition();
	void update(ofFbo &src, ofFbo &dst);
private:
	ofShader shader_;
};

class GPUParticleUpdateDamping : public GPUParticleUpdateFactor
{
public:
	float damping_;
public:
	GPUParticleUpdateDamping();
	void update(ofFbo &src, ofFbo &dst);
private:
	ofShader shader_;
};

class GPUParticleUpdateGravityPoint : public GPUParticleUpdateFactor
{
public:
	ofVec3f position_;
	float force_;
	float attenuation_;
public:
	GPUParticleUpdateGravityPoint();
	void update(ofFbo &src, ofFbo &dst);
private:
	ofShader shader_;
};