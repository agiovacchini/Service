#!/bin/bash
cppcheck --library=posix.cfg -j 4 --enable=all --suppress=missingIncludeSystem --xml --verbose -f -I /usr/include -I /usr/local/include -I /usr/local/include/boost ./ 2> err.xml
cppcheck-htmlreport --file err.xml --report-dir="CodeAnalysisReport" --title="PromoCalculator"

<--- When an object of a class is created, the constructors of all member variables are called consecutively in the order the variables are declared, even if you don't explicitly write them to the initialization list. You could avoid assigning 'description' a value by passing the value to the constructor in the initialization list.
}