#! /bin/sh 
# 
# Generates a Qwt package from sourceforge cvs
#
# Usage: cvs2package.sh [-cvs cvsuser] packagename
#

##########################
# usage
##########################

function usage() {
    echo "Usage: cvs2package.sh [-cvs cvsuser] [packagename]"
    exit 1
}

##########################
# checkout cvsuser dirname
##########################

function checkoutQwt() {

    if [ -x qwt ]
    then
        rm -rf qwt
        if [ $? -ne 0 ]
        then
            exit $?
        fi
    fi

    cvs -q -z3 \
        -d$1@qwt.cvs.sourceforge.net:/cvsroot/qwt co qwt > /dev/null
    if [ $? -ne 0 ]
    then
        echo "Can't access sourceforge CVS"
        exit $?
    fi

    if [ $2 != qwt ]
    then
        rm -rf $2
        mv qwt $2
    fi
}

##########################
# cleanQwt dirname
##########################

function cleanQwt {

    cd $1
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    find . -name CVS -print | xargs rm -rf
    find . -name .cvsignore -exec rm -f {} \;
    rm -rf makefiles admin examples/compass 

    PROFILES="qwt.pro examples/examples.pri"
    for PROFILE in $PROFILES
    do
        sed -e 's/= debug/= release/' $PROFILE > $PROFILE.sed
        mv $PROFILE.sed $PROFILE
    done

    HEADERS=`find . -type f -name '*.h' -print`
    SOURCES=`find . -type f -name '*.cpp' -print`
    PROFILES=`find . -type f -name '*.pro' -print`

    for EXPANDFILE in $HEADERS $SOURCES $PROFILES
    do
        expand -4 $EXPANDFILE > $EXPANDFILE.expand
        mv $EXPANDFILE.expand $EXPANDFILE
    done

    cd -
}

##########################
# createDocs dirname
##########################

function createDocs {

    ODIR=`pwd`

    cd $1
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    # We need LateX for the qwtdoc.pdf

    sed -e '/GENERATE_LATEX/d' Doxyfile > Doxyfile.doc
    echo 'GENERATE_LATEX = YES' >> Doxyfile.doc

    doxygen Doxyfile.doc > /dev/null
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    rm -rf Doxyfile.doc Doxygen.log doc/images 

    mkdir doc/pdf
    cd doc/latex
    make > /dev/null 2>&1
    if [ $? -ne 0 ]
    then 
        exit $?
    fi

    mv refman.pdf ../pdf/qwtdoc.pdf
    cd ..
    rm -rf latex postscript
    
    cd $ODIR
}

##########################
# posix2dos filename
##########################

function posix2dos {
    # At least one unix2dos writes to stdout instead of overwriting the input.
    # The -q option is always enabled in stdin->stdout mode.
    unix2dos <$1 >$1.dos
    mv $1.dos $1
}

##########################
# prepare4Win dirname
##########################

function prepare4Win {

    cd $1
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    rm -rf qwt.spec qwt.spec.in doc/man examples/linux

    # win files, but not uptodate
    rm -rf qwt.sln qwt.dsw examples/examples.dsw

    BATCHES=`find . -type f -name '*.bat' -print`
    HEADERS=`find . -type f -name '*.h' -print`
    SOURCES=`find . -type f -name '*.cpp' -print`
    PROFILES=`find . -type f -name '*.pro' -print`

    for FILE in $BATCHES $HEADERS $SOURCES $PROFILES
    do
        posix2dos $FILE
    done

    cd -
}

##########################
# prepare4Unix dirname
##########################

function prepare4Unix {

    cd $1
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    rm -rf msvc-qmake.bat examples/examples.dsw
    rm -rf qwt.sln qwt.dsw

    cd -
}

##########################
# main
##########################

QWTDIR=qwt
CVSUSER=:pserver:anonymous
VERSION=unknown

while [ $# -gt 0 ] ; do
    case "$1" in
        -cvs)
            CVSUSER=$2; shift 2;;
        -h|--help)
            usage; exit 1 ;;
        *) 
            QWTDIR=qwt-$1 ; VERSION=$1; shift;;
    esac
done

TMPDIR=/tmp/$QWTDIR-tmp

echo -n "checkout to $TMPDIR ... "
checkoutQwt $CVSUSER $TMPDIR
cleanQwt $TMPDIR
echo done

echo -n "generate documentation ... "
createDocs $TMPDIR
mv $TMPDIR/doc/pdf/qwtdoc.pdf $QWTDIR.pdf
rmdir $TMPDIR/doc/pdf
echo done


DIR=`pwd`
echo -n "create packages in $DIR ... "

cd /tmp

rm -rf $QWTDIR
cp -a $TMPDIR $QWTDIR
prepare4Unix $QWTDIR
sed "s/@VERSION@/$VERSION/g;s/@TARBALL@/tgz/g" $QWTDIR/qwt.spec.in >$QWTDIR/qwt.spec
tar cfz $QWTDIR.tgz $QWTDIR
sed "s/@VERSION@/$VERSION/g;s/@TARBALL@/tar.bz2/g" $QWTDIR/qwt.spec.in >$QWTDIR/qwt.spec
tar cfj $QWTDIR.tar.bz2 $QWTDIR

rm -rf $QWTDIR
cp -a $TMPDIR $QWTDIR
prepare4Win $QWTDIR
zip -r $QWTDIR.zip $QWTDIR > /dev/null

rm -rf $TMPDIR $QWTDIR

mv $QWTDIR.tgz $QWTDIR.tar.bz2 $QWTDIR.zip $DIR/
echo done

exit 0
