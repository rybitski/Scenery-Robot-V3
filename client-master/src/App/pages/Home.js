import React, { Component } from 'react';
import { Link } from 'react-router-dom';

import Gamepad from 'react-gamepad'

class Home extends Component {
 



  render() {
    return (
    <div className="App">
      <h1>Project Home</h1>
      {/* Link to List.js */}
      <Link to={'./list'}>
        <button variant="raised">
            Start Manual
        </button>
      </Link>
      

    </div>
    );
  }
}
export default Home;