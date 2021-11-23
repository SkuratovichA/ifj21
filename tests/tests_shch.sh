#!/bin/bash

# ./run_tests sasha [err_number] [valgrind | coverage]
# not it works only fow ./run_tests sasha [err_number] [valgrind | coverage]


RED='\033[0;31m'
NC='\033[0m'
GREEN='\033[0;32m'

name=$1

expected_err=$2
valgrind_en=$3
folder=""
coverage=$3
err_files=0
err_memory=0
all_files=0


# name of binary
BIN_TARGET="ifj21"
COMPILE_FLAGS="-DDEBUG=on"

# directory with source code
# in this case, tests is pwd and ../ is src
SRC_DIR="/Users/suka/Desktop/vut/sem3/ifj/proj/ifj21" #"`pwd`/.."

# directory with html for statistics and .json files with code coverage
STAT_DIR="$SRC_DIR/statistics"

# build directory(related to directory with tests)
BUILD_DIR="$SRC_DIR/cmake-build-debug"

#coverage_scanner()
#{
#    coverage_DIR="$SRC_DIR/CMakeFiles/scannerTests.dir"
#
#    cd "$BUILD_DIR" || { echo "ERROR: No directory exists: $BUILD_DIR" ; exit 1; }
#
#    { cmake $COMPILE_FLAGS .. 1>/dev/null && {
#      { make $BIN_TARGET >/dev/null 2>/dev/null  && ./scannerTests ; } || {
#        echo "ERROR: Cannot compile a target" ; exit 1; }
#      }
#    } || {
#      echo "Cannot initialize cmake. Exiting..." ; exit 1;
#    }
#
#    cd "$coverage_DIR" || { echo "ERROR: No such directory $coverage_DIR" ; exit 1 ; }
#    gcovr -b --filter "$SRC_DIR" --json > code1.json || { echo "ERROR: cannot initialize gcovr. " ; }
#}

coverage_other()
{
    NUM=$1

    cd "$BUILD_DIR"/CMakeFiles/"$BIN_TARGET".dir || {
      echo "ERROR: cannot cd $BUILD_DIR/CMakeFiles/$BIN_TARGET" ; exit 1 ;
    }

    # create .json file


    # something like if -ne $STAT_DIR
    if [ ! -d "$STAT_DIR" ] ; then
        echo "creating $STAT_DIR"
        mkdir "$STAT_DIR"
    fi

    gcovr -b --filter "$SRC_DIR" --json > "$STAT_DIR/code$NUM.json"
}


if [[ "$name" == "all" ]]; then
    # create a directory for statistics
    if [ ! -d $STAT_DIR ]; then
        echo "Cannot enter a directory $STAT_DIR. Creating one";
        rm -rf "$STAT_DIR" 1> /dev/null 2>/dev/null ;
        mkdir "$STAT_DIR" ;
        cd "$STAT_DIR" || { echo "ERROR. cannot create $STAT_DIR. Exiting." ; exit 1 ; }
    fi

    # create a directory for html pages
    mkdir "$STAT_DIR/html" || { echo "$STAT_DIR/html already exists " ; }

    for NUM_TEST in 0 2 3 4 5 6 7
    do
        echo "******** RET_VALUE = $NUM_TEST ********"
        ./run_tests.sh "*" "$NUM_TEST" 1>/dev/null 2>/dev/null
        echo "*******************************"
        echo ""

        coverage_other $NUM_TEST
    done

    gcovr --filter "$SRC_DIR" --add-tracefile "$STAT_DIR/code0.json"  \
                             --add-tracefile "$STAT_DIR/code2.json" \
                             --add-tracefile "$STAT_DIR/code3.json" \
                             --add-tracefile "$STAT_DIR/code4.json" \
                             --add-tracefile "$STAT_DIR/code5.json" \
                             --add-tracefile "$STAT_DIR/code6.json" \
                             --add-tracefile "$STAT_DIR/code7.json" \
                             --html --html-details -o "$STAT_DIR/html/coverage.html" || { echo "ERROR: cannot cover" ; }
    open "$STAT_DIR/html/coverage.html"
    exit 0 ;
fi

cd "$BUILD_DIR" || { echo "ERROR: no build dir" ; exit 1;  }
rm -rf *
cmake ..
make "$BIN_TARGET" || { echo "ERROR: cannot make" ; exit 1; }

cd "$SRC_DIR/tests" || { echo "ERROR. No tests directory" ; exit 1 ; }

echo ""
# we need folders with tests (in tests/ directory)
if [[ "$expected_err" -eq "0" ]]; then
    folder="without_errors"
elif [[ "$expected_err" -eq "2" ]]; then
    folder="syntax_errors"
elif [[ "$expected_err" -ge "3" ]] && [[ "$expected_err" -le "7" ]]; then
    folder="semantic_errors"
else
    echo "This script doesnt support this type of error $expected_err"
    exit 1
fi

for file in ${folder}/${name}*_${expected_err}.tl;
do
    if [[ "${folder}/${name}*_${expected_err}.tl" == "$file" ]]; then
        echo "Files with these arguments were not found"
        exit 1
    fi

    all_files=$((all_files+1))

    if [[ "$valgrind_en" == "valgrind" ]]; then
        valgrind --log-file="tmp.txt" ../cmake-build-debug/ifj21 <$file 2>/dev/null
        		#valgrind --log-file="tmp.txt" build/compiler <$file 2>/dev/null
        ret_val=$?
        OUT1=$(cat tmp.txt | grep -h 'in use at exit:')
        OUT2=$(cat tmp.txt | grep -h 'errors from')
        if [[ "$OUT1" == *"0 bytes in 0 blocks"* ]]; then
            if [[ "$OUT2" != *"0 errors from 0 contexts"* ]]; then
                err_memory=$((err_memory+1))
                printf "$file ${RED}ERROR${NC} with memory\n"
                echo ""
            fi
        else
            err_memory=$((err_memory+1))
            printf "$file ${RED}ERROR${NC} with memory\n"
            echo ""
        fi

        rm tmp.txt
    else
        "$BUILD_DIR"/"$BIN_TARGET" < "$file" 2>/dev/null 1>/dev/null
        ret_val=$?
    fi

    if [ $ret_val -ne "$expected_err" ]; then
        err_files=$((err_files+1))
        printf "$file ${RED}FAILED${NC} ret_val = $ret_val, expected $expected_err\n"
        echo ""
    fi
done

if [[ "$err_files" -eq "0" ]] && [[ "$err_memory" -eq "0" ]] && [[ "$valgrind_en" == "valgrind" ]]; then
    printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    printf "No errors with memory\n"
elif [[ "$err_files" -eq "0" ]] && [[ "$valgrind_en" == "" ]]; then
    printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    printf "Errors with memory were not enabled\n"
else
    if [ $err_files -ne 0 ]; then
        printf "$err_files/$all_files tests were ${RED}FAILED${NC}\n"
        err_files=$((all_files-err_files))
        printf "$err_files/$all_files tests were ${GREEN}PASSED${NC}"
    else
        printf "$all_files/$all_files tests were ${GREEN}PASSED${NC}\n"
    fi

    if [[ "$valgrind_en" == "valgrind" ]] && [ $err_memory -ne 0 ]; then
        printf "$err_memory errors with memory was detected\n"
    elif [[ "$valgrind_en" == "valgrind" ]] && [ $err_memory -eq 0 ]; then
        printf "No errors with memory\n"
    else
        echo "Errors with memory were not enabled"
    fi
fi

if [[ "$coverage" == "coverage" ]]; then

    # something like if -ne $STAT_DIR
    if [ ! -d "$STAT_DIR" ] ; then
        echo "creating $STAT_DIR"
        mkdir "$STAT_DIR"
    fi
    mkdir "$STAT_DIR/html" || { echo "$STAT_DIR/html already exists" ; }

    coverage_other 0
    echo "source directory $SRC_DIR"
    gcovr --filter "$SRC_DIR" --add-tracefile "$STAT_DIR/code0.json" --html --html-details -o "$STAT_DIR/html/coverage.html"
    open "$STAT_DIR/html/coverage.html"
fi


# delete directory with statistics
# rm -rf "$STAT_DIR"
