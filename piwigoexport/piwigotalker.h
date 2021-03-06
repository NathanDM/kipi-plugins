/* ============================================================
*
* This file is a part of kipi-plugins project
* http://www.digikam.org
*
* Date        : 2010-02-15
* Description : a plugin to export to a remote Piwigo server.
*
* Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
* Copyright (C) 2006      by Colin Guthrie <kde@colin.guthr.ie>
* Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
* Copyright (C) 2008      by Andrea Diamantini <adjam7 at gmail dot com>
* Copyright (C) 2010      by Frederic Coiffier <frederic dot coiffier at free dot com>
*
* This program is free software; you can redistribute it
* and/or modify it under the terms of the GNU General
* Public License as published by the Free Software Foundation;
* either version 2, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* ============================================================ */

#ifndef PIWIGOTALKER_H
#define PIWIGOTALKER_H

// Qt includes

#include <QObject>
#include <QList>
#include <QDateTime>
#include <QTextStream>
#include <QFile>

// KDE includes

#include <kurl.h>
#include <kio/job.h>

template <class T> class QList;

namespace KIPIPiwigoExportPlugin
{

class GAlbum;
class GPhoto;

class PiwigoTalker : public QObject
{
    Q_OBJECT

public:

    enum State
    {
        GE_LOGIN = 0,
        GE_GETVERSION,
        GE_LISTALBUMS,
        GE_CHECKPHOTOEXIST,
        GE_GETINFO,
        GE_SETINFO,
        // Support for Web API >= 2.4
        GE_ADDPHOTOCHUNK,
        GE_ADDPHOTOSUMMARY,
        // Support for Web API < 2.4
        GE_OLD_ADDPHOTOCHUNK,
        GE_OLD_ADDTHUMB,
        GE_OLD_ADDHQ,
        GE_OLD_ADDPHOTOSUMMARY
    };

    enum
    {
        CHUNK_MAX_SIZE = 500000,
        PIWIGO_VER_2_4 = 24
    };

public:

    PiwigoTalker(QWidget* const parent);
    ~PiwigoTalker();

    static QString getAuthToken()
    {
        return s_authToken;
    };

    bool loggedIn() const;

    void login(const KUrl& url, const QString& name, const QString& passwd);
    void listAlbums();
    void listPhotos(const QString& albumName);

    /* TODO Implement this function
    void createAlbum(const QString& parentAlbumName,
                     const QString& albumName,
                     const QString& albumTitle,
                     const QString& albumCaption);*/

    bool addPhoto(int albumId,
                  const QString& photoPath,
                  bool  rescale = false, int maxWidth = 800, int maxHeight = 600, int thumbDim = 128);

    void cancel();

Q_SIGNALS:

    void signalProgressInfo(const QString& msg);
    void signalError(const QString& msg);
    void signalLoginFailed(const QString& msg);
    void signalBusy(bool val);
    void signalAlbums(const QList<GAlbum>& albumList);
    void signalAddPhotoSucceeded();
    void signalAddPhotoFailed(const QString& msg);

private:

    void parseResponseLogin(const QByteArray& data);
    void parseResponseGetVersion(const QByteArray& data);
    void parseResponseListAlbums(const QByteArray& data);
    void parseResponseDoesPhotoExist(const QByteArray& data);
    void parseResponseGetInfo(const QByteArray& data);
    void parseResponseSetInfo(const QByteArray& data);

    void addNextChunk();
    void parseResponseAddPhotoChunk(const QByteArray& data);
    void addPhotoSummary();
    void parseResponseAddPhotoSummary(const QByteArray& data);
    
    // Support for Web API < 2.4
    void parseResponseOldAddPhoto(const QByteArray& data);
    void parseResponseOldAddThumbnail(const QByteArray& data);
    void addHQNextChunk();
    void parseResponseOldAddHQPhoto(const QByteArray& data);
    void addOldPhotoSummary();
    void parseResponseOldAddPhotoSummary(const QByteArray& data);

    QByteArray computeMD5Sum(const QString& filepath);

private Q_SLOTS:

    void slotTalkerData(KIO::Job* job, const QByteArray& data);
    void slotResult(KJob* job);

private:

    QWidget*          m_parent;
    State             m_state;
    QString           m_cookie;
    KUrl              m_url;
    KIO::TransferJob* m_job;
    bool              m_loggedIn;
    QByteArray        m_talker_buffer;
    uint              m_chunkId;
    int               m_version;

    QByteArray        m_md5sum;
    QString           m_path;
    int               m_albumId;
    int               m_photoId;    // Filled when the photo already exist
    QString           m_thumbpath;
    QString           m_hqpath;
    QString           m_comment;    // Synchronized with Piwigo comment
    QString           m_title;      // Synchronized with Piwigo name
    QString           m_author;     // Synchronized with Piwigo author
    QDateTime         m_date;       // Synchronized with Piwigo date

    static QString    s_authToken;
};

} // namespace KIPIPiwigoExportPlugin

#endif /* PIWIGOTALKER_H */
