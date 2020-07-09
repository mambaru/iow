pushd /tmp/
  echo "download boost"
  wget --quiet https://dl.bintray.com/boostorg/release/1.73.0/source/boost_1_73_0.tar.gz
  echo "unzip boost"
  tar -xf boost_1_73_0.tar.gz
  pushd boost_1_73_0
    echo "bootstrap boost"
    ./bootstrap.sh > /dev/null
    echo "install boost"
    sudo ./b2 link=shared runtime-link=shared threading=multi install > /dev/null
    echo "boost Done!"
  popd
popd
