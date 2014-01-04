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

#include <QtGui/QScreen>

#include "quickview.h"

namespace Hawaii {

namespace Shell {

/*
 * QuickViewPrivate
 */

class QuickViewPrivate
{
    Q_DECLARE_PUBLIC(QuickView)
public:
    QuickViewPrivate(QuickView *view);

    Types::Location location;

    void initialize();

protected:
    QuickView *const q_ptr;
};

QuickViewPrivate::QuickViewPrivate(QuickView *view)
    : location(Types::Desktop)
    , q_ptr(view)
{
}

void QuickViewPrivate::initialize()
{
    Q_Q(QuickView);

    // Transparent color
    q->setColor(Qt::transparent);

    // Set custom window type
    q->setFlags(Qt::BypassWindowManagerHint);

    // Create platform window
    q->create();

    // Emit a signal when the screen geometry is changed
    q->connect(q->screen(), &QScreen::geometryChanged,
               q, &QuickView::screenGeometryChanged);

    // View is sized by the root object
    q->setResizeMode(QQuickView::SizeViewToRootObject);
}

/*
 * QuickView
 */

QuickView::QuickView(QWindow *parent)
    : QQuickView(parent)
    , d_ptr(new QuickViewPrivate(this))
{
    Q_D(QuickView);
    d->initialize();
}

QuickView::QuickView(QQmlEngine *engine, QWindow *parent)
    : QQuickView(engine, parent)
    , d_ptr(new QuickViewPrivate(this))
{
    Q_D(QuickView);
    d->initialize();
}

QuickView::~QuickView()
{
    delete d_ptr;
}

Hawaii::Shell::Types::Location QuickView::location() const
{
    Q_D(const QuickView);
    return d->location;
}

void QuickView::setLocation(Types::Location location)
{
    Q_D(QuickView);

    if (d->location != location) {
        d->location = location;
        Q_EMIT locationChanged(location);
    }
}

QRectF QuickView::screenGeometry() const
{
    return screen()->geometry();
}

} // Shell namespace

} // Hawaii namespace

#include "moc_quickview.cpp"
