/*!
 *
 * Copyright (C) 2012 Jolla Ltd.
 *
 * Contact: Mohammed Hassan <mohammed.hassan@jollamobile.com>
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

#include "grilosearch.h"
#include "griloregistry.h"

#include <QDebug>

class GriloSearchPrivate
{
public:
    QString m_source;
    QString m_text;

    QVariantList m_slowKeys;
    QVariantList m_supportedKeys;
    bool m_available = false;
};

GriloSearch::GriloSearch(QObject *parent)
    : GriloDataSource(parent)
    , d(new GriloSearchPrivate)
{
}

GriloSearch::~GriloSearch()
{
    delete d;
}

bool GriloSearch::refresh()
{
    cancelRefresh();

    GriloRegistry *registry = getGriloRegistry();
    if (!registry) {
        qWarning() << "GriloRegistry not set";
        return false;
    }

    if (d->m_source.isEmpty()) {
        qWarning() << "source id not set";
        return false;
    }

    GrlSource *src = registry->lookupSource(d->m_source);

    if (!src) {
        qWarning() << "Failed to get source" << d->m_source;
        return false;
    }

    GList *keys = keysAsList();
    GrlOperationOptions *options = operationOptions(src, Search);
    setFetching(true);
    guint opId = grl_source_search(src, d->m_text.toUtf8().constData(),
                                   keys, options, grilo_source_result_cb, this);
    setOpId(opId);

    g_object_unref(options);
    g_list_free(keys);

    return opId != 0;
}

QString GriloSearch::source() const
{
    return d->m_source;
}

void GriloSearch::setSource(const QString &source)
{
    if (d->m_source != source) {
        d->m_source = source;
        Q_EMIT sourceChanged();
        Q_EMIT slowKeysChanged();
        Q_EMIT supportedKeysChanged();
    }
}

QString GriloSearch::text() const
{
    return d->m_text;
}

void GriloSearch::setText(const QString &text)
{
    if (d->m_text != text) {
        d->m_text = text;
        Q_EMIT textChanged();
    }
}

QVariantList GriloSearch::supportedKeys() const
{
    GriloRegistry *registry = getGriloRegistry();

    if (d->m_source.isEmpty() || !registry) {
        return QVariantList();
    }

    GrlSource *src = registry->lookupSource(d->m_source);
    if (src) {
        return listToVariantList(grl_source_supported_keys(src));
    }

    return QVariantList();
}

QVariantList GriloSearch::slowKeys() const
{
    GriloRegistry *registry = getGriloRegistry();

    if (d->m_source.isEmpty() || !registry) {
        return QVariantList();
    }

    GrlSource *src = registry->lookupSource(d->m_source);
    if (src) {
        return listToVariantList(grl_source_slow_keys(src));
    }

    return QVariantList();
}

bool GriloSearch::isAvailable() const
{
    GriloRegistry *registry = getGriloRegistry();

    return registry && !d->m_source.isEmpty() &&
           registry->availableSources().indexOf(d->m_source) != -1;
}

void GriloSearch::availableSourcesChanged()
{
    bool available = isAvailable();

    if (d->m_available != available) {
        d->m_available = available;

        Q_EMIT availabilityChanged();
    }

    if (!d->m_available && getOpId()) {
        // A source has disappeared while an operation is already running.
        // Most grilo will crash soon but we will just reset the opId
        setOpId(0);
    }
}
