/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2007-16-07
 * Description : a kipi plugin to export images to Picasa web service
 *
 * Copyright (C) 2007-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * Copyright (C) 2010 by Jens Mueller <tschenser at gmx dot de>
 * Copyright (C) 2010 by Caulier Gilles <caulier dot gilles at gmail dot com>
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

#ifndef IMGURWIDGET_H
#define IMGURWIDGET_H

// Qt includes

#include <QWidget>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/uploadwidget.h>

// Local includes

#include "imageslist.h"

class QLabel;
class QSpinBox;
class QCheckBox;
class QRadioButton;
class QProgressBar;
class KPushButton;
class KComboBox;

using namespace KIPI;
using namespace KIPIPlugins;

namespace KIPIImgurExportPlugin
{

class ImgurWidget : public QWidget
{
    Q_OBJECT

public:

    ImgurWidget(Interface* iface, bool import);
    ~ImgurWidget();

    ImagesList* imagesList() const;
    QProgressBar* progressBar() const;

Q_SIGNALS:
    void signalUserChangeRequest(bool anonymous);

// private Q_SLOTS:

private:
    QLabel*       m_headerLbl;

//    QCheckBox*    m_resizeChB;

    ImagesList*   m_imgList;
    UploadWidget* m_uploadWidget;

    QProgressBar* m_progressBar;
};

} // namespace KIPIImgurExportPlugin

#endif // IMGURWIDGET_H
