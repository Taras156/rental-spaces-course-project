#include "thememanager.h"

#include <QApplication>

ThemeManager::Theme ThemeManager::s_currentTheme = ThemeManager::Theme4_DarkBlue;

void ThemeManager::applyTheme(QApplication* app, Theme theme)
{
    if (!app)
        return;

    s_currentTheme = theme;
    app->setStyleSheet(styleForTheme(theme));
}

void ThemeManager::toggleTheme(QApplication* app)
{
    if (s_currentTheme == Theme4_DarkBlue)
        applyTheme(app, Theme9_SkyLight);
    else
        applyTheme(app, Theme4_DarkBlue);
}

ThemeManager::Theme ThemeManager::currentTheme()
{
    return s_currentTheme;
}

QString ThemeManager::currentThemeName()
{
    return s_currentTheme == Theme4_DarkBlue
            ? QString::fromUtf8("Тема 4: Темно-синий кабинет")
            : QString::fromUtf8("Тема 9: Небесный минимализм");
}

QString ThemeManager::switchButtonText()
{
    if (s_currentTheme == Theme4_DarkBlue)
        return QString::fromUtf8("Переключить на тему 9");
    return QString::fromUtf8("Переключить на тему 4");
}

QString ThemeManager::styleForTheme(Theme theme)
{
    if (theme == Theme9_SkyLight)
    {
        return QString(
            "QWidget {"
            "  background-color: #EFF6FF;"
            "  color: #0F172A;"
            "  font-family: 'Segoe UI';"
            "  font-size: 13px;"
            "}"
            "QMainWindow, QDialog {"
            "  background-color: #EFF6FF;"
            "}"
            "QLabel#MainTitle {"
            "  color: #0C4A6E;"
            "  font-size: 28px;"
            "  font-weight: 700;"
            "}"
            "QLabel#SectionTitle {"
            "  color: #0C4A6E;"
            "  font-size: 18px;"
            "  font-weight: 700;"
            "}"
            "QLabel#SoftText, QLabel#HintLabel {"
            "  color: #334155;"
            "}"
            "QGroupBox {"
            "  border: 1px solid #BFDBFE;"
            "  border-radius: 12px;"
            "  margin-top: 12px;"
            "  padding-top: 12px;"
            "  background-color: #FFFFFF;"
            "  font-weight: 600;"
            "}"
            "QGroupBox::title {"
            "  subcontrol-origin: margin;"
            "  left: 12px;"
            "  padding: 0 6px;"
            "  color: #0C4A6E;"
            "}"
            "QPushButton {"
            "  background-color: #38BDF8;"
            "  color: #FFFFFF;"
            "  border: none;"
            "  border-radius: 10px;"
            "  padding: 10px 14px;"
            "  font-weight: 600;"
            "}"
            "QPushButton:hover { background-color: #1EA7E1; }"
            "QPushButton:pressed { background-color: #0EA5E9; }"
            "QPushButton:disabled { background-color: #CBD5E1; color: #F8FAFC; }"
            "QLineEdit, QDateEdit, QSpinBox, QComboBox, QTextEdit {"
            "  background-color: #FFFFFF;"
            "  color: #0F172A;"
            "  border: 1px solid #93C5FD;"
            "  border-radius: 9px;"
            "  padding: 8px;"
            "  selection-background-color: #38BDF8;"
            "}"
            "QLineEdit:focus, QDateEdit:focus, QSpinBox:focus, QComboBox:focus, QTextEdit:focus {"
            "  border: 2px solid #38BDF8;"
            "}"
            "QTableWidget {"
            "  background-color: #FFFFFF;"
            "  alternate-background-color: #F8FBFF;"
            "  color: #0F172A;"
            "  border: 1px solid #BFDBFE;"
            "  border-radius: 10px;"
            "  gridline-color: #DBEAFE;"
            "}"
            "QHeaderView::section {"
            "  background-color: #E0F2FE;"
            "  color: #0C4A6E;"
            "  padding: 8px;"
            "  border: none;"
            "  border-bottom: 1px solid #BFDBFE;"
            "  font-weight: 700;"
            "}"
            "QTableWidget::item:selected {"
            "  background-color: #BAE6FD;"
            "  color: #0F172A;"
            "}"
            "QTabWidget::pane {"
            "  border: 1px solid #BFDBFE;"
            "  border-radius: 12px;"
            "  background: #FFFFFF;"
            "}"
            "QTabBar::tab {"
            "  background: #DBEAFE;"
            "  color: #0C4A6E;"
            "  padding: 10px 16px;"
            "  margin-right: 4px;"
            "  border-top-left-radius: 8px;"
            "  border-top-right-radius: 8px;"
            "  font-weight: 600;"
            "}"
            "QTabBar::tab:selected {"
            "  background: #38BDF8;"
            "  color: #FFFFFF;"
            "}"
            "QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; }"
            "QDialogButtonBox QPushButton { min-width: 90px; }"
            "QMessageBox QLabel { color: #0F172A; }"
            "QToolTip { background-color: #FFFFFF; color: #0F172A; border: 1px solid #93C5FD; }"
        );
    }

    return QString(
        "QWidget {"
        "  background-color: #0F172A;"
        "  color: #F8FAFC;"
        "  font-family: 'Segoe UI';"
        "  font-size: 13px;"
        "}"
        "QMainWindow, QDialog {"
        "  background-color: #0F172A;"
        "}"
        "QLabel#MainTitle {"
        "  color: #F8FAFC;"
        "  font-size: 28px;"
        "  font-weight: 700;"
        "}"
        "QLabel#SectionTitle {"
        "  color: #CBD5E1;"
        "  font-size: 18px;"
        "  font-weight: 700;"
        "}"
        "QLabel#SoftText, QLabel#HintLabel {"
        "  color: #CBD5E1;"
        "}"
        "QGroupBox {"
        "  border: 1px solid #334155;"
        "  border-radius: 12px;"
        "  margin-top: 12px;"
        "  padding-top: 12px;"
        "  background-color: #1E293B;"
        "  font-weight: 600;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  left: 12px;"
        "  padding: 0 6px;"
        "  color: #CBD5E1;"
        "}"
        "QPushButton {"
        "  background-color: #3B82F6;"
        "  color: #FFFFFF;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 10px 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #2563EB; }"
        "QPushButton:pressed { background-color: #1D4ED8; }"
        "QPushButton:disabled { background-color: #475569; color: #CBD5E1; }"
        "QLineEdit, QDateEdit, QSpinBox, QComboBox, QTextEdit {"
        "  background-color: #F8FAFC;"
        "  color: #0F172A;"
        "  border: 1px solid #94A3B8;"
        "  border-radius: 9px;"
        "  padding: 8px;"
        "  selection-background-color: #93C5FD;"
        "}"
        "QLineEdit:focus, QDateEdit:focus, QSpinBox:focus, QComboBox:focus, QTextEdit:focus {"
        "  border: 2px solid #3B82F6;"
        "}"
        "QTableWidget {"
        "  background-color: #1E293B;"
        "  alternate-background-color: #233044;"
        "  color: #F8FAFC;"
        "  border: 1px solid #334155;"
        "  border-radius: 10px;"
        "  gridline-color: #334155;"
        "}"
        "QHeaderView::section {"
        "  background-color: #243244;"
        "  color: #F8FAFC;"
        "  padding: 8px;"
        "  border: none;"
        "  border-bottom: 1px solid #334155;"
        "  font-weight: 700;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #3B82F6;"
        "  color: #FFFFFF;"
        "}"
        "QTabWidget::pane {"
        "  border: 1px solid #334155;"
        "  border-radius: 12px;"
        "  background: #1E293B;"
        "}"
        "QTabBar::tab {"
        "  background: #243244;"
        "  color: #CBD5E1;"
        "  padding: 10px 16px;"
        "  margin-right: 4px;"
        "  border-top-left-radius: 8px;"
        "  border-top-right-radius: 8px;"
        "  font-weight: 600;"
        "}"
        "QTabBar::tab:selected {"
        "  background: #3B82F6;"
        "  color: #FFFFFF;"
        "}"
        "QScrollArea, QScrollArea > QWidget > QWidget { background: transparent; }"
        "QDialogButtonBox QPushButton { min-width: 90px; }"
        "QMessageBox QLabel { color: #F8FAFC; }"
        "QToolTip { background-color: #1E293B; color: #F8FAFC; border: 1px solid #475569; }"
    );
}
