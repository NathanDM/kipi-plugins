/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2005-12-19
 * Description : a plugin to export image collections using SimpleViewer.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef SIMPLEVIEWEREXPORT_H
#define SIMPLEVIEWEREXPORT_H

// QT includes

#include <QObject>
#include <QList>
#include <QImage>
#include <QDomDocument>
#include <QDomElement>

// KDE includes.

#include <kurl.h>
#include <kzip.h>

// LibKExiv2 includes.

#include <libkexiv2/kexiv2.h>

// KIPI includes

#include <libkipi/interface.h>

namespace KIPISimpleViewerExportPlugin
{
class SimpleViewerExportPriv;

class SimpleViewerExport : public QObject
{
    Q_OBJECT

public:

    static void run(KIPI::Interface *interface, QObject *parent=0);

private:

    SimpleViewerExport(KIPI::Interface *interface, QObject *parent=0);
    ~SimpleViewerExport();

    bool configure();
    void startExport();

    /**
     * Creates the standard simpleviewer directories
     *
     * @return true=ok
     */
    bool createExportDirectories();

    /**
     * Creates the images and thumbnails
     *
     * @return true=ok
     */
    bool exportImages();

    /**
     * Creates a simpleviewer thumbnail from images
     *
     * @param image the original images
     * @param thumbnail the created thumbnail
     *
     * @return true=ok
     */
    bool createThumbnail(const QImage &image, QImage &thumbnail);

    /**
     * Resizes the image for the gallery
     *
     * @param image the original images
     * @param thumbnail the created thumbnail
     *
     * @return true=ok
     */
    bool resizeImage(const QImage &image, int maxSize, QImage &resized);

    /**
     * Adds an image to the simpleviewer config file
     *
     * @param kurl path to the images
     */
    void cfgAddImage(QDomDocument &xmlDoc, QDomElement &galleryElem, const KUrl &kurl);

    /**
     * Creates the index.html file
     */
    bool createIndex();

    /**
     * Copies simpleviewers files into the export directory
     */
    bool copySimpleViewer();

    /**
     * Is the SimpleViewer flash installed?
     */
    bool checkSimpleViewer() const;

    /**
     * Installs the SimpleViewer files for the later export 
     * on the users machine
     */
    bool installSimpleViewer();

    bool upload();

    bool unzip(const QString &url);

    bool openArchive(KZip &zip);

    bool extractArchive(KZip &zip);

    bool extractFile(const KArchiveEntry *entry);

public slots:

    void slotProcess();
    void slotCancel();

private:

    SimpleViewerExportPriv *d;
};

} // namespace KIPISimpleViewerExportPlugin

#endif /* SIMPLEVIEWEREXPORT_H */
