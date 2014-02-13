ESB
===

Enterprise service bus

Redhat based Linux
===

	#Installing nodejs
	yum groupinstall "Development Tools" -y
	yum install -y git
	cd /tmp/
	wget http://nodejs.org/dist/node-latest.tar.gz
	tar -xzvf node-latest.tar.gz
	cd node-v0.10.*
	make && make install
	
	#node-gyp
	npm install -g node-gyp
	
	#installing redis
	cd /tmp/
	wget http://download.redis.io/releases/redis-2.8.6.tar.gz
	tar -xzvf redis-2.8.6.tar.gz
	cd redis-*
	make && make install
	#run redis server as demon
	redis-server --daemonize yes
	redis-cli ping
	
	#protobuf
	cd /tmp/
	wget https://protobuf.googlecode.com/files/protobuf-2.5.0.tar.gz
	tar -xzvf protobuf-2.5.0.tar.gz
	cd protobuf-2.5.0/
	./configure
	make && make install
	
	#zmq
	cd /tmp/
	wget http://download.zeromq.org/zeromq-4.0.3.tar.gz
	tar -xzvf zeromq-4.0.3.tar.gz
	cd zeromq-*
	./configure
	make && make install
	
	#install gcc 4.8
	rpm --import http://www.scientificlinux.org/documentation/gpg/RPM-GPG-KEY-cern
	wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
	yum install devtoolset-2
	scl enable devtoolset-2 bash
	gcc --version
	
	
	#esb-proxy-server
	cd ~/
	git clone https://github.com/h0x91b/ESB-proxy-server.git
	cd ESB-proxy-server/proxy-server/
	cd src/deps/hiredis/
	make && make install
	cd ../../../
	./build.sh
	
