#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QString>

class QApplication;

class ThemeManager
{
public:
    enum Theme
    {
        Theme4_DarkBlue = 4,
        Theme9_SkyLight = 9
    };

    static void applyTheme(QApplication* app, Theme theme);
    static void toggleTheme(QApplication* app);
    static Theme currentTheme();
    static QString currentThemeName();
    static QString switchButtonText();

private:
    static QString styleForTheme(Theme theme);
    static Theme s_currentTheme;
};

#endif // THEMEMANAGER_H
