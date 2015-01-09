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

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtCore/QFileSystemWatcher>

#include "cmakedirs.h"
#include "processcontroller.h"

#define FULLSCREEN_SHELL_SOCKET "hawaii-master-"
#define HAWAII_SOCKET "hawaii-slave-"

ProcessController::ProcessController(bool nested, QObject *parent)
    : QObject(parent)
    , m_fullScreenShell(Q_NULLPTR)
{
    // Wayland sockets
    QString random = randomString();
    m_compositorSocket = QStringLiteral(HAWAII_SOCKET) + random;
    m_fullScreenShellSocket = QStringLiteral(FULLSCREEN_SHELL_SOCKET) + random;

    // Full screen shell
    if (nested) {
        m_fullScreenShell = new QProcess(this);
        m_fullScreenShell->setProcessChannelMode(QProcess::ForwardedChannels);
        m_fullScreenShell->setProgram(QStringLiteral(INSTALL_BINDIR "/weston"));
        m_fullScreenShell->setArguments(QStringList()
                                        << QStringLiteral("--shell=fullscreen-shell.so")
                                        << QStringLiteral("--socket=") + m_fullScreenShellSocket);
        connect(m_fullScreenShell, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(fullScreenShellFinished(int,QProcess::ExitStatus)));

        m_fullScreenShellWatcher = new QFileSystemWatcher(this);
        m_fullScreenShellWatcher->addPath(QString::fromUtf8(qgetenv("XDG_RUNTIME_DIR")));
        connect(m_fullScreenShellWatcher, SIGNAL(directoryChanged(QString)),
                this, SLOT(directoryChanged(QString)));
    }

    // Compositor process
    m_compositor = new QProcess(this);
    m_compositor->setProcessChannelMode(QProcess::ForwardedChannels);
    m_compositor->setProgram(QStringLiteral(INSTALL_BINDIR "/hawaii"));
    m_compositor->setArguments(QStringList()
                               << QStringLiteral("-platformtheme")
                               << QStringLiteral("Hawaii")
                               << QStringLiteral("-p")
                               << QStringLiteral("org.hawaii.desktop"));
    if (nested) {
        m_compositor->setArguments(m_compositor->arguments()
                                   << QStringLiteral("-platform")
                                   << QStringLiteral("wayland")
                                   << QStringLiteral("--socket=") + m_compositorSocket);
    }
    connect(m_compositor, SIGNAL(finished(int,QProcess::ExitStatus)),
            this, SLOT(compositorFinished(int,QProcess::ExitStatus)));
}

void ProcessController::start()
{
    if (m_fullScreenShell) {
        // Run the full screen shell compositor if enabled
        qDebug() << "Running:" << qPrintable(m_fullScreenShell->program())
                 << qPrintable(m_fullScreenShell->arguments().join(" "));
        m_fullScreenShell->start();

        if (!m_fullScreenShell->waitForStarted())
            qFatal("Full Screen Shell compositor cannot be started, aborting...");
    } else {
        // Start the compositor
        startCompositor();
    }
}

void ProcessController::stop()
{
    // Stop the compositor, this will also stop fullscreen shell
    // if it was started
    m_compositor->terminate();
    if (!m_compositor->waitForFinished())
        m_compositor->kill();
    m_compositor->deleteLater();
    m_compositor = Q_NULLPTR;
    Q_EMIT stopped();
}

QString ProcessController::randomString() const
{
    // Courtesy of Merlin069 from:
    // http://stackoverflow.com/questions/18862963/qt-c-random-string-generation

    const QString possibleCharacters("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    const int randomStringLength = 12;

    QString randomString;
    for(int i = 0; i<randomStringLength; ++i) {
        int index = qrand() % possibleCharacters.length();
        QChar nextChar = possibleCharacters.at(index);
        randomString.append(nextChar);
    }

    return randomString;
}

void ProcessController::startCompositor()
{
    // Pass arguments for full screen shell and style
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    if (m_fullScreenShell)
        env.insert(QStringLiteral("WAYLAND_DISPLAY"), m_fullScreenShellSocket);
    env.insert(QStringLiteral("KSCREEN_BACKEND"), QStringLiteral("QScreen"));
    env.insert(QStringLiteral("QT_QUICK_CONTROLS_STYLE"), QStringLiteral("Wind"));
    if (qEnvironmentVariableIsSet("DISPLAY") && !m_fullScreenShell)
        env.insert(QStringLiteral("QT_XCB_GL_INTEGRATION"), QStringLiteral("xcb_egl"));
    m_compositor->setProcessEnvironment(env);

    // Start the process
    qDebug() << "Running:" << qPrintable(m_compositor->program())
             << qPrintable(m_compositor->arguments().join(" "));
    m_compositor->start();
    if (!m_compositor->waitForStarted()) {
        // Compositor failed to start, kill full screen shell and terminate
        qFatal("Compositor won't start, aborting...");
        compositorFinished(0, QProcess::NormalExit);
    } else {
        Q_EMIT started();
    }
}

void ProcessController::directoryChanged(const QString &path)
{
    Q_UNUSED(path);
    Q_ASSERT(m_fullScreenShell);

    // Don't start until the full screen shell compositor is up
    const QString socketFileName = path + QStringLiteral("/") +
            m_fullScreenShellSocket;
    if (!QFile::exists(socketFileName))
        return;

    // Socket is here, let's bring the compositor up but first disconnect
    // and destroy the file system watcher otherwise this slot gets
    // called over and over again
    m_fullScreenShellWatcher->disconnect(this);
    m_fullScreenShellWatcher->deleteLater();
    m_fullScreenShellWatcher = Q_NULLPTR;
    startCompositor();
}

void ProcessController::compositorFinished(int code, const QProcess::ExitStatus &status)
{
    Q_UNUSED(status);

    if (code != 0)
        qWarning() << "Compositor finished with exit code" << code;

    // Whathever the reason why it finished is we need to quit the
    // full screen shell compositor, if any
    if (m_fullScreenShell) {
        m_fullScreenShell->terminate();
        if (!m_fullScreenShell->waitForFinished())
            m_fullScreenShell->kill();
        m_fullScreenShell->deleteLater();
        m_fullScreenShell = Q_NULLPTR;
    }

    // Quit
    qApp->quit();
}

void ProcessController::fullScreenShellFinished(int code, const QProcess::ExitStatus &status)
{
    Q_UNUSED(status);

    if (code != 0)
        qWarning() << "Full screen shell finished with exit code" << code;

    // Kill the compositor if for some reason is still running
    m_compositor->terminate();
    if (!m_compositor->waitForFinished())
        m_compositor->kill();

    // Quit
    qApp->quit();
}
