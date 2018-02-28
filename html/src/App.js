import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';

import Game from './Game/Game.js';

const API = '/';
const STATE_QUERY = 'state'

class App extends Component {


	constructor(props) {
		super(props);
		this.state = {
			state: props.state,
			isLoading: false,
			error: null
		};
	}

  render() {
/*
    if (this.state.error) {
      return <p>Error:<br />{this.state.error.message}</p>;
    }*/

    if (this.state.isLoading) {
      return <p>Loading ...</p>;
    }
    return (
      <div className="App">
        <header className="App-header">
          <img src={logo} className="App-logo" alt="logo" />
          <h1 className="App-title">Laser Targets</h1>
					{this.state.error ? this.state.error.message : ''}
        </header>
        <Game type={this.state.type} state={this.state.state}/>
      </div>
    );
  }
	componentDidMount() {
    return fetchResponseJson(API+STATE_QUERY).then((response) => {
				if (response.ok) {
					return response.json();
				} else {
					throw new Error('Something went wrong ...');
				}
			})
			.then(data => this.setState({ state: data.state, isLoading: false }))
			.catch(error => this.setState({ error, isLoading: false }));
  }
}
App.defaultProps = {
	state: Game.State.IDLE,
	name: "Game Type"
}

//JSON response
const fetchResponseJson = async (url) => {
    const response = await fetch(url)
    const responseJson = await response.json()
    // You can introduce here an artificial delay, both Promises and async/await will wait until the function returns
    // await sleep(DELAY_MS)
    return responseJson;
}

export default App;
