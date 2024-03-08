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

private:
    KServiceGroup::Ptr m_group;
    QList<QAction *> m_actions;
    QList<QMenu *> m_menuList;

    Ui::Config m_ui;
    QString m_menuConfig;
    bool m_showAppsByName = true;

    QString getDefaultConfig();
    void parseConfig();
    void addAction(QAction *action);
    void fillPrograms(const QString &path);
};
