# OpenBoard Always-On-Top GNOME Shell Extension (PoC)

## What is this?

This is a **Proof of Concept** extension for GNOME Shell (Wayland only) that helps keep OpenBoard palettes and the transparent drawing view on top of other windows.

## Quick Start

### Install

```bash
mkdir -p ~/.local/share/gnome-shell/extensions
cd ~/.local/share/gnome-shell/extensions
cp -r . ~/.local/share/gnome-shell/extensions/openboard-always-on-top@openboard
```

### Enable

```bash
gnome-extensions enable openboard-always-on-top@openboard
```

## How It Works

The extension exposes a D-Bus method that OpenBoard calls to request "always on top" behavior for specific windows:

```bash
gdbus call --session \
  --dest org.gnome.Shell.Extensions.OpenBoard \
  --object-path /org/gnome/Shell/Extensions/OpenBoard \
  --method org.gnome.Shell.Extensions.OpenBoard.SetAlwaysOnTop \
  "Window Title" true
```

## Current Limitations

- **PoC Stage**: This is just a POC, not usable as-is
- **Incomplete**: OpenBoard integration in `src/frameworks/UBPlatformUtils_linux.cpp` and `src/desktop/UBDesktopAnnotationController.cpp` is not complete, and could be greatly improved
- **Window Matching**: Currently matches windows by title substring (case-sensitive)

## Debugging

### Check if extension loads

```bash
gnome-extensions info openboard-always-on-top@openboard
```

Should show `État: ACTIVE` or `State: ACTIVE`.

**Visual confirmation**: If the extension is enabled, you should see a smiley face icon (😄) in the top panel (GNOME Shell top bar, top-right area next to system menu).

### View logs

```bash
journalctl /usr/bin/gnome-shell | grep -i openboard
```

### Test D-Bus directly

```bash
gdbus introspect --session \
  --dest org.gnome.Shell.Extensions.OpenBoard \
  --object-path /org/gnome/Shell/Extensions/OpenBoard
```

## Files

- `extension.js` — Main extension code (GJS)
- `metadata.json` — Extension metadata (name, version, etc.)

