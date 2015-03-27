/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 Piotr Wójcik <chocimier@tlen.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "BookmarkPropertiesDialog.h"
#include "BookmarksComboBoxWidget.h"
#include "../core/BookmarksManager.h"
#include "../core/Utils.h"

#include "ui_BookmarkPropertiesDialog.h"

#include <QtCore/QDateTime>
#include <QtWidgets/QMessageBox>

namespace Otter
{

BookmarkPropertiesDialog::BookmarkPropertiesDialog(BookmarksItem *bookmark, BookmarkMode mode, QWidget *parent) : QDialog(parent),
	m_bookmark(bookmark),
	m_model(new QStandardItemModel(this)),
	m_ui(new Ui::BookmarkPropertiesDialog)
{
	const BookmarksItem::BookmarkType type = static_cast<BookmarksItem::BookmarkType>(bookmark->data(BookmarksModel::TypeRole).toInt());

	m_ui->setupUi(this);
	m_ui->folderComboBox->setCurrentFolder((bookmark->parent() ? bookmark->parent() : BookmarksManager::getModel()->getRootItem())->index());
	m_ui->titleLineEdit->setText(m_bookmark->data(BookmarksModel::TitleRole).toString());
	m_ui->addressLineEdit->setText(m_bookmark->data(BookmarksModel::UrlRole).toString());
	m_ui->addressLineEdit->setVisible(type == BookmarksItem::UrlBookmark);
	m_ui->addressLabel->setVisible(type == BookmarksItem::UrlBookmark);
	m_ui->descriptionTextEdit->setPlainText(m_bookmark->data(BookmarksModel::DescriptionRole).toString());
	m_ui->keywordLineEdit->setText(m_bookmark->data(BookmarksModel::KeywordRole).toString());
	m_ui->addedLabelWidget->setText(m_bookmark->data(BookmarksModel::TimeAddedRole).isValid() ? m_bookmark->data(BookmarksModel::TitleRole).toDateTime().toString() : tr("Unknown"));
	m_ui->modifiedLabelWidget->setText(m_bookmark->data(BookmarksModel::TimeModifiedRole).isValid() ? m_bookmark->data(BookmarksModel::TimeModifiedRole).toDateTime().toString() : tr("Unknown"));

	if (type == BookmarksItem::UrlBookmark)
	{
		m_ui->lastVisitLabelWidget->setText(m_bookmark->data(BookmarksModel::TimeVisitedRole).isValid() ? m_bookmark->data(BookmarksModel::TimeVisitedRole).toString() : tr("Unknown"));
		m_ui->visitsLabelWidget->setText(QString::number(m_bookmark->data(BookmarksModel::VisitsRole).toInt()));
	}
	else
	{
		m_ui->visitsLabel->hide();
		m_ui->visitsLabelWidget->hide();
		m_ui->lastVisitLabel->hide();
		m_ui->lastVisitLabelWidget->hide();
	}

	if (mode == EditBookmarkMode)
	{
		setWindowTitle(tr("Edit Bookmark"));
	}
	else if (mode == AddBookmarkMode)
	{
		setWindowTitle(tr("Add Bookmark"));

		m_ui->visitsLabel->hide();
		m_ui->visitsLabelWidget->hide();
		m_ui->lastVisitLabel->hide();
		m_ui->lastVisitLabelWidget->hide();
		m_ui->addedLabel->hide();
		m_ui->addedLabelWidget->hide();
		m_ui->modifiedLabel->hide();
		m_ui->modifiedLabelWidget->hide();
	}
	else
	{
		setWindowTitle(tr("View Bookmark"));

		m_ui->folderComboBox->setEnabled(false);
		m_ui->newFolderButton->setEnabled(false);
		m_ui->titleLineEdit->setEnabled(false);
		m_ui->addressLineEdit->setEnabled(false);
		m_ui->addressLabel->setEnabled(false);
		m_ui->descriptionTextEdit->setEnabled(false);
		m_ui->keywordLineEdit->setEnabled(false);
	}

	connect(m_ui->newFolderButton, SIGNAL(clicked()), m_ui->folderComboBox, SLOT(createFolder()));
	connect(m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(saveBookmark()));
	connect(m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(close()));
}

BookmarkPropertiesDialog::~BookmarkPropertiesDialog()
{
	delete m_ui;
}

void BookmarkPropertiesDialog::changeEvent(QEvent *event)
{
	QDialog::changeEvent(event);

	switch (event->type())
	{
		case QEvent::LanguageChange:
			m_ui->retranslateUi(this);

			break;
		default:
			break;
	}
}

void BookmarkPropertiesDialog::saveBookmark()
{
	if (m_ui->titleLineEdit->text().isEmpty())
	{
		return;
	}

	if (m_ui->folderComboBox->isEnabled())
	{
		const QString keyword = m_ui->keywordLineEdit->text();

		if (m_bookmark->data(BookmarksModel::KeywordRole).toString() != keyword && BookmarksManager::getBookmark(keyword))
		{
			QMessageBox::critical(this, tr("Error"), tr("Bookmark with this keyword already exists."), QMessageBox::Close);

			return;
		}

		m_bookmark->setData(m_ui->addressLineEdit->text(), BookmarksModel::UrlRole);
		m_bookmark->setData(m_ui->titleLineEdit->text(), BookmarksModel::TitleRole);
		m_bookmark->setData(m_ui->descriptionTextEdit->toPlainText(), BookmarksModel::DescriptionRole);
		m_bookmark->setData(keyword, BookmarksModel::KeywordRole);

		if (!m_bookmark->parent())
		{
			m_ui->folderComboBox->getCurrentFolder()->appendRow(m_bookmark);
		}
	}

	accept();
}

}
