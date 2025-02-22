# Copyright (C) 2025 Jamie Howell
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

# Script to keep track of build number
# Path to the version file
$filePath = "version"

# Check if the version file exists
if (Test-Path $filePath) {
    # Read the current version from the file
    $currentVersion = Get-Content $filePath -Raw

    # Split the version string into components
    $versionParts = $currentVersion.Split('.')

    # Ensure there are at least 3 components in the version (major.minor.patch)
    if ($versionParts.Length -ge 3) {
        # Increment the last part (patch number)
        $patchVersion = [int]$versionParts[-1] + 1
        $versionParts[-1] = $patchVersion.ToString()

        # Join the version components back into a string
        $newVersion = $versionParts -join '.'

        # Write the new version back to the file
        Set-Content $filePath $newVersion

        Write-Host "Version updated to: $newVersion"
    } else {
        Write-Host "Invalid version format in file."
    }
} else{
    Write-Host "Version file not found."
}
