Write-Host "Start script for odr tests" -ForegroundColor Magenta
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path # расположение самого скрипта
$BuildDir  = Join-Path $ScriptDir "build"

Write-Host "--- Configure ---" -ForegroundColor cyan
cmake 	-S $ScriptDir `
		-B $BuildDir `
		-G "MinGW Makefiles" `
		-DCMAKE_C_COMPILER=gcc `
		-DCMAKE_CXX_COMPILER=g++ `
		-Wno-dev
			
if ($LASTEXITCODE -ne 0) {
	Write-Error "Configure failed for config $($cfg.Name)"
	exit $LASTEXITCODE
}
	
Write-Host "--- Build ---" -ForegroundColor Cyan
cmake --build "$BuildDir"

if ($LASTEXITCODE -ne 0) {
  Write-Error "Build failed"
  exit $LASTEXITCODE
}

function Run-Exe([string]$Path) {
	if (-not (Test-Path $Path)) {
		Write-Error "Executable not found: $Path"
		exit 1
	}

	Write-Host "--- Run: $Path ---" -ForegroundColor Cyan
	& $Path
	$code = $LASTEXITCODE
	Write-Host "EXIT=$code" | Out-Host

	if ($code -ne 0) {
    Write-Error "Test failed: $Path (exit code $code)"
    exit $code
  }
}

Run-Exe (Join-Path $BuildDir "src_files.exe")
Run-Exe (Join-Path $BuildDir "odr_autoinit.exe")

Write-Host "All ODR tests passed." -ForegroundColor Green