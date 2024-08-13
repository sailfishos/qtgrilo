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

#include "grilodatasource.h"
#include "grilomedia.h"
#include "grilomodel.h"
#include "griloregistry.h"

#include <QDebug>
#include <QTimerEvent>

static void fill_key_id(gpointer data, gpointer user_data)
{
    QVariantList *varList = static_cast<QVariantList *>(user_data);
    varList->append(GriloDataSource::MetadataKeys(GRLPOINTER_TO_KEYID(data)));
}


class GriloDataSourcePrivate {
public:
    GriloDataSourcePrivate();

    guint m_opId;
    GriloRegistry *m_registry;

    int m_count;
    int m_skip;
    int m_insertIndex;
    QVariantList m_metadataKeys;
    QVariantList m_typeFilter;

    bool m_updateScheduled;
    QBasicTimer m_updateTimer;
    QList<GriloMedia *> m_media;
    QList<GriloModel *> m_models;
    QHash<QString, GriloMedia *> m_hash;
    bool m_fetching;
};

GriloDataSourcePrivate::GriloDataSourcePrivate()
    : m_opId(0)
    , m_registry(nullptr)
    , m_count(0)
    , m_skip(0)
    , m_insertIndex(0)
    , m_updateScheduled(false)
    , m_fetching(false)
{
    m_metadataKeys << GriloDataSource::Title;
    m_typeFilter << GriloDataSource::None;
}

GriloDataSource::GriloDataSource(QObject *parent)
    : QObject(parent)
    , d(new GriloDataSourcePrivate)
{
}

GriloDataSource::~GriloDataSource()
{
    cancelRefresh();
    d->m_models.clear();
    delete d;
}

const QList<GriloMedia *> *GriloDataSource::media() const
{
    return &d->m_media;
}

void GriloDataSource::addModel(GriloModel *model)
{
    if (d->m_models.indexOf(model) == -1) {
        d->m_models.append(model);
    }
}

void GriloDataSource::removeModel(GriloModel *model)
{
    if (int index = d->m_models.indexOf(model) != -1) {
        d->m_models.removeAt(index);
    }
}

void GriloDataSource::prefill(GriloModel *model)
{
    if (d->m_media.isEmpty()) {
        return;
    }

    model->beginInsertRows(QModelIndex(), 0, d->m_media.size() - 1);
    model->endInsertRows();
}

void GriloDataSource::addMedia(GrlMedia *media)
{
    GriloMedia *wrappedMedia = 0;

    if (d->m_insertIndex < d->m_media.count()) {
        wrappedMedia = d->m_hash.value(QString::fromUtf8(grl_media_get_id(media)), 0);
    }

    if (wrappedMedia) {
        // If the media was already queried by a previous fetch update its position and refresh
        // the data instead of creating another item.
        bool dataChanged = false;
        int index = d->m_media.indexOf(wrappedMedia);
        if (index == d->m_insertIndex) {
            dataChanged = true;
        } else if (index != -1) {
            dataChanged = true;
            Q_FOREACH (GriloModel *model, d->m_models) {
                model->beginMoveRows(QModelIndex(), index, index, QModelIndex(), d->m_insertIndex);
            }
            d->m_media.move(index, d->m_insertIndex);
            Q_FOREACH (GriloModel *model, d->m_models) {
                model->endMoveRows();
            }
        }

        if (dataChanged) {
            wrappedMedia->setMedia(media);
            Q_FOREACH (GriloModel *model, d->m_models) {
                QModelIndex modelIndex = model->index(d->m_insertIndex, 0);
                model->dataChanged(modelIndex, modelIndex);
            }
            ++d->m_insertIndex;
            return;
        }
    }
    wrappedMedia = new GriloMedia(media);
    QString id = wrappedMedia->id();

    if (!id.isEmpty() && d->m_hash.contains(id)) {
        qWarning() << "Duplicate id detected on qtgrilo model source, ignored to keep model sane. Id:" << id;
        delete wrappedMedia;
        return;
    }

    Q_FOREACH (GriloModel *model, d->m_models) {
        model->beginInsertRows(QModelIndex(), d->m_insertIndex, d->m_insertIndex);
    }

    d->m_media.insert(d->m_insertIndex, wrappedMedia);
    ++d->m_insertIndex;

    if (!id.isEmpty()) {
        d->m_hash.insert(id, wrappedMedia);
    }

    Q_FOREACH (GriloModel *model, d->m_models) {
        model->endInsertRows();
    }
}

void GriloDataSource::removeMedia(GrlMedia *media)
{
    QString id = QString::fromUtf8(grl_media_get_id(media));

    if (id.isEmpty() || !d->m_hash.contains(id)) {
        // We really cannot do much.
        return;
    }

    GriloMedia *wrapper = d->m_hash[id];
    int index = d->m_media.indexOf(wrapper);
    if (index < d->m_insertIndex) {
        --d->m_insertIndex;
    }

    // remove from models:
    Q_FOREACH (GriloModel *model, d->m_models) {
        model->beginRemoveRows(QModelIndex(), index, index);
    }

    // remove from hash
    d->m_hash.take(id);

    // remove from list
    d->m_media.takeAt(index);

    // destroy
    wrapper->deleteLater();

    Q_FOREACH (GriloModel *model, d->m_models) {
        model->endRemoveRows();
    }
}

void GriloDataSource::clearMedia()
{
    if (d->m_media.isEmpty()) {
        return;
    }

    int size = d->m_media.size();

    Q_FOREACH (GriloModel *model, d->m_models) {
        model->beginRemoveRows(QModelIndex(), 0, size - 1);
    }

    qDeleteAll(d->m_media);
    d->m_media.clear();
    d->m_hash.clear();

    Q_FOREACH (GriloModel *model, d->m_models) {
        model->endRemoveRows();
    }
}

GriloRegistry *GriloDataSource::registry() const
{
    return d->m_registry;
}

void GriloDataSource::setRegistry(GriloRegistry *registry)
{
    // Registry change is not allowed for now.

    if (!d->m_registry && registry != d->m_registry) {

        d->m_registry = registry;

        QObject::connect(d->m_registry, SIGNAL(availableSourcesChanged()),
                         this, SLOT(availableSourcesChanged()));
        QObject::connect(d->m_registry, SIGNAL(contentChanged(QString, GrlSourceChangeType, GPtrArray *)),
                         this, SLOT(contentChanged(QString, GrlSourceChangeType, GPtrArray *)));

        Q_EMIT registryChanged();
    }
}

int GriloDataSource::count() const
{
    return d->m_count;
}

void GriloDataSource::setCount(int count)
{
    if (d->m_count != count) {
        d->m_count = count;
        Q_EMIT countChanged();
    }
}

int GriloDataSource::skip() const
{
    return d->m_skip;
}

void GriloDataSource::setSkip(int skip)
{
    if (d->m_skip != skip) {
        d->m_skip = skip;
        Q_EMIT skipChanged();
    }
}

QVariantList GriloDataSource::metadataKeys() const
{
    return d->m_metadataKeys;
}

void GriloDataSource::setMetadataKeys(const QVariantList &keys)
{
    if (d->m_metadataKeys != keys) {
        d->m_metadataKeys = keys;
        Q_EMIT metadataKeysChanged();
    }
}

QVariantList GriloDataSource::typeFilter() const
{
    return d->m_typeFilter;
}

void GriloDataSource::setTypeFilter(const QVariantList &filter)
{
    if (d->m_typeFilter != filter) {
        d->m_typeFilter = filter;
        Q_EMIT typeFilterChanged();
    }
}

bool GriloDataSource::fetching() const
{
    return d->m_fetching;
}

void GriloDataSource::setFetching(bool fetching)
{
    if (fetching != d->m_fetching) {
        d->m_fetching = fetching;
        Q_EMIT fetchingChanged();
    }
}

GrlOperationOptions *GriloDataSource::operationOptions(GrlSource *src, const OperationType &type)
{
    GrlCaps *caps = NULL;

    if (src) {
        caps = grl_source_get_caps(src, (GrlSupportedOps)type);
    }

    GrlOperationOptions *options = grl_operation_options_new(caps);

    grl_operation_options_set_resolution_flags(options, GRL_RESOLVE_IDLE_RELAY); // TODO: hardcoded
    grl_operation_options_set_skip(options, d->m_skip);

    if (d->m_count != 0) {
        grl_operation_options_set_count(options, d->m_count);
    }

    int typeFilter = 0;
    Q_FOREACH (const QVariant &var, d->m_typeFilter) {
        if (var.canConvert<int>()) {
            typeFilter |= var.toInt();
        }
    }

    grl_operation_options_set_type_filter(options, (GrlTypeFilter)typeFilter);

    return options;
}

GList *GriloDataSource::keysAsList()
{
    // TODO: Check why using  grl_metadata_key_list_new() produces a symbol error.
    GList *keys = NULL;

    Q_FOREACH (const QVariant &var, d->m_metadataKeys) {
        if (var.canConvert<int>()) {
            keys = g_list_append(keys, GRLKEYID_TO_POINTER(var.toInt()));
        }
    }

    return keys;
}

void GriloDataSource::cancelRefresh()
{
    if (d->m_opId != 0) {
        grl_operation_cancel(d->m_opId);
        d->m_opId = 0;
    }

    d->m_insertIndex = 0;
    d->m_updateScheduled = false;
    d->m_updateTimer.stop();
}

void GriloDataSource::grilo_source_result_cb(GrlSource *source, guint op_id,
                                             GrlMedia *media, guint remaining,
                                             gpointer user_data, const GError *error)
{
    Q_UNUSED(source)

    // We get an error if the operation has been canceled:
    if (error) {
        if (error->domain != GRL_CORE_ERROR || error->code != GRL_CORE_ERROR_OPERATION_CANCELLED) {
            // TODO: error reporting?
            qCritical() << "Operation failed" << error->message;
        } else {
            // Cancelled operation notification. Nothing else to be done.
            return;
        }
    }

    GriloDataSource *that = static_cast<GriloDataSource *>(user_data);

    if (that->d->m_opId != op_id) {
        qWarning() << "Got Op Id result" << op_id
                   << "but Op Id" << that->d->m_opId << "was expected.";

        if (media) {
            g_object_unref(media);
        }

        return;
    }

    if (media) {
        that->addMedia(media);
    }

    if (remaining == 0) {
        that->d->m_opId = 0;

        if (that->d->m_updateScheduled) {
            that->d->m_updateTimer.start(100, that);
        }

        // If there are items from a previous fetch still remaining remove them.
        if (that->d->m_insertIndex < that->d->m_media.count()) {
            Q_FOREACH (GriloModel *model, that->d->m_models) {
                model->beginRemoveRows(QModelIndex(), that->d->m_insertIndex, that->d->m_media.count() - 1);
            }
            while (that->d->m_media.count() > that->d->m_insertIndex) {
                GriloMedia *media = that->d->m_media.takeLast();
                that->d->m_hash.remove(media->id());
                delete media;
            }
            Q_FOREACH (GriloModel *model, that->d->m_models) {
                model->endRemoveRows();
            }
        }
        that->setFetching(false);
        Q_EMIT that->finished();
    }
}

void GriloDataSource::contentChanged(const QString &source, GrlSourceChangeType change_type,
                                     GPtrArray *changed_media)
{
    Q_UNUSED(source)
    Q_UNUSED(change_type)
    Q_UNUSED(changed_media)
}

void GriloDataSource::updateContent(GrlSourceChangeType change_type, GPtrArray *changed_media)
{
    switch (change_type) {
    case GRL_CONTENT_CHANGED:
    case GRL_CONTENT_ADDED:
        if (!d->m_updateScheduled) {
            d->m_updateScheduled = true;
            if (d->m_opId == 0) {
                d->m_updateTimer.start(100, this);
            }
        }
        break;
    case GRL_CONTENT_REMOVED:
        for (uint i = 0; i < changed_media->len; ++i) {
            removeMedia((GrlMedia *)g_ptr_array_index(changed_media, i));
        }
        break;
    default:
        break;
    }
}

QVariantList GriloDataSource::listToVariantList(const GList *keys) const
{
    QVariantList varList;

    g_list_foreach(const_cast<GList *>(keys), fill_key_id, &varList);

    return varList;
}

void GriloDataSource::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == d->m_updateTimer.timerId()) {
        d->m_updateTimer.stop();
        Q_EMIT contentUpdated();
    }
}

guint GriloDataSource::getOpId() const
{
    return d->m_opId;
}

void GriloDataSource::setOpId(guint id)
{
    d->m_opId = id;
}

GriloRegistry *GriloDataSource::getGriloRegistry() const
{
    return d->m_registry;
}
