#ifndef RADIUMENGINE_RENDERER_PASS_HPP
#define RADIUMENGINE_RENDERER_PASS_HPP

#include <Engine/RadiumEngine.hpp>
#include <Engine/Renderer/Renderer.hpp>
#include <Engine/Renderer/OpenGL/OpenGL.hpp>

namespace Ra
{
    namespace Engine
    {


        class RA_ENGINE_API Pass
        {
        public:
            Pass(const std::string& name, uint w, uint h, uint nTexIn, uint nTexOut,
                 Mesh* canvas, uint priority);
            virtual ~Pass() = 0;

            virtual std::string getPassName() const { return m_name; }

            virtual void renderPass() = 0;
            virtual void resizePass( uint w, uint h ) = 0;
            virtual void init() = 0;

            virtual void setIn(uint slot, Texture* tex);
            virtual void setOut(uint slot, Texture* tex);

            void setCanvas(Mesh* canvas);

            virtual Texture* getIn(uint slot);
            virtual Texture* getOut(uint slot);

            std::string getName() const;

            virtual std::shared_ptr<Texture> getInternTextures(uint i) = 0;

            //static void sort(std::vector<std::unique_ptr<Pass>> &passVector); // static ?

        public:
            uint m_priority;

        protected:
            std::string m_name;

            uint m_width;
            uint m_height;

            uint m_nTexIn;
            uint m_nTexOut;

            std::vector<Texture*> m_texIn;
            std::vector<std::shared_ptr<Texture>> m_texOut;

            Mesh *m_canvas;

            static const GLenum buffers[];
        };


    } // namespace Engine
} // namespace Ra

#endif // RADIUMENGINE_RENDERER_PASS_HPP
