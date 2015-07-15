//
//  GPUParticle.cpp
//  GPUParticles
//
//  Created by Iwatani Nariaki on 2015/07/13.
//
//

#include "GPUParticle.h"
#include "ofGraphics.h"
#include "ofAppRunner.h"

//#define USE_INSTANCING

#define GLSL120(shader)  "\
#version 120 \n\
#extension GL_ARB_texture_rectangle : enable \n\
#extension GL_EXT_gpu_shader4 : require \n"\
#shader

void GPUParticle::setup(size_t size)
{
	draw_shader_.unload();
#ifndef USE_INSTANCING
	string vert = GLSL120(
						  uniform sampler2DRect pos_tex;
						  void main() {
							  gl_FrontColor = gl_Color;
							  vec2 tex_coord = gl_Vertex.xy;
							  vec3 pos = texture2DRect(pos_tex, tex_coord).rgb;
							  gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);
						  }
						  );
#else
	string vert = GLSL120(
							   uniform sampler2DRect pos_tex;
							   void main() {
								   gl_FrontColor = gl_Color;
								   vec2 tex_coord = vec2(mod(gl_InstanceID,[size]),floor(gl_InstanceID/[size]));
								   vec3 pos = texture2DRect(pos_tex, tex_coord).rgb + gl_Vertex.xyz;
								   gl_Position = gl_ModelViewProjectionMatrix * vec4(pos, 1.0);
							   }
							   );
	ofStringReplace(vert, "[size]", ofToString(size));
#endif
	draw_shader_.setupShaderFromSource(GL_VERTEX_SHADER, vert.c_str());
//	const char *frag = GLSL120(
//							   void main() {
//								   gl_FragColor = gl_Color;
//							   }
//	);
//	draw_shader_.setupShaderFromSource(GL_FRAGMENT_SHADER, frag);
	draw_shader_.linkProgram();

	ofFbo::Settings s;
	s.internalformat = GL_RGB32F_ARB;
	s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	s.minFilter = GL_NEAREST;
	s.maxFilter = GL_NEAREST;
	s.wrapModeHorizontal = GL_CLAMP;
	s.wrapModeVertical = GL_CLAMP;
	s.width = s.height = size;
	s.numColorbuffers = 2;
	
	for(int i = 0; i < 2; ++i) {
		ofFbo &fbo = fbo_.front();
		fbo.allocate(s);
		fbo.begin();
		fbo.activateAllDrawBuffers();
		ofClear(0);
		fbo.end();
		fbo_.swap();
	}

	ofVec3f *initial_data = new ofVec3f[size*size];
	for(size_t y = 0; y < size; ++y) {
		for(size_t x = 0; x < size; ++x) {
			initial_data[y*size+x].set(x, y, 0);
		}
	}
	ofFbo &fbo = fbo_.back();
	for(int j = 0; j < 1; ++j) {
		fbo.getTextureReference(j).bind();
		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB32F, size, size, 0, GL_RGB, GL_FLOAT, initial_data);
		fbo.getTextureReference(j).unbind();
	}
	fbo_.swap();
	
	mesh_.clear();
#ifndef USE_INSTANCING
	mesh_.addVertices(static_cast<const ofVec3f*>(initial_data), size*size);
	mesh_.setMode(OF_PRIMITIVE_POINTS);
#else
	mesh_.addVertex(ofVec3f());
	mesh_.setMode(OF_PRIMITIVE_POINTS);
//	mesh_ = ofMesh::box(10, 10, 10);
#endif

	delete[] initial_data;
}
void GPUParticle::update()
{
	for(auto &updater : updaters_) {
		if(updater->isEnabled()) {
			updater->update(fbo_.front(), fbo_.back());
			fbo_.swap();
		}
	}
}
void GPUParticle::draw()
{
	ofFbo &src = fbo_.front();
	draw_shader_.begin();
	draw_shader_.setUniformTexture("pos_tex", src.getTextureReference(0), 0);
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofSetColor(64);
#ifndef USE_INSTANCING
	mesh_.draw();
#else
	mesh_.drawInstanced(OF_MESH_POINTS, src.getWidth()*src.getHeight());
#endif
	ofPopStyle();
	draw_shader_.end();
}

void GPUParticle::drawDebug()
{
	fbo_.get(0).getTextureReference(0).draw(0,0,256,256);
	fbo_.get(0).getTextureReference(1).draw(256,0,256,256);
//	fbo_.get(1).getTextureReference(0).draw(0,256,256,256);
//	fbo_.get(1).getTextureReference(1).draw(256,256,256,256);
}
void GPUParticle::addUpdater(GPUParticleUpdateFactor* updater) {
	updaters_.push_back(updater);
}

void GPUParticleUpdateFactor::drawQuad(float x, float y, float w, float h, float s, float t)
{
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex3f(x, y, 0);
	
	glTexCoord2f(s, 0);
	glVertex3f(x + w, y, 0);
	
	glTexCoord2f(s, t);
	glVertex3f(x + w, y + h, 0);
	
	glTexCoord2f(0, t);
	glVertex3f(x, y + h, 0);
	glEnd();
}

GPUParticleUpdatePosition::GPUParticleUpdatePosition()
:GPUParticleUpdateFactor()
{
//	const char *vert_shader_str = GLSL120(
//										  void main()
//										  {
//											  gl_TexCoord[0] = gl_MultiTexCoord0;
//											  gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
//										  }
//										  );
//	shader_.setupShaderFromSource(GL_VERTEX_SHADER, vert_shader_str);
	const char *shader_str = GLSL120(
								uniform sampler2DRect pos_tex;
								uniform sampler2DRect vel_tex;
								uniform float time;
								void main() {
									vec3 pos = texture2DRect(pos_tex, gl_TexCoord[0].st).rgb;
									vec3 vel = texture2DRect(vel_tex, gl_TexCoord[0].st).rgb;
									gl_FragData[0] = vec4(pos+vel*time, 1.0);
									gl_FragData[1] = vec4(vel, 1.0);
								}
								);
	shader_.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_str);
	shader_.linkProgram();
}
void GPUParticleUpdatePosition::update(ofFbo &src, ofFbo &dst)
{
	float w = dst.getWidth(), h = dst.getHeight();
#if 0
	dst.begin();
	ofPushStyle();
	dst.setActiveDrawBuffer(0);
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	ofSetColor(255);
	src.getTextureReference(0).draw(0,0,w,h);
	ofEnableBlendMode(OF_BLENDMODE_ADD);
	ofSetColor(time_step_*ofGetLastFrameTime()*255);
	src.getTextureReference(1).draw(0,0,w,h);
	dst.setActiveDrawBuffer(1);
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	ofSetColor(255);
	src.getTextureReference(1).draw(0,0,w,h);
	ofPopStyle();
	dst.end();
#else
	dst.begin();
	glPushAttrib(GL_ENABLE_BIT);
	glViewport(0, 0, w, h);
	glDisable(GL_BLEND);
	ofSetColor(255, 255, 255);
	dst.activateAllDrawBuffers();
	shader_.begin();
	shader_.setUniformTexture("pos_tex", src.getTextureReference(0), 0);
	shader_.setUniformTexture("vel_tex", src.getTextureReference(1), 1);
	shader_.setUniform1f("time", time_step_*ofGetLastFrameTime());
	drawQuad(0,0,w,h, w, h);
	shader_.end();
	glPopAttrib();
	dst.end();
#endif
}

GPUParticleUpdateDamping::GPUParticleUpdateDamping()
:GPUParticleUpdateFactor()
{
//	const char *vert_shader_str = GLSL120(
//										  void main()
//										  {
//											  gl_TexCoord[0] = gl_MultiTexCoord0;
//											  gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
//										  }
//										  );
//	shader_.setupShaderFromSource(GL_VERTEX_SHADER, vert_shader_str);
	const char *shader_str = GLSL120(
								   uniform sampler2DRect pos_tex;
								   uniform sampler2DRect vel_tex;
								   uniform float damping;
								   void main() {
									   vec3 pos = texture2DRect(pos_tex, gl_TexCoord[0].st).rgb;
									   vec3 vel = texture2DRect(vel_tex, gl_TexCoord[0].st).rgb;
									   gl_FragData[0] = vec4(pos, 1.0);
									   gl_FragData[1] = vec4(vel*(1.0-damping), 1.0);
								   }
								   );
	shader_.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_str);
	shader_.linkProgram();
}
void GPUParticleUpdateDamping::update(ofFbo &src, ofFbo &dst)
{
	float w = dst.getWidth(), h = dst.getHeight();
#if 0
	dst.begin();
	ofPushStyle();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	dst.setActiveDrawBuffer(0);
	ofSetColor(255);
	src.getTextureReference(0).draw(0,0,w,h);
	dst.setActiveDrawBuffer(1);
	ofSetColor((1-damping_)*255);
	src.getTextureReference(1).draw(0,0,w,h);
	ofPopStyle();
	dst.end();
#else
	dst.begin();
	glPushAttrib(GL_ENABLE_BIT);
	glViewport(0, 0, w, h);
	glDisable(GL_BLEND);
	ofSetColor(255, 255, 255);
	dst.activateAllDrawBuffers();
	shader_.begin();
	shader_.setUniformTexture("pos_tex", src.getTextureReference(0), 0);
	shader_.setUniformTexture("vel_tex", src.getTextureReference(1), 1);
	shader_.setUniform1f("damping", damping_);
	drawQuad(0,0,w,h, w, h);
	shader_.end();
	glPopAttrib();
	dst.end();
#endif
}

GPUParticleUpdateGravityPoint::GPUParticleUpdateGravityPoint()
:GPUParticleUpdateFactor()
{
//	const char *vert_shader_str = GLSL120(
//										  void main()
//										  {
//											  gl_TexCoord[0] = gl_MultiTexCoord0;
//											  gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
//										  }
//										  );
//	shader_.setupShaderFromSource(GL_VERTEX_SHADER, vert_shader_str);
	const char *shader_str = GLSL120(
								uniform sampler2DRect pos_tex;
								uniform sampler2DRect vel_tex;
								uniform vec3 core;
								uniform float force_scaler;
								uniform float attenuation;
								void main() {
									vec3 pos = texture2DRect(pos_tex, gl_TexCoord[0].st).rgb;
									vec3 vel = texture2DRect(vel_tex, gl_TexCoord[0].st).rgb;
									vec3 direction = core-pos;
									float distance_squared = dot(direction,direction);
									vec3 force = force_scaler*normalize(direction)/(1.0+attenuation*distance_squared);
									gl_FragData[0] = vec4(pos, 1.0);
									gl_FragData[1] = vec4(vel+force, 1.0);
								}
								);
	shader_.setupShaderFromSource(GL_FRAGMENT_SHADER, shader_str);
	shader_.linkProgram();
}
void GPUParticleUpdateGravityPoint::update(ofFbo &src, ofFbo &dst)
{
	float w = dst.getWidth(), h = dst.getHeight();
	dst.begin();
	ofEnableBlendMode(OF_BLENDMODE_DISABLED);
	dst.activateAllDrawBuffers();
	shader_.begin();
	shader_.setUniformTexture("pos_tex", src.getTextureReference(0), 0);
	shader_.setUniformTexture("vel_tex", src.getTextureReference(1), 1);
	shader_.setUniform3fv("core", &position_[0]);
	shader_.setUniform1f("force_scaler", force_);
	shader_.setUniform1f("attenuation", attenuation_);
	drawQuad(0,0,w,h, w, h);
	shader_.end();
	dst.end();
}
#undef GLSL120