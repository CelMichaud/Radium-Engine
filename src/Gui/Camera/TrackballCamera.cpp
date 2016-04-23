#include <Gui/Camera/TrackballCamera.hpp>

#include <iostream>
#include <QMessageBox>
#include <QApplication>

#include <Core/Event/EventEnums.hpp>
#include <Core/Event/Key.hpp>
#include <Core/Log/Log.hpp>
#include <Core/Math/Math.hpp>

#include <Engine/Renderer/Camera/Camera.hpp>
#include <Engine/Renderer/Light/Light.hpp>

namespace Ra
{
    namespace Guibase
    {
        using Core::Math::Pi;

        TrackballCamera::TrackballCamera(uint width, uint height)
            : CameraInterface(width, height)
            , m_trackballCenter(0, 0, 0)
            , m_quickCameraModifier(1.0)
            , m_wheelSpeedModifier(0.02)
            , m_distFromCenter(1.0)
            , m_cameraRadius(1.0)
            , m_rotateAround(true)
            , m_cameraRotateMode(false)
            , m_cameraPanMode(false)
            , m_cameraZoomMode(false)
            , m_walkingOn(false)
            , m_strafingOn(false)
            , m_climbingOn(false)
            , m_walking(0.0)
            , m_strafing(0.0)
            , m_climbing(0.0)
        {
            resetCamera();
        }

        TrackballCamera::~TrackballCamera()
        {
        }

        void TrackballCamera::resetCamera()
        {
            m_camera->setFrame(Core::Transform::Identity());
            m_camera->setPosition(Core::Vector3(0, 0, 1));
            m_trackballCenter = Core::Vector3::Zero();
            updatePhiTheta();
        }

        bool TrackballCamera::handleMouseEvent(const Ra::Core::MouseEvent* event)
        {
            switch (event->event)
            {
                case Core::MouseEvent_Pressed:
                    return handleMousePressEvent(event);
                case Core::MouseEvent_Released:
                    return handleMouseReleaseEvent(event);
                case Core::MouseEvent_Moved:
                    return handleMouseMoveEvent(event);
                case Core::MouseEvent_Wheel:
                    return handleWheelEvent(event);
            }
            return false;
        }

        bool TrackballCamera::handleKeyEvent(const Ra::Core::KeyEvent* event)
        {

            switch (event->event)
            {
                case Core::KeyEvent_Pressed:
                    return handleKeyPressEvent(event);
                case Core::KeyEvent_Released:
                    return handleKeyReleaseEvent(event);
            }
            return false;
        }

        bool TrackballCamera::handleMousePressEvent(const Core::MouseEvent* event)
        {
            // Whole manipulation is done with middle button and modifiers
            if (event->buttons != Core::MouseButton_Middle)
            {
                return false;
            }

            bool handled = false;
            m_lastMouseX = event->absoluteXPosition;
            m_lastMouseY = event->absoluteYPosition;

            if (event->modifier & Core::Modifier_Null)
            {
                m_cameraRotateMode = true;
                handled            = true;
            }

            if (event->modifier & Core::Modifier_Shift)
            {
                m_cameraPanMode = true;
                handled         = true;
            }

            if (event->modifier & Core::Modifier_Ctrl)
            {
                m_cameraZoomMode = true;
                handled          = true;
            }

            return handled;
        }

        bool TrackballCamera::handleMouseMoveEvent(const Core::MouseEvent* event)
        {
            Scalar dx = (event->absoluteXPosition - m_lastMouseX) / m_camera->getWidth();
            Scalar dy = (event->absoluteYPosition - m_lastMouseY) / m_camera->getHeight();

            if (event->modifier & Core::Modifier_Alt)
            {
                m_quickCameraModifier = 10.0;
            }
            else
            {
                m_quickCameraModifier = 2.0;
            }

            if (m_cameraRotateMode)
            {
                handleCameraRotate(dx, dy);
            }

            if (m_cameraPanMode)
            {
                handleCameraPan(dx, dy);
            }

            if (m_cameraZoomMode)
            {
                handleCameraZoom(dx, dy);
            }

            m_lastMouseX = event->absoluteXPosition;
            m_lastMouseY = event->absoluteYPosition;

            if (m_hasLightAttached)
            {
                m_light->setPosition(m_camera->getPosition());
                m_light->setDirection(m_camera->getDirection());
            }

            return true;
        }

        bool TrackballCamera::handleMouseReleaseEvent(const Core::MouseEvent* event)
        {
            m_cameraRotateMode    = false;
            m_cameraPanMode       = false;
            m_cameraZoomMode      = false;
            m_quickCameraModifier = 1.0;

            return true;
        }

        bool TrackballCamera::handleWheelEvent(const Core::MouseEvent* event)
        {
            handleCameraZoom((event->wheelDeltaY + event->wheelDeltaX * 0.1) * m_wheelSpeedModifier);

            if (m_hasLightAttached)
            {
                m_light->setPosition(m_camera->getPosition());
                m_light->setDirection(m_camera->getDirection());
            }

            return true;
        }

        bool TrackballCamera::handleKeyPressEvent(const Core::KeyEvent* e)
        {
            if (e->key == Core::Key_F)
            {
                m_rotateAround = !m_rotateAround;
                return true;
            }

            return false;
        }

        bool TrackballCamera::handleKeyReleaseEvent(const Core::KeyEvent* e)
        {
            return false;
        }

        void TrackballCamera::update(Scalar dt)
        {
        }

        void TrackballCamera::setCameraPosition(const Core::Vector3& position)
        {

            if (position == m_trackballCenter)
            {
                QMessageBox::warning(nullptr, "Error", "Position cannot be set to target point");
                return;
            }
            m_camera->setPosition(position);
            m_camera->setDirection(m_trackballCenter - position);

            updatePhiTheta();
        }

        void TrackballCamera::setCameraTarget(const Core::Vector3& target)
        {
            if (m_camera->getPosition() == m_trackballCenter)
            {
                QMessageBox::warning(nullptr, "Error", "Target cannot be set to current camera position");
                return;
            }

            m_trackballCenter = target;
            m_camera->setDirection(target - m_camera->getPosition());
            updatePhiTheta();
        }

        void TrackballCamera::fitScene(const Core::Aabb& aabb)
        {
            resetCamera();

            Scalar f = m_camera->getFOV();
            Scalar a = m_camera->getAspect();

            const Scalar r = (aabb.max() - aabb.min()).norm() / 2.0;
            const Scalar x = r / std::sin(f / 2.0);
            const Scalar y = r / std::sin(f * a / 2.0);
            Scalar d       = std::max(std::max(x, y), Scalar(0.001));

            m_camera->setPosition(Core::Vector3(aabb.center().x(), aabb.center().y(), d));
            m_camera->setDirection(Core::Vector3(0, 0, -1));
            m_trackballCenter = aabb.center();

            updatePhiTheta();

            m_distFromCenter = d;
            m_cameraRadius   = d;

            Scalar zfar = std::max(Scalar(d + (aabb.max().z() - aabb.min().z()) * 2.0), m_camera->getZFar());
            m_camera->setZFar(zfar);

            if (m_hasLightAttached)
            {
                m_light->setPosition(m_camera->getPosition());
                m_light->setDirection(m_camera->getDirection());
            }
        }

        void TrackballCamera::handleCameraRotate(Scalar dx, Scalar dy)
        {
            Scalar x = dx * m_cameraSensitivity * m_quickCameraModifier;
            Scalar y = -dy * m_cameraSensitivity * m_quickCameraModifier;

            Scalar phi   = m_phi + x; // Keep phi between -pi and pi
            Scalar theta = std::min(std::max(m_theta + y, Scalar(0.0)), Pi);

            Scalar dphi   = phi - m_phi;
            Scalar dtheta = theta - m_theta;

            const Core::Vector3 C  = m_trackballCenter;
            const Core::Vector3 P0 = m_camera->getPosition();
            const Scalar r         = (C - P0).norm();
            // Compute new camera position, on the sphere of radius r centered on C
            Scalar px = C.x() + r * std::cos(phi) * std::sin(theta);
            Scalar py = C.y() + r * std::cos(theta);
            Scalar pz = C.z() + r * std::sin(phi) * std::sin(theta);

            // Compute the translation from old pos to new pos
            Core::Vector3 P(px, py, pz);
            Core::Vector3 t(P - P0);

            // Translate the camera given this translation
            Core::Transform T(Core::Transform::Identity());
            T.translation() = t;

            // Rotate the camera so that it points to the center
            Core::Transform R1(Core::Transform::Identity());
            Core::Transform R2(Core::Transform::Identity());

            Core::Vector3 U = Core::Vector3(0, 1, 0);
            Core::Vector3 R = -m_camera->getRightVector().normalized();

            R1 = Core::AngleAxis(-dphi, U);
            R2 = Core::AngleAxis(-dtheta, R);

            m_camera->applyTransform(T * R1 * R2);

            m_phi   = phi;
            m_theta = theta;
        }

        void TrackballCamera::handleCameraPan(Scalar dx, Scalar dy)
        {
            Scalar x = dx * m_cameraSensitivity * m_quickCameraModifier * m_distFromCenter * 0.1;
            Scalar y = dy * m_cameraSensitivity * m_quickCameraModifier * m_distFromCenter * 0.1;
            // Move camera and trackball center, keep the distance to the center
            Core::Vector3 R = -m_camera->getRightVector();
            Core::Vector3 U = m_camera->getUpVector();

            Core::Transform T(Core::Transform::Identity());
            Core::Vector3 t = x * R + y * U;
            T.translate(t);

            m_camera->applyTransform(T);
            m_trackballCenter += t;
        }

        void TrackballCamera::handleCameraZoom(Scalar dx, Scalar dy)
        {
            handleCameraZoom(std::sqrt(dx * dx + dy * dy));
        }

        void TrackballCamera::handleCameraZoom(Scalar z)
        {
            Scalar r        = m_distFromCenter;
            Scalar y        = r * 0.1 * z * m_cameraSensitivity * m_quickCameraModifier;
            Core::Vector3 F = m_camera->getDirection();

            Core::Transform T(Core::Transform::Identity());
            Core::Vector3 t = y * F;
            T.translate(t);

            m_camera->applyTransform(T);

            m_distFromCenter = (m_trackballCenter - m_camera->getPosition()).norm();
        }

        void TrackballCamera::updatePhiTheta()
        {
            const Core::Vector3& P = m_camera->getPosition();
            const Core::Vector3& C = m_trackballCenter;
            const Core::Vector3& R = P - C;
            const Scalar r         = R.norm();

            m_theta = std::acos(R.y() / r);
            m_phi = (R.z() == 0.f && R.x() == 0.f) ? 0.f : std::atan2(R.z(), R.x());
            CORE_ASSERT(std::isfinite(m_theta) && std::isfinite(m_phi), "Error in trackball camera");
        }
    }
} // namespace Ra