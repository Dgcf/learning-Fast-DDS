# /bin/bash

cmake -H. -Bbuild -DCOMPILE_EXAMPLES=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build
export LD_LIBRARY_PATH=/FastDDS/learning-Fast-DDS/build/src/cpp:$LD_LIBRARY_PATH
export FASTRTPS_DEFAULT_PROFILES_FILE=/FastDDS/learning-Fast-DDS/testFastDDS.xml
