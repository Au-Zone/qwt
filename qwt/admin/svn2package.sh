#! /bin/sh -x
# 
# Generates a Qwt package from sourceforge cvs
#
# Usage: svn2package.sh [packagename] 
#

##########################
# usage
##########################

function usage() {
    echo "Usage: $0 [packagename]"
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

    svn -q co https://qwt.svn.sourceforge.net/svnroot/qwt/trunk/qwt
    if [ $? -ne 0 ]
    then
        echo "Can't access sourceforge SVN"
        exit $?
    fi

    if [ "$1" != "qwt" ]
    then
        rm -rf $1
        mv qwt $1
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

    find . -name .svn -print | xargs rm -rf
    rm -rf admin 

    PROFILES="qwt.pro examples/examples.pri designer/designer.pro textengines/textengines.pri"
    for PROFILE in $PROFILES
    do
        sed -e 's/= debug/= release/' $PROFILE > $PROFILE.sed
        mv $PROFILE.sed $PROFILE
    done

    # sed -e 's/PROJECT_NUMBER/PROJECT_NUMBER = $VERSION/' doc/Doxyfile > doc/Doxyfile.sed
    # mv doc/Doxyfile.sed doc/Doxyfile
    

    HEADERS=`find . -type f -name '*.h' -print`
    SOURCES=`find . -type f -name '*.cpp' -print`
    PROFILES=`find . -type f -name '*.pro' -print`
    PRIFILES=`find . -type f -name '*.pri' -print`

    for EXPANDFILE in $HEADERS $SOURCES $PROFILES $PRIFILES
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

    sed -e '/GENERATE_LATEX/d' -e '/PROJECT_NUMBER/d' Doxyfile > Doxyfile.doc
    echo 'GENERATE_LATEX = YES' >> Doxyfile.doc
    echo "PROJECT_NUMBER = $VERSION" >> Doxyfile.doc

    doxygen Doxyfile.doc > /dev/null
    if [ $? -ne 0 ]
    then
        exit $?
    fi

    rm -rf Doxyfile.doc Doxygen.log doc/images 

    cd latex
    make > /dev/null 2>&1
    if [ $? -ne 0 ]
    then 
        exit $?
    fi

    mkdir pdf
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
    PRIFILES=`find . -type f -name '*.pri' -print`

    for FILE in $BATCHES $HEADERS $SOURCES $PROFILES $PRIFILES
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

    rm -rf msvc-qmake.bat 

    cd -
}

##########################
# main
##########################

QWTDIR=qwt
VERSION=unknown

while [ $# -gt 0 ] ; do
    case "$1" in
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
createDocs $TMPDIR/doc
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
