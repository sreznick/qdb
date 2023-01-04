#!/bin/bash

# Insert >2k tuples and see the result
# In total 4 pages should be allocated (32kb)

cd cmake-build-debug
./qdb initdb qdata
echo "CREATE TABLE users (id INT);" | ./qdb prompt qdata
for i in `seq 2390`; do echo "INSERT INTO users VALUES ($i);" | ./qdb prompt ./qdata > /dev/null; done;
echo "SELECT * FROM users WHERE id > 2100;" | ./qdb prompt ./qdata;