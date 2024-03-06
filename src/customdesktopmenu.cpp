/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "customdesktopmenu.h"

#include <QDebug>

#include <KConfigGroup>
#include <KIO/ApplicationLauncherJob>
#include <KPluginFactory>
#include <Plasma/PluginLoader>
#include <KProcess>
#include <KDesktopFile>

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

  if (!menuconfig.isEmpty()) {
    QList<QMenu*> menuList;
    menuList.append(0);
    QMenu* curMenu = 0;
    QStringList configLines = menuconfig.split( "\n", Qt::SkipEmptyParts );
    foreach( QString cfgLine, configLines ) {
      if (!cfgLine.startsWith("#")) {
        if (cfgLine.startsWith("-")) {
          addSep(curMenu);
        } else if (cfgLine.endsWith(".desktop")) {
          addApp(curMenu, cfgLine);
        } else if (cfgLine.startsWith("[menu]")) {
          QStringList cfgParts = cfgLine.split( "\t", Qt::SkipEmptyParts );
          if (cfgParts.size() == 3) {
            curMenu = addMnu(curMenu, cfgParts[2], cfgParts[1]);
            menuList.append(curMenu);
          } else if (cfgParts.size() == 2) {
            curMenu = addMnu(curMenu, "", cfgParts[1]);
            menuList.append(curMenu);
          }
        } else if (cfgLine.startsWith("[end]")) {
          menuList.removeLast();
          curMenu = menuList.last();
        } else if (cfgLine == "{favorites}") {
          fillFavorites(curMenu);
        } else if (cfgLine.startsWith("{programs}")) {
          QStringList cfgParts = cfgLine.split( "\t", Qt::SkipEmptyParts );
          if (cfgParts.size() == 2) {
            fillPrograms(curMenu, cfgParts[1]);
          } else {
            fillPrograms(curMenu, "/");
          }
        } else {
          QStringList cfgParts = cfgLine.split( "\t", Qt::SkipEmptyParts );
          if (cfgParts.size() == 3) {
            addCmd(curMenu, cfgParts[1], cfgParts[0], cfgParts[2]);
          } else if (cfgParts.size() == 2) {
            addCmd(curMenu, "", cfgParts[0], cfgParts[1]);
          } else {
            addItm(curMenu, "", cfgParts[0]);
          }
        }
      }
    }
  }

  return m_actions;
}

/*
void CustomDesktopMenu::makeMenu(QMenu *menu, const KServiceGroup::Ptr &group)
{
    const auto entries = group->entries(true, true, true);
    for (const KSycocaEntry::Ptr &p : entries) {
        if (p->isType(KST_KService)) {
            const KService::Ptr service(static_cast<KService *>(p.data()));

            QString text = service->name();
            if (!m_showAppsByName && !service->genericName().isEmpty()) {
                text = service->genericName();
            }

            QAction *action = new QAction(QIcon::fromTheme(service->icon()), text, this);
            connect(action, &QAction::triggered, [action]() {
                KService::Ptr service = KService::serviceByStorageId(action->data().toString());
                auto job = new KIO::ApplicationLauncherJob(service);
                job->start();
            });
            action->setData(service->storageId());
            if (menu) {
                menu->addAction(action);
            } else {
                m_actions << action;
            }
        } else if (p->isType(KST_KServiceGroup)) {
            const KServiceGroup::Ptr service(static_cast<KServiceGroup *>(p.data()));
            if (service->childCount() == 0) {
                continue;
            }
            QAction *action = new QAction(QIcon::fromTheme(service->icon()), service->caption(), this);
            QMenu *subMenu = new QMenu();
            makeMenu(subMenu, service);
            action->setMenu(subMenu);
            if (menu) {
                menu->addAction(action);
            } else {
                m_actions << action;
            }
        } else if (p->isType(KST_KServiceSeparator)) {
            if (menu) {
                menu->addSeparator();
            }
        }
    }
}
*/

QWidget *CustomDesktopMenu::createConfigurationInterface(QWidget *parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);
    widget->setWindowTitle(i18nc("plasma_containmentactions_customdesktopmenu", "Configure Application Launcher Plugin"));

    m_ui.showAppsByName->setChecked(m_showAppsByName);
    m_ui.configData->setPlainText(menuconfig);

    return widget;
}

void CustomDesktopMenu::configurationAccepted()
{
    m_showAppsByName = m_ui.showAppsByName->isChecked();
    menuconfig = m_ui.configData->document()->toPlainText();
}

void CustomDesktopMenu::restore(const KConfigGroup &config)
{
    m_showAppsByName = config.readEntry(QStringLiteral("showAppsByName"), true);
    menuconfig = config.readEntry(QStringLiteral("menuConfig"), getDefaultMenu());

}

void CustomDesktopMenu::save(KConfigGroup &config)
{
    config.writeEntry(QStringLiteral("showAppsByName"), m_showAppsByName);
    config.writeEntry(QStringLiteral("menuConfig"), menuconfig);
}


QString CustomDesktopMenu::getDefaultMenu()
{
  QString defMenuConfig = "{favorites}\n";
  defMenuConfig += "-\n";
  defMenuConfig += "[menu]\tApplications\tkde\n";
  defMenuConfig += "{programs}\n";
  defMenuConfig += "[end]\n";
  defMenuConfig += "-\n";
  defMenuConfig += "#/usr/share/applications/org.kde.dolphin.desktop\n";
  defMenuConfig += "-\n";
  defMenuConfig += "[menu]\tSystem\tconfigure-shortcuts\n";
  defMenuConfig += "{programs}\tSettingsmenu/\n";
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

QIcon CustomDesktopMenu::getIcon(const QString &txt)
{
  QIcon icon = QIcon::fromTheme(txt);
  return icon;
}

void CustomDesktopMenu::addSep(QMenu *menu)
{
  QAction *action = new QAction(this);
  action->setSeparator(true);
  if (menu) {
    menu->addAction(action);
  } else {
    m_actions << action;
  }
}

void CustomDesktopMenu::addItm(QMenu *menu, const QString &icon, const QString &txt)
{
  QString text = txt;
  text.replace("&", "&&"); //escaping
  QAction *action;
  action = new QAction(getIcon(icon), text, this);
  if (menu) {
    menu->addAction(action);
  } else {
    m_actions << action;
  }
}

void CustomDesktopMenu::addCmd(QMenu *menu, const QString &icon, const QString &txt, const QString &cmd)
{
  QString text = txt;
  text.replace("&", "&&"); //escaping
  QAction *action;
  action = new QAction(getIcon(icon), text, this);
  action->setData(cmd);
  connect(action, &QAction::triggered, [action](){
    QString source = action->data().toString();
    if (!source.isEmpty()) {
      if (source.endsWith(".desktop")) {
        KService::Ptr service = KService::serviceByDesktopPath(action->data().toString());
        auto job = new KIO::ApplicationLauncherJob(service);
        job->start();
      } else {
        QStringList cmd = source.split(" ");
        KProcess *process = new KProcess(0);
        process->startDetached(cmd);
      }
    }
  });
  if (menu) {
    menu->addAction(action);
  } else {
    m_actions << action;
  }
}

QMenu* CustomDesktopMenu::addMnu(QMenu *menu, const QString &icon, const QString &txt)
{
  QString text = txt;
  text.replace("&", "&&"); //escaping
  QAction *action;
  action = new QAction(getIcon(icon), text, this);
  QMenu *subMenu = new QMenu();
  action->setMenu(subMenu);
  if (menu) {
    menu->addAction(action);
  } else {
    m_actions << action;
  }
  return subMenu;
}

void CustomDesktopMenu::addApp(QMenu *menu, const QString &path)
{
  if(KDesktopFile::isDesktopFile(path) == true) {
    KDesktopFile dskFile(path);
    addCmd(menu, dskFile.readIcon(), dskFile.readName(), path);
  } else {
    addCmd(menu, "", path, path);
  }
}

void CustomDesktopMenu::fillPrograms(QMenu *menu, const QString &path)
{
  KServiceGroup::Ptr root = KServiceGroup::group(path);
  KServiceGroup::List list = root->entries(true, true, true);
  for (const KSycocaEntry::Ptr &p : list){
    if(p->isType(KST_KService)) {
      addApp(menu, p->entryPath());
    } else if(p->isType(KST_KServiceGroup)) {
      const KServiceGroup::Ptr service(static_cast<KServiceGroup *>(p.data()));
      if(service->childCount() == 0) {
        continue;
      }
      QMenu* menu2 = addMnu(menu, service->icon(), service->caption());
      fillPrograms(menu2, p->name());
    } else if(p->isType(KST_KServiceSeparator)) {
      menu->addSeparator();
    }
  }
}

void CustomDesktopMenu::fillFavorites(QMenu *menu)
{
  /*
  KConfig config("kickoffrc");
  KConfigGroup favoritesGroup = config.group("Favorites");
  QList<QString> favoriteList = favoritesGroup.readEntry("FavoriteURLs", QList<QString>());
  foreach (const QString &source, favoriteList) {
    addApp(menu, source);
  }
  */
}



K_PLUGIN_CLASS_WITH_JSON(CustomDesktopMenu, "plasma-containmentactions-customdesktopmenu.json")

#include "customdesktopmenu.moc"
