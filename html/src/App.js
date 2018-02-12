import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';

import Game from './Game/Game.js';

class App extends Component {
	constructor(props) {
		super(props);
		this.state = {
			state: props.state
		};
	}

  render() {
    return (
      <div className="App">
        <header className="App-header">
          <img src={logo} className="App-logo" alt="logo" />
          <h1 className="App-title">Laser Targets</h1>
					Pew pew
        </header>
        <Game type={this.state.type} state={this.state.state}/>
      </div>
    );
  }
}
App.defaultProps = {
	state: Game.State.IDLE,
	name: "Game Type"
}

export default App;
