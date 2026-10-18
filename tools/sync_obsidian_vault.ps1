<#
.SYNOPSIS
    Asymmetric Sync Bridge between The Klang Suite repository and a dedicated Obsidian Vault.

    ========================================================================================
    ONE-WAY VAULT MIRROR RULE (ABSOLUTE — NEVER VIOLATE per GEMINI.md / GLOSSARY §15):
    TheKlangVault is a generated, read-only mirror of the repositories.
    Content flows REPO ➔ VAULT ONLY. Under NO circumstances may any script copy, sync, merge,
    or write vault content back into any repository. Vault-side edits outside Inbox/ are
    disposable and overwritten on the next sync.
    Sole exception: Human-authored Inbox notes are read in-place by check_mail.py and
    ramble_ingest.py — never duplicated into repo directories.
    ========================================================================================

.DESCRIPTION
    Decouples Obsidian Sync (mobile/desktop) from Git.
    - Docs (Repo -> Vault): Mirrors docs/ to Vault/Docs/ with sync-time link rewriting (Flight 23).
    - Telemetry: Generates real-time project dashboards and error logs in Vault/Telemetry/.
    - Heartbeat: Updates _sync_heartbeat.md in Vault root.

.PARAMETER VaultPath
    Target Obsidian Vault path. Defaults to "C:\Dev\TheKlangVault".

.PARAMETER Watch
    If specified, runs continuously in a loop watching for changes.

.PARAMETER IntervalSeconds
    Interval in seconds between watch iterations (default: 10).

.EXAMPLE
    .\tools\sync_obsidian_vault.ps1
    .\tools\sync_obsidian_vault.ps1 -Watch -IntervalSeconds 15
#>

[CmdletBinding()]
param(
    [string]$VaultPath = "C:\Dev\TheKlangVault",
    [switch]$Watch,
    [int]$IntervalSeconds = 10,
    [string[]]$ArchiveNotes = @()
)

$RepoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$RepoDocs = Join-Path $RepoRoot "docs"
$RepoInbox = Join-Path $RepoDocs "inbox"

# Universal UTF-8 Invariant (Flight 18)
$env:PYTHONUTF8 = "1"
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

$VaultInbox = Join-Path $VaultPath "Inbox"
$VaultDocs = Join-Path $VaultPath "Docs"
$VaultTelemetry = Join-Path $VaultPath "Telemetry"
$VaultCanvas = Join-Path $VaultPath "Canvas"

function Convert-WikiLinksToMarkdown([string]$text) {
    if ([string]::IsNullOrEmpty($text)) { return $text }
    # 1. [[Target|Alias]] -> [Alias](Target.md)
    $text = [System.Text.RegularExpressions.Regex]::Replace($text, '\[\[([^\]\|]+)\|([^\]]+)\]\]', '[$2]($1.md)')
    # 2. [[Target]] -> [Target](Target.md)
    $text = [System.Text.RegularExpressions.Regex]::Replace($text, '\[\[([^\]]+)\]\]', '[$1]($1.md)')
    return $text
}

function Sync-Once {
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $timestamp = (Get-Date).ToString("yyyy-MM-dd HH:mm:ss")
    $inboxSynced = 0
    $docsSynced = 0

    # Ensure all directories exist
    @($VaultPath, $VaultInbox, $VaultDocs, $VaultTelemetry, $VaultCanvas, $RepoDocs, $RepoInbox) | ForEach-Object {
        if (-not (Test-Path $_)) {
            New-Item -ItemType Directory -Path $_ -Force | Out-Null
        }
    }

    # 0. ARCHIVE SPECIFIC INBOX NOTES (If requested via -ArchiveNotes)
    if ($ArchiveNotes.Count -gt 0) {
        $archiveDir = Join-Path $VaultInbox "Archive"
        if (-not (Test-Path $archiveDir)) {
            New-Item -ItemType Directory -Path $archiveDir -Force | Out-Null
        }
        foreach ($noteName in $ArchiveNotes) {
            $matchingFiles = Get-ChildItem -Path $VaultInbox -File | Where-Object { 
                $_.Name -like "*$noteName*" -or $_.BaseName -like "*$noteName*" 
            }
            foreach ($match in $matchingFiles) {
                $targetVaultArchive = Join-Path $archiveDir $match.Name
                Move-Item -Path $match.FullName -Destination $targetVaultArchive -Force
                Write-Host "[VAULT INBOX ARCHIVED] Moved $($match.Name) -> Vault/Inbox/Archive/" -ForegroundColor Green
                
                # Clean up corresponding repo inbox file if present
                $repoCopy = Join-Path $RepoInbox $match.Name
                if (Test-Path $repoCopy) {
                    Remove-Item -Path $repoCopy -Force
                    Write-Host "[REPO INBOX REMOVED] $($match.Name)" -ForegroundColor DarkYellow
                }
            }
        }
    }

    # 1. INBOX DECOUPLING (One-Way Vault Mirror Invariant — Flight 23)
    # TheKlangVault/Inbox/ is read directly by check_mail.py and ramble_ingest.py.
    # Notes are NEVER copied into docs/inbox/ to prevent duplicate stale files.
    if (Test-Path $RepoInbox) {
        $repoInboxFiles = Get-ChildItem -Path $RepoInbox -File -Recurse | Where-Object {
            $_.Name -ne "README.md" -and $_.Name -ne ".gitkeep"
        }
        foreach ($repoFile in $repoInboxFiles) {
            Remove-Item -Path $repoFile.FullName -Force
            Write-Host "[INBOX CLEANUP] Removed stale duplicate $($repoFile.Name) from docs/inbox/" -ForegroundColor DarkYellow
        }

        # Remove empty directories in RepoInbox
        Get-ChildItem -Path $RepoInbox -Directory -Recurse | Where-Object {
            (Get-ChildItem -Path $_.FullName -Recurse -File | Measure-Object).Count -eq 0
        } | ForEach-Object {
            Remove-Item -Path $_.FullName -Recurse -Force
        }
    }

    # 2. SYNC DOCS: Repo -> Vault (Excluding inbox and temp/hidden files)
    if (Test-Path $RepoDocs) {
        $docsFiles = Get-ChildItem -Path $RepoDocs -File -Recurse | Where-Object {
            $_.FullName -notmatch '[\\/]inbox[\\/]' -and
            $_.FullName -notmatch '[\\/]\.obsidian[\\/]' -and
            -not ($_.Name.StartsWith(".")) -and
            -not ($_.Name.StartsWith("_sync_")) -and
            $_.Extension -ne ".tmp"
        }

        foreach ($file in $docsFiles) {
            $relPath = $file.FullName.Substring($RepoDocs.Length).TrimStart('\', '/')
            $destPath = Join-Path $VaultDocs $relPath
            $destDir = Split-Path $destPath

            if (-not (Test-Path $destDir)) {
                New-Item -ItemType Directory -Path $destDir -Force | Out-Null
            }

            $shouldCopy = $false
            if (-not (Test-Path $destPath)) {
                $shouldCopy = $true
            } else {
                $destItem = Get-Item $destPath
                if ($file.LastWriteTimeUtc -gt $destItem.LastWriteTimeUtc) {
                    $shouldCopy = $true
                }
            }

            if ($shouldCopy) {
                if ($file.Extension -eq ".md") {
                    $rawContent = Get-Content -Path $file.FullName -Raw -Encoding UTF8
                    $rewriterScript = Join-Path $RepoRoot "tools\vault_link_rewriter.py"
                    if (Test-Path $rewriterScript) {
                        try {
                            $pinfo = New-Object System.Diagnostics.ProcessStartInfo
                            $pinfo.FileName = "python"
                            $pinfo.Arguments = "`"$rewriterScript`" `"$($file.FullName)`""
                            $pinfo.RedirectStandardOutput = $true
                            $pinfo.UseShellExecute = $false
                            $pinfo.StandardOutputEncoding = [System.Text.Encoding]::UTF8
                            $p = [System.Diagnostics.Process]::Start($pinfo)
                            $rewritten = $p.StandardOutput.ReadToEnd()
                            $p.WaitForExit()
                            if ($p.ExitCode -eq 0 -and (-not [string]::IsNullOrEmpty($rewritten))) {
                                $rawContent = $rewritten
                            }
                        } catch {}
                    }
                    Set-Content -Path $destPath -Value $rawContent -Encoding UTF8 -NoNewline
                } else {
                    Copy-Item -Path $file.FullName -Destination $destPath -Force
                }
                $docsSynced++
                Write-Host "[REPO -> DOCS] $relPath (links rewritten)" -ForegroundColor DarkGray
            }
        }

        # Clean up stale files in VaultDocs that no longer exist in RepoDocs
        if (Test-Path $VaultDocs) {
            $vaultDocsFiles = Get-ChildItem -Path $VaultDocs -File -Recurse
            foreach ($vFile in $vaultDocsFiles) {
                $relPath = $vFile.FullName.Substring($VaultDocs.Length).TrimStart('\', '/')
                $repoCounterpart = Join-Path $RepoDocs $relPath
                if (-not (Test-Path $repoCounterpart)) {
                    Remove-Item -Path $vFile.FullName -Force
                    Write-Host "[DOCS CLEANUP] Removed stale $relPath from Vault/Docs/" -ForegroundColor DarkYellow
                }
            }
            # Clean up empty directories in VaultDocs
            Get-ChildItem -Path $VaultDocs -Directory -Recurse | Where-Object { 
                (Get-ChildItem -Path $_.FullName -Recurse -File).Count -eq 0 
            } | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    # 2.5 BACKLOG INTEGRITY LINTER
    $backlogPath = Join-Path $RepoDocs "BACKLOG.md"
    if (Test-Path $backlogPath) {
        $staleMatches = Select-String -Path $backlogPath -Pattern '\[`?PLAN\.md`?\]\([^)]*PLAN\.md\)'
        if ($staleMatches) {
            foreach ($match in $staleMatches) {
                Write-Host "[BACKLOG LINTER WARNING] Line $($match.LineNumber): Stale PLAN.md reference detected! Please update to archived plan in .agents/pipeline/plans/completed/" -ForegroundColor Yellow
            }
        }
    }

    # 3. TELEMETRY: Git Inspection
    $gitBranch = "unknown"
    $gitCommit = "unknown"
    $gitChanges = 0
    try {
        $gitBranch = (git -C $RepoRoot branch --show-current).Trim()
        $gitCommit = (git -C $RepoRoot log -1 --pretty=format:"%h - %s (%cr)").Trim()
        $statusLines = git -C $RepoRoot status --porcelain
        if ($statusLines) {
            $gitChanges = ($statusLines | Measure-Object).Count
        }
    } catch {
        Write-Warning "Failed to query Git status: $_"
    }

    # 4. TELEMETRY: DevLogger Inspection
    $devLogPath = "$env:LOCALAPPDATA\TheKlangSuite\dev.log"
    $recentErrors = @()
    $recentWarns = @()
    if (Test-Path $devLogPath) {
        try {
            $logLines = Get-Content -Path $devLogPath -Tail 200 -ErrorAction SilentlyContinue
            foreach ($line in $logLines) {
                if ($line -match "\[ERROR\]") { $recentErrors += $line }
                elseif ($line -match "\[WARN\]") { $recentWarns += $line }
            }
        } catch {}
    }

    # 5. WRITE TELEMETRY: Dashboard.md
    $statusEmoji = if ($recentErrors.Count -gt 0) { "[ALERT]" } else { "[OK]" }
    $q = [char]96 # Markdown backtick
    $dashboardLines = @(
        ("# {0} System Telemetry Dashboard" -f $statusEmoji),
        "> [!INFO] Live Status Feed",
        ("> **Last Synchronized:** {0}" -f $timestamp),
        ("> **Active Git Branch:** {0}{1}{0}" -f $q, $gitBranch),
        ("> **Latest Git Commit:** {0}" -f $gitCommit),
        ("> **Uncommitted Changes:** {0} files" -f $gitChanges),
        "",
        "---",
        "",
        "## Build & Deployment Status",
        ("- **VST3 System Deployment:** {0}C:\Program Files\Common Files\VST3{0}" -f $q),
        ("- **Local Artifacts:** {0}TheKlangSuite\current_build{0}" -f $q),
        ("- **Sync Stats:** {0} inbox notes ingested | {1} docs mirrored" -f $inboxSynced, $docsSynced),
        "",
        "---",
        "",
        "## Runtime Diagnostics (dev.log)"
    )

    if ($recentErrors.Count -gt 0) {
        $dashboardLines += ("> [!CAUTION] Active Errors Detected in dev.log ({0})" -f $recentErrors.Count)
        $dashboardLines += "> Review [[Active_Errors]] for complete tracebacks."
    } else {
        $dashboardLines += "> [!NOTE] Zero Active Errors"
        $dashboardLines += "> No [ERROR] log entries detected in the local developer log. Engine is running smoothly!"
    }

    if ($recentWarns.Count -gt 0) {
        $dashboardLines += ""
        $dashboardLines += ("### Recent Warnings ({0})" -f $recentWarns.Count)
        $dashboardLines += '```text'
        foreach ($w in ($recentWarns | Select-Object -Last 5)) { $dashboardLines += $w }
        $dashboardLines += '```'
    }

    Set-Content -Path (Join-Path $VaultTelemetry "Dashboard.md") -Value $dashboardLines -Encoding UTF8

    # 6. WRITE TELEMETRY: Active_Errors.md
    $errorsLines = @(
        "# Active Runtime Errors and Diagnostic Logs",
        ("**Last Checked:** {0}" -f $timestamp),
        ("**Source:** {0}{1}{0}" -f $q, $devLogPath),
        ""
    )

    if ($recentErrors.Count -gt 0) {
        $errorsLines += "> [!WARNING] The following errors were parsed from dev.log:"
        $errorsLines += ""
        $errorsLines += '```text'
        foreach ($e in ($recentErrors | Select-Object -Last 20)) { $errorsLines += $e }
        $errorsLines += '```'
    } else {
        $errorsLines += "> [!TIP] All Clean!"
        $errorsLines += "> No errors currently found in dev.log."
    }

    Set-Content -Path (Join-Path $VaultTelemetry "Active_Errors.md") -Value $errorsLines -Encoding UTF8

    # 6.5 SYNC RESEARCH & COMPILE MASTER INDEX & ROADMAP
    $roadmapScript = Join-Path $PSScriptRoot "sync_roadmap_sidecar.py"
    if (Test-Path $roadmapScript) {
        python $roadmapScript 2>$null | Out-Null
    }

    $researchRepo = "C:\Dev\Research"
    $vaultResearch = Join-Path $VaultPath "Research"
    if (Test-Path $researchRepo) {
        if (-not (Test-Path $vaultResearch)) { New-Item -ItemType Directory -Path $vaultResearch -Force | Out-Null }
        
        $indexLines = @(
            "# [INDEX] Master Research & Architecture Index",
            "> Statically compiled from [TheKlangResearch](https://github.com/codygratner/TheKlangResearch). Zero Dataview lag, 100% portable Markdown.",
            "",
            "## Domain Catalogs",
            ""
        )

        @("guides", "lore", "dsp", "hardware", "library", "library/manuals", "ui_ux", "subagents", "agentic", "qa_autopsies", "retrospectives") | ForEach-Object {
            $domName = $_
            $srcDom = Join-Path $researchRepo $domName
            $dstDom = Join-Path $vaultResearch $domName
            if (Test-Path $srcDom) {
                if (-not (Test-Path $dstDom)) { New-Item -ItemType Directory -Path $dstDom -Force | Out-Null }
                
                $domFiles = Get-ChildItem -Path $srcDom -File | Where-Object { $_.Name -notmatch "^\." }
                if ($domFiles.Count -gt 0) {
                    $indexLines += ("### [CATALOG] {0}" -f $domName.ToUpper())
                    $domFiles | ForEach-Object {
                        $dFile = Join-Path $dstDom $_.Name
                        if ((-not (Test-Path $dFile)) -or ($_.LastWriteTimeUtc -gt (Get-Item $dFile).LastWriteTimeUtc)) {
                            Copy-Item -Path $_.FullName -Destination $dFile -Force
                        }
                        $indexLines += ("- [[{0}/{1}|{1}]]" -f $domName, $_.Name)
                    }
                    $indexLines += ""
                }
            }
        }

        # Mirror lore to root Vault Lore/ as well
        $loreSrc = Join-Path $researchRepo "lore"
        $vaultRootLore = Join-Path $VaultPath "Lore"
        if ((Test-Path $loreSrc) -and (Test-Path $vaultRootLore)) {
            Get-ChildItem -Path $loreSrc -File | ForEach-Object {
                $dstLore = Join-Path $vaultRootLore $_.Name
                if ((-not (Test-Path $dstLore)) -or ($_.LastWriteTimeUtc -gt (Get-Item $dstLore).LastWriteTimeUtc)) {
                    Copy-Item -Path $_.FullName -Destination $dstLore -Force
                }
            }
        }

        # Remove stale empty hardware directories in Vault
        @("CTAG-TBD", "TBD-16-DSP", "TBD-16-UI") | ForEach-Object {
            $staleDir = Join-Path (Join-Path $vaultResearch "hardware") $_
            if (Test-Path $staleDir) { Remove-Item -Path $staleDir -Recurse -Force -ErrorAction SilentlyContinue }
        }

        Set-Content -Path (Join-Path $vaultResearch "Master_Research_Index.md") -Value $indexLines -Encoding UTF8
    }

    # 7. WRITE HEARTBEAT: _sync_heartbeat.md
    $sw.Stop()
    $durationMs = [math]::Round($sw.Elapsed.TotalMilliseconds, 1)

    $heartbeatLines = @(
        "# Vault Sync Heartbeat",
        "- **Status:** ACTIVE & IN SYNC",
        ("- **Last Sync:** {0}" -f $timestamp),
        ("- **Duration:** {0} ms" -f $durationMs),
        ("- **Branch:** {0}{1}{0}" -f $q, $gitBranch),
        ("- **Inbox Ingested:** {0}" -f $inboxSynced),
        ("- **Docs Mirrored:** {0}" -f $docsSynced),
        ("- **Active Errors:** {0}" -f $recentErrors.Count)
    )

    Set-Content -Path (Join-Path $VaultPath "_sync_heartbeat.md") -Value $heartbeatLines -Encoding UTF8

    Write-Host "[OK] Sync pass complete (${durationMs}ms) - Inbox: $inboxSynced | Docs: $docsSynced | Errors: $($recentErrors.Count)" -ForegroundColor Green
}

# Execution Entry Point
if ($Watch) {
    Write-Host "[WATCH] Starting Obsidian Vault Sync Watcher (Interval: ${IntervalSeconds}s)... Press Ctrl+C to stop." -ForegroundColor Yellow
    while ($true) {
        try {
            Sync-Once
        } catch {
            Write-Warning "Sync iteration encountered error: $_"
        }
        Start-Sleep -Seconds $IntervalSeconds
    }
} else {
    Sync-Once
}
