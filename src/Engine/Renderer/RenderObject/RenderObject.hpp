#ifndef RADIUMENGINE_RENDEROBJECT_HPP
#define RADIUMENGINE_RENDEROBJECT_HPP

#include <Engine/RaEngine.hpp>

#include <string>
#include <mutex>
#include <memory>

#include <Core/Index/IndexedObject.hpp>
#include <Core/Math/LinearAlgebra.hpp>
#include <Engine/Renderer/RenderTechnique/Material.hpp>
#include <Engine/Renderer/RenderTechnique/RenderParameters.hpp>
#include <Engine/Renderer/RenderObject/RenderObjectTypes.hpp>
#include <Engine/ItemModel/ItemEntry.hpp>

namespace Ra
{
    namespace Asset
    {
        class GeometryData;
    }

    namespace Engine
    {
        class Light;
        struct RenderTechnique;
        class Component;
        class Mesh;
        class RenderQueue;
    }
}

namespace Ra
{
    namespace Engine
    {

        struct RenderData;

        // FIXME(Charly): Does this need a bit of cleanup ?
        class RA_ENGINE_API RenderObject : public Core::IndexedObject
        {
        public:
            RA_CORE_ALIGNED_NEW

            /// A -1 (or any other negative value) lifetime is considered infinite,
            /// 0 is an "invalid value" (would mean the render object has to die immediatly),
            /// hence it's considered as infinite,
            /// any other positive value will be taken into account.
            RenderObject( const std::string& name, const RenderObjectType& type);
            ~RenderObject();

            static RenderObject* createRenderObject(const std::string& name, const RenderObjectType& type, const std::shared_ptr<Mesh>& mesh, const ShaderConfiguration& shaderConfig = ShaderConfiguration(), Material* material = nullptr);
            static RenderObject* createFancyFromAsset(const std::string& name, Ra::Asset::GeometryData* asset);

            // FIXME(Charly): Remove this
            void updateGL();

            //
            // Getters and setters.
            //
            const std::string& getName() const;

            virtual void setIndex(Index index) override;
            
            void setComponent(ItemEntry entry);

            const RenderObjectType& getType() const;
            void setType( const RenderObjectType& t);

            void setVisible( bool visible );
            void toggleVisible();
            bool isVisible() const;

            void setXRay( bool xray );
            void toggleXRay();
            bool isXRay() const;

            void setTransparent( bool transparent );
            void toggleTransparent();
            bool isTransparent() const;
            
            bool isDirty() const;

            void setRenderTechnique( RenderTechnique* technique );
            const RenderTechnique* getRenderTechnique() const;
            RenderTechnique* getRenderTechnique();

            void setMesh( const std::shared_ptr<Mesh>& mesh );
            std::shared_ptr<const Mesh> getMesh() const;
            const std::shared_ptr<Mesh>& getMesh();

            Core::Transform getTransform() const;
            Core::Matrix4 getTransformAsMatrix() const;

            Core::Aabb getAabb() const;

            void setLocalTransform( const Core::Transform& transform );
            void setLocalTransform( const Core::Matrix4& transform );
            const Core::Transform& getLocalTransform() const;
            const Core::Matrix4& getLocalTransformAsMatrix() const;

            virtual void render( const RenderParameters& lightParams, const RenderData& rdata, const ShaderProgram* altShader = nullptr );

        private:
            Core::Transform m_localTransform;

            ItemEntry m_component;
            ItemEntry m_entry;
            
            std::string m_name;

            RenderObjectType m_type;
            RenderTechnique* m_renderTechnique;
            std::shared_ptr<Mesh> m_mesh;

            // FIXME(Charly): Remove this
            RenderParameters m_renderParameters;

            mutable std::mutex m_updateMutex;

            bool m_visible;
            bool m_xray;
            bool m_transparent;
            bool m_dirty;
            bool m_hasLifetime;
        };

    } // namespace Engine
} // namespace Ra

#endif // RADIUMENGINE_RENDEROBJECT_HPP
