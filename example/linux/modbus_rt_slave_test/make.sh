# create build dir if not exists
if [ ! -d "build" ]; then
  mkdir build
fi

cd build
cmake ..
cd ..

# build
cd build
  make
cd ..
