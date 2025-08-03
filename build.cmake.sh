#!/bin/bash
mkdir -p build/ext/wxwidgets-3.3.1

# wxWidgets (-DwxBUILD_SAMPLES=SOME | ALL | OFF )
pushd build
pushd ext
pushd wxwidgets-3.3.1
cmake ../../../ext/wxwidgets-3.3.1 -DwxBUILD_SHARED=OFF -DwxBUILD_SAMPLES=OFF
cmake --build .  --parallel 9
popd
popd
popd
 
# obdc_explorer 
pushd build
cmake .. --fresh
cmake --build . --parallel 9
popd

