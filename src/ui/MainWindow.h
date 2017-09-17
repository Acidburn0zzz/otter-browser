/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2017 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 - 2015 Piotr Wójcik <chocimier@tlen.pl>
* Copyright (C) 2015 Jan Bajer aka bajasoft <jbajer@gmail.com>
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

#ifndef OTTER_MAINWINDOW_H
#define OTTER_MAINWINDOW_H

#include "../core/ActionsManager.h"
#include "../core/SessionsManager.h"

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QShortcut>

namespace Otter
{

namespace Ui
{
	class MainWindow;
}

class BookmarksItem;
class ContentsWidget;
class MenuBarWidget;
class Shortcut;
class StatusBarWidget;
class TabBarWidget;
class TabSwitcherWidget;
class ToolBarWidget;
class Window;
class WorkspaceWidget;

class MainWindow final : public QMainWindow, public ActionExecutor
{
	Q_OBJECT

public:
	explicit MainWindow(const QVariantMap &parameters = {}, const SessionMainWindow &session = SessionMainWindow(), QWidget *parent = nullptr);
	~MainWindow();

	void restore(const SessionMainWindow &session);
	void restore(int index = 0);
	void moveWindow(Window *window, MainWindow *mainWindow = nullptr, int index = -1);
	static MainWindow* findMainWindow(QObject *parent);
	TabBarWidget* getTabBar() const;
	Window* getActiveWindow() const;
	Window* getWindowByIndex(int index) const;
	Window* getWindowByIdentifier(quint64 identifier) const;
	QVariant getOption(int identifier) const;
	QString getTitle() const;
	QUrl getUrl() const;
	ActionsManager::ActionDefinition::State getActionState(int identifier, const QVariantMap &parameters = {}) const override;
	SessionMainWindow getSession() const;
	QVector<ToolBarWidget*> getToolBars(Qt::ToolBarArea area) const;
	QVector<ClosedWindow> getClosedWindows() const;
	quint64 getIdentifier() const;
	int getCurrentWindowIndex() const;
	int getWindowCount() const;
	int getWindowIndex(quint64 identifier) const;
	bool hasUrl(const QUrl &url, bool activate = false);
	bool isAboutToClose() const;
	bool isPrivate() const;
	bool eventFilter(QObject *object, QEvent *event) override;

public slots:
	void triggerAction(int identifier, const QVariantMap &parameters = {}) override;
	void storeWindowState();
	void restoreWindowState();
	void raiseWindow();
	void search(const QString &query, const QString &searchEngine, SessionsManager::OpenHints hints = SessionsManager::DefaultOpen);
	void clearClosedWindows();
	void addWindow(Window *window, SessionsManager::OpenHints hints = SessionsManager::DefaultOpen, int index = -1, const WindowState &state = WindowState(), bool isAlwaysOnTop = false);
	void setActiveWindowByIndex(int index, bool updateLastActivity = true);
	void setActiveWindowByIdentifier(quint64 identifier, bool updateLastActivity = true);
	void setOption(int identifier, const QVariant &value);

protected:
	void timerEvent(QTimerEvent *event) override;
	void closeEvent(QCloseEvent *event) override;
	void keyPressEvent(QKeyEvent *event) override;
	void keyReleaseEvent(QKeyEvent *event) override;
	void contextMenuEvent(QContextMenuEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void beginToolBarDragging(bool isSidebar = false);
	void endToolBarDragging();
	QVector<quint64> createOrderedWindowList(bool includeMinimized) const;
	bool event(QEvent *event) override;

protected slots:
	void saveToolBarPositions();
	void open(const QUrl &url = {}, SessionsManager::OpenHints hints = SessionsManager::DefaultOpen);
	void open(BookmarksItem *bookmark, SessionsManager::OpenHints hints = SessionsManager::DefaultOpen);
	void removeStoredUrl(const QString &url);
	void handleOptionChanged(int identifier);
	void handleWindowClose(Window *window);
	void handleWindowIsPinnedChanged(bool isPinned);
	void handleToolBarAdded(int identifier);
	void handleToolBarModified(int identifier);
	void handleToolBarMoved(int identifier);
	void handleToolBarRemoved(int identifier);
	void handleTransferStarted();
	void setCurrentWindow(Window *window);
	void setStatusMessage(const QString &message);
	void updateWindowTitle();
	void updateShortcuts();
	Window* openWindow(ContentsWidget *widget, SessionsManager::OpenHints hints = SessionsManager::DefaultOpen, int index = -1);

private:
	TabSwitcherWidget *m_tabSwitcher;
	WorkspaceWidget *m_workspace;
	TabBarWidget *m_tabBar;
	MenuBarWidget *m_menuBar;
	StatusBarWidget *m_statusBar;
	Window *m_currentWindow;
	QString m_currentBookmark;
	QString m_windowTitle;
	QVector<Shortcut*> m_shortcuts;
	QVector<Window*> m_privateWindows;
	QVector<ClosedWindow> m_closedWindows;
	QVector<quint64> m_tabSwitchingOrderList;
	QHash<quint64, Window*> m_windows;
	Qt::WindowStates m_previousState;
	Qt::WindowStates m_previousRaisedState;
	quint64 m_identifier;
	int m_mouseTrackerTimer;
	int m_tabSwitchingOrderIndex;
	bool m_isAboutToClose;
	bool m_isDraggingToolBar;
	bool m_isPrivate;
	bool m_isRestored;
	bool m_hasToolBars;
	Ui::MainWindow *m_ui;

	static quint64 m_identifierCounter;

signals:
	void activated(MainWindow *window);
	void statusMessageChanged(const QString &message);
	void titleChanged(const QString &title);
	void windowAdded(quint64 identifier);
	void windowRemoved(quint64 identifier);
	void currentWindowChanged(quint64 identifier);
	void closedWindowsAvailableChanged(bool available);
	void areToolBarsVisibleChanged(bool areVisible);
	void actionsStateChanged();
	void actionsStateChanged(const QVector<int> &identifiers);
	void actionsStateChanged(ActionsManager::ActionDefinition::ActionCategories categories);

friend class ToolBarDropZoneWidget;
friend class ToolBarWidget;
};

class Shortcut final : public QShortcut
{
	Q_OBJECT

public:
	explicit Shortcut(int identifier, const QKeySequence &sequence, const QVariantMap &parameters, MainWindow *parent);

protected slots:
	void triggerAction();

private:
	MainWindow *m_mainWindow;
	QVariantMap m_parameters;
	int m_identifier;
};

}

#endif
