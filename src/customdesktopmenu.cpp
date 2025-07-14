#include "customdesktopmenu.h"

#include <KConfigGroup>
#include <KIO/ApplicationLauncherJob>
#include <KPluginFactory>
#include <KProcess>
#include <KDesktopFile>
#include <KConfig>
#include <KService>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QSettings>
#ifdef HAVE_KACTIVITIES
#include <PlasmaActivities/Stats/ResultModel>
#include <PlasmaActivities/Stats/Terms>
#include <PlasmaActivities/Controller>
#include <PlasmaActivities/Stats/Query>
#endif

CustomDesktopMenu::CustomDesktopMenu(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
    , m_group(new KServiceGroup(QStringLiteral("/")))
{
}
CustomDesktopMenu::~CustomDesktopMenu()
{
}

void CustomDesktopMenu::init(const KConfigGroup &)
{
}

QList<QAction *> CustomDesktopMenu::contextualActions()
{
  qDeleteAll(m_actions);
  m_actions.clear();
  
  parseConfig();
  
  return m_actions;
}

QWidget *CustomDesktopMenu::createConfigurationInterface(QWidget *parent)
{
  QWidget *widget = new QWidget(parent);
  m_ui.setupUi(widget);
  widget->setWindowTitle(i18nc("plasma_containmentactions_customdesktopmenu", "Configure Custom Desktop Menu Plugin"));
  widget->setFixedWidth(660);
  widget->setFixedHeight(400);
  m_ui.configData->setPlainText(m_menuConfig);
  m_ui.showAppsByName->setChecked(m_showAppsByName);
  return widget;
}

void CustomDesktopMenu::configurationAccepted()
{
  m_menuConfig = m_ui.configData->document()->toPlainText();
  m_showAppsByName = m_ui.showAppsByName->isChecked();
}

void CustomDesktopMenu::restore(const KConfigGroup &config)
{
  m_menuConfig = config.readEntry(QStringLiteral("menuConfig"), getDefaultConfig());
  m_showAppsByName = config.readEntry(QStringLiteral("showAppsByName"), true);
}

void CustomDesktopMenu::save(KConfigGroup &config)
{
  config.writeEntry(QStringLiteral("menuConfig"), m_menuConfig);
  config.writeEntry(QStringLiteral("showAppsByName"), m_showAppsByName);
}

QString CustomDesktopMenu::getDefaultConfig()
{
  QString defMenuConfig = "{favorites}\n";
  defMenuConfig += "-\n";
  defMenuConfig += "[menu]\tApplications\tkde\n";
  defMenuConfig += "{programs}\n";
  defMenuConfig += "[end]\n";
  defMenuConfig += "-\n";
  defMenuConfig += "/usr/share/applications/org.kde.dolphin.desktop\n";
  defMenuConfig += "/usr/share/applications/org.kde.kate.desktop\n";
	defMenuConfig += "#/usr/share/applications/org.kde.kcalc.desktop\n";
  defMenuConfig += "-\n";
  defMenuConfig += "[menu]\tSystem\tconfigure-shortcuts\n";
  defMenuConfig += "{programs}\tSettingsmenu/\n";
  defMenuConfig += "-\n";
  defMenuConfig += "{programs}\tSystem/\n";
  defMenuConfig += "[end]\n";
  defMenuConfig += "[menu]\tExit\tsystem-shutdown\n";
  defMenuConfig += "Lock\tsystem-lock-screen\tqdbus6 org.kde.KWin /ScreenSaver Lock\n";
  defMenuConfig += "Disconnect\tsystem-log-out\tqdbus6 org.kde.LogoutPrompt /LogoutPrompt promptLogout\n";
  defMenuConfig += "Switch User\tsystem-switch-user\tqdbus6 org.kde.KWin /ScreenSaver org.kde.screensaver.SwitchUser\n";
  defMenuConfig += "-\n";
  defMenuConfig += "Sleep\tsystem-suspend\tqdbus6 org.freedesktop.PowerManagement /org/freedesktop/PowerManagement Suspend\n";
  defMenuConfig += "Hibernate\tsystem-suspend-hibernate\tqdbus6 org.freedesktop.PowerManagement /org/freedesktop/PowerManagement Hibernate\n";
  defMenuConfig += "-\n";
  defMenuConfig += "Restart\tsystem-reboot\tqdbus6 org.kde.LogoutPrompt /LogoutPrompt promptReboot\n";
  defMenuConfig += "Shut down\tsystem-shutdown\tqdbus6 org.kde.LogoutPrompt /LogoutPrompt promptShutDown\n";
  defMenuConfig += "[end]\n";
  defMenuConfig += "-\n";
  defMenuConfig += "/usr/share/applications/org.kde.plasma-systemmonitor.desktop\n";
  defMenuConfig += "/usr/share/applications/org.kde.konsole.desktop\n";
  return defMenuConfig;
}

void CustomDesktopMenu::parseConfig()
{
  if (m_menuConfig.isEmpty()) return;
  QStringList configLines = m_menuConfig.split("\n", Qt::SkipEmptyParts);
  foreach(QString cfgLine, configLines) {
    if (cfgLine.startsWith("#")) continue;
    QAction *action = nullptr;
    if (cfgLine.startsWith("-")) {
      // Separator
      action = new QAction(this);
      action->setSeparator(true);
    } else if (cfgLine.startsWith("[menu]")) {
      // Begin menu
      QStringList cfgParts = cfgLine.split("\t", Qt::SkipEmptyParts);
      QString text = "";
      QIcon icon;
      if (cfgParts.size() > 1) text = cfgParts[1].replace("&", "&&");
      if (cfgParts.size() > 2) icon = QIcon::fromTheme(cfgParts[2]);
      QMenu *subMenu = new QMenu();
      action = new QAction(icon, text, this);
      action->setMenu(subMenu);
      addAction(action);
      m_menuList.append(subMenu);
      action = nullptr;
    } else if (cfgLine.startsWith("[end]")) {
      // End menu
      m_menuList.removeLast();
    } else if (cfgLine.endsWith(".desktop")) {
      // .desktop file
      if(KDesktopFile::isDesktopFile(cfgLine) == true) {
        KDesktopFile desktopFile(cfgLine);
        QString text = desktopFile.readName();
        if (!m_showAppsByName && !desktopFile.readGenericName().isEmpty()) {
          text = desktopFile.readGenericName();
        }
        action = new QAction(QIcon::fromTheme(desktopFile.readIcon()), text, this);
        action->setData(cfgLine);
        connect(action, &QAction::triggered, [action](){
          KService::Ptr service = KService::serviceByDesktopPath(action->data().toString());
          auto job = new KIO::ApplicationLauncherJob(service);
          job->start();
        });
      } else {
        action = new QAction(cfgLine, this);
      }
    } else if (cfgLine.startsWith("{programs}")) {
      // programs
      QStringList cfgParts = cfgLine.split("\t", Qt::SkipEmptyParts);
      QString path = "/";
      if (cfgParts.size() > 1) path = cfgParts[1];
      fillPrograms(path);
    } else if (cfgLine == "{favorites}") {
      // Implementar favoritos usando KActivities::Stats
      fillFavorites();
    } else {
      // cmd
      QStringList cfgParts = cfgLine.split("\t", Qt::SkipEmptyParts);
      QString text = "";
      QIcon icon;
      QString cmd = "";
      text = cfgParts[0].replace("&", "&&");
      if (cfgParts.size() > 1) icon = QIcon::fromTheme(cfgParts[1]);
      action = new QAction(icon, text, this);
      if (cfgParts.size() > 2) {
        action->setData(cfgParts[2]);
        connect(action, &QAction::triggered, [action](){
          KProcess *process = new KProcess(0);
          process->startDetached(action->data().toString().split(" "));
        });
      }
    }
    addAction(action);
  }
}

void CustomDesktopMenu::addAction(QAction* action)
{
  if (action) {
    if (m_menuList.size() == 0) m_actions << action;
    else m_menuList.last()->addAction(action);
  }
}

void CustomDesktopMenu::fillPrograms(const QString& path)
{
  KServiceGroup::Ptr root = KServiceGroup::group(path);
  KServiceGroup::List list = root->entries(true, true, true);
  for (const KSycocaEntry::Ptr &p : list){
    QAction *action = nullptr;
    if(p->isType(KST_KService)) {
      if(KDesktopFile::isDesktopFile(p->entryPath()) == true) {
        KDesktopFile desktopFile(p->entryPath());
        QString text = p->name();
        if (!m_showAppsByName && !desktopFile.readGenericName().isEmpty()) {
          text = desktopFile.readGenericName();
        }
        action = new QAction(QIcon::fromTheme(desktopFile.readIcon()), text, this);
        action->setData(p->entryPath());
        connect(action, &QAction::triggered, [action](){
          KService::Ptr service = KService::serviceByDesktopPath(action->data().toString());
          auto job = new KIO::ApplicationLauncherJob(service);
          job->start();
        });
        addAction(action);
      } else {
        action = new QAction(p->entryPath(), this);
        addAction(action);
      }
    } else if(p->isType(KST_KServiceGroup)) {
      const KServiceGroup::Ptr service(static_cast<KServiceGroup *>(p.data()));
      if(service->childCount() == 0) continue;
      QString text = service->caption();
      QIcon icon = QIcon::fromTheme(service->icon());
      QMenu *subMenu = new QMenu();
      action = new QAction(icon, text, this);
      action->setMenu(subMenu);
      addAction(action);
      m_menuList.append(subMenu);
      fillPrograms(p->name());
      m_menuList.removeLast();
    } else if(p->isType(KST_KServiceSeparator)) {
      action = new QAction(this);
      action->setSeparator(true);
      addAction(action);
    }
  }
}

void CustomDesktopMenu::fillFavorites()
{
  // Get current activity
  QString currentActivity = getCurrentActivityId();
  if (currentActivity.isEmpty()) {
    addCommonApplications();
    return;
  }
  
  int addedCount = 0;
  
#ifdef HAVE_KACTIVITIES
  // LEVEL 1: Try KActivities API
  addedCount = tryKActivitiesAPI(currentActivity);
#endif
  
  // LEVEL 2: If KActivities didn't work, use optimized manual method
  if (addedCount == 0) {
    addedCount = tryManualFavorites(currentActivity);
  }
  
  // LEVEL 3: Last resort - common applications
  if (addedCount == 0) {
    addCommonApplications();
  }
}

#ifdef HAVE_KACTIVITIES
int CustomDesktopMenu::tryKActivitiesAPI(const QString &currentActivity)
{
  using namespace KActivities::Stats;
  using namespace KActivities::Stats::Terms;
  
  QStringList agents = {
    QStringLiteral("org.kde.plasma.favorites"),
    QStringLiteral("org.kde.plasma.kicker"),
    QStringLiteral("org.kde.plasma.kickoff")
  };
  
  int maxFavorites = m_maxFavorites > 0 ? m_maxFavorites : 10;
  
  for (const QString &agent : agents) {
    auto query = UsedResources
                | Agent(agent)
                | Type(QStringLiteral("application/x-desktop"))
                | Activity(currentActivity)
                | HighScoredFirst
                | Limit(maxFavorites);
    
    ResultModel *model = new ResultModel(query, this);
    
    if (model->rowCount() > 0) {
      int addedCount = 0;
      
      for (int i = 0; i < model->rowCount() && addedCount < maxFavorites; ++i) {
        QModelIndex index = model->index(i, 0);
        QString resource = model->data(index, ResultModel::ResourceRole).toString();
        
        QString desktopPath = extractDesktopPath(resource);
        if (addFavoriteApplication(desktopPath)) {
          addedCount++;
        }
      }
      
      delete model;
      
      if (addedCount > 0) {
        return addedCount;
      }
    }
    
    delete model;
  }
  
  return 0;
}

QString CustomDesktopMenu::extractDesktopPath(const QString &resource)
{
  if (resource.startsWith("applications:")) {
    QString desktopFile = resource.mid(13);
    QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + desktopFile;
      if (QFile::exists(fullPath)) {
        return fullPath;
      }
    }
  } else if (resource.startsWith("file://")) {
    QString filePath = resource.mid(7);
    if (filePath.endsWith(".desktop") && QFile::exists(filePath)) {
      return filePath;
    }
  }
  
  return QString();
}
#endif

int CustomDesktopMenu::tryManualFavorites(const QString &currentActivity)
{
  QString statsConfigPath = QDir::homePath() + "/.config/kactivitymanagerd-statsrc";
  
  if (!QFile::exists(statsConfigPath)) {
    return 0;
  }
  
  KConfig statsConfig(statsConfigPath, KConfig::SimpleConfig);
  QStringList groups = statsConfig.groupList();
  
  // Find the best section (with most favorites)
  QString bestSection;
  QStringList bestFavorites;
  int maxCount = 0;
  
  for (const QString &groupName : groups) {
    if (groupName.contains("Favorites") && 
        (groupName.contains("org.kde.plasma.kicker") || groupName.contains("org.kde.plasma.kickoff")) &&
        groupName.contains(currentActivity)) {
      
      KConfigGroup favoritesGroup = statsConfig.group(groupName);
      
      if (favoritesGroup.hasKey("ordering")) {
        QString orderingStr = favoritesGroup.readEntry("ordering", QString());
        
        if (!orderingStr.isEmpty()) {
          QStringList favorites = orderingStr.split(',', Qt::SkipEmptyParts);
          
          if (favorites.size() > maxCount) {
            maxCount = favorites.size();
            bestSection = groupName;
            bestFavorites = favorites;
          }
        }
      }
    }
  }
  
  if (bestFavorites.isEmpty()) {
    return 0;
  }
  
  // Process favorites from the best section
  int addedCount = 0;
  int maxFavorites = m_maxFavorites > 0 ? m_maxFavorites : 10;
  
  for (const QString &favorite : bestFavorites) {
    if (addedCount >= maxFavorites) break;
    
    QString cleanFavorite = favorite.trimmed();
    if (!cleanFavorite.isEmpty()) {
      QString desktopPath = convertFavoriteToDesktopPath(cleanFavorite);
      if (addFavoriteApplication(desktopPath)) {
        addedCount++;
      }
    }
  }
  
  return addedCount;
}

void CustomDesktopMenu::addCommonApplications()
{
  QStringList commonApps = {
    "/usr/share/applications/org.kde.dolphin.desktop",
    "/usr/share/applications/org.kde.kate.desktop", 
    "/usr/share/applications/org.kde.konsole.desktop",
    "/usr/share/applications/org.kde.plasma-systemmonitor.desktop"
  };
  
  int maxFavorites = m_maxFavorites > 0 ? m_maxFavorites : 4;
  int addedCount = 0;
  
  for (const QString &desktopPath : commonApps) {
    if (addedCount >= maxFavorites) break;
    
    if (addFavoriteApplication(desktopPath)) {
      addedCount++;
    }
  }
}

QString CustomDesktopMenu::convertFavoriteToDesktopPath(const QString &favorite)
{
  QString cleanFavorite = favorite.trimmed();
  
  if (cleanFavorite.startsWith("applications:")) {
    // Format: applications:org.kde.dolphin.desktop
    QString desktopFile = cleanFavorite.mid(13); // Remove "applications:"
    QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + desktopFile;
      if (QFile::exists(fullPath)) {
        return fullPath;
      }
    }
    
  } else if (cleanFavorite.startsWith("preferred://")) {
    // Format: preferred://browser, preferred://filemanager, etc.
    QString type = cleanFavorite.mid(12); // Remove "preferred://"
    
    if (type == "browser") {
      return findDefaultBrowser();
    } else if (type == "filemanager") {
      return findDefaultFileManager();
    } else if (type == "terminal") {
      return findDefaultTerminal();
    }
    
  } else if (cleanFavorite.endsWith(".desktop")) {
    // Direct format: org.kde.dolphin.desktop
    QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + cleanFavorite;
      if (QFile::exists(fullPath)) {
        return fullPath;
      }
    }
  }
  
  return QString();
}

QString CustomDesktopMenu::findDefaultBrowser()
{
  // METHOD 1: xdg-settings
  QProcess xdgProcess;
  xdgProcess.start("xdg-settings", QStringList() << "get" << "default-web-browser");
  xdgProcess.waitForFinished(3000);
  
  if (xdgProcess.exitCode() == 0) {
    QString defaultBrowser = QString::fromUtf8(xdgProcess.readAllStandardOutput()).trimmed();
    if (!defaultBrowser.isEmpty()) {
      QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
      for (const QString &path : searchPaths) {
        QString fullPath = path + "/" + defaultBrowser;
        if (QFile::exists(fullPath)) {
          return fullPath;
        }
      }
    }
  }
  
  // METHOD 2: mimeapps.list
  QString mimeAppsPath = QDir::homePath() + "/.config/mimeapps.list";
  if (QFile::exists(mimeAppsPath)) {
    QSettings mimeSettings(mimeAppsPath, QSettings::IniFormat);
    mimeSettings.beginGroup("Default Applications");
    
    QStringList browserMimes = {"text/html", "x-scheme-handler/http", "x-scheme-handler/https"};
    
    for (const QString &mimeType : browserMimes) {
      if (mimeSettings.contains(mimeType)) {
        QString browserDesktop = mimeSettings.value(mimeType).toString().split(';').first();
        if (!browserDesktop.isEmpty()) {
          QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
          for (const QString &path : searchPaths) {
            QString fullPath = path + "/" + browserDesktop;
            if (QFile::exists(fullPath)) {
              return fullPath;
            }
          }
        }
      }
    }
    mimeSettings.endGroup();
  }
  
  // METHOD 3: Known browsers (including Brave)
  QStringList knownBrowsers = {
    "brave-browser.desktop", "brave.desktop", "com.brave.Browser.desktop",
    "firefox.desktop", "org.mozilla.firefox.desktop", "google-chrome.desktop",
    "chromium.desktop", "org.kde.falkon.desktop", "opera.desktop"
  };
  
  QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
  
  for (const QString &browser : knownBrowsers) {
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + browser;
      if (QFile::exists(fullPath)) {
        // Verify it's a valid browser
        KDesktopFile desktopFile(fullPath);
        QString categories = desktopFile.desktopGroup().readEntry("Categories", QString());
        QString mimeTypes = desktopFile.desktopGroup().readEntry("MimeType", QString());
        
        if (categories.contains("WebBrowser") || 
            mimeTypes.contains("text/html") || 
            mimeTypes.contains("x-scheme-handler/http")) {
          return fullPath;
        }
      }
    }
  }
  
  return QString();
}

QString CustomDesktopMenu::findDefaultFileManager()
{
  QStringList fileManagers = {
    "org.kde.dolphin.desktop", "nautilus.desktop", "thunar.desktop",
    "nemo.desktop", "pcmanfm.desktop", "caja.desktop"
  };
  
  QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
  
  for (const QString &fm : fileManagers) {
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + fm;
      if (QFile::exists(fullPath)) {
        return fullPath;
      }
    }
  }
  
  return QString();
}

QString CustomDesktopMenu::findDefaultTerminal()
{
  QStringList terminals = {
    "org.kde.konsole.desktop", "gnome-terminal.desktop", "xfce4-terminal.desktop",
    "terminator.desktop", "alacritty.desktop", "kitty.desktop"
  };
  
  QStringList searchPaths = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
  
  for (const QString &term : terminals) {
    for (const QString &path : searchPaths) {
      QString fullPath = path + "/" + term;
      if (QFile::exists(fullPath)) {
        return fullPath;
      }
    }
  }
  
  return QString();
}



bool CustomDesktopMenu::addFavoriteApplication(const QString &desktopPath)
{
  if (desktopPath.isEmpty() || !QFile::exists(desktopPath)) {
    return false;
  }
  
  KDesktopFile desktopFile(desktopPath);
  if (desktopFile.desktopGroup().readEntry("NoDisplay", false) ||
      desktopFile.desktopGroup().readEntry("Hidden", false)) {
    return false;
  }
  
  QAction *action = new QAction(this);
  action->setText(m_showAppsByName ? desktopFile.readName() : desktopFile.readGenericName());
  action->setIcon(QIcon::fromTheme(desktopFile.readIcon()));
  action->setToolTip(desktopFile.readComment());
  action->setProperty("desktopPath", desktopPath);
  
  connect(action, &QAction::triggered, this, [this, desktopPath]() {
    KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(KService::serviceByDesktopPath(desktopPath));
    job->start();
  });
  
  addAction(action);
  return true;
}


QString CustomDesktopMenu::getCurrentActivityId()
{
  // Read current activity from kactivitymanagerdrc
  QString activityConfigPath = QDir::homePath() + "/.config/kactivitymanagerdrc";
  
  if (!QFile::exists(activityConfigPath)) {
    return QString();
  }
  
  KConfig activityConfig(activityConfigPath, KConfig::SimpleConfig);
  KConfigGroup mainGroup = activityConfig.group("main");
  
  if (mainGroup.hasKey("currentActivity")) {
    return mainGroup.readEntry("currentActivity", QString());
  }
  
  return QString();
}



K_PLUGIN_CLASS_WITH_JSON(CustomDesktopMenu, "plasma-containmentactions-customdesktopmenu.json")

#include "customdesktopmenu.moc"
