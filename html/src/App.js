import React, { Component } from 'react';
import logo from './logo.svg';
import './App.css';
import APP from './Constants.js';
import * as GAME from './Game/constants.js';
import Game from './Game/Game.js';

const API = '/';
const STATE_QUERY = 'state';
const GAME_QUERY = 'game';

let ws = null;

class App extends Component {


	constructor(props) {
		super(props);
		this.state = {
			state: props.state,
			game: {type: 'None', config: {}},
			isLoading: true,
			isConnected: false,
			error: null
		};
	}

  render() {
    if (this.state.isLoading) {
      return <p>Loading ...</p>;
    }
		var className = "App";
		if(!this.state.isConnected)
		{
			className += " not-connected";
		}
    return (
      <div className={className}>
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
					typeHandler={(state) => this.typeChangeHandler(state)}
				/>
      </div>
    );
  }
	componentDidMount() {
		this.connectWS();
		this.fetchState();
		this.setState({isLoading: false});
  }
	/**
	 * Handle requests (from client) to change the current game state
	 * @param int state One of the state constants
	 */
	stateChangeHandler(state) {
		try {
			ws.send(JSON.stringify({state:state}));
		} catch (error) {
			this.setState({error: error});
		}
	}

	/**
	 * Handle user wants to change the current game type - tell server
	 *
	 * @param string type One of the game types
	 */
	typeChangeHandler(type) {
		try {
			ws.send(JSON.stringify({game: {type: type}}));
		} catch (error) {
			this.setState({error: error});
		}
	}

	/**
	 * Set up WebSocket
	 */
	connectWS(e) {
		// Set up WebSocket
		if(ws) {
			ws.close();
		}
		ws = new WebSocket(APP.WS_URL);
		ws.onopen = () => {
			this.setState({isConnected: true});
		};
		ws.onerror = (e) => {
			console.error('WS error', e);
			this.setState({error: e});
		}
		ws.onclose = (e) => {
			console.log('WS closed', e.code, e.reason);
			this.setState({isConnected: false});

			// Reconnect
			window.setTimeout(() => {
				this.connectWS();
			},APP.WS_RECONNECT);
		}
		ws.onmessage = (e) => {
			this.handleWsMessage(e);
		}
	}

	handleWsMessage(e) {
		console.log("WS message: ", e.data);
		var data = {};
		try {
			data = JSON.parse(e.data) || {};
		} catch (error) {
			return this.setState({error});
		}
		var state = {error: null};
		if(typeof data.state !== "undefined") state.state = data.state;
		if(data.game) state.game = data.game;
		this.setState(state);
	}

	/**
	 * Get current game state from server
	 * Server responds with an integer, one of the game states in Game.State
	 */
	fetchState() {
    return fetchResponseJson(API+STATE_QUERY).then((response) => {
				if (response !== false && GAME.StateName[response]) {
					return response;//parseInt(response, 10);
				} else {
					throw new Error('Something went wrong getting state. Response:'+ JSON.stringify(response));
				}
			})
			.then(data => this.setState({ state: data}))
			.catch(error => {this.setState({ error})});
	}

	/**
	 * Get current game type from server
	 * Server should respond with current game type, and its settings
	 */
	fetchGame() {
		return fetchResponseJson(API+GAME_QUERY).then((response) => {
			console.log(response);
				if (response !== false) {
					return response;
				} else {
					throw new Error('Something went wrong getting game. Response:'+ JSON.stringify(response));
				}
			})
			.then(data => this.setState({ game: data}))
			.catch(error => {this.setState({ error })});
	}

}
App.defaultProps = {
	state: GAME.State.IDLE,
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

export default App;
