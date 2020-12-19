import { Landing } from './components/landing';
import * as React from 'react';
import * as reactDOM from 'react-dom';


const root = document.getElementById('root');
if (!root) {
  console.log('no root to mount');
}
reactDOM.render(<Landing/>, root);
