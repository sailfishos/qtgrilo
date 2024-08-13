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

#include "grilomultisearch.h"
#include "griloregistry.h"

#include <QDebug>

GriloMultiSearch::GriloMultiSearch(QObject *parent)
    : GriloDataSource(parent)
{
}

GriloMultiSearch::~GriloMultiSearch()
{
}

bool GriloMultiSearch::refresh()
{
    cancelRefresh();

    GriloRegistry *registry = getGriloRegistry();
    if (!registry) {
        qWarning() << "GriloRegistry not set";
        return false;
    }

    GList *sources = NULL;

    Q_FOREACH (const QString &src, m_sources) {
        GrlSource *elem = registry->lookupSource(src);
        if (elem) {
            sources = g_list_append(sources, elem);
        } else {
            qWarning() << "Failed to obtain source for" << src;
        }
    }

    GList *keys = keysAsList();
    GrlOperationOptions *options = operationOptions(NULL, Search);

    setFetching(true);
    guint opId = grl_multiple_search(sources, m_text.toUtf8().constData(),
                                     keys, options, grilo_source_result_cb, this);

    setOpId(opId);
    g_list_free(sources);

    g_object_unref(options);
    g_list_free(keys);

    return opId != 0;
}

QStringList GriloMultiSearch::sources() const
{
    return m_sources;
}

void GriloMultiSearch::setSources(const QStringList &sources)
{
    if (m_sources != sources) {
        m_sources = sources;
        Q_EMIT sourcesChanged();
    }
}

QString GriloMultiSearch::text() const
{
    return m_text;
}

void GriloMultiSearch::setText(const QString &text)
{
    if (m_text != text) {
        m_text = text;
        Q_EMIT textChanged();
    }
}
