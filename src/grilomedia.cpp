/*!
 *
 * Copyright (C) 2012-2014 Jolla Ltd.
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

#include "grilomedia.h"

#include <QDebug>

GriloMedia::GriloMedia(GrlMedia *media, QObject *parent)
    : QObject(parent), m_media(media)
{
}

GriloMedia::~GriloMedia()
{
    g_object_unref(m_media);
    m_media = 0;
}

GrlMedia *GriloMedia::media()
{
    return m_media;
}

void GriloMedia::setMedia(GrlMedia *media)
{
    if (m_media != media) {
        g_object_unref(m_media);
        m_media = media;
    }
}

QVariant GriloMedia::get(const QString &keyId) const
{
    // There is a GriloRegistry Qt object, but it is not smart to add a
    // dependency to it here since GrlRegistry is, actually, a
    // singleton.
    GrlKeyID actualKey = grl_registry_lookup_metadata_key(grl_registry_get_default(),
                                                          keyId.toUtf8().constData());

    if (GRL_METADATA_KEY_INVALID == actualKey) {
        qWarning() << "Grilo key doesn't exist in the registry:" << keyId;
        return QVariant();
    }

    return get(actualKey);
}

QVariant GriloMedia::get(const quint32 keyId) const
{
    const GValue *gValue = grl_data_get(GRL_DATA(m_media), keyId);

    return convertValue(gValue);
}

QString GriloMedia::serialize()
{
    QString result;
    gchar *str = grl_media_serialize_extended(m_media, GRL_MEDIA_SERIALIZE_FULL, NULL);

    if (str) {
        result = QString::fromUtf8(str);
        g_free(str);
    }

    return result;
}

QString GriloMedia::id() const
{
    return QString::fromUtf8(grl_media_get_id(m_media));
}

QString GriloMedia::title() const
{
    return QString::fromUtf8(grl_media_get_title(m_media));
}

QUrl GriloMedia::url() const
{
    QUrl url = QUrl::fromEncoded(QByteArray(grl_media_get_url(m_media)));

    return url;
}

int GriloMedia::duration() const
{
    return grl_media_get_duration(m_media);
}

bool GriloMedia::isContainer() const
{
    return grl_media_is_container(m_media) == TRUE;
}

QString GriloMedia::author() const
{
    return QString::fromUtf8(grl_media_get_author(m_media));
}

QString GriloMedia::album() const
{
    return QString::fromUtf8(grl_media_get_album(m_media));
}

QString GriloMedia::artist() const
{
    return QString::fromUtf8(grl_media_get_artist(m_media));
}

QString GriloMedia::genre() const
{
    return QString::fromUtf8(grl_media_get_genre(m_media));
}

QUrl GriloMedia::thumbnail() const
{
    return QUrl(grl_media_get_thumbnail(m_media));
}

int GriloMedia::year() const
{
    return g_date_time_get_year(grl_media_get_creation_date(m_media));
 }

int GriloMedia::trackNumber() const
{
    return grl_media_get_track_number(m_media);
}

int GriloMedia::childCount() const
{
    return grl_media_get_childcount(m_media);
}

QString GriloMedia::mimeType() const
{
    return QString::fromUtf8(grl_media_get_mime(m_media));
}

QDateTime GriloMedia::modificationDate() const
{
    GDateTime *dateTime = grl_media_get_modification_date(m_media);

    if (dateTime) {
        return QDateTime::fromTime_t(g_date_time_to_unix(dateTime));
    }

    return QDateTime();
}

int GriloMedia::height() const
{
    return grl_media_get_height(m_media);
}

int GriloMedia::orientation() const
{
    return grl_media_get_orientation(m_media);
}

int GriloMedia::width() const
{
    return grl_media_get_width(m_media);
}

QVariant GriloMedia::convertValue(const GValue *value) const
{
    if (!value) {
        return QVariant();
    }

    switch (G_VALUE_TYPE(value)) {
    case G_TYPE_BOOLEAN:
        return QVariant::fromValue(static_cast<bool>(g_value_get_boolean(value)));
    case G_TYPE_BOXED: {
        GByteArray *array;

        array = static_cast<GByteArray *>(g_value_get_boxed(value));
        const char *arrayData = reinterpret_cast<char *>(array->data);
        return QVariant::fromValue(QByteArray::fromRawData(arrayData, array->len));
    }
    case G_TYPE_DOUBLE:
        return QVariant::fromValue(g_value_get_double(value));
    case G_TYPE_ENUM:
        return QVariant::fromValue(g_value_get_enum(value));
    case G_TYPE_FLAGS:
        return QVariant::fromValue(g_value_get_flags(value));
    case G_TYPE_FLOAT:
        return QVariant::fromValue(g_value_get_float(value));
    case G_TYPE_INT:
        return QVariant::fromValue(g_value_get_int(value));
    case G_TYPE_INT64:
        return QVariant::fromValue(g_value_get_int64(value));
    case G_TYPE_LONG:
        return QVariant::fromValue(g_value_get_long(value));
    case G_TYPE_CHAR:
        return QVariant::fromValue(QChar(g_value_get_schar(value)));
    case G_TYPE_STRING:
        return QVariant::fromValue(QString::fromUtf8(g_value_get_string(value)));
    case G_TYPE_UCHAR:
        return QVariant::fromValue(QChar(g_value_get_uchar(value)));
    case G_TYPE_UINT:
        return QVariant::fromValue(g_value_get_uint(value));
    case G_TYPE_UINT64:
        return QVariant::fromValue(g_value_get_uint64(value));
    case G_TYPE_ULONG:
        return QVariant::fromValue(g_value_get_ulong(value));
    }

    // Non constants; they cannot be part of the switch expression
    if (G_VALUE_HOLDS(value, G_TYPE_DATE_TIME)) {
        GDateTime *dateTime = static_cast<GDateTime *>(g_value_get_boxed(value));

        if (dateTime) {
            return QVariant::fromValue(QDateTime::fromTime_t(g_date_time_to_unix(dateTime)));
        }

        return QVariant::fromValue(QDateTime());
    } else if (G_UNLIKELY(G_VALUE_HOLDS_GTYPE(value))) {
        return QVariant::fromValue(g_value_get_gtype(value));
    }

    qWarning() << "GValue not converted to QVariant.";

    return QVariant();
}

