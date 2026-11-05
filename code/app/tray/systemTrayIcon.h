// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#pragma once

// STL headers
#include <memory>

// Qt headers
#include <QAction>
#include <QMenu>
#include <QObject>
#include <QQmlListProperty>
#include <QQmlParserStatus>
#include <QSystemTrayIcon>
#include <QUrl>
#include <QtQmlIntegration/qqmlintegration.h>

namespace kemai
{
    /*!
     * One entry of the tray menu, in the order it is declared in its SystemTrayIcon.
     */
    class TrayMenuItem : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

        Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
        Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
        Q_PROPERTY(bool separator READ isSeparator WRITE setSeparator NOTIFY separatorChanged)

    public:
        explicit TrayMenuItem(QObject *parent = nullptr);

        QString text() const;
        void    setText(const QString &text);
        bool    isEnabled() const;
        void    setEnabled(bool enabled);
        bool    isSeparator() const;
        void    setSeparator(bool separator);

        QAction *action();

    signals:
        void textChanged();
        void enabledChanged();
        void separatorChanged();
        void triggered();

    private:
        QAction *m_action;
    };

    /*!
     * Tray icon and its menu, over QSystemTrayIcon: the platform's native tray where there is one (Windows notification
     * area, macOS status item, StatusNotifierItem on Linux, through the platform theme on KDE). Qt.labs.platform used to
     * provide it without Qt Widgets, but KDE's platform theme builds its tray on widgets and aborts the app without a
     * QApplication. Children: TrayMenuItem entries make the menu; other objects (Timer, Connections) are only kept.
     */
    class SystemTrayIcon : public QObject, public QQmlParserStatus
    {
        Q_OBJECT
        QML_ELEMENT
        Q_INTERFACES(QQmlParserStatus)

        // Whether the desktop has a tray area, read once: a tray host lost or appearing later is not detected
        Q_PROPERTY(bool available READ isAvailable CONSTANT)
        Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
        Q_PROPERTY(QUrl iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged)
        // Monochrome template image, recolored by the system (macOS menu bar)
        Q_PROPERTY(bool iconMask READ iconMask WRITE setIconMask NOTIFY iconMaskChanged)
        Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged)
        Q_PROPERTY(QQmlListProperty<QObject> data READ data)
        Q_CLASSINFO("DefaultProperty", "data")

    public:
        enum ActivationReason
        {
            Unknown     = QSystemTrayIcon::Unknown,
            Context     = QSystemTrayIcon::Context,
            DoubleClick = QSystemTrayIcon::DoubleClick,
            Trigger     = QSystemTrayIcon::Trigger,
            MiddleClick = QSystemTrayIcon::MiddleClick
        };
        Q_ENUM(ActivationReason)

        explicit SystemTrayIcon(QObject *parent = nullptr);
        ~SystemTrayIcon() override;

        bool    isAvailable() const;
        bool    isVisible() const;
        void    setVisible(bool visible);
        QUrl    iconSource() const;
        void    setIconSource(const QUrl &source);
        bool    iconMask() const;
        void    setIconMask(bool mask);
        QString tooltip() const;
        void    setTooltip(const QString &tooltip);

        QQmlListProperty<QObject> data();

        void classBegin() override;
        void componentComplete() override;

    signals:
        void visibleChanged();
        void iconSourceChanged();
        void iconMaskChanged();
        void tooltipChanged();
        void activated(kemai::SystemTrayIcon::ActivationReason reason);
        void menuAboutToShow();

    private:
        static void      appendData(QQmlListProperty<QObject> *list, QObject *object);
        static qsizetype dataCount(QQmlListProperty<QObject> *list);
        static QObject  *dataAt(QQmlListProperty<QObject> *list, qsizetype index);

        void updateIcon();
        void updateVisibility();

        // Declared before the icon, which only refers to it: destroyed after it
        std::unique_ptr<QMenu>           m_menu;
        std::unique_ptr<QSystemTrayIcon> m_tray;
        QList<QObject *>                 m_data;
        QUrl                             m_iconSource;
        bool                             m_iconMask  = false;
        bool                             m_visible   = true;
        bool                             m_completed = false;
    };
} // namespace kemai
