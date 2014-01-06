/****************************************************************************
 * This file is part of Hawaii Shell.
 *
 * Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
 *
 * Author(s):
 *    Pier Luigi Fiorini
 *
 * $BEGIN_LICENSE:LGPL2.1+$
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * $END_LICENSE$
 ***************************************************************************/

#include "shellpopupsurface.h"

ShellPopupSurface::ShellPopupSurface(QWaylandSurface *surface)
    : QtWaylandServer::wl_hawaii_popup_surface()
    , m_surface(surface)
    , m_popupGrabber(nullptr)
{
}

ShellPopupSurface::~ShellPopupSurface()
{
    if (m_popupGrabber)
        m_popupGrabber->setPopupSurface(nullptr);
}

QWaylandSurface *ShellPopupSurface::surface() const
{
    return m_surface;
}

void ShellPopupSurface::setSurface(QWaylandSurface *surface)
{
    m_surface = surface;
}

PopupGrabber *ShellPopupSurface::popupGrabber() const
{
    return m_popupGrabber;
}

void ShellPopupSurface::setPopupGrabber(PopupGrabber *grabber)
{
    m_popupGrabber = grabber;
    m_popupGrabber->setPopupSurface(this);
}

void ShellPopupSurface::hawaii_popup_surface_dismiss(Resource *resource)
{
    PopupGrabber *grabber = static_cast<PopupGrabber *>(
                wl_resource_get_user_data(resource->handle));
    if (grabber) {
        wl_resource_set_user_data(resource->handle, nullptr);
        grabber->setPopupSurface(nullptr);
        delete grabber;
    }
}