/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2017 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2017 Jan Bajer aka bajasoft <jbajer@gmail.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "OpenAddressDialog.h"
#include "../core/InputInterpreter.h"
#include "../modules/widgets/address/AddressWidget.h"

#include "ui_OpenAddressDialog.h"

#include <QtGui/QKeyEvent>
#include <QtWidgets/QLineEdit>

namespace Otter
{

OpenAddressDialog::OpenAddressDialog(QWidget *parent) : Dialog(parent),
	m_addressWidget(nullptr),
	m_ui(new Ui::OpenAddressDialog)
{
	m_ui->setupUi(this);

	m_addressWidget = new AddressWidget(nullptr, this);
	m_addressWidget->setFocus();

	m_ui->verticalLayout->insertWidget(1, m_addressWidget);
	m_ui->label->setBuddy(m_addressWidget);

	connect(this, SIGNAL(accepted()), this, SLOT(handleUserInput()));
}

OpenAddressDialog::~OpenAddressDialog()
{
	delete m_ui;
}

void OpenAddressDialog::changeEvent(QEvent *event)
{
	QDialog::changeEvent(event);

	if (event->type() == QEvent::LanguageChange)
	{
		m_ui->retranslateUi(this);
	}
}

void OpenAddressDialog::keyPressEvent(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
	{
		handleUserInput();

		event->accept();
	}
	else
	{
		QDialog::keyPressEvent(event);
	}
}

void OpenAddressDialog::handleUserInput()
{
	const QString text(m_addressWidget->text().trimmed());

	if (text.isEmpty())
	{
		const InputInterpreter::InterpreterResult result(InputInterpreter::interpret(text, InputInterpreter::NoBookmarkKeywordsFlag));

		if (result.isValid())
		{
			const SessionsManager::OpenHints hints(SessionsManager::calculateOpenHints(SessionsManager::CurrentTabOpen));

			switch (result.type)
			{
				case InputInterpreter::InterpreterResult::BookmarkType:
					emit requestedOpenBookmark(result.bookmark, hints);

					break;
				case InputInterpreter::InterpreterResult::UrlType:
					emit requestedOpenUrl(result.url, hints);

					break;
				case InputInterpreter::InterpreterResult::SearchType:
					emit requestedSearch(result.searchQuery, result.searchEngine, hints);

					break;
				default:
					break;
			}
		}
	}

	close();
}

void OpenAddressDialog::setText(const QString &text)
{
	m_addressWidget->setText(text);
	m_addressWidget->activate(Qt::OtherFocusReason);
}

}
