#grep '#include.*"' *.h
grep '#include' *.cpp | grep -v '"' | grep -v Pegasus | grep 'include.*\.h'
grep '#include' *.h| grep -v '"' | grep -v Pegasus | grep 'include.*\.h'
