
Push-Location -Path "..\data\dictionary"
New-Item -ItemType file "TWPhrases.txt" -force 
$file1 = Get-Content "TWPhrasesIT.txt" -Encoding UTF8
$file2 = Get-Content "TWPhrasesName.txt" -Encoding UTF8
$file3 = Get-Content "TWPhrasesOther.txt" -Encoding UTF8
Add-Content "TWPhrases.txt" $file1 -Encoding UTF8
Add-Content "TWPhrases.txt" $file2 -Encoding UTF8
Add-Content "TWPhrases.txt" $file3 -Encoding UTF8
Pop-Location