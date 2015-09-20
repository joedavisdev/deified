#!/usr/bin/env bash
orig_dir=${pwd}
cd thirdParty/pvrSdk/framework/build/ios/
cmake ../../ -DTARGET_OS=iOS -DIOS_PLATFORM=OS -G Xcode -DCMAKE_TOOLCHAIN_FILE=../../../../cmake/ios/iOS.cmake
cd $orig_dir
