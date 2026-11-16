// SPDX-FileCopyrightText: 2026 Alexandre Petitjean
// SPDX-License-Identifier: MIT
#include "systemTrayIcon.h"

// Qt headers
#include <QIcon>
#include <QQmlFile>

namespace kemai
{
    TrayMenuItem::TrayMenuItem(QObject *parent):
    QObject(parent),
    m_action(new QAction(this))
    {
        connect(m_action, &QAction::triggered, this, &TrayMenuItem::triggered);
    }

    QString TrayMenuItem::text() const
    {
        return m_action->text();
    }

    void TrayMenuItem::setText(const QString &text)
    {
        if (text == m_action->text())
        {
            return;
        }
        m_action->setText(text);
        emit textChanged();
    }

    bool TrayMenuItem::isEnabled() const
    {
        return m_action->isEnabled();
    }

    void TrayMenuItem::setEnabled(bool enabled)
    {
        if (enabled == m_action->isEnabled())
        {
            return;
        }
        m_action->setEnabled(enabled);
        emit enabledChanged();
    }

    bool TrayMenuItem::isSeparator() const
    {
        return m_action->isSeparator();
    }

    void TrayMenuItem::setSeparator(bool separator)
    {
        if (separator == m_action->isSeparator())
        {
            return;
        }
        m_action->setSeparator(separator);
        emit separatorChanged();
    }

    QAction *TrayMenuItem::action()
    {
        return m_action;
    }

    SystemTrayIcon::SystemTrayIcon(QObject *parent):
    QObject(parent),
    m_menu(std::make_unique<QMenu>()),
    m_tray(std::make_unique<QSystemTrayIcon>())
    {
        m_tray->setContextMenu(m_menu.get());
        connect(m_tray.get(), &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason)
                { emit activated(static_cast<ActivationReason>(reason)); });
        connect(m_menu.get(), &QMenu::aboutToShow, this, &SystemTrayIcon::menuAboutToShow);
    }

    SystemTrayIcon::~SystemTrayIcon() = default;

    bool SystemTrayIcon::isAvailable() const
    {
        return QSystemTrayIcon::isSystemTrayAvailable();
    }

    bool SystemTrayIcon::isVisible() const
    {
        return m_visible;
    }

    void SystemTrayIcon::setVisible(bool visible)
    {
        if (visible == m_visible)
        {
            return;
        }
        m_visible = visible;
        updateVisibility();
        emit visibleChanged();
    }

    QUrl SystemTrayIcon::iconSource() const
    {
        return m_iconSource;
    }

    void SystemTrayIcon::setIconSource(const QUrl &source)
    {
        if (source == m_iconSource)
        {
            return;
        }
        m_iconSource = source;
        updateIcon();
        emit iconSourceChanged();
    }

    bool SystemTrayIcon::iconMask() const
    {
        return m_iconMask;
    }

    void SystemTrayIcon::setIconMask(bool mask)
    {
        if (mask == m_iconMask)
        {
            return;
        }
        m_iconMask = mask;
        updateIcon();
        emit iconMaskChanged();
    }

    QString SystemTrayIcon::tooltip() const
    {
        return m_tray->toolTip();
    }

    void SystemTrayIcon::setTooltip(const QString &tooltip)
    {
        if (tooltip == m_tray->toolTip())
        {
            return;
        }
        m_tray->setToolTip(tooltip);
        emit tooltipChanged();
    }

    QQmlListProperty<QObject> SystemTrayIcon::data()
    {
        return {this, nullptr, &SystemTrayIcon::appendData, &SystemTrayIcon::dataCount, &SystemTrayIcon::dataAt, nullptr};
    }

    void SystemTrayIcon::classBegin()
    {
    }

    void SystemTrayIcon::componentComplete()
    {
        // Shown once its icon, tooltip and menu are set: no empty icon flashes in the tray
        m_completed = true;
        updateVisibility();
    }

    void SystemTrayIcon::appendData(QQmlListProperty<QObject> *list, QObject *object)
    {
        auto *self = static_cast<SystemTrayIcon *>(list->object);
        self->m_data.append(object);
        if (auto *item = qobject_cast<TrayMenuItem *>(object); item != nullptr)
        {
            self->m_menu->addAction(item->action());
        }
    }

    qsizetype SystemTrayIcon::dataCount(QQmlListProperty<QObject> *list)
    {
        return static_cast<SystemTrayIcon *>(list->object)->m_data.size();
    }

    QObject *SystemTrayIcon::dataAt(QQmlListProperty<QObject> *list, qsizetype index)
    {
        return static_cast<SystemTrayIcon *>(list->object)->m_data.at(index);
    }

    void SystemTrayIcon::updateIcon()
    {
        QIcon icon(QQmlFile::urlToLocalFileOrQrc(m_iconSource));
        icon.setIsMask(m_iconMask);
        m_tray->setIcon(icon);
    }

    void SystemTrayIcon::updateVisibility()
    {
        if (m_completed)
        {
            m_tray->setVisible(m_visible);
        }
    }
} // namespace kemai
