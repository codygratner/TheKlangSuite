---
title: "ELI5 Guide: Remote Antigravity Access with Tailscale & SSH"
date: 2026-10-12
tags: [guide, tailscale, ssh, remote, mobile, nkai]
pillars: [AGY, NK]
---

# 📱 ELI5 Guide: Remote Antigravity Access with Tailscale & SSH

> **The Goal**: Check plugin builds, monitor tests, run git commands, and triage your Morning Mailbag directly from your phone on the couch, in bed, or at a coffee shop—without opening ports on your home Wi-Fi router or configuring complex firewalls.

---

## 🧭 The Big Picture (How It Works)

```
 [ Your Phone ] (Cellular / Coffee Shop / Couch)
       │
       ▼ (Encrypted WireGuard Mesh VPN)
 [ Tailscale Network (Tailnet) ]
       │
       ▼ (Direct, Secure Connection)
 [ Your Windows PC ] (Running Antigravity, N'kai & OpenSSH)
```

1. **Tailscale** creates a secure, encrypted "invisible cable" between your phone and your PC. It assigns your PC a special private IP address (like `100.85.20.15`).
2. **OpenSSH** is Windows' built-in terminal server that lets you log into PowerShell from your phone.
3. **Termius (or JuiceSSH)** is the phone app that acts as your pocket terminal screen.
4. **N'kai Mobile Companion** is the browser UI served by `tools/nkai_serve.py --lan` for visual touch triage.

---

## 🛠️ Step 1: Install Tailscale (5 Minutes, Zero Router Setup)

1. **On your PC**:
   - Go to [tailscale.com](https://tailscale.com) and download Tailscale for Windows.
   - Install it and log in with your Google, GitHub, or Microsoft account.
   - Once logged in, click the Tailscale icon in your Windows system tray. Note your PC's **Tailscale IP** (it starts with `100.x.y.z`, e.g., `100.80.25.10`) and your PC's **Tailscale Machine Name** (e.g., `klang-desktop`).
2. **On your Phone**:
   - Download **Tailscale** from the App Store (iOS) or Google Play (Android).
   - Log in using the **exact same account** you used on your PC.
   - Flip the switch to **Connected**.
3. **Test It**:
   - Turn off Wi-Fi on your phone so you're on cellular data.
   - In the Tailscale phone app, you should see your PC listed with a green dot 🟢!

---

## 📱 Step 2: Couch Triage with N'kai Mobile (Web Companion)

Want to browse your Morning Mailbag or Flight Runway from your phone with smooth touch scrolling and tap-to-triage?

1. **On your PC terminal**, launch the mobile server:
   ```powershell
   python tools/nkai_serve.py --lan
   ```
2. The terminal will print a scannable ASCII QR code and a local URL:
   ```
   📱 N'KAI MOBILE COMPANION READY!
   🌐 Local URL: http://100.80.25.10:3030/morning_mail_inbox.html
   ```
3. **On your Phone**:
   - Open your browser (Safari, Chrome) and type the Tailscale URL (or scan the QR code).
   - Bookmark it or tap **"Add to Home Screen"** to turn it into an app icon!
   - You now have 1-tap mobile touch triage with the bottom thumb navigation bar.

---

## 💻 Step 3: Terminal Access via SSH (Termius / JuiceSSH)

Want to run git commands, check builds, or run `check_mail.py` from your phone?

### A. Enable OpenSSH on Windows (One-Time Setup)
1. Open Windows **Settings ➔ System ➔ Optional Features**.
2. Click **View features** (or "Add a feature") and search for **OpenSSH Server**.
3. Click **Install**.
4. Once installed, open PowerShell as Administrator and run:
   ```powershell
   # Start the SSH service
   Start-Service sshd

   # Set it to start automatically when your PC boots
   Set-Service -Name sshd -StartupType 'Automatic'
   ```

### B. Install the Phone App
- **iOS / Android (Recommended)**: Download **Termius** (clean, modern, free tier is great).
- **Android Alternative**: **JuiceSSH** (classic, fast).
- **iOS Alternative**: **Blink Shell** (hardcore terminal with Mosh).

### C. Connect to Your PC
1. Open **Termius** on your phone.
2. Tap **New Host**:
   - **Label**: `Klang PC`
   - **Hostname**: Your PC's Tailscale IP (e.g. `100.80.25.10`)
   - **Port**: `22` (default)
   - **Username**: Your Windows username (e.g. `codyg`)
   - **Password**: Your Windows account password or PIN
3. Tap **Save** and then tap **Connect**.
4. Accept the host key fingerprint.
5. 🎉 **You're in!** You now have a full, high-speed PowerShell prompt on your phone!

---

## ⚡ Daily Pocket Workflow (Cheat Sheet)

Once set up, your daily routine from the couch is effortless:

1. **Wake PC (if needed)**: Tailscale keeps the connection ready.
2. **Open Termius on phone** ➔ tap `Klang PC`.
3. Quick health check commands:
   ```powershell
   # Jump to repository
   cd C:\Dev\TheKlangSuite

   # Check active branch and git status
   git status

   # Run mail check
   python tools/check_mail.py --chat

   # Launch mobile web server for couch review
   python tools/nkai_serve.py --lan
   ```
4. Switch to your phone browser to review the visual cards!

---

## 🛡️ Security & Battery Invariants
- **Zero Exposed Ports**: Tailscale never opens incoming ports on your home router. Your home network remains 100% stealth and invisible to the public internet.
- **WireGuard Encryption**: All traffic between phone and PC is end-to-end encrypted with public-key cryptography.
- **Auto-Sleep**: If your PC goes to sleep, enable **Wake-on-LAN** in your PC's BIOS/Network card if you want to wake it remotely from your phone.
