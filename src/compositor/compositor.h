/****************************************************************************
 * This file is part of Hawaii Shell.
 *
 * Copyright (C) 2012-2013 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QProcess>
#include <QQuickView>

#include <VCompositor>

#include "shell.h"
#include "keybinding.h"
#include "clientwindow.h"
#include "workspace.h"
#include "grab.h"
#include "notifications.h"

class Compositor : public QQuickView, public VCompositor
{
    Q_OBJECT
    Q_PROPERTY(QWaylandSurface *currentSurface READ currentSurface WRITE setCurrentSurface NOTIFY currentSurfaceChanged)
public:
    explicit Compositor(const QRect &geometry);
    ~Compositor();

    void runShell();
    void closeShell();

    QWaylandSurface *currentSurface() const {
        return m_currentSurface;
    }

    void surfaceCreated(QWaylandSurface *surface);
    void surfaceAboutToBeDestroyed(QWaylandSurface *surface);

Q_SIGNALS:
    void windowAdded(QVariant window);
    void windowDestroyed(QVariant window);
    void windowResized(QVariant window);
    void currentSurfaceChanged();

public Q_SLOTS:
    void destroyWindow(QVariant window);
    void destroyClientForWindow(QVariant window);

    void setCurrentSurface(QWaylandSurface *surface);

private Q_SLOTS:
    void shellStarted();
    void shellFailed(QProcess::ProcessError error);
    void shellReadyReadStandardOutput();
    void shellReadyReadStandardError();
    void shellAboutToClose();

    void surfaceMapped();
    void surfaceUnmapped();
    void surfaceDestroyed(QObject *object);
    void sceneGraphInitialized();
    void frameSwapped();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Shell *m_shell;
    QList<KeyBinding *> m_keyBindings;
    QList<ClientWindow *> m_clientWindows;
    QList<Workspace *> m_workspaces;
    Notifications *m_notifications;
    QWaylandSurface *m_currentSurface;
    QProcess *m_shellProcess;
};

#endif // COMPOSITOR_H
