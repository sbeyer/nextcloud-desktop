/*
 * Copyright (C) by Cédric Bellegarde <gnumdk@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 */

#include "accountmanager.h"
#include "systray.h"
#include "theme.h"
#include "config.h"
#include "common/utility.h"
#include "tray/UserModel.h"

#include <QCursor>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QScreen>

#ifdef USE_FDO_NOTIFICATIONS
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCall>
#define NOTIFICATIONS_SERVICE "org.freedesktop.Notifications"
#define NOTIFICATIONS_PATH "/org/freedesktop/Notifications"
#define NOTIFICATIONS_IFACE "org.freedesktop.Notifications"
#endif

namespace OCC {

Systray *Systray::_instance = nullptr;

Systray *Systray::instance()
{
    if (_instance == nullptr) {
        _instance = new Systray();
    }
    return _instance;
}

Systray::Systray()
    : _trayEngine(new QQmlApplicationEngine(this))
{
    _trayEngine->addImportPath("qrc:/qml/theme");
    _trayEngine->addImageProvider("avatars", new ImageProvider);
    _trayEngine->rootContext()->setContextProperty("userModelBackend", UserModel::instance());
    _trayEngine->rootContext()->setContextProperty("appsMenuModelBackend", UserAppsModel::instance());
    _trayEngine->rootContext()->setContextProperty("systrayBackend", this);

    connect(UserModel::instance(), &UserModel::newUserSelected,
        this, &Systray::slotNewUserSelected);

    connect(AccountManager::instance(), &AccountManager::accountAdded,
        this, &Systray::showWindow);

    qmlRegisterUncreatableType<Systray>("com.nextcloud.gui", 1, 0, "Systray", "This type is uncreatable, it is exported for its enums");
}

void Systray::create()
{
    if (!AccountManager::instance()->accounts().isEmpty()) {
        _trayEngine->rootContext()->setContextProperty("activityModel", UserModel::instance()->currentActivityModel());
    }
    _trayEngine->load(QStringLiteral("qrc:/qml/src/gui/tray/Window.qml"));
    hideWindow();
    emit activated(QSystemTrayIcon::ActivationReason::Unknown);
}

void Systray::slotNewUserSelected()
{
    // Change ActivityModel
    _trayEngine->rootContext()->setContextProperty("activityModel", UserModel::instance()->currentActivityModel());

    // Rebuild App list
    UserAppsModel::instance()->buildAppList();
}

bool Systray::isOpen()
{
    return _isOpen;
}

Q_INVOKABLE void Systray::setOpened()
{
    _isOpen = true;
}

Q_INVOKABLE void Systray::setClosed()
{
    _isOpen = false;
}

void Systray::showMessage(const QString &title, const QString &message, MessageIcon icon)
{
#ifdef USE_FDO_NOTIFICATIONS
    if (QDBusInterface(NOTIFICATIONS_SERVICE, NOTIFICATIONS_PATH, NOTIFICATIONS_IFACE).isValid()) {
        const QVariantMap hints = {{QStringLiteral("desktop-entry"), LINUX_APPLICATION_ID}};
        QList<QVariant> args = QList<QVariant>() << APPLICATION_NAME << quint32(0) << APPLICATION_ICON_NAME
                                                 << title << message << QStringList() << hints << qint32(-1);
        QDBusMessage method = QDBusMessage::createMethodCall(NOTIFICATIONS_SERVICE, NOTIFICATIONS_PATH, NOTIFICATIONS_IFACE, "Notify");
        method.setArguments(args);
        QDBusConnection::sessionBus().asyncCall(method);
    } else
#endif
#ifdef Q_OS_OSX
        if (canOsXSendUserNotification()) {
        sendOsXUserNotification(title, message);
    } else
#endif
    {
        QSystemTrayIcon::showMessage(title, message, icon);
    }
}

void Systray::setToolTip(const QString &tip)
{
    QSystemTrayIcon::setToolTip(tr("%1: %2").arg(Theme::instance()->appNameGUI(), tip));
}

bool Systray::syncIsPaused()
{
    return _syncIsPaused;
}

void Systray::pauseResumeSync()
{
    if (_syncIsPaused) {
        _syncIsPaused = false;
        emit resumeSync();
    } else {
        _syncIsPaused = true;
        emit pauseSync();
    }
}

/********************************************************************************************/
/* Helper functions for cross-platform tray icon position and taskbar orientation detection */
/********************************************************************************************/

/// Return the current screen index based on cursor position
int Systray::screenIndex()
{
    auto qPos = QCursor::pos();
    for (int i = 0; i < QGuiApplication::screens().count(); i++) {
        if (QGuiApplication::screens().at(i)->geometry().contains(qPos)) {
            return i;
        }
    }
    return 0;
}

Systray::TaskBarPosition Systray::taskbarOrientation()
{
// macOS: Always on top
#if defined(Q_OS_MACOS)
    return TaskBarPosition::Top;
// Windows: Check registry for actual taskbar orientation
#elif defined(Q_OS_WIN)
    auto taskbarGeometry = Utility::registryGetKeyValue(HKEY_CURRENT_USER,
        "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StuckRects3",
        "Settings");
    switch (taskbarGeometry.toInt()) {
    // Mapping windows binary value (0 = left, 1 = top, 2 = right, 3 = bottom) to qml logic (0 = bottom, 1 = left...)
    case 0:
        return TaskBarPosition::Left;
    case 1:
        return TaskBarPosition::Top;
    case 2:
        return TaskBarPosition::Right;
    case 3:
        return TaskBarPosition::Bottom;
    default:
        return TaskBarPosition::Bottom;
    }
// Probably Linux
#else
    auto currentScreen = screenIndex();
    auto screenWidth = QGuiApplication::screens().at(currentScreen)->geometry().width();
    auto screenHeight = QGuiApplication::screens().at(currentScreen)->geometry().height();
    auto virtualY = QGuiApplication::screens().at(currentScreen)->virtualGeometry().y();
    auto virtualX = QGuiApplication::screens().at(currentScreen)->virtualGeometry().x();
    QPoint trayIconCenter = calcTrayIconCenter();

    auto distBottom = screenHeight - (trayIconCenter.y() - virtualY);
    auto distRight = screenWidth - (trayIconCenter.x() - virtualX);
    auto distLeft = trayIconCenter.x() - virtualX;
    auto distTop = trayIconCenter.y() - virtualY;

    if (distBottom < distRight && distBottom < distTop && distBottom < distLeft) {
        return TaskBarPosition::Bottom;
    } else if (distLeft < distTop && distLeft < distRight && distLeft < distBottom) {
        return TaskBarPosition::Left;
    } else if (distTop < distRight && distTop < distBottom && distTop < distLeft) {
        return TaskBarPosition::Top;
    } else {
        return TaskBarPosition::Right;
    }
#endif
}

// TODO: Get real taskbar dimensions Linux as well
QRect Systray::taskbarRect()
{
#if defined(Q_OS_WIN)
    QRect tbRect = Utility::getTaskbarDimensions();
    //QML side expects effective pixels, convert taskbar dimensions if necessary
    auto pixelRatio = QGuiApplication::screens().at(screenIndex())->devicePixelRatio();
    if (pixelRatio != 1) {
        tbRect.setHeight(tbRect.height() / pixelRatio);
        tbRect.setWidth(tbRect.width() / pixelRatio);
    }
    return tbRect;
#elif defined(Q_OS_MACOS)
    // Finder bar is always 22px height on macOS (when treating as effective pixels)
    auto screenWidth = QGuiApplication::screens().at(screenIndex())->geometry().width();
    return QRect(0, 0, screenWidth, 22);
#else
    if (taskbarOrientation() == TaskBarPosition::Bottom || taskbarOrientation() == TaskBarPosition::Top) {
        auto screenWidth = QGuiApplication::screens().at(screenIndex())->geometry().width();
        return QRect(0, 0, screenWidth, 32);
    } else {
        auto screenHeight = QGuiApplication::screens().at(screenIndex())->geometry().height();
        return QRect(0, 0, 32, screenHeight);
    }
#endif
}

/// Returns a QPoint that constitutes the center coordinate of the tray icon
QPoint Systray::calcTrayIconCenter()
{
// QSystemTrayIcon::geometry() is broken for ages on most Linux DEs (invalid geometry returned)
// thus we can use this only for Windows and macOS
#if defined(Q_OS_WIN) || defined(Q_OS_MACOS)
    auto trayIconCenter = geometry().center();
    return trayIconCenter;
#else
// On Linux, fall back to mouse position (assuming tray icon is activated by mouse click)
    return QCursor::pos();
#endif
}

} // namespace OCC
