/*
 * Copyright (c) 2015 WinT 3794 <http://wint3794.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "JoystickManager.h"

DS_JoystickManager::DS_JoystickManager()
{
    m_currentJoystick = 0;
    m_tempJoystick = new Joystick;
}

DS_JoystickManager::~DS_JoystickManager()
{
    for (int i = 0; i < m_joysticks.count(); ++i) {
        delete m_joysticks.at (i)->axes;
        delete m_joysticks.at (i)->hats;
        delete m_joysticks.at (i)->buttons;
    }

    delete m_tempJoystick;
}

QByteArray DS_JoystickManager::getData()
{
    QByteArray data;

    /* Send the data of a different joystick each time the function is run */
    if (m_joysticks.count() > 0) {
        /* Update the joystick count to generate the index */
        m_currentJoystick += 1;
        if (m_currentJoystick > m_joysticks.count() - 1)
            m_currentJoystick = 0;

        /* Get the joystick data */
        m_tempJoystick = m_joysticks.at (m_currentJoystick);
        data.append (getJoystickSize (m_tempJoystick) - 1);
        data.append (0xc + m_currentJoystick);

        /* Add axis data */
        data.append (sizeof (m_tempJoystick->axes));
        for (int i = 0; i < m_tempJoystick->numAxes; ++i)
            data.append (m_tempJoystick->axes [i]);

        /* Add button data */
        data.append (sizeof (m_tempJoystick->buttons));
        for (int i = 0; i < m_tempJoystick->numButtons; ++i)
            data.append (m_tempJoystick->buttons [i]);

        /* Add hat/pov data */
        data.append (sizeof (m_tempJoystick->hats));
        for (int i = 0; i < m_tempJoystick->numHats; ++i)
            data.append (m_tempJoystick->hats [i]);
    }

    return data;
}

void DS_JoystickManager::removeAllJoysticks()
{
    m_joysticks.clear();
}

void DS_JoystickManager::removeJoystick (int joystick)
{
    if (m_joysticks.count() > joystick)
        m_joysticks.removeAt (joystick);
}

void DS_JoystickManager::addJoystick (int axes, int buttons, int hats)
{
    Joystick* m_tempJoystick = new Joystick;

    m_tempJoystick->numAxes = axes;
    m_tempJoystick->numHats = hats;
    m_tempJoystick->numButtons = buttons;

    m_tempJoystick->axes = new char [m_tempJoystick->numAxes];
    m_tempJoystick->hats = new char [m_tempJoystick->numHats];
    m_tempJoystick->buttons = new char [m_tempJoystick->numButtons];

    m_joysticks.append (m_tempJoystick);
}

void DS_JoystickManager::updateAxis (int joystick, int axis, double value)
{
    if (m_joysticks.count() > joystick)
        m_joysticks.at (joystick)->axes [axis] = value * 127.5;
}

void DS_JoystickManager::updateButton (int joystick, int button, bool pressed)
{
    if (m_joysticks.count() > joystick)
        m_joysticks.at (joystick)->buttons [button] = pressed;
}

void DS_JoystickManager::updateHat (int joystick, int hat, short angle)
{
    if (m_joysticks.count() > joystick)
        m_joysticks.at (joystick)->hats [hat] = angle;
}

int DS_JoystickManager::getJoystickSize (const Joystick* joystick)
{
    return  5
            + (sizeof (joystick->axes))
            + (sizeof (joystick->buttons) / 8)
            + (sizeof (joystick->buttons) % 8 == 0 ? 0 : 1)
            + (sizeof (joystick->hats) * 2);
}