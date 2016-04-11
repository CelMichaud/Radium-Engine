#include "PassHighpass.hpp"

namespace Ra {
    namespace Engine {

        PassHighpass::PassHighpass(const std::string& name, uint w, uint h, uint nTexIn=1, uint nTexOut=1)
            : Pass(name, w, h, nTexIn, nTexOut)
        {
            m_texOut[TEX_HPASS].reset( new Texture("HighPass", GL_TEXTURE_2D) );
        }

        PassHighpass::~PassHighpass()
        {
        }

        void PassHighpass::initFbos()
        {
            // create internal FBOs
            m_fbo[FBO_COLOR].reset( new FBO( FBO::Components(FBO::COLOR), m_width, m_height ));
        }

        void PassHighpass::resizePass(uint w, uint h)
        {
            m_width  = w;
            m_height = h;
            resizePass();
        }

        void PassHighpass::resizePass()
        {
            // resize output tex.
            m_texOut[TEX_HPASS]->initGL(GL_RGBA32F, m_width, m_height, GL_RGBA, GL_FLOAT, nullptr);

            // initiate, bind and configure the main fbo
            m_fbo[FBO_COLOR]->bind();
            m_fbo[FBO_COLOR]->setSize( m_width, m_height );
            m_fbo[FBO_COLOR]->attachTexture( GL_COLOR_ATTACHMENT0, m_texIn[TEX_LIT] );
            m_fbo[FBO_COLOR]->attachTexture( GL_COLOR_ATTACHMENT1, m_texIn[TEX_LUM] );
            m_fbo[FBO_COLOR]->attachTexture( GL_COLOR_ATTACHMENT2, m_texOut[TEX_HPASS].get() );
            m_fbo[FBO_COLOR]->unbind( true );

            GL_CHECK_ERROR;

            // Reset framebuffer state
            GL_ASSERT( glBindFramebuffer( GL_FRAMEBUFFER, 0 ) );

            GL_ASSERT( glDrawBuffer( GL_BACK ) );
            GL_ASSERT( glReadBuffer( GL_BACK ) );
        }

        void PassHighpass::renderPass(ShaderProgramManager* shaderMgr, Mesh *screen)
        {
            const ShaderProgram* shader = nullptr;

            m_fbo[FBO_COLOR]->useAsTarget();

            GL_ASSERT( glViewport(0, 0, m_width, m_height) );
            GL_ASSERT( glDrawBuffers(1, buffers+2) );

            shader = shaderMgr->getShaderProgram("HighpassTEST");
            shader->bind();
            shader->setUniform("hdr", m_texIn[TEX_LIT], 0);
            shader->setUniform("lum", m_texIn[TEX_LUM], 1);
            screen->render();
        }

        std::shared_ptr<Texture> PassHighpass::getInternTextures(uint i)
        {
            std::shared_ptr<Texture> empty( nullptr );
            return empty;
        }

    }
}

