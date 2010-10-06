REM Batch file to make all Makefiles or all Visual Studio project files
REM (*.dsp for MSVC-6.0 or *.vcproj for MSVC-7.0) for Qwt with qmake.
REM
REM BUG: the designer plugin *.dsp file may not work; the Makefile does.
REM
REM To make Makefiles, type: msvc-qmake
REM To make project files type: msvc-qmake vc

REM For the Qwt library:
cd src
qmake -t %1lib% src.pro
cd ..

REM For the designer plugin:
cd textengines\mathml
qmake -t %1lib mathml.pro
cd ..\..

REM For the designer plugin:
cd designer
qmake -t %1lib designer.pro
cd ..

REM For the examples:
cd examples
cd bode
qmake -t %1app bode.pro
cd ..\cpuplot
qmake -t %1app cpuplot.pro
cd ..\curvdemo1
qmake -t %1app curvdemo1.pro
cd ..\curvdemo2
qmake -t %1app curvdemo2.pro
cd ..\dials
qmake -t %1app dials.pro
cd ..\event_filter
qmake -t %1app event_filter.pro
cd ..\friedberg
qmake -t %1app friedberg.pro
cd ..\navigation
qmake -t %1app navigation.pro
cd ..\oscilloscope
qmake -t %1app oscilloscope.pro
cd ..\plotmatrix
qmake -t %1app plotmatrix.pro
cd ..\radio
qmake -t %1app radio.pro
cd ..\rasterview
qmake -t %1app rasterview.pro
cd ..\realtime
qmake -t %1app realtime.pro
cd ..\refreshtest
qmake -t %1app refreshtest.pro
cd ..\sinusplot
qmake -t %1app sinusplot.pro
cd ..\sliders
qmake -t %1app sliders.pro
cd ..\spectrogram
qmake -t %1app spectrogram.pro
cd ..\svgmap
qmake -t %1app svgmap.pro
cd ..\sysinfo
qmake -t %1app sysinfo.pro
cd ..\tvplot
qmake -t %1app tvplot.pro
cd ..\..

REM EOF
