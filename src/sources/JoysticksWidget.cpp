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

#include <QPointer>
#include <QPushButton>
#include <QProgressBar>

#include "JoysticksWidget.h"

JoysticksWidget::JoysticksWidget (QWidget* parent) : QWidget (parent) {
    ui.setupUi (this);
    ui.Axes->setVisible (false);
    ui.Rumble->setVisible (false);
    ui.Buttons->setVisible (false);
    ui.Rumble->setMinimumWidth (ui.JoystickList->width());

    m_manager = new GamepadManager;

    connect (m_manager, SIGNAL (axisEvent      (GM_Axis)),
             this,      SLOT   (onAxisEvent    (GM_Axis)));
    connect (m_manager, SIGNAL (buttonEvent    (GM_Button)),
             this,      SLOT   (onButtonEvent  (GM_Button)));
    connect (m_manager, SIGNAL (countChanged   (QStringList)),
             this,      SLOT   (onCountChanged (QStringList)));
    connect (ui.Rumble, SIGNAL (clicked()),
             this,      SLOT   (onRumbleClicked()));

    connect (ui.JoystickList, SIGNAL (currentRowChanged (int)),
             this,            SLOT   (onRowChanged      (int)));

    m_manager->init();
}

JoysticksWidget::~JoysticksWidget() {
    delete m_manager;
}

void JoysticksWidget::onRumbleClicked() {
    m_manager->rumble (ui.JoystickList->currentRow(), 1000);
}

void JoysticksWidget::onRowChanged (int row) {
    /* Remove all buttons and progress bars in the widget */
    foreach (QPushButton *  c, findChildren<QPushButton*>())  delete c;
    foreach (QProgressBar * p, findChildren<QProgressBar*>()) delete p;

    /* Clear joystick data */
    m_axes.clear();
    m_buttons.clear();
    ui.Axes->setVisible (false);
    ui.Buttons->setVisible (false);

    /* Avoid crashing the application when there are no joysticks */
    if (row < 0)  return;

    /* Get joystick information */
    int axisCount = m_manager->getNumAxes (row);
    int buttonCount = m_manager->getNumButtons (row);

    ui.Axes->setVisible (axisCount > 0);
    ui.Buttons->setVisible (buttonCount > 0);

    /* Create a progress bar for each axis */
    for (int i = 0; i < axisCount; ++i) {
        QPointer<QProgressBar> bar = new QProgressBar (this);

        bar->setMaximumHeight (19);
        bar->setMinimumHeight (19);

        bar->setValue (0);
        bar->setMaximum (100);
        bar->setMinimum (-100);
        bar->setFormat (m_manager->getAxisName (i));

        m_axes.append (bar);
        ui.AxesWidget->layout()->addWidget (bar);
    }

    /* Create a button for each joystick button */
    for (int i = 0; i < buttonCount; ++i) {
        QPointer<QPushButton> button = new QPushButton (this);

        button->setEnabled (false);
        button->setCheckable (true);
        button->setMaximumSize (18, 12);
        button->setMinimumSize (18, 12);
        button->setToolTip (m_manager->getButtonName (i));

        /* Distribute the button items in a nice layout */
        int row = (i <= 7) ? i : i - 8;
        int column = (i <= 7) ? 0 : (i / 8);

        m_buttons.append (button);
        ui.ButtonLayout->addWidget (button, row, column);
    }
}

void JoysticksWidget::onCountChanged (const QStringList& list) {
    /* Current count is less and previous count */
    if (list.count() < ui.JoystickList->count())
        emit joystickRemoved();

    /* Reset joystick list */
    ui.JoystickList->clear();

    /* Add an item for each registered joystick */
    if (list.count() > 0) {
        for (int i = 1; i <= list.count(); ++i)
            ui.JoystickList->addItem (QString ("%1: ").arg (i) + list.at (i - 1));

        ui.JoystickList->setCurrentRow (0);
    }

    /* Notify other objects about the new/removed joysticks */
    emit statusChanged (list.count() > 0);

    /* Show or hide the rumble button if needed */
    ui.Rumble->setVisible (list.count() > 0);
}

void JoysticksWidget::onAxisEvent (const GM_Axis& axis) {
    /* Event is from another joystick */
    if (ui.JoystickList->currentRow() != axis.joystick.id)
        return;

    /* Input axis is invalid */
    if (axis.rawId < m_axes.count())
        m_axes.at (axis.rawId)->setValue (axis.value * 100);
}

void JoysticksWidget::onButtonEvent (const GM_Button& button) {
    /* Event is from another joystick */
    if (ui.JoystickList->currentRow() != button.joystick.id)
        return;

    /* Input button is invalid */
    if (button.rawId < m_buttons.count())
        m_buttons.at (button.rawId)->setChecked (button.pressed);
}
