/*
 * UOS-ROS packages - Robot Operating System code by the University of Osnabrück
 * Copyright (C) 2014 University of Osnabrück
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * fps_motion_tool.cpp
 *
 * Author: Henning Deeken {hdeeken@uos.de}
 *
 */

#include <fps_motion_tool.h>

namespace rviz
{

FPSMotionTool::FPSMotionTool()
{
  shortcut_key_ = 'q';
  access_all_keys_ = true;
}

FPSMotionTool::~FPSMotionTool() {}

void FPSMotionTool::onInitialize()
{
  setName( "FPS Motion" );

  step_length_property_ = new FloatProperty( "Step Length", 0.1,
                                                            "The length by with the position is updated on each step.",
                                                            getPropertyContainer(), SLOT( setOffset() ), this );

  boost_property_ = new FloatProperty( "Boost Property", 0.5,
                                                            "Gives the boost factor which is applied if pressing shift.",
                                                            getPropertyContainer(), SLOT( setBoost() ), this );

  fly_property_ = new BoolProperty( "Fly Mode", false,
                                                            "In fly mode it is possible to move along the z axis as well.",
                                                            getPropertyContainer(), SLOT( setFlyMode() ), this );

  left_hand_property_ = new BoolProperty( "Left Hand Mode", false,
                                                            "In left hand mode one uses the arrows to move around, instead of wasd.",
                                                            getPropertyContainer(), SLOT( setLeftHandMode() ), this );

  fallback_tool_property_ = new EnumProperty( "Fallback Tool", QString("rviz/Interact"),
                                                            "Determines to which tool the control switches, if the tool is deactivated.",
                                                            getPropertyContainer(), SLOT( setFallbackTool() ), this );

  fallback_view_controller_property_ = new EnumProperty( "Fallback ViewController", QString("rviz/Orbit"),
                                                            "Determines to which view controller the control switches, if the tool is deactivated.",
                                                            getPropertyContainer(), SLOT( setFallbackViewController() ), this );
  m_pos_offset = 0.1;
  m_boost = 0.5;
  m_fly_mode = false;
  m_left_hand_mode = false;

  setFallbackToolProperty();
  setFallbackViewControllerProperty();
}

void FPSMotionTool::setFallbackViewControllerProperty()
{
  fallback_view_controller_property_->clearOptions();
  m_view_controller_classes.clear();

  m_view_controller_classes = context_->getViewManager()->getFactory()->getDeclaredClassIds();

  for( int i = 0; i < m_view_controller_classes.size(); i++ )
  {
    if(m_view_controller_classes[i] != QString("rviz/FPSMotion"))
    {
      fallback_view_controller_property_->addOption(m_view_controller_classes[i], i);
      m_view_controller.push_back(context_->getViewManager()->getViewAt(i));
    }

  }

  fallback_view_controller_property_->show();
  setFallbackViewController();
}

void FPSMotionTool::setFallbackToolProperty()
{
  fallback_tool_property_->clearOptions();
  m_tools.clear();

  m_tool_classes = context_->getToolManager()->getToolClasses();

  for(int i = 0; i < m_tool_classes.size(); i++)
  {
    if(m_tool_classes[i] != getClassId())
    {
      fallback_tool_property_->addOption(m_tool_classes[i], i);
      m_tools.push_back(context_->getToolManager()->getTool(i));
    }
  }

  fallback_tool_property_->show();
  setFallbackTool();
}

void FPSMotionTool::activate()
{
  context_->getViewManager()->setCurrentViewControllerType(QString("rviz/FPSMotion"));
  setFallbackToolProperty();
  setFallbackViewControllerProperty();
}

void FPSMotionTool::deactivate(){ }

int FPSMotionTool::processKeyEvent(QKeyEvent *event, rviz::RenderPanel* panel)
{
  if(panel->getViewController()->getClassId().toStdString() != "rviz/FPSMotion")
  {
    ROS_WARN("The FPS Motion Tool only works with an active rviz/FPSMotion ViewController. \n If you use the shortkeys to select ('e') and deselect ('e') this tool, the switching will be automatized for you.");
  }
  else
  {
    double update = m_pos_offset;

    if(event->modifiers() & Qt::ShiftModifier)
    {
      update += m_boost * m_pos_offset * 0.01;
    }

    // move forward / backward
    if ((event->key() == Qt::Key_W && !m_left_hand_mode) || (event->key() == Qt::Key_Up && m_left_hand_mode))
    {
      if(m_fly_mode)
        ((rviz::FPSMotionViewController*) panel->getViewController())->fly(0.0, 0.0, -update);
      else
        ((rviz::FPSMotionViewController*) panel->getViewController())->move(0.0, 0.0, -update);
    }

    if ((event->key() == Qt::Key_S && !m_left_hand_mode) || (event->key() == Qt::Key_Down && m_left_hand_mode))
    {
      if(m_fly_mode)
        ((rviz::FPSMotionViewController*) panel->getViewController())->fly(0.0, 0.0, update);
      else
        ((rviz::FPSMotionViewController*) panel->getViewController())->move(0.0, 0.0, update);
    }

    // move left / right
    if ((event->key() == Qt::Key_A && !m_left_hand_mode) || (event->key() == Qt::Key_Left && m_left_hand_mode))
    {
      if(m_fly_mode)
      ((rviz::FPSMotionViewController*) panel->getViewController())->fly(-update*0.7, 0.0, 0.0);
      else
      ((rviz::FPSMotionViewController*) panel->getViewController())->move(-update*0.7, 0.0, 0.0);
    }

    if ((event->key() == Qt::Key_D && !m_left_hand_mode) || (event->key() == Qt::Key_Right && m_left_hand_mode))
    {
      if(m_fly_mode)
        ((rviz::FPSMotionViewController*) panel->getViewController())->fly(update*0.7, 0.0, 0.0);
      else
        ((rviz::FPSMotionViewController*) panel->getViewController())->move(update*0.7, 0.0, 0.0);
    }

    // move up / down
    if ((event->key() == Qt::Key_Up && !m_left_hand_mode) || (event->key() == Qt::Key_W && m_left_hand_mode))
    {
        ((rviz::FPSMotionViewController*) panel->getViewController())->changeZ(update*0.4);
    }

    if ((event->key() == Qt::Key_Down && !m_left_hand_mode) || (event->key() == Qt::Key_S && m_left_hand_mode))
    {
      ((rviz::FPSMotionViewController*) panel->getViewController())->changeZ(-update*0.4);
    }

    // yaw left / down
    if ((event->key() == Qt::Key_Left && !m_left_hand_mode) || (event->key() == Qt::Key_A && m_left_hand_mode))
    {
        ((rviz::FPSMotionViewController*) panel->getViewController())->yaw(update*0.08);
    }

    if ((event->key() == Qt::Key_Right && !m_left_hand_mode) || (event->key() == Qt::Key_D && m_left_hand_mode))
    {
      ((rviz::FPSMotionViewController*) panel->getViewController())->yaw(-update*0.08);
    }

    // switch walk/fly mode
    if (event->key() == Qt::Key_F)
    {
      m_fly_mode = !m_fly_mode;
      fly_property_->setValue(m_fly_mode);
      fly_property_->show();
    }

    // reset the view
    if (event->key() == Qt::Key_R)
    {
      m_fly_mode = false;
      fly_property_->setValue(m_fly_mode);
      fly_property_->show();

      ((rviz::FPSMotionViewController*) panel->getViewController())->reset();
    }

    // deactivate tool and switch back into interactive mode
    if (event->key() == Qt::Key_E)
    {
      context_->getToolManager()->setCurrentTool(m_fallback_tool);
      context_->getViewManager()->setCurrentViewControllerType(m_fallback_view_controller);
    }
  }

  return Render;
}

int FPSMotionTool::processMouseEvent(rviz::ViewportMouseEvent& event)
{
  if (event.panel->getViewController())
  {
    event.panel->getViewController()->handleMouseEvent(event);
    setCursor( event.panel->getViewController()->getCursor() );
  }
  return 0;
}

} // end namespace rviz

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(rviz::FPSMotionTool, rviz::Tool)
