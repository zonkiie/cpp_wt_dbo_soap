# How to install Wt
I've choosen /opt/Wt for installation because it can simply removed.
    cd /dir/to/compile
    wget -O wt-4.5.0.tar.gz https://github.com/emweb/wt/archive/4.5.0.tar.gz
    tar -xavf wt-4.5.0.tar.gz
    cd wt-4.5.0
    cmake -DCMAKE_INSTALL_PREFIX:PATH=/opt/Wt
    make
    make install
    echo -e "# Wt Library\n/opt/Wt/lib/\n" > /etc/ld.so.conf.d/wt.conf
    ldconfig

