#pragma once

#include <QMenu>
#include <QSet>
#include <QAbstractItemModel>
#include <KServiceGroup>
#ifdef HAVE_KACTIVITIES
#include <PlasmaActivities/Stats/ResultModel>
#include <PlasmaActivities/Stats/Terms>
#endif
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
    int m_maxFavorites = 10;

    QString getDefaultConfig();
    void parseConfig();
    void addAction(QAction *action);
    void fillPrograms(const QString &path);
    void fillFavorites();
    QString getCurrentActivityId();
    QString convertFavoriteToDesktopPath(const QString &favorite);
    bool addFavoriteApplication(const QString &desktopPath);
    
    // Optimized methods
#ifdef HAVE_KACTIVITIES
    int tryKActivitiesAPI(const QString &currentActivity);
    QString extractDesktopPath(const QString &resource);
#endif
    int tryManualFavorites(const QString &currentActivity);
    void addCommonApplications();
    
    // Default application detection methods
    QString findDefaultBrowser();
    QString findDefaultFileManager();
    QString findDefaultTerminal();
};
