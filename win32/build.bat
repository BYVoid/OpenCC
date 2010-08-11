@echo off

rm -rf release

echo Compiling opencc.dll
gcc ../src/*.c ../src/dictionary/*.c -Wall -c -O2 -DBYTEORDER=LITTLE_ENDIAN -I.
gcc *.o -o opencc.dll -liconv -shared
rm *.o

echo Compiling opencc_dict.exe
gcc ../src/tools/opencc_dict.c -Wall -c -O2 -I.
gcc *.o -o opencc_dict.exe -lopencc -L.
rm *.o

echo Compiling opencc.exe
gcc ../src/tools/opencc.c -Wall -c -O2 -I.
gcc *.o -o opencc.exe -lopencc -L.
rm *.o

echo Generating dictionaries
opencc_dict.exe -i ../data/simp_to_trad/characters.txt -o simp_to_trad_characters.ocd
opencc_dict.exe -i ../data/simp_to_trad/phrases.txt -o simp_to_trad_phrases.ocd
opencc_dict.exe -i ../data/trad_to_simp/characters.txt -o trad_to_simp_characters.ocd
opencc_dict.exe -i ../data/trad_to_simp/phrases.txt -o trad_to_simp_phrases.ocd

cp ../data/config/*.ini .

echo Testing zhs2zht
opencc.exe -i ../test/testcases/zhs2zht.in -o zhs2zht.out -c zhs2zht.ini
if errorlevel 1 pause
cp ../test/testcases/zhs2zht.ans .
fc zhs2zht.ans zhs2zht.out
if errorlevel 1 pause
rm zhs2zht.ans zhs2zht.out

echo Testing zht2zhs
opencc.exe -i ../test/testcases/zht2zhs.in -o zht2zhs.out -c zht2zhs.ini
if errorlevel 1 pause
cp ../test/testcases/zht2zhs.ans .
fc zht2zhs.ans zht2zhs.out
if errorlevel 1 pause
rm zht2zhs.ans zht2zhs.out

echo Testing mix2zht
opencc.exe -i ../test/testcases/mix2zht.in -o mix2zht.out -c mix2zht.ini
if errorlevel 1 pause
cp ../test/testcases/mix2zht.ans .
fc mix2zht.ans mix2zht.out
if errorlevel 1 pause
rm mix2zht.ans mix2zht.out

echo Testing mix2zhs
opencc.exe -i ../test/testcases/mix2zhs.in -o mix2zhs.out -c mix2zhs.ini
if errorlevel 1 pause
cp ../test/testcases/mix2zhs.ans .
fc mix2zhs.ans mix2zhs.out
if errorlevel 1 pause
rm mix2zhs.ans mix2zhs.out

mkdir release
mv *.ini release
mv *.exe release
mv *.dll release
mv *.ocd release

echo Successfully built
pause