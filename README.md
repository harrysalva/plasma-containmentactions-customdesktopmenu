# plasma-containmentactions-customdesktopmenu

Custom desktop menu for Plasma 6

<img width="250px" title="menu" alt="menu" src=".assets/menu.png"> <img width="350px" title="config" alt="config" src=".assets/config.png">

## Features

- ✅ Custom desktop context menu
- ✅ Configurable menu structure
- ✅ Support for applications, commands, and submenus
- ✅ **Favorites support** - Shows most used applications via KActivitiesStats
- ✅ Configurable number of favorites (1-20)
- ✅ Fallback to common applications when no statistics available

## Build and install

### Prerequisites

- extra-cmake-modules >= 6.0.0
- kdeplasma-addons >= 6.0.0
- kf6-activities >= 6.0.0
- kf6-activitiesstats >= 6.0.0

Archlinux :
```
pacman -S --needed base-devel cmake extra-cmake-modules kdeplasma-addons kf6-activities kf6-activitiesstats
```

### Build

```
cd src
cmake --fresh -B ../_build .
cd ../_build
make clean
make
```

### Install / Update

```
chmod 755 bin/plasma/containmentactions/matmoul-customdesktopmenu.so
sudo cp bin/plasma/containmentactions/matmoul-customdesktopmenu.so /usr/lib/qt6/plugins/plasma/containmentactions
```

### Quick Build and Install

Use the provided script:
```
./build_and_test.sh
```

## Configuration

### Menu Configuration Syntax

- `{favorites}` - Shows most used applications (NEW!)
- `{programs}` - Shows all applications
- `{programs}\tCategory/` - Shows applications from specific category
- `[menu]\tName\tIcon` - Start submenu
- `[end]` - End submenu
- `/path/to/app.desktop` - Specific application
- `Name\tIcon\tCommand` - Custom command
- `-` - Separator
- `#Comment` - Comment line (ignored)

### Example Configuration

```
{favorites}
-
[menu]	Applications	kde
{programs}
[end]
-
/usr/share/applications/org.kde.dolphin.desktop
/usr/share/applications/org.kde.kate.desktop
-
[menu]	System	configure-shortcuts
{programs}	Settingsmenu/
-
{programs}	System/
[end]
```

## Favorites Feature

The favorites functionality uses KActivitiesStats to show your most frequently used applications. See [FAVORITES.md](FAVORITES.md) for detailed information.

### Configuration Options

- **Show applications by name**: Display app name vs generic name
- **Max favorites**: Number of favorite apps to show (1-20)

## Other

- When you update the library, KWin crashes and restarts with the new library.
- ✅ **Favorites is now fully implemented!**

## Sources

- https://github.com/MatMoul/plasma-containmentactions-customdesktopmenu/tree/plasma5
- https://invent.kde.org/plasma/plasma-workspace/-/tree/ea415539fc6256494d5c12296a6216e522e12b0a/containmentactions
- https://invent.kde.org/plasma/plasma-workspace/-/blob/master/applets/kicker/plugin/kastatsfavoritesmodel.cpp?ref_type=heads
