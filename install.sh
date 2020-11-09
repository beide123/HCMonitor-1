yum install -y mysql-devel
yum install -y numactl-devel
yum install -y python-devel

#wget https://hyperrealm.github.io/libconfig/dist/libconfig-1.7.2.tar.gz

cd downloads

tar -xvf libconfig-1.7.2.tar.gz

cd libconfig-1.7.2

./configure 

make && make check && make install

cp /usr/local/lib/libconfig* /usr/lib
