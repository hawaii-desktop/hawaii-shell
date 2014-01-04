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

#ifndef QUICKVIEW_H
#define QUICKVIEW_H

#include <QtQuick/QQuickView>

#include <HawaiiShell/HawaiiShell>

namespace Hawaii {

namespace Shell {

class QuickViewPrivate;

class HAWAIISHELL_EXPORT QuickView : public QQuickView
{
    Q_OBJECT
    Q_PROPERTY(Hawaii::Shell::Types::Location location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(QRectF screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)
public:
    /*!
     * Constructor.
     * \param parent the QWindow this view is parented to
     */
    explicit QuickView(QWindow *parent = 0);

    /*!
     * Constructor.
     * \param engine the QML engine this view will use
     * \param parent the QWindow this view is parented to
     */
    QuickView(QQmlEngine *engine, QWindow *parent = 0);

    /*!
     * Destructor.
     */
    virtual ~QuickView();

    /*!
     * \return the location of this view.
     */
    Hawaii::Shell::Types::Location location() const;

    /*!
     * Sets the location of this view.
     * \param location the location of the view
     */
    void setLocation(Types::Location location);

    /*!
     * \return the geometry of the screen where this view is located.
     */
    QRectF screenGeometry() const;

Q_SIGNALS:
    /*!
     * Emitted when the location is changed.
     * \param location the new location of the view
     */
    void locationChanged(Hawaii::Shell::Types::Location location);

    /*!
     * Emitted when the screen geometry is changed.
     * \param geometry the new screen geometry
     */
    void screenGeometryChanged(const QRect &geometry);

private:
    Q_DECLARE_PRIVATE(QuickView)
    QuickViewPrivate *const d_ptr;
};

} // namespace Shell

} // namespace Hawaii

#endif // QUICKVIEW_H
