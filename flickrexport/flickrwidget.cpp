/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.kipi-plugins.org
 *
 * Date        : 2005-07-07
 * Description : a kipi plugin to export images to Flickr web service
 *
 * Copyright (C) 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpushbutton.h>
#include <qlabel.h>
#include <qframe.h>
#include <qheader.h>
#include <qlistview.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qsplitter.h>
#include <qwhatsthis.h>
#include <qlineedit.h>

// KDE includes.

#include <kdialog.h>
#include <klocale.h>
#include <khtml_part.h>
#include <khtmlview.h>

// Local includes.

#include "flickrwidget.h"
#include "flickrwidget.moc"

namespace KIPIFlickrExportPlugin
{

FlickrWidget::FlickrWidget(QWidget* parent, const char* name, WFlags fl)
            : QWidget(parent, name, fl)
{
    if ( !name )
        setName( "FlickrWidget" );

    // ------------------------------------------------------------------------

    QVBoxLayout* flickrWidgetLayout = new QVBoxLayout(this, 5, 5, "FlickrWidgetLayout");

    QLabel *headerLabel = new QLabel(this, "headerLabel");
    QFrame *headerLine  = new QFrame(this, "headerLine");
    QSplitter* splitter = new QSplitter(this);
    m_tagView           = new QListView(splitter, "m_tagView");

    //m_tagView->addColumn( i18n( "Albums" ) );
    //m_tagView->setResizeMode( QListView::AllColumns );

    headerLine->setFrameShape(QFrame::HLine);
    headerLine->setFrameShadow(QFrame::Sunken );

    // ------------------------------------------------------------------------

    m_photoView = NULL;//new KHTMLPart( splitter, "m_photoView" );

    // ------------------------------------------------------------------------

    QButtonGroup* rightButtonGroup      = new QButtonGroup(splitter, "rightButtonGroup");
    QVBoxLayout* rightButtonGroupLayout = new QVBoxLayout(rightButtonGroup);

    //m_newAlbumBtn = new QPushButton( rightButtonGroup, "m_newAlbumBtn" );
    //m_newAlbumBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //rightButtonGroupLayout->addWidget( m_newAlbumBtn, 0, Qt::AlignHCenter );

    m_fileSrcButtonGroup = new QButtonGroup(splitter, "fileSourceButton");
    m_fileSrcButtonGroup->setRadioButtonExclusive(true);
    m_fileSrcButtonGroup->setColumnLayout(0, Qt::Vertical);
    m_fileSrcButtonGroup->layout()->setSpacing(KDialog::spacingHint());
    m_fileSrcButtonGroup->layout()->setMargin(KDialog::spacingHint());

    QVBoxLayout* m_fileSrcButtonGroupLayout = new QVBoxLayout(m_fileSrcButtonGroup->layout());

    // ------------------------------------------------------------------

    m_currentSelectionButton = new QRadioButton(m_fileSrcButtonGroup);
    m_currentSelectionButton->setText(i18n("Upload Currently Selected Images"));
    m_currentSelectionButton->setChecked(true);

    // ------------------------------------------------------------------

    m_selectImagesButton = new QRadioButton(m_fileSrcButtonGroup);
    m_selectImagesButton->setText(i18n("Select Images For uploading"));
    m_selectImagesButton->setEnabled( true );

    m_addPhotoBtn = new QPushButton(m_fileSrcButtonGroup, "m_addPhotoBtn");
    m_addPhotoBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    m_fileSrcButtonGroupLayout->setAlignment(Qt::AlignTop);
    m_fileSrcButtonGroupLayout->addWidget(m_currentSelectionButton);
    m_fileSrcButtonGroupLayout->addWidget(m_selectImagesButton);
    m_fileSrcButtonGroupLayout->addWidget(m_addPhotoBtn);

    // -------------------------------------------------------------------

    QGridLayout* tagsLayout = new QGridLayout(rightButtonGroupLayout, 2, 2);
    QLabel* tagsLabel       = new QLabel(i18n("Tags:"), rightButtonGroup);
    m_tagsLineEdit          = new QLineEdit(rightButtonGroup, "m_tagsLineEdit");
    m_exportApplicationTags = new QCheckBox(rightButtonGroup);
    m_exportApplicationTags->setText(i18n("Use application tags ?"));

    // ------------------------------------------------------------------------

    QGroupBox* optionsBox = new QGroupBox(i18n("Override Default Options"), rightButtonGroup);
    optionsBox->setColumnLayout(0, Qt::Vertical);
    optionsBox->layout()->setSpacing(KDialog::spacingHint());
    optionsBox->layout()->setMargin(KDialog::spacingHint());

    QGroupBox* loginDetailsBox = new QGroupBox(i18n("User Details"), rightButtonGroup);
    loginDetailsBox->setColumnLayout(0, Qt::Vertical);
    loginDetailsBox->layout()->setSpacing(KDialog::spacingHint());
    loginDetailsBox->layout()->setMargin(KDialog::spacingHint());

    QGridLayout* optionsBoxLayout      = new QGridLayout(optionsBox->layout(), 3, 3);
    QGridLayout* loginDetailsBoxLayout = new QGridLayout(loginDetailsBox->layout(), 3, 3);

    // ------------------------------------------------------------------------

    m_publicCheckBox = new QCheckBox(optionsBox);
    m_publicCheckBox->setText(i18n("As in accessible for people", "Public ?"));
    //m_publicCheckBox->show();

    m_familyCheckBox = new QCheckBox(optionsBox);
    m_familyCheckBox->setText(i18n("Family ?"));

    m_friendsCheckBox = new QCheckBox(optionsBox);
    m_friendsCheckBox->setText(i18n("Friends ?"));

    m_resizeCheckBox = new QCheckBox(optionsBox);
    m_resizeCheckBox->setText(i18n("Resize photos before uploading"));
    // m_resizeCheckBox->show();

    m_dimensionSpinBox = new QSpinBox(0, 5000, 10, optionsBox);
    m_dimensionSpinBox->setValue(600);
    m_dimensionSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* resizeLabel = new QLabel(i18n("Maximum dimension:"), optionsBox);

    m_imageQualitySpinBox = new QSpinBox(0, 100, 1, optionsBox);
    m_imageQualitySpinBox->setValue(85);
    m_imageQualitySpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    //The term Compression factor may be to technical to write in the label    
    QLabel* imageQualityLabel = new QLabel(i18n("Image Quality (higher is better):"), optionsBox);

    m_resizeCheckBox->setChecked(false);
    m_dimensionSpinBox->setEnabled(false);

    // ------------------------------------------------------------------------

    headerLabel->setText(i18n("<h2>Flickr Export</h2>"));
    m_tagView->hide();
    //m_tagView->header()->setLabel( 0, i18n( "Albums" ) );
    //m_newAlbumBtn->setText( i18n( "&New Album" ) );
    m_addPhotoBtn->setText(i18n("&Add Photos"));

    QLabel *userNameLabel  = new QLabel(i18n("Username:"), loginDetailsBox);
    m_userNameDisplayLabel = new QLabel(i18n(" "), loginDetailsBox);
    m_changeUserButton     = new QPushButton(loginDetailsBox, "m_changeUserButton");
    m_changeUserButton->setText(i18n("Login with a different account"));

    // ------------------------------------------------------------------------

    flickrWidgetLayout->addWidget(headerLabel, 0);
    flickrWidgetLayout->addWidget(headerLine, 0);
    flickrWidgetLayout->addWidget(splitter, 5);

    tagsLayout->addWidget(tagsLabel,               0, 0);
    tagsLayout->addWidget(m_tagsLineEdit,          0, 1);
    tagsLayout->addWidget(m_exportApplicationTags, 1, 1);

    optionsBoxLayout->addWidget(m_publicCheckBox,          0, 1);
    optionsBoxLayout->addWidget(m_familyCheckBox,          0, 2 );
    optionsBoxLayout->addWidget(m_friendsCheckBox,         0, 3);
    optionsBoxLayout->addMultiCellWidget(m_resizeCheckBox, 1, 1, 0, 4, 1);
    optionsBoxLayout->addWidget(m_dimensionSpinBox,        2, 1);
    optionsBoxLayout->addWidget(resizeLabel,               2, 0);
    optionsBoxLayout->addWidget(m_imageQualitySpinBox,     3, 1);
    optionsBoxLayout->addWidget(imageQualityLabel,         3, 0);

    loginDetailsBoxLayout->addWidget(userNameLabel,          0, 1);
    loginDetailsBoxLayout->addWidget(m_userNameDisplayLabel, 0, 2);
    loginDetailsBoxLayout->addWidget(m_changeUserButton,     0, 3);

    rightButtonGroupLayout->setSpacing(KDialog::spacingHint());
    rightButtonGroupLayout->setMargin(KDialog::spacingHint());
    rightButtonGroupLayout->addWidget(optionsBox);
    rightButtonGroupLayout->addItem(new QSpacerItem(20, 100, QSizePolicy::Minimum, QSizePolicy::Expanding));
    rightButtonGroupLayout->addWidget(loginDetailsBox);

    // ------------------------------------------------------------------------

    connect(m_selectImagesButton, SIGNAL(clicked()),
            this, SLOT(slotSelectionChecked()));

    connect(m_resizeCheckBox, SIGNAL(clicked()),
            this, SLOT(slotResizeChecked()));

    // ------------------------------------------------------------------------

    resize( QSize(600, 400).expandedTo(minimumSizeHint()) );
    clearWState(WState_Polished);
}

FlickrWidget::~FlickrWidget()
{
}

void FlickrWidget::slotSelectionChecked()
{
    kdDebug() << "Slot Selection Checked" << endl;
    m_addPhotoBtn->setEnabled(m_selectImagesButton->isChecked());
}

void FlickrWidget::slotResizeChecked()
{
    m_dimensionSpinBox->setEnabled(m_resizeCheckBox->isChecked());
}

} // namespace KIPIFlickrExportPlugin
