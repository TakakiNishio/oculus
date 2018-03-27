/*
 * Copyright (c) 2009, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <OgreViewport.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreCamera.h>

#include <rviz/uniform_string_stream.h>
#include <rviz/display_context.h>
#include <rviz/viewport_mouse_event.h>
#include <rviz/geometry.h>
#include <rviz/ogre_helpers/shape.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/vector_property.h>
#include <rviz/properties/bool_property.h>

#include <fps_motion_view_controller.h>

namespace rviz
{

static const Ogre::Quaternion ROBOT_TO_CAMERA_ROTATION =
  Ogre::Quaternion( Ogre::Radian( -Ogre::Math::HALF_PI ), Ogre::Vector3::UNIT_Y ) *
  Ogre::Quaternion( Ogre::Radian( -Ogre::Math::HALF_PI ), Ogre::Vector3::UNIT_Z );

static const float PITCH_LIMIT_LOW = -Ogre::Math::HALF_PI + 0.001;
static const float PITCH_LIMIT_HIGH = Ogre::Math::HALF_PI - 0.001;

FPSMotionViewController::FPSMotionViewController()
{
  yaw_property_ = new FloatProperty( "Yaw", 0, "Rotation of the camera around the Z (up) axis.", this );
  pitch_property_ = new FloatProperty( "Pitch", 0, "How much the camera is tipped downward.", this);
  pitch_property_->setMax( Ogre::Math::HALF_PI - 0.001 );
  pitch_property_->setMin( -pitch_property_->getMax() );

  position_property_ = new VectorProperty( "Position", Ogre::Vector3( 5, 5, 10 ), "Position of the camera.", this );

}

FPSMotionViewController::~FPSMotionViewController()
{
}

void FPSMotionViewController::onInitialize()
{
  FramePositionTrackingViewController::onInitialize();
  camera_->setProjectionType( Ogre::PT_PERSPECTIVE );
}

void FPSMotionViewController::reset()
{
  camera_->setPosition( Ogre::Vector3( 0.0, 0.0, 1.75 ));
  camera_->lookAt( 0.0, 0.0, 0.0);
  setPropertiesFromCamera( camera_ );

  updateCamera();
  camera_->lookAt( -1.0, 0.0, 1.75);
  setPropertiesFromCamera( camera_ );
}

void FPSMotionViewController::handleMouseEvent(ViewportMouseEvent& event)
{
  bool moved = false;

  int32_t diff_x = 0;
  int32_t diff_y = 0;

  if( event.type == QEvent::MouseMove )
  {
    diff_x = event.x - event.last_x;
    diff_y = event.y - event.last_y;
    moved = true;
  }

  if( event.left() )
  {
    setCursor( Rotate3D );
    yaw( -diff_x * 0.005 );
    pitch( diff_y * 0.005 );
  }

  if (moved)
  {
    context_->queueRender();
  }
}

void FPSMotionViewController::setPropertiesFromCamera( Ogre::Camera* source_camera )
{
  Ogre::Quaternion quat = source_camera->getOrientation() * ROBOT_TO_CAMERA_ROTATION.Inverse();
  float yaw = quat.getRoll( false ).valueRadians(); // OGRE camera frame looks along -Z, so they call rotation around Z "roll".
  float pitch = quat.getYaw( false ).valueRadians(); // OGRE camera frame has +Y as "up", so they call rotation around Y "yaw".

  Ogre::Vector3 direction = quat * Ogre::Vector3::NEGATIVE_UNIT_Z;

  if ( direction.dotProduct( Ogre::Vector3::NEGATIVE_UNIT_Z ) < 0 )
  {
    if ( pitch > Ogre::Math::HALF_PI )
    {
      pitch -= Ogre::Math::PI;
    }
    else if ( pitch < -Ogre::Math::HALF_PI )
    {
      pitch += Ogre::Math::PI;
    }

    yaw = -yaw;

    if ( direction.dotProduct( Ogre::Vector3::UNIT_X ) < 0 )
    {
      yaw -= Ogre::Math::PI;
    }
    else
    {
      yaw += Ogre::Math::PI;
    }
  }

  pitch_property_->setFloat( pitch );
  yaw_property_->setFloat( mapAngleTo0_2Pi( yaw ));
  position_property_->setVector( source_camera->getPosition() );
}

void FPSMotionViewController::mimic( ViewController* source_view )
{
  FramePositionTrackingViewController::mimic( source_view );
  setPropertiesFromCamera( source_view->getCamera() );
}

void FPSMotionViewController::update(float dt, float ros_dt)
{
  FramePositionTrackingViewController::update( dt, ros_dt );
  updateCamera();
}

void FPSMotionViewController::lookAt( const Ogre::Vector3& point )
{
  camera_->lookAt( point );
  setPropertiesFromCamera( camera_ );
}

void FPSMotionViewController::onTargetFrameChanged(const Ogre::Vector3& old_reference_position, const Ogre::Quaternion& old_reference_orientation)
{
  position_property_->add( old_reference_position - reference_position_ );
}

void FPSMotionViewController::updateCamera()
{
  camera_->setOrientation( getOrientation() );
  camera_->setPosition( position_property_->getVector() );
}

void FPSMotionViewController::updateCamera(Ogre::Vector3& position, Ogre::Quaternion& orientation )
{
  camera_->setOrientation(orientation);
  camera_->setPosition(position);
  context_->queueRender();
}

void FPSMotionViewController::yaw( float angle )
{
  yaw_property_->setFloat( mapAngleTo0_2Pi( yaw_property_->getFloat() + angle ));
}

void FPSMotionViewController::pitch( float angle )
{
  pitch_property_->add( angle );
}

Ogre::Quaternion FPSMotionViewController::getOrientation()
{
  Ogre::Quaternion pitch, yaw;

  yaw.FromAngleAxis( Ogre::Radian( yaw_property_->getFloat() ), Ogre::Vector3::UNIT_Z );
  pitch.FromAngleAxis( Ogre::Radian( pitch_property_->getFloat() ), Ogre::Vector3::UNIT_Y );

  return yaw * pitch * ROBOT_TO_CAMERA_ROTATION;
}

void FPSMotionViewController::move( float x, float y, float z )
{
  Ogre::Vector3 translate( x, y, z );
  Ogre::Vector3 update = getOrientation() * translate;
  update.z = 0.0;
  position_property_->add( update );
}

void FPSMotionViewController::changeZ(float z)
{
  Ogre::Vector3 translate( 0.0, 0.0, z );
  Ogre::Vector3 update = getOrientation() * translate;
  position_property_->add( translate );
}

void FPSMotionViewController::fly( float x, float y, float z )
{
  Ogre::Vector3 translate( x, y, z );
  Ogre::Vector3 update = getOrientation() * translate;
  position_property_->add( update );
}

} // end namespace rviz

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz::FPSMotionViewController, rviz::ViewController)
