---
title: "ELI5 Guide: iOS Focus Modes, Control Center & N'kai Mobile Ergonomics"
date: 2026-10-13
tags: [guide, ios, focus-mode, dnd, mobile, ergonomics, nkai, tailscale]
pillars: [NK, AGY, COSMOS]
---

# 📱 ELI5 Guide: iOS Focus Modes, Control Center & N'kai Mobile Ergonomics

> **The Problem**: When you're actively working on your phone via N'kai Chat or reading a board, Google Antigravity or browser web push notifications fire for every agent thought or tool completion—causing notification buzz spam right under your thumbs.  
> **The Solution**: A dedicated **"N'kai / Deep Work" iOS Focus Mode** that you can turn on and off in 1 second from the top-right drag-down menu (**Control Center**), silencing browser/chat noise while keeping real-world calls and messages open.

---

## 🧭 The Big Picture

```text
┌────────────────────────────────────────────────────────────────────────┐
│                        IOS NOTIFICATION SHIELD                         │
├────────────────────────────────────────────────────────────────────────┤
│                                                                        │
│   [ Swipe Down Top-Right ] ──► [ Tap "Focus" 🌙 ] ──► [ "N'kai Mode" ] │
│                                                              │         │
│         ┌────────────────────────────────────────────────────┘         │
│         ▼                                                              │
│   🟢 ALLOWED: Phone calls from Favorites, family SMS, calendar alerts  │
│   🔕 SILENCED: Antigravity SPA, Safari/Chrome push banners, work pings │
│                                                                        │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 🛠️ Part 1: Create Your Custom Focus Mode (2-Minute One-Time Setup)

1. Open the **Settings** app on your iPhone.
2. Scroll down and tap **Focus** *(the purple crescent moon icon)*.
3. Tap the **`+`** (plus sign in the top-right corner).
4. Tap **Custom**:
   - **Name**: `N'kai` (or `Deep Work`, `Lab Mode`, `Audio Lab`).
   - **Icon**: Pick a laptop 💻, brain 🧠, or rocket 🚀.
   - **Color**: Pick your favorite color (Cyan or Purple recommended!).
   - Tap **Next** $\to$ tap **Customize Focus**.

---

## 🔕 Part 2: Choose What Gets Silenced

You will see two primary configuration tiles: **People** and **Apps**.

### 1. Silence Noisy Apps
1. Tap **Apps**.
2. Under *Notification Filter*, select **"Silence Notifications From"** *(the red minus icon)*:
   - Tap **Add Apps**.
   - Select **Safari**, **Chrome**, or any Google / Antigravity app sending push banners.
   - Tap **Done**.
3. *(Alternative "Nuclear Focus" Method)*: Select **"Allow Notifications From"** and check *only* **Phone** and **Messages**. This automatically mutes 100% of all other apps without having to select them one by one.

### 2. Protect Emergency Contacts
1. Tap **People**.
2. Ensure **Allow Calls From** is set to **"Favorites"** (or "Everyone") so important personal calls or emergency family contacts always ring through.

---

## ⚡ Part 3: Turning It On in 1 Second (Control Center)

You never need to open Settings again. Whenever you sit down to code or open N'kai on your phone:

1. **Swipe down from the top-right corner** of your iPhone screen (where the battery percentage and Wi-Fi bars live) to open **Control Center**.
2. Look for the card labeled **Focus** *(next to the screen mirroring button)*.
3. **Tap and hold** on the Focus button (or tap directly on the word "Focus"):
   - A list of all your Focus modes pops up.
   - Tap **N'kai**.
4. **Done!** A subtle icon appears on your lock screen and dynamic island/status bar. All push spam is quietly held in the Notification Center without buzzing or lighting up your screen.
5. When you’re finished, swipe down again and tap the icon to turn it off.

---

## 💡 Pro-Tips for Maximum Laziness

### 1. The 1-Hour Auto-Disarm (Never Forget It's On!)
* In the Control Center Focus menu, tap the **`...`** (three dots) next to **N'kai**.
* Tap **"For 1 hour"** or **"Until this evening"**.
* The mode will silently disarm itself when the timer expires, so you never miss evening texts.

### 2. Dedicated N'kai Lock Screen
* Inside **Settings $\to$ Focus $\to$ N'kai**, scroll down to **Customize Screens**.
* You can link this Focus mode to a clean, minimal lock screen wallpaper. When the wallpaper switches, you have instant visual confirmation that your notification shield is active.

### 3. Siri Voice Command
* You can say: *"Hey Siri, turn on N'kai focus"* or *"Hey Siri, turn off focus"* completely hands-free.

---

## 🛸 Part 4: The N'kai Mobile Companion Setup (Refresher)

To run the complete sovereign mobile setup over Tailscale:

1. **Start the Formless Spawn**: Ensure `tools/install_startup.bat` has been run on your PC so the background daemon runs on Windows login.
2. **Open over Tailscale**: On your phone Safari, navigate to:
   ```text
   http://<Your-PC-Tailscale-IP>:3030/cockpit.html
   ```
3. **Save to Home Screen**: Tap **Share** $\to$ **Add to Home Screen**.
4. **Fumble-Proof Flush Safety**: The top-header **Flush & Harvest** button requires confirmation via a bottom-sliding drawer that automatically disarms in 7 seconds, guaranteeing accidental palm taps while walking or dropping your phone cannot wipe context.

---

*The Klang Suite & New Klang City Technical Documentation — 2026-10-13.*
