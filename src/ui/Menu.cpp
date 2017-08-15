/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2017 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
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

#include "Menu.h"
#include "Action.h"
#include "ContentsWidget.h"
#include "ImportDialog.h"
#include "MainWindow.h"
#include "Window.h"
#include "../core/Application.h"
#include "../core/BookmarksManager.h"
#include "../core/Console.h"
#include "../core/HistoryManager.h"
#include "../core/NetworkManagerFactory.h"
#include "../core/NotesManager.h"
#include "../core/SearchEnginesManager.h"
#include "../core/SessionModel.h"
#include "../core/SessionsManager.h"
#include "../core/ThemesManager.h"
#include "../core/ToolBarsManager.h"
#include "../core/Utils.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QMetaEnum>
#include <QtCore/QMimeDatabase>
#include <QtCore/QTextCodec>
#include <QtGui/QMouseEvent>

namespace Otter
{

int Menu::m_menuRoleIdentifierEnumerator(-1);

Menu::Menu(int role, QWidget *parent) : QMenu(parent),
	m_actionGroup(nullptr),
	m_bookmark(nullptr),
	m_role(role),
	m_option(-1)
{
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "File"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Edit"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "View"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "History"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Tools"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Help"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Page"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Print"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Settings"))
	Q_UNUSED(QT_TRANSLATE_NOOP("actions", "Frame"))

	switch (role)
	{
		case BookmarksMenuRole:
		case BookmarkSelectorMenuRole:
		case NotesMenuRole:
			{
				setTitle((role == NotesMenuRole) ? QT_TRANSLATE_NOOP("actions", "Notes") : QT_TRANSLATE_NOOP("actions", "Bookmarks"));
				installEventFilter(this);

				const Menu *parentMenu(qobject_cast<Menu*>(parent));

				if (!parentMenu || parentMenu->getRole() != m_role)
				{
					if (m_role == NotesMenuRole)
					{
						connect(NotesManager::getModel(), SIGNAL(modelModified()), this, SLOT(clearBookmarksMenu()));
					}
					else
					{
						connect(BookmarksManager::getModel(), SIGNAL(modelModified()), this, SLOT(clearBookmarksMenu()));
					}
				}

				connect(this, SIGNAL(aboutToShow()), this, SLOT(populateBookmarksMenu()));
			}

			break;
		case CharacterEncodingMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Character Encoding"));

			m_option = SettingsManager::Content_DefaultCharacterEncodingOption;

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateCharacterEncodingMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectOption(QAction*)));

			break;
		case ClosedWindowsMenu:
			{
				setTitle(QT_TRANSLATE_NOOP("actions", "Closed Tabs and Windows"));
				setIcon(ThemesManager::createIcon(QLatin1String("user-trash")));

				const MainWindow *mainWindow(MainWindow::findMainWindow(parent));

				if (mainWindow)
				{
					setEnabled(!SessionsManager::getClosedWindows().isEmpty() || !mainWindow->getClosedWindows().isEmpty());

					connect(mainWindow, SIGNAL(closedWindowsAvailableChanged(bool)), this, SLOT(updateClosedWindowsMenu()));
				}

				connect(SessionsManager::getInstance(), SIGNAL(closedWindowsChanged()), this, SLOT(updateClosedWindowsMenu()));
				connect(this, SIGNAL(aboutToShow()), this, SLOT(populateClosedWindowsMenu()));
			}

			break;
		case ImportExportMenuRole:
			{
				setTitle(QT_TRANSLATE_NOOP("actions", "Import and Export"));

				Action *importOperaBookmarksAction(addAction());
				importOperaBookmarksAction->setData(QLatin1String("OperaBookmarks"));
				importOperaBookmarksAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Import Opera Bookmarks…"));

				Action *importHtmlBookmarksAction(addAction());
				importHtmlBookmarksAction->setData(QLatin1String("HtmlBookmarks"));
				importHtmlBookmarksAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Import HTML Bookmarks…"));

				addSeparator();

				Action *importOperaNotesAction(addAction());
				importOperaNotesAction->setData(QLatin1String("OperaNotes"));
				importOperaNotesAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Import Opera Notes…"));

				addSeparator();

				Action *importOperaSearchEnginesAction(addAction());
				importOperaSearchEnginesAction->setData(QLatin1String("OperaSearchEngines"));
				importOperaSearchEnginesAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Import Opera Search Engines…"));

				addSeparator();

				Action *importOperaSessionAction(addAction());
				importOperaSessionAction->setData(QLatin1String("OperaSession"));
				importOperaSessionAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Import Opera Session…"));

				connect(this, SIGNAL(triggered(QAction*)), this, SLOT(openImporter(QAction*)));
			}

			break;
		case OpenInApplicationMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Open with"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateOpenInApplicationMenu()));

			break;
		case ProxyMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Proxy"));

			m_option = SettingsManager::Network_ProxyOption;

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateProxiesMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectOption(QAction*)));

			break;
		case SearchMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Search Using"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateSearchMenu()));

			break;
		case SessionsMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Sessions"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateSessionsMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(openSession(QAction*)));

			break;
		case StyleSheetsMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Style"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateStyleSheetsMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectStyleSheet(QAction*)));

			break;
		case ToolBarsMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Toolbars"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateToolBarsMenu()));

			break;
		case UserAgentMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "User Agent"));

			m_option = SettingsManager::Network_UserAgentOption;

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateUserAgentMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectOption(QAction*)));

			break;
		case ValidateMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Validate Using"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateSearchMenu()));

			break;
		case WindowsMenuRole:
			setTitle(QT_TRANSLATE_NOOP("actions", "Tabs and Windows"));

			connect(this, SIGNAL(aboutToShow()), this, SLOT(populateWindowsMenu()));
			connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectWindow(QAction*)));

			break;
		default:
			break;
	}
}

void Menu::changeEvent(QEvent *event)
{
	QMenu::changeEvent(event);

	if (event->type() == QEvent::LanguageChange && !m_title.isEmpty())
	{
		QMenu::setTitle(QCoreApplication::translate("actions", m_title.toUtf8().constData()));
	}
}

void Menu::mouseReleaseEvent(QMouseEvent *event)
{
	if (m_role == BookmarksMenuRole && (event->button() == Qt::LeftButton || event->button() == Qt::MiddleButton))
	{
		const QAction *action(actionAt(event->pos()));

		if (action && action->isEnabled() && action->data().type() == QVariant::ULongLong)
		{
			QWidget *menu(this);

			while (menu)
			{
				menu->close();
				menu = menu->parentWidget();

				if (!menu || !menu->inherits(QLatin1String("QMenu").data()))
				{
					break;
				}
			}

			const BookmarksItem *bookmark(BookmarksManager::getModel()->getBookmark(action->data().toULongLong()));

			if (bookmark)
			{
				Application::triggerAction(ActionsManager::OpenBookmarkAction, {{QLatin1String("bookmark"), bookmark->data(BookmarksModel::IdentifierRole)}, {QLatin1String("hints"), QVariant(SessionsManager::calculateOpenHints(SessionsManager::DefaultOpen, event->button(), event->modifiers()))}}, parentWidget());
			}

			return;
		}
	}

	QMenu::mouseReleaseEvent(event);
}

void Menu::contextMenuEvent(QContextMenuEvent *event)
{
	if (m_role == BookmarksMenuRole)
	{
		const QAction *action(actionAt(event->pos()));

		if (action && action->isEnabled() && action->data().type() == QVariant::ULongLong)
		{
			m_bookmark = BookmarksManager::getModel()->getBookmark(action->data().toULongLong());

			QMenu contextMenu(this);
			contextMenu.addAction(ThemesManager::createIcon(QLatin1String("document-open")), tr("Open"), this, SLOT(openBookmark()));
			contextMenu.addAction(tr("Open in New Tab"), this, SLOT(openBookmark()))->setData(SessionsManager::NewTabOpen);
			contextMenu.addAction(tr("Open in New Background Tab"), this, SLOT(openBookmark()))->setData(static_cast<int>(SessionsManager::NewTabOpen | SessionsManager::BackgroundOpen));
			contextMenu.addSeparator();
			contextMenu.addAction(tr("Open in New Window"), this, SLOT(openBookmark()))->setData(SessionsManager::NewWindowOpen);
			contextMenu.addAction(tr("Open in New Background Window"), this, SLOT(openBookmark()))->setData(static_cast<int>(SessionsManager::NewWindowOpen | SessionsManager::BackgroundOpen));
			contextMenu.exec(event->globalPos());

			return;
		}
	}

	QMenu::contextMenuEvent(event);
}

void Menu::load(const QString &path, const QStringList &options, ActionExecutor::Object executor)
{
	QFile file(SessionsManager::getReadableDataPath(path));

	if (file.open(QIODevice::ReadOnly))
	{
		load(QJsonDocument::fromJson(file.readAll()).object(), options, executor);

		file.close();
	}
}

void Menu::load(const QJsonObject &definition, const QStringList &options, ActionExecutor::Object executor)
{
	const QString identifier(definition.value(QLatin1String("identifier")).toString());

	if (m_role == NoMenuRole)
	{
		clear();
	}

	setObjectName(identifier);
	setTitle(definition.value(QLatin1String("title")).toString());

	m_executor = executor;

	executor = getExecutor();

	const QJsonArray actions(definition.value(QLatin1String("actions")).toArray());

	for (int i = 0; i < actions.count(); ++i)
	{
		if (actions.at(i).isObject())
		{
			const QJsonObject object(actions.at(i).toObject());
			const QString type(object.value(QLatin1String("type")).toString());
			const QVariantMap parameters(object.value(QLatin1String("parameters")).toVariant().toMap());

			if (type == QLatin1String("action"))
			{
				const QString rawIdentifier(object.value(QLatin1String("identifier")).toString());
				const int identifier(ActionsManager::getActionIdentifier(rawIdentifier));

				if (identifier >= 0)
				{
					const QString text(object.value(QLatin1String("title")).toString());
					Action *action(new Action(identifier, parameters, this));
					action->setExecutor(executor);

					if (object.contains(QLatin1String("group")))
					{
						const QString group(object.value(QLatin1String("group")).toString());

						if (m_actionGroups.contains(group))
						{
							m_actionGroups[group]->addAction(action);
						}
						else
						{
							QActionGroup *actionGroup(new QActionGroup(this));
							actionGroup->setExclusive(true);
							actionGroup->addAction(action);

							m_actionGroups[group] = actionGroup;
						}
					}

					if (!text.isEmpty())
					{
						action->setOverrideText(text);
					}

					if (object.contains(QLatin1String("icon")))
					{
						const QString data(object.value(QLatin1String("icon")).toString());

						if (data.startsWith(QLatin1String("data:image/")))
						{
							action->setOverrideIcon(QIcon(Utils::loadPixmapFromDataUri(data)));
						}
						else
						{
							action->setOverrideIcon(ThemesManager::createIcon(data));
						}
					}

					QMenu::addAction(action);
				}
				else
				{
					Console::addMessage(tr("Failed to create menu action: %1").arg(rawIdentifier), Console::OtherCategory, Console::ErrorLevel);
				}

				continue;
			}

			if (object.contains(QLatin1String("excludeFrom")) && options.contains(object.value(QLatin1String("excludeFrom")).toString()))
			{
				continue;
			}

			if (object.contains(QLatin1String("includeIn")) && !options.contains(object.value(QLatin1String("includeIn")).toString()))
			{
				continue;
			}

			Menu *menu(new Menu(getMenuRoleIdentifier(object.value(QLatin1String("identifier")).toString()), this));

			if (parameters.contains(QLatin1String("option")))
			{
				menu->load(SettingsManager::getOptionIdentifier(parameters.value(QLatin1String("option")).toString()));
			}
			else
			{
				menu->load(object, options, executor);
			}

			if (type == QLatin1String("menu"))
			{
				addMenu(menu);
			}
			else if (type == QLatin1String("include"))
			{
				for (int j = 0; j < menu->actions().count(); ++j)
				{
					QMenu::addAction(menu->actions().at(j));
				}
			}
		}
		else
		{
			const QString rawIdentifier(actions.at(i).toString());
			const int role(rawIdentifier.endsWith(QLatin1String("Menu")) ? getMenuRoleIdentifier(rawIdentifier) : NoMenuRole);

			if (rawIdentifier == QLatin1String("separator"))
			{
				addSeparator();
			}
			else if (role != NoMenuRole)
			{
				addMenu(new Menu(role, this));
			}
			else
			{
				const int identifier(ActionsManager::getActionIdentifier(rawIdentifier));

				if (identifier >= 0)
				{
					Action *action(new Action(identifier, this));
					action->setExecutor(executor);

					QMenu::addAction(action);
				}
				else
				{
					Console::addMessage(tr("Failed to create menu action: %1").arg(rawIdentifier), Console::OtherCategory, Console::ErrorLevel);
				}
			}
		}
	}
}

void Menu::load(int option)
{
	m_option = option;

	switch (option)
	{
		case SettingsManager::Network_CookiesKeepModeOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Keep Cookie Until"));

			break;
		case SettingsManager::Network_CookiesPolicyOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Accept Cookies"));

			break;
		case SettingsManager::Network_ThirdPartyCookiesPolicyOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Accept Third-party Cookies"));

			break;
		case SettingsManager::Permissions_EnableFullScreenOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Full Screen"));

			break;
		case SettingsManager::Permissions_EnableGeolocationOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Geolocation"));

			break;
		case SettingsManager::Permissions_EnableImagesOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Images"));

			break;
		case SettingsManager::Permissions_EnableMediaCaptureAudioOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Capture Audio"));

			break;
		case SettingsManager::Permissions_EnableMediaCaptureVideoOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Capture Video"));

			break;
		case SettingsManager::Permissions_EnableMediaPlaybackAudioOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Playback Audio"));

			break;
		case SettingsManager::Permissions_EnableNotificationsOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Notifications"));

			break;
		case SettingsManager::Permissions_EnablePluginsOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Plugins"));

			break;
		case SettingsManager::Permissions_EnablePointerLockOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Pointer Lock"));

			break;
		case SettingsManager::Permissions_ScriptsCanCloseWindowsOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Closing Windows by JavaScript"));

			break;
		case SettingsManager::Permissions_ScriptsCanOpenWindowsOption:
			setTitle(QT_TRANSLATE_NOOP("actions", "Pop-Ups"));

			break;
		default:
			setTitle(SettingsManager::getOptionName(option));

			break;
	}

	connect(this, SIGNAL(aboutToShow()), this, SLOT(populateOptionMenu()));
	connect(this, SIGNAL(triggered(QAction*)), this, SLOT(selectOption(QAction*)));
}

void Menu::populateBookmarksMenu()
{
	Menu *menu(qobject_cast<Menu*>(sender()));

	if (!menu || !menu->menuAction() || ((!menu->actions().isEmpty() && !(m_role == BookmarksMenuRole && menuAction()->data().toULongLong() == 0 && menu->actions().count() == 3))))
	{
		return;
	}

	const BookmarksModel *model((m_role == NotesMenuRole) ? NotesManager::getModel() : BookmarksManager::getModel());
	BookmarksItem *bookmark(model->getBookmark(menu->menuAction()->data().toULongLong()));

	if (!bookmark)
	{
		bookmark = model->getRootItem();
	}

	if (bookmark->rowCount() > 1 && m_role == BookmarksMenuRole)
	{
		Action *openAllAction(menu->addAction());
		openAllAction->setData(bookmark->data(BookmarksModel::IdentifierRole).toULongLong());
		openAllAction->setIcon(ThemesManager::createIcon(QLatin1String("document-open-folder")));
		openAllAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Open All"));

		menu->addSeparator();

		connect(openAllAction, SIGNAL(triggered()), this, SLOT(openBookmark()));
	}

	if (m_role == BookmarkSelectorMenuRole)
	{
		Action *addFolderAction(menu->addAction());
		addFolderAction->setData(bookmark->data(BookmarksModel::IdentifierRole).toULongLong());
		addFolderAction->setIcon(ThemesManager::createIcon(QLatin1String("document-open-folder")));
		addFolderAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "This Folder"));

		menu->addSeparator();
	}

	for (int i = 0; i < bookmark->rowCount(); ++i)
	{
		const QModelIndex index(bookmark->index().child(i, 0));

		if (!index.isValid())
		{
			continue;
		}

		const BookmarksModel::BookmarkType type(static_cast<BookmarksModel::BookmarkType>(index.data(BookmarksModel::TypeRole).toInt()));

		if (type == BookmarksModel::RootBookmark || type == BookmarksModel::FolderBookmark || type == BookmarksModel::UrlBookmark)
		{
			Action *action(menu->addAction());
			action->setData(index.data(BookmarksModel::IdentifierRole));
			action->setOverrideIcon(index.data(Qt::DecorationRole).value<QIcon>());
			action->setToolTip(index.data(BookmarksModel::DescriptionRole).toString());
			action->setStatusTip(index.data(BookmarksModel::UrlRole).toString());

			if (index.data(BookmarksModel::TitleRole).toString().isEmpty())
			{
				action->setOverrideText(QT_TRANSLATE_NOOP("actions", "(Untitled)"));
			}
			else
			{
				action->setOverrideText(Utils::elideText(QString(index.data(BookmarksModel::TitleRole).toString()).replace(QLatin1Char('&'), QLatin1String("&&")), menu));
			}

			if (type == BookmarksModel::UrlBookmark && m_role == BookmarksMenuRole)
			{
				connect(action, SIGNAL(triggered()), this, SLOT(openBookmark()));
			}
			else if (type == BookmarksModel::FolderBookmark)
			{
				if (model->rowCount(index) > 0)
				{
					action->setMenu(new Menu(m_role, this));
				}
				else
				{
					action->setEnabled(false);
				}
			}
		}
		else
		{
			menu->addSeparator();
		}
	}
}

void Menu::populateOptionMenu()
{
	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	const QString value(mainWindow ? mainWindow->getOption(m_option).toString() : QString());

	if (m_actionGroup)
	{
		for (int i = 0; i < actions().count(); ++i)
		{
			QAction *action(actions().at(i));

			if (action && action->data().toString() == value)
			{
				action->setChecked(true);

				break;
			}
		}

		return;
	}

	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(true);

	const QVector<SettingsManager::OptionDefinition::ChoiceDefinition> choices(SettingsManager::getOptionDefinition(m_option).choices);

	if (choices.isEmpty())
	{
		return;
	}

	for (int i = 0; i < choices.count(); ++i)
	{
		Action *action(addAction());
		action->setCheckable(true);
		action->setChecked(choices.at(i).value == value);
		action->setData(choices.at(i).value);
		action->setIcon(choices.at(i).icon);

		m_actionGroup->addAction(action);

		if (choices.at(i).value == QLatin1String("ask"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Ask What to Do"));
		}
		else if (choices.at(i).value == QLatin1String("allow"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Always Allow"));
		}
		else if (choices.at(i).value == QLatin1String("disallow"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Always Deny"));
		}
		else if (choices.at(i).value == QLatin1String("keepUntilExpires"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Expires"));
		}
		else if (choices.at(i).value == QLatin1String("keepUntilExit"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Current Session is Closed"));
		}
		else if (choices.at(i).value == QLatin1String("acceptAll"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Always"));
		}
		else if (choices.at(i).value == QLatin1String("acceptExisting"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Only Existing"));
		}
		else if (choices.at(i).value == QLatin1String("readOnly"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Only Read Existing"));
		}
		else if (choices.at(i).value == QLatin1String("ignore"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Ignore"));
		}
		else if (choices.at(i).value == QLatin1String("openAll"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Open All"));
		}
		else if (choices.at(i).value == QLatin1String("openAllInBackground"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Open in Background"));
		}
		else if (choices.at(i).value == QLatin1String("blockAll"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Block All"));
		}
		else if (choices.at(i).value == QLatin1String("onlyCached"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Only Cached"));
		}
		else if (choices.at(i).value == QLatin1String("enabled"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Enabled"));
		}
		else if (choices.at(i).value == QLatin1String("onDemand"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "On Demand"));
		}
		else if (choices.at(i).value == QLatin1String("disabled"))
		{
			action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Disabled"));
		}
		else
		{
			action->setOverrideText(choices.at(i).getTitle());
		}
	}
}

void Menu::populateCharacterEncodingMenu()
{
	if (!m_actionGroup)
	{
		const QVector<int> textCodecs({106, 1015, 1017, 4, 5, 6, 7, 8, 82, 10, 85, 12, 13, 109, 110, 112, 2250, 2251, 2252, 2253, 2254, 2255, 2256, 2257, 2258, 18, 39, 17, 38, 2026});

		m_actionGroup = new QActionGroup(this);
		m_actionGroup->setExclusive(true);

		Action *defaultAction(addAction());
		defaultAction->setData(QLatin1String("auto"));
		defaultAction->setCheckable(true);
		defaultAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Auto Detect"));

		m_actionGroup->addAction(defaultAction);

		addSeparator();

		for (int i = 0; i < textCodecs.count(); ++i)
		{
			const QTextCodec *codec(QTextCodec::codecForMib(textCodecs.at(i)));

			if (!codec)
			{
				continue;
			}

			QAction *textCodecAction(QMenu::addAction(Utils::elideText(codec->name(), this)));
			textCodecAction->setData(codec->name().toLower());
			textCodecAction->setCheckable(true);

			m_actionGroup->addAction(textCodecAction);
		}
	}

	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	const QString encoding(mainWindow ? mainWindow->getOption(SettingsManager::Content_DefaultCharacterEncodingOption).toString().toLower() : QString());

	for (int i = 2; i < actions().count(); ++i)
	{
		QAction *action(actions().at(i));

		if (!action)
		{
			continue;
		}

		action->setChecked(encoding == action->text().toLower());

		if (action->isChecked())
		{
			break;
		}
	}

	if (!m_actionGroup->checkedAction() && !actions().isEmpty())
	{
		actions().first()->setChecked(true);
	}
}

void Menu::populateClosedWindowsMenu()
{
	clear();

	Action *clearAction(addAction());
	clearAction->setData(0);
	clearAction->setIcon(ThemesManager::createIcon(QLatin1String("edit-clear")));
	clearAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Clear"));

	addSeparator();

	const QStringList windows(SessionsManager::getClosedWindows());

	if (!windows.isEmpty())
	{
		for (int i = 0; i < windows.count(); ++i)
		{
			QMenu::addAction(Utils::elideText(tr("Window - %1").arg(windows.at(i)), this), this, SLOT(restoreClosedWindow()))->setData(-(i + 1));
		}

		addSeparator();
	}

	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

	if (mainWindow)
	{
		const QVector<ClosedWindow> tabs(mainWindow->getClosedWindows());

		for (int i = 0; i < tabs.count(); ++i)
		{
			QAction *action(QMenu::addAction((tabs.at(i).isPrivate ? ThemesManager::createIcon(QLatin1String("tab-private")) : tabs.at(i).icon), Utils::elideText(tabs.at(i).window.getTitle().replace(QLatin1Char('&'), QLatin1String("&&")), this), this, SLOT(restoreClosedWindow())));
			action->setData(i + 1);
			action->setStatusTip(tabs.at(i).window.getUrl());
		}
	}

	connect(clearAction, SIGNAL(triggered()), this, SLOT(clearClosedWindows()));
}

void Menu::populateOpenInApplicationMenu()
{
	const QVector<ApplicationInformation> applications(Utils::getApplicationsForMimeType(QMimeDatabase().mimeTypeForName(m_menuOptions.value(QLatin1String("mimeType")).toString())));
	const ActionExecutor::Object executor(getExecutor());
	QVariantMap parameters(m_actionParameters);

	if (applications.isEmpty())
	{
		parameters[QLatin1String("application")] = QString();

		Action *action(new Action(ActionsManager::OpenUrlAction, parameters, this));
		action->setOverrideText(QT_TRANSLATE_NOOP("actions", "Default Application"));
		action->setExecutor(executor);

		QMenu::addAction(action);
	}
	else
	{
		for (int i = 0; i < applications.count(); ++i)
		{
			parameters[QLatin1String("application")] = applications.at(i).command;

			Action *action(new Action(ActionsManager::OpenUrlAction, parameters, this));
			action->setOverrideIcon(applications.at(i).icon);
			action->setOverrideText(((applications.at(i).name.isEmpty()) ? QT_TRANSLATE_NOOP("actions", "Unknown") : applications.at(i).name));
			action->setExecutor(executor);

			QMenu::addAction(action);

			if (i == 0)
			{
				addSeparator();
			}
		}
	}
}

void Menu::populateProxiesMenu()
{
	if (m_actionGroup)
	{
		m_actionGroup->deleteLater();

		clear();
	}

	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	const QString proxy(mainWindow ? mainWindow->getOption(SettingsManager::Network_ProxyOption).toString() : QString());
	const QStringList proxies((!menuAction() || menuAction()->data().toString().isEmpty()) ? NetworkManagerFactory::getProxies() : NetworkManagerFactory::getProxy(menuAction()->data().toString()).children);

	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(true);

	for (int i = 0; i < proxies.count(); ++i)
	{
		if (proxies.at(i).isEmpty())
		{
			addSeparator();
		}
		else
		{
			const ProxyDefinition definition(NetworkManagerFactory::getProxy(proxies.at(i)));
			Action *action(addAction());
			action->setData(proxies.at(i));
			action->setText(Utils::elideText(definition.getTitle(), this));

			if (definition.isFolder)
			{
				action->setIcon(ThemesManager::createIcon(QLatin1String("inode-directory")));

				if (definition.children.count() > 0)
				{
					action->setMenu(new Menu(m_role, this));
				}
				else
				{
					action->setEnabled(false);
				}
			}
			else
			{
				action->setCheckable(true);
				action->setChecked(proxy == proxies.at(i));
			}

			m_actionGroup->addAction(action);
		}
	}
}

void Menu::populateSearchMenu()
{
	clear();

	const QStringList searchEngines((m_role == ValidateMenuRole) ? SettingsManager::getOption(SettingsManager::Browser_ValidatorsOrderOption).toStringList() : SearchEnginesManager::getSearchEngines());
	const MainWindow *mainWindow(MainWindow::findMainWindow(this));

	for (int i = 0; i < searchEngines.count(); ++i)
	{
		const SearchEnginesManager::SearchEngineDefinition searchEngine(SearchEnginesManager::getSearchEngine(searchEngines.at(i)));

		if (searchEngine.isValid())
		{
			Action *action(addAction(ActionsManager::SearchAction));
			action->setParameters({{QLatin1String("searchEngine"), searchEngine.identifier}, {QLatin1String("queryPlaceholder"), ((m_role == ValidateMenuRole) ? QLatin1String("{pageUrl}") : QLatin1String("{selection}"))}});
			action->setIcon(searchEngine.icon);
			action->setOverrideText(searchEngine.title);
			action->setToolTip(searchEngine.description);

			if (mainWindow)
			{
				connect(action, SIGNAL(triggered(bool)), mainWindow, SLOT(triggerAction(bool)));
			}
		}
	}
}

void Menu::populateSessionsMenu()
{
	if (m_actionGroup)
	{
		m_actionGroup->deleteLater();
	}

	clear();

	addAction(ActionsManager::SaveSessionAction, true);
	addAction(ActionsManager::SessionsAction, true);

	addSeparator();

	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(true);

	const QStringList sessions(SessionsManager::getSessions());
	QMultiHash<QString, SessionInformation> information;

	for (int i = 0; i < sessions.count(); ++i)
	{
		const SessionInformation session(SessionsManager::getSession(sessions.at(i)));

		information.insert((session.title.isEmpty() ? tr("(Untitled)") : session.title), session);
	}

	const QList<SessionInformation> sorted(information.values());
	const QString currentSession(SessionsManager::getCurrentSession());

	for (int i = 0; i < sorted.count(); ++i)
	{
		int windows(0);

		for (int j = 0; j < sorted.at(i).windows.count(); ++j)
		{
			windows += sorted.at(i).windows.at(j).windows.count();
		}

		QAction *action(QMenu::addAction(tr("%1 (%n tab(s))", "", windows).arg(sorted.at(i).title.isEmpty() ? tr("(Untitled)") : QString(sorted.at(i).title).replace(QLatin1Char('&'), QLatin1String("&&")))));
		action->setData(sorted.at(i).path);
		action->setCheckable(true);
		action->setChecked(sorted.at(i).path == currentSession);

		m_actionGroup->addAction(action);
	}
}

void Menu::populateStyleSheetsMenu()
{
	clear();

	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	Action *defaultAction(addAction());
	defaultAction->setData(-1);
	defaultAction->setCheckable(true);
	defaultAction->setChecked(true);
	defaultAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Default Style"));

	if (!mainWindow)
	{
		return;
	}

	Window *window(mainWindow->getWindowByIndex(-1));

	if (!window)
	{
		return;
	}

	addSeparator();

	const QString activeStyleSheet(window->getContentsWidget()->getActiveStyleSheet());
	const QStringList styleSheets(window->getContentsWidget()->getStyleSheets());
	QActionGroup *actionGroup(new QActionGroup(this));
	actionGroup->setExclusive(true);
	actionGroup->addAction(defaultAction);

	for (int i = 0; i < styleSheets.count(); ++i)
	{
		QAction *action(QMenu::addAction(styleSheets.at(i)));

		action->setCheckable(true);
		action->setChecked(styleSheets.at(i) == activeStyleSheet);

		actionGroup->addAction(action);
	}

	defaultAction->setChecked(activeStyleSheet.isEmpty() || !styleSheets.contains(activeStyleSheet));
}

void Menu::populateToolBarsMenu()
{
	clear();

	MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	const QVector<ToolBarsManager::ToolBarDefinition> definitions(ToolBarsManager::getToolBarDefinitions());

	if (mainWindow)
	{
		for (int i = 0; i < definitions.count(); ++i)
		{
			Action *action(new Action(ActionsManager::ShowToolBarAction, {{QLatin1String("toolBar"), definitions.at(i).identifier}}, this));
			action->setExecutor(ActionExecutor::Object(mainWindow, mainWindow));

			QMenu::addAction(action);
		}

		addSeparator();
	}

	Menu *addNewMenu(new Menu(NoMenuRole, this));
	Action *addNewAction(addAction());
	addNewAction->setMenu(addNewMenu);
	addNewAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Add New"));

	Action *addToolBarAction(addNewMenu->addAction());
	addToolBarAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Add Toolbar…"));

	Action *addBookmarksBarAction(addNewMenu->addAction());
	addBookmarksBarAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Add Bookmarks Bar…"));

	Action *addSideBarAction(addNewMenu->addAction());
	addSideBarAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Add Sidebar…"));

	addSeparator();
	addAction(ActionsManager::LockToolBarsAction, true);
	addSeparator();
	addAction(ActionsManager::ResetToolBarsAction, true);

	connect(addToolBarAction, SIGNAL(triggered()), ToolBarsManager::getInstance(), SLOT(addToolBar()));
	connect(addBookmarksBarAction, SIGNAL(triggered()), ToolBarsManager::getInstance(), SLOT(addBookmarksBar()));
	connect(addSideBarAction, SIGNAL(triggered()), ToolBarsManager::getInstance(), SLOT(addSideBar()));
}

void Menu::populateUserAgentMenu()
{
	if (m_actionGroup)
	{
		m_actionGroup->deleteLater();

		clear();
	}

	const bool isRoot(!menuAction() || menuAction()->data().toString().isEmpty());
	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));
	const QString userAgent(mainWindow ? mainWindow->getOption(SettingsManager::Network_UserAgentOption).toString() : QString());
	const QStringList userAgents(isRoot ? NetworkManagerFactory::getUserAgents() : NetworkManagerFactory::getUserAgent(menuAction()->data().toString()).children);

	m_actionGroup = new QActionGroup(this);
	m_actionGroup->setExclusive(true);

	for (int i = 0; i < userAgents.count(); ++i)
	{
		if (userAgents.at(i).isEmpty())
		{
			addSeparator();
		}
		else
		{
			const UserAgentDefinition definition(NetworkManagerFactory::getUserAgent(userAgents.at(i)));
			Action *action(addAction());
			action->setData(userAgents.at(i));
			action->setText(Utils::elideText(definition.getTitle(), this));

			if (definition.isFolder)
			{
				action->setIcon(ThemesManager::createIcon(QLatin1String("inode-directory")));

				if (definition.children.count() > 0)
				{
					action->setMenu(new Menu(m_role, this));
				}
				else
				{
					action->setEnabled(false);
				}
			}
			else
			{
				action->setCheckable(true);
				action->setChecked(userAgent == userAgents.at(i));
			}

			m_actionGroup->addAction(action);
		}
	}

	if (isRoot)
	{
		addSeparator();

		Action *customAction(addAction());
		customAction->setData(QLatin1String("custom"));
		customAction->setCheckable(true);
		customAction->setChecked(userAgent.startsWith(QLatin1String("custom;")));
		customAction->setOverrideText(QT_TRANSLATE_NOOP("actions", "Custom User Agent…"));

		m_actionGroup->addAction(customAction);
	}
}

void Menu::populateWindowsMenu()
{
	if (actions().isEmpty())
	{
		const MainWindow *mainWindow(MainWindow::findMainWindow(this));

		if (mainWindow)
		{
			connect(mainWindow, SIGNAL(titleChanged(QString)), this, SLOT(populateWindowsMenu()));
			connect(mainWindow, SIGNAL(windowAdded(quint64)), this, SLOT(populateWindowsMenu()));
			connect(mainWindow, SIGNAL(windowRemoved(quint64)), this, SLOT(populateWindowsMenu()));
		}

		disconnect(this, SIGNAL(aboutToShow()), this, SLOT(populateWindowsMenu()));
	}

	clear();

	const MainWindowSessionItem *mainWindowItem(SessionsManager::getModel()->getMainWindowItem(MainWindow::findMainWindow(this)));

	if (mainWindowItem)
	{
		for (int i = 0; i < mainWindowItem->rowCount(); ++i)
		{
			const WindowSessionItem *windowItem(static_cast<WindowSessionItem*>(mainWindowItem->child(i, 0)));

			if (windowItem)
			{
				Action *action(addAction());
				action->setData(windowItem->getActiveWindow()->getIdentifier());
				action->setIcon(windowItem->getActiveWindow()->getIcon());

				if (windowItem->getActiveWindow()->getTitle().isEmpty())
				{
					action->setOverrideText(QT_TRANSLATE_NOOP("actions", "(Untitled)"));
				}
				else
				{
					action->setText(Utils::elideText(windowItem->getActiveWindow()->getTitle()));
				}
			}
		}
	}
}

void Menu::clearBookmarksMenu()
{
	const int offset((m_role == BookmarksMenuRole && menuAction() && menuAction()->data().toULongLong() == 0) ? 3 : 0);

	for (int i = (actions().count() - 1); i >= offset; --i)
	{
		actions().at(i)->deleteLater();

		removeAction(actions().at(i));
	}

	if (m_role == NotesMenuRole && menuAction())
	{
		const BookmarksItem *bookmark(NotesManager::getModel()->getBookmark(menuAction()->data().toULongLong()));

		menuAction()->setEnabled(bookmark && bookmark->rowCount() > 0);
	}

	connect(this, SIGNAL(aboutToShow()), this, SLOT(populateBookmarksMenu()));
}

void Menu::clearClosedWindows()
{
	MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

	if (mainWindow)
	{
		mainWindow->clearClosedWindows();
	}

	SessionsManager::clearClosedWindows();
}

void Menu::restoreClosedWindow()
{
	const QAction *action(qobject_cast<QAction*>(sender()));
	const int index((action && action->data().type() == QVariant::Int) ? action->data().toInt() : 0);

	if (index > 0)
	{
		MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

		if (mainWindow)
		{
			mainWindow->restore(index - 1);
		}
	}
	else if (index < 0)
	{
		SessionsManager::restoreClosedWindow(-index - 1);
	}
}

void Menu::openBookmark()
{
	QWidget *menu(this);

	while (menu)
	{
		menu->close();
		menu = menu->parentWidget();

		if (!menu || !menu->inherits(QLatin1String("QMenu").data()))
		{
			break;
		}
	}

	const QAction *action(qobject_cast<QAction*>(sender()));

	if (action && action->data().type() == QVariant::ULongLong)
	{
		m_bookmark = BookmarksManager::getModel()->getBookmark(action->data().toULongLong());
	}

	const SessionsManager::OpenHints hints((action && action->data().type() != QVariant::ULongLong) ? static_cast<SessionsManager::OpenHints>(action->data().toInt()) : SessionsManager::DefaultOpen);

	Application::triggerAction(ActionsManager::OpenBookmarkAction, {{QLatin1String("bookmark"), m_bookmark->data(BookmarksModel::IdentifierRole)}, {QLatin1String("hints"), QVariant((hints == SessionsManager::DefaultOpen) ? SessionsManager::calculateOpenHints() : hints)}}, parentWidget());

	m_bookmark = nullptr;
}

void Menu::openImporter(QAction *action)
{
	if (action)
	{
		ImportDialog::createDialog(action->data().toString(), MainWindow::findMainWindow(this));
	}
}

void Menu::openSession(QAction *action)
{
	if (!action->data().isNull())
	{
		SessionsManager::restoreSession(SessionsManager::getSession(action->data().toString()), (SettingsManager::getOption(SettingsManager::Sessions_OpenInExistingWindowOption).toBool() ? Application::getActiveWindow() : nullptr));
	}
}

void Menu::selectOption(QAction *action)
{
	MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

	if (mainWindow)
	{
		mainWindow->setOption(m_option, action->data().toString());
	}
}

void Menu::selectStyleSheet(QAction *action)
{
	MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

	if (!mainWindow)
	{
		return;
	}

	Window *window(mainWindow->getWindowByIndex(-1));

	if (window && action)
	{
		window->getContentsWidget()->setActiveStyleSheet(action->data().isNull() ? action->text() : QString());
	}
}

void Menu::selectWindow(QAction *action)
{
	if (action)
	{
		Application::triggerAction(ActionsManager::ActivateTabAction, {{QLatin1String("tab"), action->data().toULongLong()}}, parentWidget());
	}
}

void Menu::updateClosedWindowsMenu()
{
	const MainWindow *mainWindow(MainWindow::findMainWindow(parent()));

	setEnabled((mainWindow && mainWindow->getClosedWindows().count() > 0) || SessionsManager::getClosedWindows().count() > 0);
}

void Menu::setTitle(const QString &title)
{
	m_title = title;

	QMenu::setTitle(QCoreApplication::translate("actions", m_title.toUtf8().constData()));
}

void Menu::setExecutor(ActionExecutor::Object executor)
{
	m_executor = executor;
}

void Menu::setActionParameters(const QVariantMap &parameters)
{
	m_actionParameters = parameters;
}

void Menu::setMenuOptions(const QVariantMap &options)
{
	m_menuOptions = options;
}

Action* Menu::addAction(int identifier, bool useGlobal)
{
	Action *action(useGlobal ? Application::createAction(identifier, {}, true, this) : new Action(identifier, {}, Action::NoFlag, this));

	QMenu::addAction(action);

	return action;
}

ActionExecutor::Object Menu::getExecutor()
{
	if (m_executor.isValid())
	{
		return m_executor;
	}

	MainWindow *mainWindow(MainWindow::findMainWindow(this));

	if (mainWindow)
	{
		return ActionExecutor::Object(mainWindow, mainWindow);
	}

	return ActionExecutor::Object(Application::getInstance(), Application::getInstance());
}

int Menu::getRole() const
{
	return m_role;
}

int Menu::getMenuRoleIdentifier(const QString &name)
{
	if (m_menuRoleIdentifierEnumerator < 0)
	{
		m_menuRoleIdentifierEnumerator = Menu::staticMetaObject.indexOfEnumerator(QLatin1String("MenuRole").data());
	}

	if (!name.endsWith(QLatin1String("Role")))
	{
		return Menu::staticMetaObject.enumerator(m_menuRoleIdentifierEnumerator).keyToValue(QString(name + QLatin1String("Role")).toLatin1());
	}

	return Menu::staticMetaObject.enumerator(m_menuRoleIdentifierEnumerator).keyToValue(name.toLatin1());
}

}
