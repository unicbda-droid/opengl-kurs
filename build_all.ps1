param([string]$Mode = "status")

$root = "C:\BlenderProjekte\Projekt Buch Germanische Bibel\OpenGL_Kurs"
$glfw = "C:/Users/ionco/vcpkg/installed/x64-windows/share/glfw3"

function Get-Status {
    $results = @()
    Get-ChildItem "$root\Lektion*" -Directory | Sort-Object Name | ForEach-Object {
        $name = $_.Name
        $num = [int]($name -replace '\D','')
        $exe = Get-ChildItem "$_\build\Release\$name.exe" -ErrorAction SilentlyContinue
        $exe2 = Get-ChildItem "$_\build\Release\Lektion*.exe" -ErrorAction SilentlyContinue
        $hasExe = ($exe -or $exe2)
        $hasBuild = Test-Path "$_\build"
        $hasCMake = Test-Path "$_\CMakeLists.txt"
        $status = if ($hasExe) { "READY" } elseif ($hasBuild) { "BUILT_FAIL" } else { "NOT_BUILT" }
        $results += [PSCustomObject]@{
            Lesson = $name
            Num = $num
            Status = $status
            Exe = if ($exe) { $exe.FullName } elseif ($exe2) { $exe2.FullName } else { "" }
        }
    }
    return $results
}

function Build-Lesson {
    param($Dir)
    $name = Split-Path $Dir -Leaf
    Write-Host "`n=== Building $name ==="
    $buildDir = "$Dir\build"
    if (!(Test-Path $buildDir)) { New-Item -ItemType Directory -Path $buildDir -Force | Out-Null }
    
    # Check if GLEW is needed (Lektion 35+)
    $cmake = Get-Content "$Dir\CMakeLists.txt" -Raw
    $useGlew = $cmake -match "GLEW"
    
    $args = @("-B", $buildDir, "-S", $Dir, "-Dglfw3_DIR=$glfw")
    if ($useGlew) {
        $args += "-Dglew_DIR=C:/Users/ionco/vcpkg/installed/x64-windows/share/glew"
    }
    
    $result = cmake @args 2>&1
    if ($LASTEXITCODE -eq 0) {
        $result2 = cmake --build $buildDir --config Release 2>&1
        if ($LASTEXITCODE -eq 0) {
            Write-Host "  OK" -ForegroundColor Green
            return $true
        } else {
            Write-Host "  BUILD FAILED" -ForegroundColor Red
            return $false
        }
    } else {
        Write-Host "  CMAKE FAILED" -ForegroundColor Red
        return $false
    }
}

# ─── Main ───
$all = Get-Status
$ready = $all | Where-Object Status -eq "READY"
$fail = $all | Where-Object Status -eq "BUILT_FAIL"
$missing = $all | Where-Object Status -eq "NOT_BUILT"

Write-Host "`n===== OpenGL Kurs Status =====" -ForegroundColor Cyan
Write-Host "`nFERTIG ($($ready.Count)):"
$ready | ForEach-Object { Write-Host "  $($_.Lesson) -> $($_.Exe)" -ForegroundColor Green }

if ($fail.Count -gt 0) {
    Write-Host "`nFEHLGESCHLAGEN ($($fail.Count)):"
    $fail | ForEach-Object { Write-Host "  $($_.Lesson)" -ForegroundColor Red }
}

Write-Host "`nFEHLT ($($missing.Count)):"
$missing | ForEach-Object { Write-Host "  $($_.Lesson)" -ForegroundColor Yellow }

if ($Mode -eq "buildall") {
    Write-Host "`n===== Baue fehlende Lektionen =====" -ForegroundColor Cyan
    $missing | ForEach-Object {
        $dir = "$root\$($_.Lesson)"
        Build-Lesson $dir
    }
    Write-Host "`n===== Fertig. Neuer Status: =====" -ForegroundColor Cyan
    Get-Status | Format-Table Lesson, Status -AutoSize
}
