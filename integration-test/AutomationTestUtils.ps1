# Helpers for running Unreal automation (unit) tests inside the packaged sample app
#
# The app is launched with `-ExecCmds="Automation RunTests Sentry;Quit"`. The automation controller
# then logs one `Test Completed. Result={...} Name={...} Path={...}` line per test and finishes with
# `**** TEST COMPLETE. EXIT CODE: <code> ****` before quitting.

# Parses automation controller output into a summary of the test run
function Get-AutomationTestResults {
    param (
        [Parameter(Mandatory = $true)]
        [AllowNull()]
        [AllowEmptyCollection()]
        [object]$AppOutput
    )

    $foundCount = $null
    $exitCode = $null
    $tests = @()

    $lines = (@($AppOutput) -join "`n") -split "`r?`n"

    foreach ($line in $lines) {
        if ($line -match 'Found (\d+) automation tests') {
            $foundCount = [int]$Matches[1]
        }
        elseif ($line -match 'Test Completed\. Result=\{(?<result>[^}]*)\} Name=\{(?<name>[^}]*)\} Path=\{(?<path>[^}]*)\}') {
            # Log output may echo the same line more than once (e.g. log and stdout)
            if ($tests | Where-Object { $_.Path -eq $Matches['path'] }) {
                continue
            }
            $tests += [PSCustomObject]@{
                Result = $Matches['result']
                Name   = $Matches['name']
                Path   = $Matches['path']
            }
        }
        elseif ($line -match '\*\*\*\* TEST COMPLETE\. EXIT CODE: (-?\d+) \*\*\*\*') {
            $exitCode = [int]$Matches[1]
        }
    }

    return [PSCustomObject]@{
        FoundCount  = $foundCount
        Tests       = $tests
        # UE 4.27 reports passing tests as 'Passed', newer versions as 'Success'
        FailedTests = @($tests | Where-Object { $_.Result -notin @('Success', 'Passed') })
        ExitCode    = $exitCode
    }
}
