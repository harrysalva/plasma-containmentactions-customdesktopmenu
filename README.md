# plasma-containmentactions-customdesktopmenu

Custom desktop menu for Plasma 6

<img width="250px" title="menu" alt="menu" src=".assets/menu.png"> <img width="350px" title="config" alt="config" src=".assets/config.png">

## Features

- ✅ Custom desktop context menu
- ✅ Configurable menu structure
- ✅ Support for applications, commands, and submenus
- ✅ **Advanced favorites support** - Shows most used applications via KActivitiesStats
- ✅ **Activity-specific filtering** - Only shows favorites from current activity
- ✅ **Dynamic browser detection** - Automatically detects default browser (including Brave)
- ✅ Configurable number of favorites (1-20)
- ✅ **Intelligent fallback system** - Multiple levels of fallback for maximum compatibility
- ✅ **Optimized performance** - Clean, efficient code with minimal resource usage

## Build and install

### Prerequisites

- extra-cmake-modules >= 6.0.0
- kdeplasma-addons >= 6.0.0
- plasma-activities >= 6.0.0 (replaces kf6-activities)
- plasma-activities-stats >= 6.0.0 (replaces kf6-activitiesstats)

Arch Linux:
```bash
pacman -S --needed base-devel cmake extra-cmake-modules kdeplasma-addons plasma-activities plasma-activities-stats
```

### Quick Build and Install

Use the provided script:
```bash
./build_and_test.sh
```

### Manual Build

```bash
cd src
cmake --fresh -B ../_build .
cd ../_build
make clean
make
```

### Manual Install / Update

```bash
chmod 755 bin/plasma/containmentactions/matmoul-customdesktopmenu.so
sudo cp bin/plasma/containmentactions/matmoul-customdesktopmenu.so /usr/lib/qt6/plugins/plasma/containmentactions
```

## Configuration

### Menu Configuration Syntax

- `{favorites}` - Shows most used applications from current activity
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
{programs}	Settings/
-
{programs}	System/
[end]
```

## Favorites Feature

The favorites functionality uses a sophisticated 3-level detection system:

### Level 1: KActivities API Integration
- Uses native KDE KActivitiesStats API
- Queries multiple agents (plasma.favorites, kicker, kickoff)
- Filters by current activity automatically
- Provides real-time favorite applications

### Level 2: Optimized Manual Detection
- Reads KDE configuration files directly
- Selects the section with most favorites
- Processes activity-specific data only
- Handles multiple favorite formats

### Level 3: Common Applications Fallback
- Provides default applications when no favorites found
- Includes essential KDE applications
- Ensures menu is never empty

### Supported Application Formats
- `applications:app.desktop` - Standard KDE format
- `preferred://browser` - Dynamic default browser detection
- `preferred://filemanager` - Default file manager
- `preferred://terminal` - Default terminal
- `app.desktop` - Direct desktop file reference

### Configuration Options

- **Show applications by name**: Display app name vs generic name
- **Max favorites**: Number of favorite apps to show (1-20)

### Dynamic Browser Detection

The plugin automatically detects your default browser using multiple methods:
1. **xdg-settings** - System default browser
2. **mimeapps.list** - MIME type associations
3. **Known browsers** - Searches for popular browsers including:
   - Brave Browser (all variants)
   - Firefox
   - Google Chrome
   - Chromium
   - Falkon
   - Opera

## Usage

1. **Install the plugin** using `./build_and_test.sh`
2. **Restart KWin**: `kwin_x11 --replace &` (or `kwin_wayland --replace &`)
3. **Configure desktop actions**:
   - Go to System Settings > Desktop Behavior > Desktop Actions
   - Configure 'Custom Desktop Menu' for right-click
4. **Add favorites to your menu** by including `{favorites}` in your configuration
5. **Enjoy activity-specific favorites** that update automatically

## Technical Details

### Architecture
- **Production-ready code** with professional English comments
- **Optimized performance** with eliminated redundant functions
- **Memory efficient** with smart resource management
- **Robust error handling** with multiple fallback levels

### Compatibility
- **Plasma 6** native support
- **KF6 framework** integration
- **Multiple desktop environments** compatibility
- **Wayland and X11** support

## Notes

- When you update the plugin, KWin will restart automatically with the new version
- The plugin automatically adapts to your current KDE activity
- Favorites are updated dynamically when you modify them in KDE
- The plugin works without KActivities but with reduced functionality

## Sources

- https://github.com/MatMoul/plasma-containmentactions-customdesktopmenu/tree/plasma5
- https://invent.kde.org/plasma/plasma-workspace/-/tree/ea415539fc6256494d5c12296a6216e522e12b0a/containmentactions
- https://invent.kde.org/plasma/plasma-workspace/-/blob/master/applets/kicker/plugin/kastatsfavoritesmodel.cpp?ref_type=heads
