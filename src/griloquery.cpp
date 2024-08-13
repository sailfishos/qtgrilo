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

#include "griloquery.h"
#include "griloregistry.h"

#include <QDebug>

GriloQuery::GriloQuery(QObject *parent)
    : GriloDataSource(parent),
      m_available(false)
{
}

GriloQuery::~GriloQuery()
{
}

bool GriloQuery::refresh()
{
    cancelRefresh();
    GriloRegistry *registry = getGriloRegistry();

    if (!registry) {
        qWarning() << "GriloRegistry not set";
        return false;
    }

    if (m_source.isEmpty()) {
        qWarning() << "source id not set";
        return false;
    }

    GrlSource *src = registry->lookupSource(m_source);

    if (!src) {
        qWarning() << "Failed to get source" << m_source;
        return false;
    }

    GList *keys = keysAsList();
    GrlOperationOptions *options = operationOptions(src, Search);
    setFetching(true);
    guint opId = grl_source_query(src, m_query.toUtf8().constData(),
                                  keys, options, grilo_source_result_cb, this);
    setOpId(opId);

    g_object_unref(options);
    g_list_free(keys);

    return opId != 0;
}

QString GriloQuery::source() const
{
    return m_source;
}

void GriloQuery::setSource(const QString &source)
{
    if (m_source != source) {
        m_source = source;
        Q_EMIT sourceChanged();
        Q_EMIT slowKeysChanged();
        Q_EMIT supportedKeysChanged();
    }
}

QString GriloQuery::query() const
{
    return m_query;
}

void GriloQuery::setQuery(const QString &query)
{
    if (m_query != query) {
        m_query = query;
        Q_EMIT queryChanged();
    }
}

QVariantList GriloQuery::supportedKeys() const
{
    GriloRegistry *registry = getGriloRegistry();

    if (m_source.isEmpty() || !registry) {
        return QVariantList();
    }

    GrlSource *src = registry->lookupSource(m_source);
    if (src) {
        return listToVariantList(grl_source_supported_keys(src));
    }

    return QVariantList();
}

QVariantList GriloQuery::slowKeys() const
{
    GriloRegistry *registry = getGriloRegistry();

    if (m_source.isEmpty() || !registry) {
        return QVariantList();
    }

    GrlSource *src = registry->lookupSource(m_source);
    if (src) {
        return listToVariantList(grl_source_slow_keys(src));
    }

    return QVariantList();
}

bool GriloQuery::isAvailable() const
{
    GriloRegistry *registry = getGriloRegistry();

    return registry && !m_source.isEmpty() &&
           registry->availableSources().indexOf(m_source) != -1;
}

void GriloQuery::availableSourcesChanged()
{
    bool available = isAvailable();

    if (m_available != available) {
        m_available = available;

        Q_EMIT availabilityChanged();
    }

    if (!m_available && getOpId()) {
        // A source has disappeared while an operation is already running.
        // Not sure how will grilo behave but we will just reset the opId
        setOpId(0);
    }
}

void GriloQuery::contentChanged(const QString &source, GrlSourceChangeType change_type,
                                GPtrArray *changed_media)
{
    if (source == m_source) {
        updateContent(change_type, changed_media);
    }
}
