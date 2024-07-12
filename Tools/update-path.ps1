#
# Given a semicolon-delimited list of directories, check machine PATH from registry to see
# if they're present. If not, insert them at the beginning.
# Updating the user PATH is insufficient as the machine PATH takes precedence.
# We need the specified directories to be checked before others to avoid the
# wrong tool being executed (make, sh, etc.)
#

param (
    [Parameter(Mandatory=$true)]
    [String]$dirstoinsert
)

function contains{
    param(
        [array]$array,
        [string]$value
    )
    foreach ($elem in $array) {
        if ($elem -eq $value) {
            return $true
        }
    }
    return $false
}

try {
    $syspath = [Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::Machine)
    $syspathArray = ($syspath -split ";").where({ "" -ne $_ })
    $userpath = [Environment]::GetEnvironmentVariable("PATH", [System.EnvironmentVariableTarget]::User)
    $userpathArray = ($userpath -split ";").where({ "" -ne $_ })
    $dirstoinsertArray = ($dirstoinsert -split ";")

    Write-Host "Checking system PATH..."

    # Check if we need to update path
    $needupdate = $false
    foreach ($dir in $dirstoinsertArray) {
        if (! (contains $syspathArray $dir) ) {
            $needupdate = $true
            break
        }
    }
    if (! $needupdate) {
        Write-Host "No changes to system PATH required"
        return
    }

    # Update and de-duplicate entries (PATH can get very messy over time)
    $newsyspathArray = @()
    foreach ($dir in $dirstoinsertArray+$syspathArray) {
        if (contains $newsyspathArray $dir) {
            continue
        }
        if (contains $userpathArray $dir) {
            Write-Host "WARNING: '$dir' in user path"
        }
        $newsyspathArray += $dir
    }

    Write-Host "Updating system PATH"
    $newsyspath = ($newsyspathArray -join ";")
    Write-Host "  From: $syspath"
    Write-Host "  To:   $newsyspath"
    [Environment]::SetEnvironmentVariable("PATH", $newsyspath, [EnvironmentVariableTarget]::Machine)
    Write-Host "System PATH updated"
}
catch {
    $_
    exit 1
}
