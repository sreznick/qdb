#!/bin/bash

cd cmake-build-debug
rm -rf qdata
./qdb initdb qdata

echo "CREATE TABLE users (id INT);" | ./qdb prompt qdata
for i in `seq 1 1 100 | shuf`; do echo "INSERT INTO users VALUES ($i);" | ./qdb prompt ./qdata > /dev/null; done;
echo "CREATE INDEX users ON (id);" | ./qdb prompt ./qdata;