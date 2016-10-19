#include <Engine/RadiumEngine.hpp>

#include <thread>
#include <chrono>
#include <mutex>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>

#include <Core/Log/Log.hpp>
#include <Core/String/StringUtils.hpp>
#include <Core/Event/EventEnums.hpp>
#include <Core/Event/KeyEvent.hpp>
#include <Core/Event/MouseEvent.hpp>

#include <Engine/FrameInfo.hpp>
#include <Engine/System/System.hpp>
#include <Engine/Entity/Entity.hpp>
#include <Engine/Component/Component.hpp>

#include <Engine/Managers/AssetManager.hpp>
#include <Engine/Managers/EntityManager/EntityManager.hpp>
#include <Engine/Managers/ComponentMessenger/ComponentMessenger.hpp>
#include <Engine/Managers/SystemDisplay/SystemDisplay.hpp>

#include <Engine/Assets/FileData.hpp>

namespace Ra
{
    namespace Engine
    {

        RadiumEngine::RadiumEngine()
        {
        }

        RadiumEngine::~RadiumEngine()
        {
        }

        void RadiumEngine::initialize()
        {
            LOG(logINFO) << "*** Radium Engine ***";
            m_signalManager.reset( new SignalManager );
            m_entityManager.reset( new EntityManager );
            m_renderObjectManager.reset( new RenderObjectManager );
            ComponentMessenger::createInstance();
            AssetManager::createInstance();
        }

        void RadiumEngine::cleanup()
        {
            m_signalManager->setOn( false );
            m_entityManager.reset();
            m_renderObjectManager.reset();

            for ( auto& system : m_systems )
            {
                system.second.reset();
            }

            ComponentMessenger::destroyInstance();
        }

        void RadiumEngine::endFrameSync()
        {
            m_entityManager->swapBuffers();
        }

        void RadiumEngine::getTasks( Core::TaskQueue* taskQueue,  Scalar dt )
        {
            static uint frameCounter = 0;
            FrameInfo frameInfo;
            frameInfo.m_dt = dt;
            frameInfo.m_numFrame = frameCounter++;
            for ( auto& syst : m_systems )
            {
                syst.second->generateTasks( taskQueue, frameInfo );
            }
        }

        void RadiumEngine::registerSystem( const std::string& name, System* system )
        {
            CORE_ASSERT( m_systems.find( name ) == m_systems.end(),
                         "Same system added multiple times." );

            m_systems[name] = std::shared_ptr<System> ( system );
            LOG(logINFO) << "Loaded : " << name;
        }

        System* RadiumEngine::getSystem( const std::string& system ) const
        {
            System* sys = nullptr;
            auto it = m_systems.find( system );

            if ( it != m_systems.end() )
            {
                sys = it->second.get();
            }

            return sys;
        }

        bool RadiumEngine::loadFile( const std::string& filename )
        {
            Asset::FileData fileData( filename, false );

            std::string entityName = Core::StringUtils::getBaseName( filename, false );

            Entity* entity = m_entityManager->createEntity( entityName );

            for( auto& system : m_systems ) {
                system.second->handleAssetLoading( entity, &fileData );
            }

            for (auto& comp :entity->getComponents())
            {
                comp->initialize();
            }

            return true;
        }

        RenderObjectManager* RadiumEngine::getRenderObjectManager() const
        {
            return m_renderObjectManager.get();
        }

        EntityManager* RadiumEngine::getEntityManager() const
        {
            return m_entityManager.get();
        }

        SignalManager *RadiumEngine::getSignalManager() const
        {
           return m_signalManager.get();
        }

        RA_SINGLETON_IMPLEMENTATION( RadiumEngine );
    } // Namespace engine
} // namespace Ra
