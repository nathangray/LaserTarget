import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';

import Game from './Game/Game.js';

const API = '/';
const STATE_QUERY = 'state';
const GAME_QUERY = 'game';

class App extends Component {


	constructor(props) {
		super(props);
		this.state = {
			state: props.state,
			game: {type: 'None', config: {time:'yarp'}},
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
					<div className="error">{this.state.error ? this.state.error.message : ''}</div>
        </header>
        <Game
					state={this.state.state}
					type={this.state.game.type}
					config={this.state.game.config}
					stateHandler={(state) => this.stateChangeHandler(state)}
				/>
      </div>
    );
  }
	componentDidMount() {
		this.fetchState();
  }
	/**
	 * Handle requests (from client) to change the current game state
	 * @param int state One of the state constants
	 */
	stateChangeHandler(state) {
		putJson(API+STATE_QUERY, state)
			.then(this.fetchState())
			.catch(error => this.setState({error, isLoading: false}));
	}

	/**
	 * Get current game state from server
	 * Server responds with an integer, one of the game states in Game.State
	 */
	fetchState() {
    return fetchResponseJson(API+STATE_QUERY).then((response) => {
				if (response !== false && Game.StateName[response]) {
					return response;//parseInt(response, 10);
				} else {
					throw new Error('Something went wrong getting state. Response:'+ JSON.stringify(response));
				}
			})
			.then(data => this.setState({ state: data, isLoading: false }))
			.catch(error => {this.setState({ error, isLoading: false })});
	}

	/**
	 * Get current game type from server
	 * Server should respond with current game type, and its settings
	 */
	fetchGame() {
		return fetchResponseJson(API+GAME_QUERY).then((response) => {
				if (response !== false && response.isObject) {
					return response;
				} else {
					throw new Error('Something went wrong getting game. Response:'+ JSON.stringify(response));
				}
			})
			.then(data => this.setState({ game: data, isLoading: false }))
			.catch(error => {this.setState({ error, isLoading: false })});
	}

}
App.defaultProps = {
	state: Game.State.IDLE,
	name: "Game Type"
}

//JSON response
const fetchResponseJson = async (url) => {
    const response = await fetch(url);
		if(typeof response === 'undefined') return false;
		const responseJson = await response.json();
    // You can introduce here an artificial delay, both Promises and async/await will wait until the function returns
    // await sleep(DELAY_MS)
    return responseJson;
}

// JSON put
const putJson = async (url, data) => {
	const response = fetch(url, {
		body: JSON.stringify(data),
		cache: 'no-cache',
		headers: {
			'content-type': 'application/json'
		},
		method: 'PUT'
	}).then(response => response.json());
	return response;
}

export default App;
