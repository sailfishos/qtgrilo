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

#include "griloregistry.h"
#include <QDebug>

class GriloRegistryPrivate
{
public:
    GrlRegistry *m_registry = nullptr;
    QStringList m_sources;
    QString m_configurationFile;
};

GriloRegistry::GriloRegistry(QObject *parent)
    : QObject(parent)
    , d(new GriloRegistryPrivate)
{
    grl_init(0, 0);

    d->m_registry = grl_registry_get_default();

    g_signal_connect(d->m_registry, "source-added", G_CALLBACK(grilo_source_added), this);
    g_signal_connect(d->m_registry, "source-removed", G_CALLBACK(grilo_source_removed), this);

    GList *sources = grl_registry_get_sources(d->m_registry, FALSE);
    g_list_foreach(sources, connect_source, this);
    g_list_free(sources);
}

GriloRegistry::~GriloRegistry()
{
    Q_FOREACH (const QString &source, d->m_sources) {
        GrlSource *src = lookupSource(source);
        if (src) {
            g_signal_handlers_disconnect_by_data(src, this);
        }
    }
    g_signal_handlers_disconnect_by_data(d->m_registry, this);
    d->m_registry = 0;
    delete d;
}

QStringList GriloRegistry::availableSources()
{
    return d->m_sources;
}

bool GriloRegistry::loadAll()
{
    // TODO: error reporting
    return grl_registry_load_all_plugins(d->m_registry, TRUE, NULL) == TRUE;
}

bool GriloRegistry::loadPluginById(const QString &pluginId)
{
    if (!d->m_registry) {
        qCritical() << "No registry object!!!";
        return false;
    }

    QByteArray id = pluginId.toLocal8Bit();

    // Let's try to get the plugin first so we avoid a nasty warning ...
    if (grl_registry_lookup_plugin(d->m_registry, id.constData())) {
        return true;
    }

    // TODO: error reporting
    grl_registry_load_all_plugins(d->m_registry, FALSE, NULL);
    return grl_registry_activate_plugin_by_id(d->m_registry, id.constData(), NULL) == TRUE;
}

QString GriloRegistry::configurationFile() const
{
    return d->m_configurationFile;
}

void GriloRegistry::setConfigurationFile(const QString &file)
{
    if (d->m_configurationFile != file) {
        d->m_configurationFile = file;
        Q_EMIT configurationFileChanged();

        loadConfigurationFile();
    }
}

void GriloRegistry::loadConfigurationFile()
{
    if (!d->m_configurationFile.isEmpty()) {
        grl_registry_add_config_from_file(d->m_registry,
                                          d->m_configurationFile.toLocal8Bit().constData(),
                                          NULL);
    }
}

void GriloRegistry::connect_source(gpointer data, gpointer user_data)
{
    GriloRegistry *reg = static_cast<GriloRegistry *>(user_data);
    grilo_source_added(reg->d->m_registry, static_cast<GrlSource *>(data), user_data);
}

void GriloRegistry::grilo_source_added(GrlRegistry *registry, GrlSource *src,
                                       gpointer user_data)
{
    Q_UNUSED(registry);

    GriloRegistry *reg = static_cast<GriloRegistry *>(user_data);

    const char *id = grl_source_get_id(src);

    if (reg->d->m_sources.indexOf(id) == -1) {
        reg->d->m_sources << id;
        g_signal_connect(src, "content-changed", G_CALLBACK(grilo_content_changed_cb), reg);
        grl_source_notify_change_start(src, 0);

        Q_EMIT reg->availableSourcesChanged();
    }
}

void GriloRegistry::grilo_source_removed(GrlRegistry *registry, GrlSource *src,
                                         gpointer user_data)
{
    Q_UNUSED(registry);

    GriloRegistry *reg = static_cast<GriloRegistry *>(user_data);

    const char *id = grl_source_get_id(src);

    if (int index = reg->d->m_sources.indexOf(id) != -1) {
        reg->d->m_sources.removeAt(index);
        g_signal_handlers_disconnect_by_data(src, reg);
        Q_EMIT reg->availableSourcesChanged();
    }
}

void GriloRegistry::grilo_content_changed_cb(GrlSource *source, GPtrArray *changed_media,
                                             GrlSourceChangeType change_type, gboolean location_unknown,
                                             gpointer user_data)
{
    Q_UNUSED(location_unknown);
    GriloRegistry *reg = static_cast<GriloRegistry *>(user_data);

    const char *id = grl_source_get_id(source);
    Q_EMIT reg->contentChanged(id, change_type, changed_media);
}

GrlSource *GriloRegistry::lookupSource(const QString &id)
{
    if (!d->m_registry) {
        qCritical() << "No registry object!!!";
        return 0;
    }

    return grl_registry_lookup_source(d->m_registry, id.toUtf8().constData());
}
