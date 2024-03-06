/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QMenu>

#include <KServiceGroup>

#include <plasma/containmentactions.h>

#include "ui_config.h"

class QAction;

class CustomDesktopMenu : public Plasma::ContainmentActions
{
    Q_OBJECT
public:
    CustomDesktopMenu(QObject *parent, const QVariantList &args);
    ~CustomDesktopMenu() override;

    void init(const KConfigGroup &config);

    QList<QAction *> contextualActions() override;

    QWidget *createConfigurationInterface(QWidget *parent) override;
    void configurationAccepted() override;

    void restore(const KConfigGroup &config) override;
    void save(KConfigGroup &config) override;

protected:
    // void makeMenu(QMenu *menu, const KServiceGroup::Ptr &group);

private:
    KServiceGroup::Ptr m_group;
    QList<QAction *> m_actions;

    Ui::Config m_ui;
    bool m_showAppsByName = true;

    QString menuconfig;
    QString getDefaultMenu();
    // QTextEdit *configtextbox;
    QIcon getIcon(const QString &txt);
    void addSep(QMenu *menu);
    void addItm(QMenu *menu, const QString &icon, const QString &txt);
    void addCmd(QMenu *menu, const QString &icon, const QString &txt, const QString &cmd);
    QMenu* addMnu(QMenu *menu, const QString &icon, const QString &txt);
    void addApp(QMenu *menu, const QString &path);
    void fillPrograms(QMenu *menu, const QString &path);
    void fillFavorites(QMenu *menu);
};
