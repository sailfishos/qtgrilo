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

#include "grilobrowse.h"
#include "griloregistry.h"
#include "grilomedia.h"

#include <QDebug>

class GriloBrowsePrivate {
public:
    GriloBrowsePrivate();

    QString m_source;

    GriloMedia *m_media;
    QString m_baseMedia;
    QVariantList m_slowKeys;
    QVariantList m_supportedKeys;
    bool m_available;
};

GriloBrowsePrivate::GriloBrowsePrivate()
    : m_media(nullptr)
    , m_available(false)
{
}

GriloBrowse::GriloBrowse(QObject *parent)
    : GriloDataSource(parent)
    , d(new GriloBrowsePrivate)
{
}

GriloBrowse::~GriloBrowse()
{
    setBaseMedia(QString());
    delete d;
}

bool GriloBrowse::refresh()
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
    GrlOperationOptions *options = operationOptions(src, Browse);

    setFetching(true);
    guint opId = grl_source_browse(src, rootMedia(),
                                   keys, options, grilo_source_result_cb, this);
    setOpId(opId);

    g_object_unref(options);
    g_list_free(keys);

    return opId != 0;
}

QString GriloBrowse::source() const
{
    return d->m_source;
}

void GriloBrowse::setSource(const QString &source)
{
    if (d->m_source != source) {
        d->m_source = source;
        Q_EMIT sourceChanged();
        Q_EMIT slowKeysChanged();
        Q_EMIT supportedKeysChanged();
    }
}

QString GriloBrowse::baseMedia() const
{
    return d->m_baseMedia;
}

void GriloBrowse::setBaseMedia(const QString &media)
{
    if (d->m_baseMedia == media) {
        return;
    }

    delete d->m_media;
    d->m_media = nullptr;
    d->m_baseMedia = media;

    Q_EMIT baseMediaChanged();
}

QVariantList GriloBrowse::supportedKeys() const
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

QVariantList GriloBrowse::slowKeys() const
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

bool GriloBrowse::isAvailable() const
{
    GriloRegistry *registry = getGriloRegistry();

    return registry && !d->m_source.isEmpty()
            && registry->availableSources().indexOf(d->m_source) != -1;
}

void GriloBrowse::availableSourcesChanged()
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

GrlMedia *GriloBrowse::rootMedia()
{
    if (d->m_media) {
        return d->m_media->media();
    } else if (d->m_baseMedia.isEmpty()) {
        return NULL;
    }

    GrlMedia *m = grl_media_unserialize(d->m_baseMedia.toUtf8().constData());
    if (m) {
        d->m_media = new GriloMedia(m);
        return d->m_media->media();
    } else {
        qDebug() << "Failed to create GrlMedia from" << d->m_baseMedia;
    }

    return NULL;
}
