/****************************************************************************
 * This file is part of Hawaii Shell.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:GPL2+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

import QtQuick 2.0
import Fluid.Ui 1.0 as FluidUi
import Hawaii.Shell 1.0
import Hawaii.Shell.Styles 1.0

Item {
    id: panelView

    property bool configuring: false

    property Item containment

    onContainmentChanged: {
        containment.visible = true;
        containment.anchors.fill: styledItem
    }

    Connections {
        target: containment
        onMinimumWidthChanged: {
            if (containment.formFactor === Types.Horizontal)
                view.width = Math.max(view.width, view.minimumLength);
        }
        onImplicitWidthChanged: {
            if (containment.formFactor === Types.Horizontal)
                view.width = Math.max(view.maximumLength, Math.max(containment.implicitWidth, view.minimumLength));
        }
        onMinimumWidthChanged: {
            if (containment.formFactor === Types.Horizontal)
                view.width = Math.min(view.width, view.maximumLength);
        }
        onMinimumHeightChanged: {
            if (containment.formFactor === Types.Vertical)
                view.height = Math.max(view.height, view.minimumLength);
        }
        onImplicitHeightChanged: {
            if (containment.formFactor === Types.Vertical)
                view.height = Math.min(view.maximumLength, Math.max(containment.implicitHeight, view.minimumLength));
        }
        onMinimumHeightChanged: {
            if (containment.formFactor === Types.Vertical)
                view.height = Math.min(view.height, view.maximumLength);
        }
    }

    StyledItem {
        id: styledItem
        anchors.fill: parent
        style: Qt.createComponent(StyleSettings.path + "/PanelStyle.qml", panelView)
    }

    Item {
        id: configButton
        width: view.formFactor === Types.Horizontal ? parent.height : parent.width
        height: width
        visible: false
        states: [
            State {
                name: "horizontal"
                when: view.formFactor === Types.Horizontal

                AnchorChanges {
                    target: configButton
                    anchors.top: parent.top
                    anchors.right: parent.right
                }
            },
            State {
                name: "vertical"
                when: view.formFactor === Types.Vertical

                AnchorChanges {
                    target: configButton
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                }
                PropertyChanges {
                    target: configButton
                    anchors.leftMargin: (parent.width / 2) - (configButton.width / 2)
                }
            }
        ]

        FluidUi.Icon {
            anchors.fill: parent
            iconName: "preferences-system-windows"
        }

        Behavior on width {
            PropertyAnimation {}
        }

        Behavior on height {
            PropertyAnimation {}
        }
    }
}