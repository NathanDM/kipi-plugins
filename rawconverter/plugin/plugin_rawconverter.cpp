/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a kipi plugin to convert Raw file in single
 *               or batch mode.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "plugin_rawconverter.moc"

// C++ includes

#include <cstdlib>

// KDE includes

#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <klibloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwindowsystem.h>

// LibKIPI includes

#include <libkipi/interface.h>
#include <libkipi/imagecollection.h>

// Local includes

#include "kpmetadata.h"
#include "myimagelist.h"
#include "singledialog.h"
#include "batchdialog.h"

namespace KIPIRawConverterPlugin
{

K_PLUGIN_FACTORY( RawConverterFactory, registerPlugin<Plugin_RawConverter>(); )
K_EXPORT_PLUGIN ( RawConverterFactory("kipiplugin_rawconverter") )

Plugin_RawConverter::Plugin_RawConverter(QObject* const parent, const QVariantList&)
    : Plugin(RawConverterFactory::componentData(), parent, "RawConverter")
{
    kDebug(AREA_CODE_LOADING) << "Plugin_RawConverter plugin loaded";
}

void Plugin_RawConverter::setup(QWidget* widget)
{
    m_singleDlg = 0;
    m_batchDlg  = 0;

    Plugin::setup( widget );

    KGlobal::locale()->insertCatalog("libkdcraw");

    m_singleAction = actionCollection()->addAction("raw_converter_single");
    m_singleAction->setText(i18n("RAW Image Converter..."));
    m_singleAction->setIcon(KIcon("rawconverter"));

    connect(m_singleAction, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateSingle()));

    addAction(m_singleAction);

    m_batchAction = actionCollection()->addAction("raw_converter_batch");
    m_batchAction->setText(i18n("Batch RAW Converter..."));
    m_batchAction->setIcon(KIcon("rawconverter"));

    connect(m_batchAction, SIGNAL(triggered(bool)),
            this, SLOT(slotActivateBatch()));

    addAction(m_batchAction);

    Interface* interface = dynamic_cast<Interface*>( parent() );
    if ( !interface )
    {
           kError() << "Kipi interface is null!";
           return;
    }

    connect(interface, SIGNAL(selectionChanged(bool)),
            m_singleAction, SLOT(setEnabled(bool)));

    connect(interface, SIGNAL(currentAlbumChanged(bool)),
            m_batchAction, SLOT(setEnabled(bool)));
}

Plugin_RawConverter::~Plugin_RawConverter()
{
}

void Plugin_RawConverter::slotActivateSingle()
{
    Interface* interface = dynamic_cast<Interface*>( parent() );
    if (!interface)
    {
        kError() << "Kipi interface is null!";
        return;
    }

    ImageCollection images = interface->currentSelection();

    if (!images.isValid())
        return;

    if ( images.images().isEmpty() )
        return;

    if (!KPMetadata::isRawFile(images.images()[0]))
    {
        KMessageBox::error(kapp->activeWindow(),
                           i18n("\"%1\" is not a RAW file.", images.images()[0].fileName()));
        return;
    }

    if (!m_singleDlg)
    {
        m_singleDlg = new SingleDialog(images.images()[0].path());
    }
    else
    {
        if (m_singleDlg->isMinimized())
            KWindowSystem::unminimizeWindow(m_singleDlg->winId());

        KWindowSystem::activateWindow(m_singleDlg->winId());
        m_singleDlg->setFile(images.images()[0].path());
    }

    m_singleDlg->show();
}

void Plugin_RawConverter::slotActivateBatch()
{
    Interface* interface = dynamic_cast<Interface*>( parent() );
    if (!interface)
    {
        kError() << "Kipi interface is null!";
        return;
    }

    ImageCollection images;
    images = interface->currentSelection();

    if (!images.isValid())
        return;

    if (!m_batchDlg)
    {
        m_batchDlg = new BatchDialog();
    }
    else
    {
        if (m_batchDlg->isMinimized())
            KWindowSystem::unminimizeWindow(m_batchDlg->winId());

        KWindowSystem::activateWindow(m_batchDlg->winId());
    }

    m_batchDlg->show();
    m_batchDlg->addItems(images.images());
}

Category Plugin_RawConverter::category(KAction* action) const
{
    if ( action == m_singleAction )
       return ToolsPlugin;
    else if ( action == m_batchAction )
       return BatchPlugin;

    kWarning() << "Unrecognized action for plugin category identification";
    return ToolsPlugin; // no warning from compiler, please
}

} // namespace KIPIRawConverterPlugin
