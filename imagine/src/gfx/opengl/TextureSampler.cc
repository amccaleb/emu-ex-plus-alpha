/*  This file is part of Imagine.

	Imagine is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Imagine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Imagine.  If not, see <http://www.gnu.org/licenses/> */

#define LOGTAG "GLTextureSampler"
#include <imagine/gfx/Renderer.hh>
#include <imagine/gfx/RendererTask.hh>
#include <imagine/gfx/TextureSampler.hh>
#include "private.hh"
#include <limits>

namespace Gfx
{

static void setTexParameteriImpl(GLenum target, GLenum pname, GLint param, const char *pnameStr)
{
	runGLCheckedVerbose(
		[&]()
		{
			glTexParameteri(target, pname, param);
		}, "glTexParameteri()");
}

#define setTexParameteri(target, pname, param) setTexParameteriImpl(target, pname, param, #pname);

static void setSamplerParameteriImpl(const Renderer &r, GLuint sampler, GLenum pname, GLint param, const char *pnameStr)
{
	runGLCheckedVerbose(
		[&]()
		{
			r.support.glSamplerParameteri(sampler, pname, param);
		}, "glSamplerParameteri()");
}

#define setSamplerParameteri(r, sampler, pname, param) setSamplerParameteriImpl(r, sampler, pname, param, #pname);

// make sure sampler-related enums can fit into a 16-bit int
static_assert(GL_LINEAR < std::numeric_limits<uint16_t>::max());
static_assert(GL_NEAREST < std::numeric_limits<uint16_t>::max());
static_assert(GL_LINEAR_MIPMAP_NEAREST < std::numeric_limits<uint16_t>::max());
static_assert(GL_NEAREST_MIPMAP_NEAREST < std::numeric_limits<uint16_t>::max());
static_assert(GL_LINEAR_MIPMAP_LINEAR < std::numeric_limits<uint16_t>::max());
static_assert(GL_NEAREST_MIPMAP_LINEAR < std::numeric_limits<uint16_t>::max());
static_assert(GL_CLAMP_TO_EDGE < std::numeric_limits<uint16_t>::max());
static_assert(GL_REPEAT < std::numeric_limits<uint16_t>::max());

static GLint makeMinFilter(bool linearFiltering, MipFilterMode mipFiltering)
{
	switch(mipFiltering)
	{
		case MIP_FILTER_NONE: return linearFiltering ? GL_LINEAR : GL_NEAREST;
		case MIP_FILTER_NEAREST: return linearFiltering ? GL_LINEAR_MIPMAP_NEAREST : GL_NEAREST_MIPMAP_NEAREST;
		case MIP_FILTER_LINEAR: return linearFiltering ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_LINEAR;
		default: bug_unreachable("mipFiltering == %d", (int)mipFiltering); return 0;
	}
}

static GLint makeMagFilter(bool linearFiltering)
{
	return linearFiltering ? GL_LINEAR : GL_NEAREST;
}

static GLint makeWrapMode(WrapMode mode)
{
	return mode == WRAP_CLAMP ? GL_CLAMP_TO_EDGE : GL_REPEAT;
}

GLTextureSampler::GLTextureSampler(RendererTask &rTask, TextureSamplerConfig config):
	rTask{&rTask}
{
	if(config.debugLabel())
		debugLabel = config.debugLabel();
	magFilter = makeMagFilter(config.minLinearFilter());
	minFilter = makeMinFilter(config.magLinearFilter(), config.mipFilter());
	xWrapMode_ = makeWrapMode(config.xWrapMode());
	yWrapMode_ = makeWrapMode(config.yWrapMode());
	auto &r = rTask.renderer();
	if(r.support.hasSamplerObjects)
	{
		rTask.runSync(
			[this, &r = std::as_const(r)]()
			{
				r.support.glGenSamplers(1, &name_);
				if(magFilter != GL_LINEAR) // GL_LINEAR is the default
					setSamplerParameteri(r, name_, GL_TEXTURE_MAG_FILTER, magFilter);
				if(minFilter != GL_NEAREST_MIPMAP_LINEAR) // GL_NEAREST_MIPMAP_LINEAR is the default
					setSamplerParameteri(r, name_, GL_TEXTURE_MIN_FILTER, minFilter);
				if(xWrapMode_ != GL_REPEAT) // GL_REPEAT is the default
					setSamplerParameteri(r, name_, GL_TEXTURE_WRAP_S, xWrapMode_);
				if(yWrapMode_ != GL_REPEAT) // GL_REPEAT​​ is the default
					setSamplerParameteri(r, name_, GL_TEXTURE_WRAP_T, yWrapMode_);
			});
	}
	else
	{
		r.samplerNames++;
		name_ = r.samplerNames;
	}
	logMsg("created sampler:0x%X (%s)", name_, label());
}

TextureSampler::TextureSampler(TextureSampler &&o)
{
	*this = std::move(o);
}

TextureSampler &TextureSampler::operator=(TextureSampler &&o)
{
	deinit();
	GLTextureSampler::operator=(o);
	o.name_ = 0;
	return *this;
}

Renderer &TextureSampler::renderer() const
{
	return task().renderer();
}

RendererTask &TextureSampler::task() const
{
	assumeExpr(rTask);
	return *rTask;
}

GLTextureSampler::~GLTextureSampler()
{
	deinit();
}

void GLTextureSampler::deinit()
{
	if(!name_ || !rTask || !*rTask || !rTask->renderer().support.hasSamplerObjects)
		return;
	logDMsg("deleting sampler object:0x%X (%s)", name_, label());
	rTask->run(
		[&r = std::as_const(rTask->renderer()), name = name_]()
		{
			r.support.glDeleteSamplers(1, &name);
		});
	name_ = 0;
}

TextureSampler::operator bool() const
{
	return name_;
}

void GLTextureSampler::setTexParams(GLenum target) const
{
	assert(!rTask->renderer().support.hasSamplerObjects);
	setTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilter);
	setTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilter);
	setTexParameteri(target, GL_TEXTURE_WRAP_S, xWrapMode_);
	setTexParameteri(target, GL_TEXTURE_WRAP_T, yWrapMode_);
}

GLuint GLTextureSampler::name() const
{
	return name_;
}

const char *GLTextureSampler::label() const
{
	return debugLabel;
}

}
