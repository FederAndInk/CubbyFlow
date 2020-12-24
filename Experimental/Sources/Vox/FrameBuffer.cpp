/*************************************************************************
> File Name: FrameBuffer.hpp
> Project Name: CubbyFlow
> This code is based on Jet Framework that was created by Doyub Kim.
> References: https://github.com/doyubkim/fluid-engine-dev
> Purpose: OpenGL FrameBuffer wrapper class
> Created Time: 2020/08/03
> Copyright (c) 2020, Ji-Hong snowapril
*************************************************************************/
#include <Vox/FrameBuffer.hpp>
#include <Vox/Texture.hpp>
#include <glad/glad.h>

namespace Vox {

    FrameBuffer::FrameBuffer(GLuint framebufferID)
        : _id(framebufferID)
    {
        //! Do nothing.
    }

    FrameBuffer::~FrameBuffer()
    {
        if (_id) glDeleteFramebuffers(1, &_id);
        if (_rbo) glDeleteRenderbuffers(1, &_rbo);
    }

    void FrameBuffer::BindFrameBuffer(GLuint target)
    {
        glBindFramebuffer(target, _id);
    }

    void FrameBuffer::AttachTexture(GLsizei index, const std::shared_ptr<Texture>& texture, bool bMultisample)
    {
		glBindFramebuffer(GL_FRAMEBUFFER, _id);
        const GLenum target = bMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target, texture->GetTextureID(), 0);

        _textures.push_back(texture);
	}

    void FrameBuffer::AttachRenderBuffer(GLuint rbo)
    {
		glBindFramebuffer(GL_FRAMEBUFFER, _id);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        _rbo = rbo;
    }

    bool FrameBuffer::ValidateFrameBufferStatus()
    {
        return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }

};