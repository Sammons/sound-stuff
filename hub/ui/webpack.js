const path = require('path');

module.exports = {
  entry: './built/app.js',
  output: {
    filename: 'app.js',
    path: path.resolve(__dirname, 'dist'),
  },
  optimization: {
    minimize: false
  },
  externals: {
    'react': 'React',
    'react-dom': 'ReactDOM'
  }
};