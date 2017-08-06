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

#ifndef OTTER_MENU_H
#define OTTER_MENU_H

#include <QtCore/QJsonObject>
#include <QtWidgets/QMenu>

namespace Otter
{

class Action;
class BookmarksItem;

class Menu : public QMenu
{
	Q_OBJECT

public:
	enum MenuRole
	{
		NoMenuRole = 0,
		BookmarksMenuRole,
		BookmarkSelectorMenuRole,
		NotesMenuRole,
		CharacterEncodingMenuRole,
		ClosedWindowsMenu,
		ImportExportMenuRole,
		ProxyMenuRole,
		SearchMenuRole,
		SessionsMenuRole,
		StyleSheetsMenuRole,
		ToolBarsMenuRole,
		UserAgentMenuRole,
		ValidateMenuRole,
		WindowsMenuRole
	};

	explicit Menu(MenuRole role = NoMenuRole, QWidget *parent = nullptr);

	void load(const QString &path, const QStringList &options = {});
	void load(const QJsonObject &definition, const QStringList &options = {});
	void load(int option);
	void setTitle(const QString &title);
	Action* addAction(int identifier = -1, bool useGlobal = false);
	MenuRole getRole() const;
	static MenuRole getRole(const QString &identifier);

protected:
	void changeEvent(QEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;

protected slots:
	void populateBookmarksMenu();
	void populateOptionMenu();
	void populateCharacterEncodingMenu();
	void populateClosedWindowsMenu();
	void populateProxiesMenu();
	void populateSearchMenu();
	void populateSessionsMenu();
	void populateStyleSheetsMenu();
	void populateToolBarsMenu();
	void populateUserAgentMenu();
	void populateWindowsMenu();
	void clearBookmarksMenu();
	void clearClosedWindows();
	void restoreClosedWindow();
	void openBookmark();
	void openImporter(QAction *action);
	void openSession(QAction *action);
	void selectOption(QAction *action);
	void selectStyleSheet(QAction *action);
	void selectWindow(QAction *action);
	void updateClosedWindowsMenu();

private:
	QActionGroup *m_actionGroup;
	BookmarksItem *m_bookmark;
	QString m_title;
	QHash<QString, QActionGroup*> m_actionGroups;
	MenuRole m_role;
	int m_option;
};

}

#endif
