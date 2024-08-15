// -*- c++ -*-

/*!
 *
 * Copyright (C) 2012-2014 Jolla Ltd.
 *
 * Contact: Mohammed Hassan <mohammed.hassan@jollamobile.com>
 * Authors: Mohammed Hassan <mohammed.hassan@jollamobile.com>,
 *          Andres Gomez <agomez@igalia.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef GRILO_MODEL_H
#define GRILO_MODEL_H

#include <GriloQt>

#include <QAbstractListModel>

class GriloMedia;
class GriloDataSource;
class GriloModelPrivate;

class GRILO_QT_EXPORT GriloModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(GriloDataSource *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

    friend class GriloDataSource;

public:
    enum {
        MediaRole = Qt::UserRole + 1,
    };

    GriloModel(QObject *parent = 0);
    virtual ~GriloModel();

    QHash<int, QByteArray> roleNames() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    GriloDataSource *source() const;
    void setSource(GriloDataSource *source);

    int count() const;

    Q_INVOKABLE GriloMedia* getMediaItem(int index);

Q_SIGNALS:
    void sourceChanged();
    void countChanged();

private:
    GriloModelPrivate *d;
};

#endif /* GRILO_MODEL_H */
