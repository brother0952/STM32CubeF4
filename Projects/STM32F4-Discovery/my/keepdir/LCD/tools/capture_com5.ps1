param(
  [string]$PortName = "COM5",
  [int]$TimeoutSeconds = 300
)

$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$outBin = Join-Path $root "serial_capture.bin"
$outTxt = Join-Path $root "serial_text.txt"

Write-Host "Opening $PortName at 115200 8N1 ..."
$port = New-Object System.IO.Ports.SerialPort
$port.PortName = $PortName
$port.BaudRate = 115200
$port.Parity = [System.IO.Ports.Parity]::None
$port.DataBits = 8
$port.StopBits = [System.IO.Ports.StopBits]::One
$port.Handshake = [System.IO.Ports.Handshake]::None
$port.ReadTimeout = 500

try {
  $port.Open()
} catch {
  Write-Host "FAILED to open $PortName : $($_.Exception.Message)"
  Write-Host "Close any serial terminal using $PortName and run this again."
  exit 1
}

Write-Host "Listening. Press RESET on the board now."
Write-Host "The program stops automatically after all 8 polarity frames."

$fs = [System.IO.File]::Create($outBin)
$text = New-Object System.Text.StringBuilder
$deadline = (Get-Date).AddSeconds($TimeoutSeconds)
$count = 0
$lastReport = Get-Date
$lastData = Get-Date
$buf = New-Object byte[] 4096

try {
  while ((Get-Date) -lt $deadline) {
    $available = $port.BytesToRead
    if ($available -gt 0) {
      if ($available -gt $buf.Length) { $available = $buf.Length }
      $read = $port.Read($buf, 0, $available)
      if ($read -gt 0) {
        $fs.Write($buf, 0, $read)
        $count += $read
        $lastData = Get-Date
        for ($i = 0; $i -lt $read; $i++) {
          $b = $buf[$i]
          if ($b -eq 13) {
            [void]$text.Append("`r")
          } elseif ($b -eq 10) {
            [void]$text.Append("`n")
          } elseif ($b -ge 32 -and $b -lt 127) {
            [void]$text.Append([char]$b)
          } else {
            [void]$text.Append(".")
          }
        }
        $fs.Flush()
      }
    } else {
      Start-Sleep -Milliseconds 5
    }

    if (($count -gt 0) -and (((Get-Date) - $lastData).TotalSeconds -ge 5)) {
      Write-Host "No data for 5 seconds, stopping."
      break
    }

    if (((Get-Date) - $lastReport).TotalSeconds -ge 5) {
      Write-Host ("... waiting, received {0} bytes" -f $count)
      $lastReport = Get-Date
    }
  }
} finally {
  $fs.Close()
  $port.Close()
  [System.IO.File]::WriteAllText($outTxt, $text.ToString())
}

if ($count -eq 0) {
  Write-Host "TIMEOUT: no data received."
  exit 2
}

Write-Host ""
Write-Host ("DONE: captured {0} bytes" -f $count)
Write-Host "Binary: $outBin"
Write-Host "Text:   $outTxt"
Write-Host "Please tell the agent that the capture is finished."
