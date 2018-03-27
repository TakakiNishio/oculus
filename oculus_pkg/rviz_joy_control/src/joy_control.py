#!/usr/bin/env python
from pynput.keyboard import Key, Controller

import rospy
from sensor_msgs.msg import Joy

class RvizJoy(object):

    def __init__(self):
        print "Rviz Joy Controller"
        self.keyboard = Controller()
        self.sleep_rate = rospy.Rate(100)
        self._joy_sub = rospy.Subscriber('joy', Joy, self.joy_callback, queue_size=1, buff_size=2**24)

    def joy_callback(self, joy_msg):

        # x+
        if joy_msg.buttons[5] == 1:
            self.keyboard.press('d')
            self.keyboard.release('d')
            return

        # x-
        if joy_msg.buttons[7] == 1:
            self.keyboard.press('a')
            self.keyboard.release('a')
            return

        # y+
        if joy_msg.buttons[4] == 1:
            self.keyboard.press('w')
            self.keyboard.release('w')
            return

        # y-
        if joy_msg.buttons[6] == 1:
            self.keyboard.press('s')
            self.keyboard.release('s')
            return
        
        # up
        if joy_msg.buttons[12] == 1:
            self.keyboard.press(Key.up)
            self.keyboard.release(Key.up)
            self.sleep_rate.sleep()
            return

        # down
        if joy_msg.buttons[14] == 1:
            self.keyboard.press(Key.down)
            self.keyboard.release(Key.down)
            self.sleep_rate.sleep()
            return

        # right
        if joy_msg.buttons[13] == 1:
            self.keyboard.press(Key.right)
            self.keyboard.release(Key.right)
            self.sleep_rate.sleep()
            return

        # left
        if joy_msg.buttons[15] == 1:
            self.keyboard.press(Key.left)
            self.keyboard.release(Key.left)
            self.sleep_rate.sleep()
            return



if __name__ == '__main__':
    rospy.init_node('rviz_joy_control')
    joy_twist = RvizJoy()
    rospy.spin()
