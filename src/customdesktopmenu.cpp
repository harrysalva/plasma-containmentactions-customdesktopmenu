#include "customdesktopmenu.h"

#include <KConfigGroup>
#include <KIO/ApplicationLauncherJob>
#include <KPluginFactory>
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
      // favorites
      // Not implemented
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


K_PLUGIN_CLASS_WITH_JSON(CustomDesktopMenu, "plasma-containmentactions-customdesktopmenu.json")

#include "customdesktopmenu.moc"
