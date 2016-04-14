// -*- c++ -*-

/*!
 *
 * Copyright (C) 2014 Jolla Ltd.
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

#ifndef DECLARATIVE_GRILO_MODEL_H
#define DECLARATIVE_GRILO_MODEL_H

#include <GriloModel>

class DeclarativeGriloModel : public GriloModel
{
    Q_OBJECT

public:
    DeclarativeGriloModel(QObject *parent = 0);
    ~DeclarativeGriloModel();

    Q_INVOKABLE QObject *get(int rowIndex) const;
};

#endif /* DECLARATIVE_GRILO_MODEL_H */
