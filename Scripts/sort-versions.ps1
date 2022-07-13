param(
    [Parameter(Mandatory = $true)][string[]] $List
)

Set-StrictMode -Version latest

function GetComparablePart([Parameter(Mandatory = $true)][string] $value)
{
    $value.PadLeft(10, '0')
}

function GetComparableVersion([Parameter(Mandatory = $true)][string] $value)
{
    $value = $value -replace '^v', ''
    $output = ''
    $buffer = ''
    for ($i = 0; $i -lt $value.Length; $i++)
    {
        $char = $value[$i]
        if ("$char" -match '[^a-zA-Z0-9]')
        {
            # Found a separtor, update the current buffer
            $output += GetComparablePart $buffer
            $output += $char
            $buffer = ''
        }
        else
        {
            $buffer += $char
        }
    }
    if ($buffer.Length -gt 0)
    {
        $output += GetComparablePart $buffer
    }

    $output
}

$List | Sort-Object -Property @{Expression = { GetComparableVersion $_ } }