param([int]$Start = 1, [int]$End = 50)

$root = "C:\BlenderProjekte\Projekt Buch Germanische Bibel\OpenGL_Kurs"
$glfwDll = "C:\Users\ionco\vcpkg\installed\x64-windows\bin\glfw3.dll"

for ($i = $Start; $i -le $End; $i++) {
    $num = "{0:D2}" -f $i
    $dir = "$root\Lektion$num"
    if (!(Test-Path $dir)) {
        Write-Host ("Lektion{0}: nicht gefunden, ueberspringe" -f $num) -ForegroundColor Yellow
        continue
    }

    $exe = Get-ChildItem ("{0}\build\Release\Lektion{1}.exe" -f $dir, $num) -ErrorAction SilentlyContinue
    if (!$exe) {
        $exe = Get-ChildItem "$dir\build\Release\*.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    }
    if (!$exe) {
        Write-Host ("Lektion{0}: kein .exe gefunden" -f $num) -ForegroundColor Red
        continue
    }

    $dllDir = $exe.Directory.FullName
    if (!(Test-Path "$dllDir\glfw3.dll")) {
        Copy-Item $glfwDll $dllDir -Force
    }

    Write-Host ("`n===== Lektion{0}: {1} =====" -f $num, $exe.Name) -ForegroundColor Cyan
    Write-Host "Druecke ESC oder schliesse das Fenster fuer naechste Lektion." -ForegroundColor Gray

    $proc = Start-Process -FilePath $exe.FullName -WorkingDirectory $exe.Directory -PassThru
    $proc.WaitForExit()

    Write-Host ("Lektion{0} beendet (Exit: {1})" -f $num, $proc.ExitCode) -ForegroundColor Green
}

Write-Host "`n===== Alle Lektionen durchlaufen =====" -ForegroundColor Cyan
