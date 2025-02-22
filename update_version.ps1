# Path to the version file
$filePath = "C:\Users\Jamie\source\repos\Clock\version"

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