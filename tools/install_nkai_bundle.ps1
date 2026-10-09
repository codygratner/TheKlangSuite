<#
.SYNOPSIS
    Installs the N'kai Universal Distribution Bundle into a target repository.

.DESCRIPTION
    Part of the N'kai Asymmetric Sidecar Framework (Topic 6.12).
    Deploys canonical skills (visual-grill-me, deep-research, post-mortem, web-qa),
    sidecar templates, context clues skeletons, validation tools, and optional
    real-time audio thread guardrails into any target software project.

.PARAMETER TargetDir
    Root directory of the destination repository. Defaults to current directory ($PWD).

.PARAMETER Audio
    When specified, installs the Audio Guardrails Add-on for C++ audio plugin DSP engines.

.PARAMETER DryRun
    Simulates the installation and prints the execution plan without modifying files.

.PARAMETER Force
    Overwrites existing target files without prompting.

.EXAMPLE
    .\install_nkai_bundle.ps1
    Installs universal pairing core into current directory.

.EXAMPLE
    .\install_nkai_bundle.ps1 -TargetDir "C:\Dev\ToadTracker" -Audio
    Installs universal pairing core plus audio guardrails into ToadTracker.
#>

[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [string]$TargetDir = $PWD,

    [switch]$Audio,
    [switch]$DryRun,
    [switch]$Force
)

# Resolve target directory
$ResolvedTarget = (Resolve-Path $TargetDir -ErrorAction SilentlyContinue)
if (-not $ResolvedTarget) {
    if ($DryRun) {
        $ResolvedTarget = $TargetDir
    } else {
        New-Item -ItemType Directory -Force -Path $TargetDir | Out-Null
        $ResolvedTarget = (Resolve-Path $TargetDir).Path
    }
} else {
    $ResolvedTarget = $ResolvedTarget.Path
}

# Locate bundle root relative to script location
$BundleRoot = Join-Path $PSScriptRoot "bundle"
if (-not (Test-Path $BundleRoot)) {
    # Fallback to canonical location if script was copied elsewhere
    $Fallback = "C:\Dev\nkai\bundle"
    if (Test-Path $Fallback) {
        $BundleRoot = $Fallback
    } else {
        Write-Error "Could not locate N'kai bundle directory at $BundleRoot or $Fallback"
        exit 1
    }
}

Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  N'kai Universal Distribution Bundle Installer (Topic 6.12)" -ForegroundColor Cyan
Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  * Bundle Source:    $BundleRoot" -ForegroundColor Gray
Write-Host "  * Target Workspace: $ResolvedTarget" -ForegroundColor Yellow
$audioStatus = if ($Audio) { 'ENABLED (DSP Audio Add-on)' } else { 'DISABLED (Core Only)' }
Write-Host "  * Audio Add-on:     $audioStatus" -ForegroundColor $(if ($Audio) { 'Green' } else { 'Gray' })
if ($DryRun) {
    Write-Host "  * Execution Mode:   DRY-RUN (Simulated - No disk writes)" -ForegroundColor Magenta
}
Write-Host "----------------------------------------------------------------------" -ForegroundColor DarkGray

$InstalledCount = 0

# Helper: safe copy file
function Install-File {
    param([string]$Src, [string]$Dest)
    $destDir = Split-Path $Dest -Parent
    if (-not (Test-Path $destDir)) {
        if (-not $DryRun) {
            New-Item -ItemType Directory -Force -Path $destDir | Out-Null
        }
    }
    $action = if (Test-Path $Dest) { "OVERWRITE" } else { "CREATE" }
    Write-Host "  [$action] $Dest" -ForegroundColor $(if ($action -eq "CREATE") { 'Green' } else { 'Yellow' })
    if (-not $DryRun) {
        Copy-Item -Path $Src -Destination $Dest -Force
    }
    $script:InstalledCount++
}

# Helper: safe copy directory
function Install-Directory {
    param([string]$SrcDir, [string]$DestDir)
    if (-not (Test-Path $DestDir)) {
        if (-not $DryRun) {
            New-Item -ItemType Directory -Force -Path $DestDir | Out-Null
        }
    }
    $action = if (Test-Path $DestDir) { "SYNC" } else { "CREATE" }
    Write-Host "  [$action] $DestDir" -ForegroundColor $(if ($action -eq "CREATE") { 'Green' } else { 'Cyan' })
    if (-not $DryRun) {
        Copy-Item -Path "$SrcDir\*" -Destination $DestDir -Recurse -Force
    }
    $script:InstalledCount++
}

# 1. Install Templates
Write-Host "`n[1/5] Installing Canonical Templates..." -ForegroundColor White
$templateSrc = Join-Path $BundleRoot "templates\template.html"
$templateDest = Join-Path $ResolvedTarget ".agents\sidecar\template.html"
if (Test-Path $templateSrc) {
    Install-File $templateSrc $templateDest
}

$contextCluesSrc = Join-Path $BundleRoot "templates\context_clues_template.md"
$contextCluesDest = Join-Path $ResolvedTarget "docs\templates\context_clues_template.md"
if (Test-Path $contextCluesSrc) {
    Install-File $contextCluesSrc $contextCluesDest
}

# 2. Install Canonical Skills
Write-Host "`n[2/5] Installing Canonical Skills..." -ForegroundColor White
$skillsSrc = Join-Path $BundleRoot "skills"
if (Test-Path $skillsSrc) {
    Get-ChildItem -Directory $skillsSrc | ForEach-Object {
        $skillName = $_.Name
        $destSkillDir = Join-Path $ResolvedTarget ".agents\skills\$skillName"
        Install-Directory $_.FullName $destSkillDir
    }
}

# 3. Install Rules & Guidelines
Write-Host "`n[3/5] Installing Pairing Guidelines & Add-ons..." -ForegroundColor White
$guidelinesSrc = Join-Path $BundleRoot "rules\nkai_guidelines.md"
$guidelinesDest = Join-Path $ResolvedTarget ".agents\rules\nkai_guidelines.md"
if (Test-Path $guidelinesSrc) {
    Install-File $guidelinesSrc $guidelinesDest
}

if ($Audio) {
    $audioRulesSrc = Join-Path $BundleRoot "rules\audio_guardrails_addon.md"
    $audioRulesDest = Join-Path $ResolvedTarget ".agents\rules\audio_guardrails_addon.md"
    if (Test-Path $audioRulesSrc) {
        Install-File $audioRulesSrc $audioRulesDest
    }
}

# 4. Install Verification Tools
Write-Host "`n[4/5] Installing Validation Tools..." -ForegroundColor White
$toolsSrc = Join-Path $BundleRoot "tools"
if (Test-Path $toolsSrc) {
    Get-ChildItem -File $toolsSrc | ForEach-Object {
        $destToolPath = Join-Path $ResolvedTarget "tools\$($_.Name)"
        Install-File $_.FullName $destToolPath
    }
}

# 5. Patch .gitignore Boundary
Write-Host "`n[5/5] Enforcing Git-Ignore Sidecar Boundary..." -ForegroundColor White
$gitIgnorePath = Join-Path $ResolvedTarget ".gitignore"
$ignoreLines = @(
    "",
    "# =====================================================================",
    "# N'kai Interactive Sidecars and Webview Artifacts (Topic 6.12)",
    "# =====================================================================",
    ".agents/sidecar/*.html",
    "!.agents/sidecar/template.html",
    ".agents/sidecar/*.json",
    ".agents/sidecar/packages/"
)
$ignoreBlock = ($ignoreLines -join "`r`n") + "`r`n"

$alreadyIgnored = $false
if (Test-Path $gitIgnorePath) {
    $existingContent = Get-Content $gitIgnorePath -Raw
    if ($existingContent -and $existingContent.Contains(".agents/sidecar/*.html")) {
        $alreadyIgnored = $true
    }
}

if ($alreadyIgnored) {
    Write-Host "  [OK] Target .gitignore already protects sidecar artifacts." -ForegroundColor Green
} else {
    Write-Host "  [PATCH] Adding sidecar ignore boundary to $gitIgnorePath" -ForegroundColor Yellow
    if (-not $DryRun) {
        Add-Content -Path $gitIgnorePath -Value $ignoreBlock
    }
}

# Summary Report
$statusText = if ($DryRun) { "PREVIEW COMPLETE" } else { "SUCCESSFUL" }
Write-Host "`n======================================================================" -ForegroundColor Cyan
Write-Host "  [PASS] INSTALLATION $statusText" -ForegroundColor Green
Write-Host "======================================================================" -ForegroundColor Cyan
Write-Host "  * Total Actions:        $InstalledCount" -ForegroundColor White
Write-Host "  * Destination:          $ResolvedTarget" -ForegroundColor Gray
Write-Host "  * Run Health Audit:     python tools\audit_sidecar.py .agents\sidecar\template.html" -ForegroundColor DarkGray
Write-Host "  * Run Link Validator:   python tools\verify_context_clues.py" -ForegroundColor DarkGray
Write-Host "======================================================================" -ForegroundColor Cyan
