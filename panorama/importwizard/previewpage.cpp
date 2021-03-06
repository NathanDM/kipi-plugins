/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : a plugin to create panorama by fusion of several images.
 *
 * Copyright (C) 2011-2012 by Benjamin Girault <benjamin dot girault at gmail dot com>
 * Copyright (C) 2011-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewpage.moc"

// Qt includes

#include <QLabel>
#include <QMutex>
#include <QMutexLocker>
#include <QTextDocument>

// KDE includes

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kvbox.h>
#include <klocale.h>
#include <kdialog.h>
#include <kdebug.h>

// Local includes

#include "kppreviewmanager.h"
#include "kpbatchprogressdialog.h"
#include "manager.h"
#include "enblendbinary.h"
#include "makebinary.h"
#include "nonabinary.h"
#include "pto2mkbinary.h"

namespace KIPIPanoramaPlugin
{

struct PreviewPage::PreviewPagePriv
{
    PreviewPagePriv(Manager* const m)
        : title(0), 
          previewWidget(0), 
          previewBusy(false), 
          stitchingBusy(false),
          postProcessing(0), 
          canceled(false), 
          mngr(m)
    {}

    QLabel*                title;

    KPPreviewManager*      previewWidget;
    bool                   previewBusy;
    bool                   stitchingBusy;
    KPBatchProgressWidget* postProcessing;
    int                    curProgress, totalProgress;
    QMutex                 actionMutex;      // This is a precaution in case the user does a back / next action at the wrong moment
    bool                   canceled;

    QString                output;

    Manager*               mngr;
};

PreviewPage::PreviewPage(Manager* const mngr, KAssistantDialog* const dlg)
        : KPWizardPage(dlg, i18n("<b>Preview and Post-Processing</b>")),
          d(new PreviewPagePriv(mngr))
{
    KVBox* vbox       = new KVBox(this);
    d->title          = new QLabel(vbox);
    d->title->setOpenExternalLinks(true);
    d->title->setWordWrap(true);

    d->previewWidget  = new KPPreviewManager(vbox);
    d->previewWidget->setButtonText(i18n("Details..."));
    d->previewWidget->show();

    d->postProcessing = new KPBatchProgressWidget(vbox);
    d->postProcessing->hide();

    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(KDialog::spacingHint());

    setPageWidget(vbox);

    QPixmap leftPix = KStandardDirs::locate("data", "kipiplugin_panorama/pics/assistant-hugin.png");
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));

    connect(d->mngr->thread(), SIGNAL(starting(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

    connect(d->mngr->thread(), SIGNAL(stepFinished(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));

    connect(d->mngr->thread(), SIGNAL(finished(KIPIPanoramaPlugin::ActionData)),
            this, SLOT(slotAction(KIPIPanoramaPlugin::ActionData)));
}

PreviewPage::~PreviewPage()
{
    delete d;
}

bool PreviewPage::cancel()
{
    d->canceled = true;

    d->mngr->thread()->cancel();

    QMutexLocker lock(&d->actionMutex);
    if (d->previewBusy)
    {
        d->previewBusy = false;
        d->previewWidget->setBusy(false);
        d->previewWidget->setText(i18n("Preview Processing Cancelled."));
        return true;
    }
    else if (d->stitchingBusy)
    {
        d->stitchingBusy = false;
        lock.unlock();
        resetPage();
        return false;
    }
    return true;
}

void PreviewPage::computePreview()
{
    // Cancel any stitching being processed
    if (d->stitchingBusy)
    {
        cancel();
    }
    d->mngr->thread()->finish();

    QMutexLocker lock(&d->actionMutex);
    d->canceled = false;

    d->previewWidget->setBusy(true, i18n("Processing Panorama Preview..."));
    d->previewBusy = true;

    d->mngr->thread()->generatePanoramaPreview(d->mngr->autoOptimiseUrl(),
                                               d->mngr->previewUrl(),
                                               d->mngr->preProcessedMap(),
                                               d->mngr->makeBinary().path(),
                                               d->mngr->pto2MkBinary().path(),
                                               d->mngr->enblendBinary().path(),
                                               d->mngr->nonaBinary().path());
    if (!d->mngr->thread()->isRunning())
        d->mngr->thread()->start();
}

void PreviewPage::startStitching()
{
    // Cancel any preview being processed
    if (d->previewBusy)
    {
        cancel();
        d->mngr->thread()->finish();
    }

    QMutexLocker lock(&d->actionMutex);
    d->canceled = false;

    d->stitchingBusy = true;
    d->curProgress   = 0;
    d->totalProgress = d->mngr->preProcessedMap().size() + 1;
    d->previewWidget->hide();
    d->title->setText(i18n("<qt>"
                           "<p><h1>Panorama Post-Processing</h1></p>"
                           "</qt>"));

    d->postProcessing->reset();
    d->postProcessing->setTotal(d->totalProgress);
    d->postProcessing->show();

    d->mngr->thread()->compileProject(d->mngr->autoOptimiseUrl(),
                                      d->mngr->panoUrl(),
                                      d->mngr->preProcessedMap(),
                                      d->mngr->format(),
                                      d->mngr->makeBinary().path(),
                                      d->mngr->pto2MkBinary().path(),
                                      d->mngr->enblendBinary().path(),
                                      d->mngr->nonaBinary().path());
    if (!d->mngr->thread()->isRunning())
        d->mngr->thread()->start();
}

void PreviewPage::resetPage()
{
    d->title->setText(i18n("<qt>"
                           "<p><h1>Panorama Preview</h1></p>"
                           "<p>Pressing the <i>Next</i> button launches the final "
                           "stitching process.</p>"
                           "</qt>"));
    d->postProcessing->hide();
    d->previewWidget->show();
    computePreview();
}

void PreviewPage::slotAction(const KIPIPanoramaPlugin::ActionData& ad)
{
    QString text;

    QMutexLocker lock(&d->actionMutex);
    if (!ad.starting)           // Something is complete...
    {
        if (!ad.success)        // Something is failed...
        {
            if (d->canceled)    // In that case, the error is expected
            {
                return;
            }
            switch (ad.action)
            {
                case CREATEPREVIEWPTO:
                case NONAFILEPREVIEW:
                case STITCHPREVIEW:
                {
                    if (!d->previewBusy)
                    {
                        return;
                    }
                    d->output = ad.message;
                    d->previewWidget->setBusy(false);
                    d->previewBusy = false;
                    kDebug() << "Preview compilation failed: " << ad.message;
                    QString errorString(i18n("<qt><h2><b>Error</b></h2><p>%1</p></qt>", Qt::escape(ad.message)));
                    errorString.replace('\n', "</p><p>");
                    d->previewWidget->setText(errorString);
                    d->previewWidget->setSelectionAreaPossible(false);

                    emit signalPreviewStitchingFinished(false);

                    break;
                }
                case NONAFILE:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }
                    d->stitchingBusy = false;
                    QString message = i18n("Processing file %1 / %2: %3",
                                           QString::number(ad.id + 1),
                                           QString::number(d->totalProgress - 1),
                                           ad.message
                                          );
                    kDebug() << "Nona call failed for file #" << ad.id;
                    d->postProcessing->addedAction(message, ErrorMessage);
                    emit signalStitchingFinished(false);
                    break;
                }
                case STITCH:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }
                    d->stitchingBusy = false;
                    d->postProcessing->addedAction(i18n("Panorama compilation: %1", Qt::escape(ad.message)), ErrorMessage);
                    kDebug() << "Enblend call failed";
                    emit signalStitchingFinished(false);
                    break;
                }
                default:
                {
                    kWarning() << "Unknown action " << ad.action;
                    break;
                }
            }
        }
        else                    // Something is done...
        {
            switch (ad.action)
            {
                case CREATEPREVIEWPTO:
                case NONAFILEPREVIEW:
                {
                    // Nothing to do yet, a step is finished, that's all
                    break;
                }
                case STITCHPREVIEW:
                {
                    if (!d->previewBusy)
                    {
                        return;
                    }

                    kDebug() << "Preview Stitching finished";
                    d->previewBusy = false;

                    d->previewWidget->load(d->mngr->previewUrl().toLocalFile(), true);
                    //     d->previewWidget->setSelectionAreaPossible(true);
                    kDebug() << "Preview URL: " << d->mngr->previewUrl();

                    emit signalPreviewStitchingFinished(true);

                    break;
                }
                case NONAFILE:
                {
                    QString message = i18n("Processing file %1 / %2", QString::number(ad.id + 1), QString::number(d->totalProgress - 1));
                    d->postProcessing->addedAction(message, SuccessMessage);
                    d->curProgress++;
                    d->postProcessing->setProgress(d->curProgress, d->totalProgress);
                    kDebug() << "Nona URL #" << ad.id;
                    break;
                }
                case STITCH:
                {
                    if (!d->stitchingBusy)
                    {
                        return;
                    }
                    d->stitchingBusy = false;
                    d->postProcessing->addedAction(i18n("Panorama compilation"), SuccessMessage);
                    d->curProgress++;
                    d->postProcessing->setProgress(d->curProgress, d->totalProgress);
                    d->postProcessing->hide();
                    kDebug() << "Panorama stitched";
                    emit signalStitchingFinished(true);
                    break;
                }
                default:
                {
                    kWarning() << "Unknown action " << ad.action;
                    break;
                }
            }
        }
    }
    else           // Some step is started...
    {
        switch (ad.action)
        {
            case CREATEPREVIEWPTO:
            case NONAFILEPREVIEW:
            case STITCHPREVIEW:
            {
                // Nothing to do...
                break;
            }
            case NONAFILE:
            {
                QString message = i18n("Processing file %1 / %2", QString::number(ad.id + 1), QString::number(d->totalProgress - 1));
                d->postProcessing->addedAction(message, StartingMessage);
                break;
            }
            case STITCH:
            {
                d->postProcessing->addedAction(i18n("Panorama compilation"), StartingMessage);
                break;
            }
            default:
            {
                kWarning() << "Unknown starting action " << ad.action;
                break;
            }
        }
    }
}

}   // namespace KIPIPanoramaPlugin
