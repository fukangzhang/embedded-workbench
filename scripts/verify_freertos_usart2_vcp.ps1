param(
    [string]$Port = "",
    [int]$BaudRate = 9600,
    [int]$ReadTimeoutMs = 1500,
    [string[]]$Commands = @(
        "STATUS?",
        "CONFIG?",
        "SAMPLE 360 600 250 20",
        "STATUS?"
    ),
    [string[]]$Expect = @(
        "OK result=ok",
        "CONFIG temp_warn=",
        "STATUS state=warning"
    ),
    [string]$LogPath = "",
    [string]$ReplayLogPath = "",
    [switch]$ListPorts,
    [switch]$NoExpect,
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"

function Read-SerialForWindow {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [int]$TimeoutMs
    )

    $Stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $Text = New-Object System.Text.StringBuilder

    while ($Stopwatch.ElapsedMilliseconds -lt $TimeoutMs) {
        $Chunk = $Serial.ReadExisting()
        if ($Chunk.Length -gt 0) {
            [void]$Text.Append($Chunk)
        }
        Start-Sleep -Milliseconds 25
    }

    return $Text.ToString()
}

function Write-TranscriptLine {
    param(
        [System.Collections.Generic.List[string]]$Transcript,
        [string]$Line
    )

    $Transcript.Add($Line)
    Write-Host $Line
}

function Test-ExpectedText {
    param(
        [System.Collections.Generic.List[string]]$Transcript,
        [string]$Text,
        [string[]]$ExpectedSnippets,
        [bool]$Disabled
    )

    $Failed = $false

    if ($Disabled) {
        Write-TranscriptLine $Transcript "[vcp] Expectations: disabled"
        return $false
    }

    foreach ($Expected in $ExpectedSnippets) {
        if ($Text.Contains($Expected)) {
            Write-TranscriptLine $Transcript "[vcp] EXPECT ok: $Expected"
        } else {
            Write-TranscriptLine $Transcript "[vcp] EXPECT missing: $Expected"
            $Failed = $true
        }
    }

    return $Failed
}

$AvailablePorts = [System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object

if ($ListPorts) {
    if ($AvailablePorts.Count -eq 0) {
        Write-Host "[vcp] No serial ports were reported by Windows."
    } else {
        Write-Host "[vcp] Available serial ports:"
        $AvailablePorts | ForEach-Object { Write-Host "[vcp]   $_" }
    }

    if (-not $DryRun -and $Port.Length -eq 0) {
        exit 0
    }
}

if ($DryRun) {
    $DisplayPort = if ($Port.Length -gt 0) { $Port } else { "COMx" }
    Write-Host "[vcp] Dry run only; serial port was not opened."
    Write-Host "[vcp] Port: $DisplayPort"
    Write-Host "[vcp] Baud: $BaudRate 8N1"
    Write-Host "[vcp] Read timeout per command: ${ReadTimeoutMs}ms"
    Write-Host "[vcp] Commands:"
    $Commands | ForEach-Object { Write-Host "[vcp]   $_" }
    if ($ReplayLogPath.Length -gt 0) {
        Write-Host "[vcp] Replay log: $ReplayLogPath"
    }
    if ($NoExpect) {
        Write-Host "[vcp] Expectations: disabled"
    } else {
        Write-Host "[vcp] Expected response snippets:"
        $Expect | ForEach-Object { Write-Host "[vcp]   $_" }
    }
    Write-Host "[vcp] Example real run:"
    Write-Host ".\scripts\verify_freertos_usart2_vcp.ps1 -Port $DisplayPort"
    exit 0
}

$Transcript = [System.Collections.Generic.List[string]]::new()
$ExpectationFailed = $false

if ($ReplayLogPath.Length -gt 0) {
    $ReplayText = Get-Content -Path $ReplayLogPath -Raw
    Write-TranscriptLine $Transcript "[vcp] Replaying transcript: $ReplayLogPath"
    $ExpectationFailed = Test-ExpectedText $Transcript $ReplayText $Expect $NoExpect

    if ($ExpectationFailed) {
        throw "VCP response expectations failed. Check the transcript for missing expected text."
    }

    exit 0
}

if ($Port.Length -eq 0) {
    throw "Pass -Port with the ST-LINK VCP port, for example -Port COM5. Use -ListPorts to inspect candidates."
}

$Serial = [System.IO.Ports.SerialPort]::new($Port, $BaudRate, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
$Serial.NewLine = "`n"
$Serial.ReadTimeout = $ReadTimeoutMs
$Serial.WriteTimeout = $ReadTimeoutMs

$AllResponseText = New-Object System.Text.StringBuilder

try {
    $Serial.Open()
    $Serial.DiscardInBuffer()
    $Serial.DiscardOutBuffer()

    Write-TranscriptLine $Transcript "[vcp] Port opened: $Port @ $BaudRate 8N1"
    Write-TranscriptLine $Transcript "[vcp] Firmware expected: build-fw-scheduler-real-usart2-io/embedded_firmware.elf"

    foreach ($Command in $Commands) {
        Write-TranscriptLine $Transcript "[vcp] >>> $Command"
        $Serial.WriteLine($Command)
        $Response = Read-SerialForWindow $Serial $ReadTimeoutMs

        if ($Response.Length -eq 0) {
            Write-TranscriptLine $Transcript "[vcp] <<< <no response within ${ReadTimeoutMs}ms>"
        } else {
            $Normalized = $Response -replace "`r", ""
            [void]$AllResponseText.Append($Normalized)
            foreach ($Line in ($Normalized -split "`n")) {
                if ($Line.Length -gt 0) {
                    Write-TranscriptLine $Transcript "[vcp] <<< $Line"
                }
            }
        }
    }

    $ExpectationFailed = Test-ExpectedText $Transcript $AllResponseText.ToString() $Expect $NoExpect
} finally {
    if ($Serial.IsOpen) {
        $Serial.Close()
    }
}

if ($LogPath.Length -gt 0) {
    $LogDirectory = Split-Path -Parent $LogPath
    if ($LogDirectory.Length -gt 0) {
        New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null
    }
    Set-Content -Path $LogPath -Value $Transcript -Encoding UTF8
    Write-Host "[vcp] Transcript written: $LogPath"
}

if ($ExpectationFailed) {
    throw "VCP response expectations failed. Check the transcript for missing expected text."
}
