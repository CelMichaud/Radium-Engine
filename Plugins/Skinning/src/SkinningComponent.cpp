 #include <SkinningComponent.hpp>

#include <Core/Geometry/Normal/Normal.hpp>
#include <Core/Animation/Pose/PoseOperation.hpp>

#include <Core/Animation/Skinning/DualQuaternionSkinning.hpp>
#include <Core/Animation/Skinning/RotationCenterSkinning.hpp>

using Ra::Core::Quaternion;
using Ra::Core::DualQuaternion;

using Ra::Core::TriangleMesh;
using Ra::Core::Animation::Skeleton;
using Ra::Core::Animation::Pose;
using Ra::Core::Animation::RefPose;
using Ra::Core::Animation::WeightMatrix;

typedef Ra::Core::Animation::Handle::SpaceType SpaceType;

using Ra::Core::Skinning::RefData;
using Ra::Core::Skinning::FrameData;

using Ra::Engine::ComponentMessenger;
namespace SkinningPlugin
{

void SkinningComponent::setupSkinning()
{
    // get the current animation data.
    bool hasSkel    = ComponentMessenger::getInstance()->canGet<Skeleton>(getEntity(), m_contentsName);
    bool hasWeights = ComponentMessenger::getInstance()->canGet<WeightMatrix>(getEntity(), m_contentsName);
    bool hasRefPose = ComponentMessenger::getInstance()->canGet<RefPose>(getEntity(), m_contentsName);
    bool hasMesh    = ComponentMessenger::getInstance()->canGet<TriangleMesh>(getEntity(), m_contentsName);


    if ( hasSkel && hasWeights && hasMesh && hasRefPose )
    {
        m_skeletonGetter = ComponentMessenger::getInstance()->ComponentMessenger::getterCallback<Skeleton>( getEntity(), m_contentsName );
        m_verticesWriter = ComponentMessenger::getInstance()->ComponentMessenger::rwCallback<Ra::Core::Vector3Array>( getEntity(), m_contentsName+"v" );
        m_normalsWriter  = ComponentMessenger::getInstance()->ComponentMessenger::rwCallback<Ra::Core::Vector3Array>( getEntity(), m_contentsName+"n" );

        m_refData.m_skeleton        = ComponentMessenger::getInstance()->get<Skeleton>( getEntity(), m_contentsName );
        m_refData.m_referenceMesh   = ComponentMessenger::getInstance()->get<TriangleMesh>( getEntity(), m_contentsName );
        m_refData.m_refPose         = ComponentMessenger::getInstance()->get<RefPose> ( getEntity(), m_contentsName );
        m_refData.m_weights         = ComponentMessenger::getInstance()->get<WeightMatrix> ( getEntity(), m_contentsName );

        m_frameData.m_previousPose = m_refData.m_refPose;
        m_frameData.m_doSkinning = false;
        m_frameData.m_doReset = false;

        m_frameData.m_previousPos   = m_refData.m_referenceMesh.m_vertices;
        m_frameData.m_currentPos    = m_refData.m_referenceMesh.m_vertices;
        m_frameData.m_currentNormal = m_refData.m_referenceMesh.m_normals;


        // Do some debug checks:  Attempt to write to the mesh and check the weights match skeleton and mesh.
        ON_DEBUG( bool skinnable = ComponentMessenger::getInstance()->canSet<Ra::Core::TriangleMesh>(getEntity(), m_contentsName ));
        CORE_ASSERT( skinnable, "Mesh cannot be skinned. It could be because the mesh is set to nondeformable" );
        CORE_ASSERT( m_refData.m_skeleton.size() == m_refData.m_weights.cols(), "Weights are incompatible with bones" );
        CORE_ASSERT( m_refData.m_referenceMesh.m_vertices.size() == m_refData.m_weights.rows(), "Weights are incompatible with Mesh" );

        m_isReady = true;
        setupSkinningType( m_skinningType );
    }
}
void SkinningComponent::skin()
{
    CORE_ASSERT( m_isReady, "Skinning is not setup");

    const Skeleton* skel = m_skeletonGetter();

    bool reset = ComponentMessenger::getInstance()->get<bool>(getEntity(), m_contentsName);

    // Reset the skin if it wasn't done before
    if (reset && !m_frameData.m_doReset )
    {
        m_frameData.m_doReset = true;
    }
    else
    {
        m_frameData.m_currentPose = skel->getPose(SpaceType::MODEL);
        if ( !Ra::Core::Animation::areEqual( m_frameData.m_currentPose, m_frameData.m_previousPose))
        {
            m_frameData.m_doSkinning = true;
            m_frameData.m_refToCurrentRelPose = Ra::Core::Animation::relativePose(m_frameData.m_currentPose, m_refData.m_refPose);
            m_frameData.m_prevToCurrentRelPose = Ra::Core::Animation::relativePose(m_frameData.m_currentPose, m_frameData.m_previousPose);

            switch ( m_skinningType )
            {
            case LBS:
            {
                Ra::Core::Animation::linearBlendSkinning( m_refData.m_referenceMesh.m_vertices, m_frameData.m_refToCurrentRelPose, m_refData.m_weights, m_frameData.m_currentPos );
                break;
            }
            case DQS:
            {
                Ra::Core::AlignedStdVector< DualQuaternion > DQ;
                //computeDQ( m_frameData.m_prevToCurrentRelPose, m_refData.m_weights, DQ );
                Ra::Core::Animation::computeDQ( m_frameData.m_refToCurrentRelPose, m_refData.m_weights, DQ );
                Ra::Core::Animation::dualQuaternionSkinning( m_refData.m_referenceMesh.m_vertices, DQ, m_frameData.m_currentPos );
                break;
            }
            case COR:
            {
                Ra::Core::Animation::corSkinning( m_refData.m_referenceMesh.m_vertices, m_frameData.m_refToCurrentRelPose, m_refData.m_weights, m_refData.m_CoR, m_frameData.m_currentPos );
                break;
            }
            }
            Ra::Core::Animation::computeDQ( m_frameData.m_refToCurrentRelPose, m_refData.m_weights, m_DQ );
        }
    }
}

void SkinningComponent::endSkinning()
{
    if (m_frameData.m_doSkinning)
    {
        Ra::Core::Vector3Array& vertices = *(m_verticesWriter());
        Ra::Core::Vector3Array& normals = *(m_normalsWriter());

        vertices = m_frameData.m_currentPos;

        Ra::Core::Geometry::uniformNormal( vertices, m_refData.m_referenceMesh.m_triangles, normals );

        std::swap( m_frameData.m_previousPose, m_frameData.m_currentPose );
        std::swap( m_frameData.m_previousPos, m_frameData.m_currentPos );

        m_frameData.m_doSkinning = false;

    }
    else if (m_frameData.m_doReset)
    {
        // Reset mesh to its initial state.
        Ra::Core::Vector3Array& vertices = *(m_verticesWriter());
        Ra::Core::Vector3Array& normals =  *(m_normalsWriter());

        vertices = m_refData.m_referenceMesh.m_vertices;
        normals = m_refData.m_referenceMesh.m_normals;

        m_frameData.m_doReset = false;
        m_frameData.m_currentPose   = m_refData.m_refPose;
        m_frameData.m_previousPose  = m_refData.m_refPose;
        m_frameData.m_currentPos    = m_refData.m_referenceMesh.m_vertices;
        m_frameData.m_previousPos   = m_refData.m_referenceMesh.m_vertices;
        m_frameData.m_currentNormal = m_refData.m_referenceMesh.m_normals;
    }
}

void SkinningComponent::handleWeightsLoading(const Ra::Asset::HandleData *data)
{
    m_contentsName = data->getName();
    setupIO(m_contentsName);
}

void SkinningComponent::setupIO( const std::string& id )
{
    typedef Ra::Core::AlignedStdVector< Ra::Core::DualQuaternion > DualQuatVector;

    ComponentMessenger::CallbackTypes<DualQuatVector>::Getter dqOut = std::bind( &SkinningComponent::getDQ, this );
    ComponentMessenger::getInstance()->registerOutput<DualQuatVector>( getEntity(), this, id, dqOut);

    ComponentMessenger::CallbackTypes<RefData>::Getter refData = std::bind( &SkinningComponent::getRefData, this );
    ComponentMessenger::getInstance()->registerOutput<Ra::Core::Skinning::RefData>( getEntity(), this, id, refData);

    ComponentMessenger::CallbackTypes<FrameData>::Getter frameData = std::bind( &SkinningComponent::getFrameData, this );
    ComponentMessenger::getInstance()->registerOutput<FrameData>( getEntity(), this, id, frameData);
}

void SkinningComponent::setSkinningType( SkinningType type )
{
    m_skinningType = type;
    if ( m_isReady )
    {
        setupSkinningType( type );
    }
}

void SkinningComponent::setupSkinningType( SkinningType type )
{
    CORE_ASSERT( m_isReady, "component is not ready" );
    switch ( type )
    {
    case LBS: break;
    case DQS:
    {
        if ( m_DQ.empty() )
        {
            m_DQ.resize( m_refData.m_weights.rows(), DualQuaternion( Quaternion( 0.0, 0.0, 0.0, 0.0 ),
                Quaternion( 0.0, 0.0, 0.0, 0.0 ) ) );
        }
        break;
    }
    case COR:
    {
        if ( m_refData.m_CoR.empty() )
        {
            Ra::Core::Animation::computeCoR( m_refData );
/*
            for ( const auto& v :m_refData.m_CoR )
            {
                RA_DISPLAY_POINT( v, Ra::Core::Colors::Red(), 0.1f );
            }
*/
        }
    }
    } // end of switch.
}
} // End namespace Skinning plugin
