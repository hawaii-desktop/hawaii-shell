/****************************************************************************
 * This file is part of Hawaii Shell.
 *
 * Copyright (C) 2013-2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QtCompositor/QWaylandSurfaceItem>
#include <QtCompositor/private/qwlsurface_p.h>

#include "compositor.h"
#include "screensaver.h"
#include "shell.h"
#include "popupgrabber.h"
#include "shellsurface.h"

Shell::Shell(struct ::wl_display *display)
    : QObject()
    , QtWaylandServer::hawaii_shell(display)
    , m_compositor(nullptr)
    , m_lockSurface(nullptr)
    , m_locked(false)
    , m_prepareEventSent(false)
{
}

Shell::~Shell()
{
    qDeleteAll(m_keyBindings);
}

void Shell::setCompositor(Compositor *compositor)
{
    m_compositor = compositor;
}

bool Shell::isLocked() const
{
    return m_locked;
}

KeyBindings Shell::keyBindings() const
{
    return m_keyBindings;
}

void Shell::lockSession()
{
    // If session is already locked, set DPMS off
    if (m_locked) {
        m_compositor->setState(Compositor::Sleeping);
        return;
    }

    // Session is now locked
    m_locked = true;
    Q_EMIT lockedChanged(true);

    // Hide all surfaces
    m_compositor->shellSurface()->setSurfacesVisible(false);

    // Run screensaver
    m_compositor->screenSaver()->launchProcess();
}

void Shell::unlockSession()
{
    // If unlocked or the lock surface is already mapped we
    // just return, the QML side will hide the splash screen
    if (!m_locked || m_lockSurface) {
        Q_EMIT m_compositor->fadeIn();
        return;
    }

    // If the shell client has gone away, unlock immediately
    if (!m_compositor->isShellClientRunning()) {
        resumeDesktop();
        return;
    }

    // Just return if we already asked the shell client to
    // prepare the lock surface
    if (m_prepareEventSent)
        return;

    // Ask the shell client to prepare the lock surface
    for (Resource *resource: resourceMap())
        send_prepare_lock_surface(resource->handle);
    m_prepareEventSent = true;
}

void Shell::hawaii_shell_bind_resource(Resource *resource)
{
    send_loaded(resource->handle);
}

void Shell::hawaii_shell_add_key_binding(Resource *resource, uint32_t id,
                                  uint32_t key, uint32_t modifiers)
{
    Q_UNUSED(resource);

    KeyBinding *keyBinding = new KeyBinding(resource->client(), id);
    keyBinding->setModifiers(modifiers);
    keyBinding->setKey(key);
    m_keyBindings.append(keyBinding);
}

void Shell::hawaii_shell_set_position(Resource *resource,
                               struct ::wl_resource *surface_resource,
                               int32_t x, int32_t y)
{
    Q_UNUSED(resource);

    QWaylandSurface *surface =
            QtWayland::Surface::fromResource(surface_resource)->waylandSurface();
    surface->setWindowProperty(QStringLiteral("position"), QPointF(x, y));
}

void Shell::hawaii_shell_set_lock_surface(Resource *resource,
                                   struct ::wl_resource *surface_resource)
{
    Q_UNUSED(resource);

    m_prepareEventSent = false;

    if (!m_locked)
        return;

    // FIXME: Find real output coords
    QPointF coords = m_compositor->outputGeometry().topLeft();

    m_lockSurface =
            QtWayland::Surface::fromResource(surface_resource)->waylandSurface();
    m_lockSurface->setWindowProperty(QStringLiteral("role"), Compositor::BackgroundRole);
    m_lockSurface->setWindowProperty(QStringLiteral("position"), coords);

    connect(m_lockSurface, &QWaylandSurface::mapped, [=]() {
        m_lockSurface->surfaceItem()->takeFocus();
        Q_EMIT m_compositor->fadeIn();
    });

    connect(m_lockSurface, &QObject::destroyed, [=](QObject *object = 0) {
        m_lockSurface = nullptr;
    });
}

void Shell::hawaii_shell_quit(Resource *resource)
{
    Q_UNUSED(resource);
}

void Shell::hawaii_shell_lock(Resource *resource)
{
    Q_UNUSED(resource);

    // Don't lock again in case we're already locked
    if (m_locked || m_lockSurface)
        return;

    m_compositor->setState(Compositor::Idle);
}

void Shell::hawaii_shell_unlock(Resource *resource)
{
    Q_UNUSED(resource);

    // Reset the flag for the prepare lock surface event
    m_prepareEventSent = false;

    // Resume the desktop if we are unlocked
    if (m_locked)
        resumeDesktop();
}

void Shell::hawaii_shell_set_background(Resource *resource,
                                 struct ::wl_resource *output_resource,
                                 struct ::wl_resource *surface_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(output_resource);

    // FIXME: Find real output coords
    QPointF coords = m_compositor->outputGeometry().topLeft();

    QWaylandSurface *surface =
            QtWayland::Surface::fromResource(surface_resource)->waylandSurface();
    surface->setWindowProperty(QStringLiteral("role"), Compositor::BackgroundRole);
    surface->setWindowProperty(QStringLiteral("position"), coords);
}

void Shell::hawaii_shell_set_overlay(Resource *resource,
                              struct ::wl_resource *surface_resource)
{
    Q_UNUSED(resource);

    // FIXME: Find real output coords
    QPointF coords = m_compositor->outputGeometry().topLeft();

    QWaylandSurface *surface =
            QtWayland::Surface::fromResource(surface_resource)->waylandSurface();
    surface->setWindowProperty(QStringLiteral("role"), Compositor::OverlayRole);
    surface->setWindowProperty(QStringLiteral("position"), coords);
}

void Shell::hawaii_shell_set_desktop(Resource *resource,
                              struct ::wl_resource *output_resource,
                              struct ::wl_resource *surface_resource)
{
    Q_UNUSED(resource);
    Q_UNUSED(output_resource);

    // FIXME: Find real output coords
    QPointF coords = m_compositor->outputGeometry().topLeft();

    QWaylandSurface *surface =
            QtWayland::Surface::fromResource(surface_resource)->waylandSurface();
    surface->setWindowProperty(QStringLiteral("role"), Compositor::DesktopRole);
    surface->setWindowProperty(QStringLiteral("position"), coords);
}

void Shell::hawaii_shell_set_grab_surface(Resource *resource,
                                   struct ::wl_resource *surface_resource)
{
    Q_UNUSED(resource);
}

void Shell::hawaii_shell_desktop_ready(Resource *resource)
{
    Q_UNUSED(resource);
    Q_EMIT ready();
}

void Shell::hawaii_shell_minimize_windows(Resource *resource)
{
    Q_UNUSED(resource);
}

void Shell::hawaii_shell_restore_windows(Resource *resource)
{
    Q_UNUSED(resource);
}

void Shell::hawaii_shell_add_workspace(Resource *resource)
{
    Q_UNUSED(resource);
    Q_EMIT workspaceAdded();
}

void Shell::hawaii_shell_select_workspace(Resource *resource,
                                   struct ::wl_resource *workspace)
{
    Q_UNUSED(resource);
}

void Shell::resumeDesktop()
{
    // Terminate screen saver process
    m_compositor->screenSaver()->terminateProcess();

    // Show all surfaces and hide the lock surface
    m_compositor->shellSurface()->setSurfacesVisible(true);
    if (m_lockSurface && m_lockSurface->surfaceItem())
        m_lockSurface->surfaceItem()->setVisible(false);

    // Unlock the session and reveal the desktop
    m_locked = false;
    Q_EMIT lockedChanged(false);
}

#include "moc_shell.cpp"
