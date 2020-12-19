#!/bin/node

const fs = require('fs');

if (!fs.existsSync('./server/built/index.js')) {
  console.error(`Failed. please build the server folder to run the server code.`);
  process.exit(1);
}

require('./server/built/index.js');