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

#include "grilomodel.h"
#include "grilomedia.h"
#include "grilodatasource.h"

#include <QDebug>

class GriloModelPrivate
{
public:
    GriloDataSource *m_source;

    mutable QHash<int, QByteArray> m_roleNames;
};

GriloModel::GriloModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new GriloModelPrivate)
{
    d->m_source = nullptr;
    QObject::connect(this, SIGNAL(rowsInserted(const QModelIndex &, int, int)),
                     this, SIGNAL(countChanged()));
    QObject::connect(this, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                     this, SIGNAL(countChanged()));
}

GriloModel::~GriloModel()
{
    setSource(0);
    delete d;
}

int GriloModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return d->m_source ? d->m_source->media()->count() : 0;
    }

    return 0;
}

QVariant GriloModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    switch (role) {
    case MediaRole:
        return QVariant::fromValue(d->m_source->media()->at(index.row()));
    default: {
        QList<QByteArray> keys = roleNames().values(role);
        if (keys.length() > 0) {
            return d->m_source->media()->at(index.row())->get(role - MediaRole);
        }
    }
    }

    return QVariant();
}

GriloDataSource *GriloModel::source() const
{
    return d->m_source;
}

void GriloModel::setSource(GriloDataSource *source)
{
    if (d->m_source == source) {
        return;
    }

    beginResetModel();

    if (d->m_source) {
        d->m_source->removeModel(this);
    }

    d->m_source = source;

    if (d->m_source) {
        d->m_source->addModel(this);
    }

    endResetModel();

    Q_EMIT sourceChanged();

    if (d->m_source) {
        d->m_source->prefill(this);
    }
}

int GriloModel::count() const
{
    return rowCount();
}

QHash<int, QByteArray> GriloModel::roleNames() const
{
    if (d->m_roleNames.isEmpty())
        grl_init(0, 0);

    d->m_roleNames[MediaRole] = "media";

    int cursor = GRL_METADATA_KEY_INVALID;

    while (const char *metadataKey = GRL_METADATA_KEY_GET_NAME(++cursor)) {
        d->m_roleNames[MediaRole + cursor] = metadataKey;

        QStringList splitKey = QString(metadataKey).split("-", QString::SkipEmptyParts);
        if (splitKey.length() > 1) {
            QByteArray camelCaseKey = splitKey[0].toUtf8();

            for (int i = 1; i < splitKey.length(); i++) {
                QString camelCase = splitKey[i];
                camelCase[0] = camelCase[0].toUpper();
                camelCaseKey += camelCase.toUtf8();
            }

            d->m_roleNames.insertMulti(MediaRole + cursor, camelCaseKey);
        }
    }

    return d->m_roleNames;
}

GriloMedia* GriloModel::getMediaItem(int index)
{
    if (index < 0 || index >= rowCount()) {
        return nullptr;
    }

    return d->m_source->media()->at(index);
}
