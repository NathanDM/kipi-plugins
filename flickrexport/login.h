/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a kipi plugin to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef FLICKRLOGIN_H
#define FLICKRLOGIN_H

// Qt includes

#include <QLabel>
#include <QString>

// KDE includes

#include <kdialog.h>

class QLabel;
class KLineEdit;

namespace KIPIFlickrExportPlugin
{

class FlickrLogin : public KDialog
{
    Q_OBJECT

public:

    FlickrLogin(QWidget* parent, const QString& header,
                const QString& _name=QString(),
                const QString& _passwd=QString());
    ~FlickrLogin();

    QString name()     const;
    QString password() const;

private:

    QLabel*    m_headerLabel;
    KLineEdit* m_nameEdit;
    KLineEdit* m_passwdEdit;
};

} // namespace KIPIFlickrExportPlugin

#endif // FLICKRLOGIN_H
