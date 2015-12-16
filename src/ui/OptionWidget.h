/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2015 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
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

#ifndef OTTER_OPTIONWIDGET_H
#define OTTER_OPTIONWIDGET_H

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>

namespace Otter
{

class ColorWidget;
class FilePathWidget;
class IconWidget;

class OptionWidget : public QWidget
{
	Q_OBJECT

public:
	enum OptionType
	{
		UnknownType = 0,
		BooleanType,
		ColorType,
		EnumerationType,
		FontType,
		IconType,
		IntegerType,
		PathType,
		StringType
	};

	struct EnumerationChoice
	{
		QString text;
		QString value;
		QIcon icon;
	};

	explicit OptionWidget(const QString &option, const QVariant &value, OptionType type, QWidget *parent = NULL);

	void setIndex(const QModelIndex &index);
	void setChoices(const QStringList &choices);
	void setChoices(const QList<EnumerationChoice> &choices);
	void setControlsVisible(bool isVisible);
	QString getOption() const;
	QVariant getValue() const;
	QModelIndex getIndex() const;

protected:
	void focusInEvent(QFocusEvent *event);

protected slots:
	void markModified();
	void reset();
	void save();

private:
	QWidget *m_widget;
	ColorWidget *m_colorWidget;
	FilePathWidget *m_filePathWidget;
	IconWidget *m_iconWidget;
	QComboBox *m_comboBox;
	QFontComboBox *m_fontComboBox;
	QLineEdit *m_lineEdit;
	QSpinBox *m_spinBox;
	QPushButton *m_resetButton;
	QPushButton *m_saveButton;
	QString m_option;
	QVariant m_value;
	QModelIndex m_index;

signals:
	void commitData(QWidget *editor);
};

}

#endif
