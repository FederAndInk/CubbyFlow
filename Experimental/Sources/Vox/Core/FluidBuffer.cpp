/*************************************************************************
> File Name: FluidBuffer.cpp
> Project Name: CubbyFlow
> This code is based on Jet Framework that was created by Doyub Kim.
> References: https://github.com/doyubkim/fluid-engine-dev
> Purpose: The buffer class which store buffer desc and own raw pointer optionally.
> Created Time: 2020/07/26
> Copyright (c) 2020, Ji-Hong snowapril
*************************************************************************/
#include <Vox/Core/FluidBuffer.hpp>
#include <Vox/Core/FrameContext.hpp>
#include <Vox/Mesh/Mesh.hpp>
#include <Vox/Scene/GeometryCacheManager.hpp>
#include <Vox/Scene/GeometryCache.hpp>
#include <Vox/Core/Material.hpp>
#include <Vox/Core/Vertex.hpp>
#include <Vox/Scene/VoxScene.hpp>
#include <Core/Utils/Logging.hpp>
#include <glad/glad.h>

using namespace CubbyFlow;

namespace Vox {

    FluidBuffer::FluidBuffer(const size_t numBuffer)
    {
        Resize(numBuffer);
    }   

    FluidBuffer::~FluidBuffer()
    {
        _fences.clear();
    }

    void FluidBuffer::Resize(const size_t numBuffer)
    {
        _meshes.resize(numBuffer, std::make_shared<Mesh>());
        if (_numBuffer <= numBuffer) //! In case new number of the buffer is larger than original.
        {
            const GLsizei numCreate = static_cast<GLsizei>(numBuffer - _numBuffer);
            for (size_t i = _numBuffer; i < numBuffer; ++i)
            {
                _meshes[i]->SetBufferUsage(GL_STREAM_DRAW);
                _meshes[i]->GenerateEmptyMesh(VertexFormat::Position3Normal3, kMaxBufferSize, kMaxBufferSize);
            }
            glBindVertexArray(0);
        }
        _numBuffer = numBuffer;
        _fences.resize(_numBuffer);
    }

    bool FluidBuffer::CheckFence(GLuint64 timeout) 
    {
        const size_t bufferNum = _frameIndex % _numBuffer;
        const GLsync& fence = _fences[bufferNum];

        if (fence)
        {
            GLenum result = glClientWaitSync(fence, 0, timeout); //! CPU side synchronization
            return result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED;
        }
        return true;
    }

    void FluidBuffer::AsyncBufferTransfer()
    {
        const size_t bufferNum = _frameIndex % _numBuffer;

        //! Take Modulo with geometry cache size for repeating transfer.
        const size_t index = _frameIndex % _cacheManager->GetNumberOfCache();
        const auto& shape = _cacheManager->GetGeometryCache(index)->GetShape(0);

        //! Transfer the vertices data
        _meshes[bufferNum]->AsyncTransfer(shape.interleaved.data(), shape.interleaved.size() * sizeof(float));

        //! Transfer the indices data
        _meshes[bufferNum]->AsyncTransfer(shape.indices.data(), shape.indices.size() * sizeof(unsigned int), true);
    }

    void FluidBuffer::DrawFrame(const std::shared_ptr<FrameContext>& ctx)
    {
        const size_t bufferNum = _frameIndex % _numBuffer;

        //! Bind material to the context.
        _material->BindMaterial(ctx);
        //! Render the mesh.
        _meshes[bufferNum]->DrawMesh(ctx);
        
        //! Make fence to draw call.
        _fences[bufferNum] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0); 
    }

    void FluidBuffer::AdvanceFrame()
    {
        ++_frameIndex; //! Advance frame index;
    }

    void FluidBuffer::AttachGeometryCacheManager(const std::shared_ptr<GeometryCacheManager>& manager)
    {
        _cacheManager = manager;
    }

    void FluidBuffer::AttachMaterial(const std::shared_ptr<Material>& material)
    {
        _material = material;
    }
};