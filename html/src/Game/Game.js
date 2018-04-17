/**
 * Component for rendering a Game
 */
import React, { Component } from 'react';
import './Game.css';
import GameConfig from './GameConfig.js';
import GameRun from './GameRun.js';
import Controls from './Controls.js';
import GameSelection from './GameSelection.js';
import * as GAME from './constants.js';

class Game extends Component {
	propTypes: {
    state:              React.PropTypes.number,
		type:               React.PropTypes.string,
		config:             React.PropTypes.object,
    stateHandler:       React.PropTypes.func,
		typeHandler:        React.PropTypes.func,
  }
	render() {
		let state = GAME.StateName[this.props.state] || 'Unknown';
		let game = GAME.TypeInfo[this.props.type] || {name: 'None'};
		let className = "Game STATE_" + state.toUpperCase();

		return (<div className={className}>
			<GameSelection
				value={this.props.type}
				state={this.props.state}
				onChange={this.props.typeHandler}
			/> - {state}<br />
			{game.description}
			{this.renderConfig(this.props.type, this.props.config)}
			{this.renderRun()}
			{this.renderControls()}
		</div>);
	}

	/**
	 * Render the configuration panel
	 */
	renderConfig(type, config) {
		if(this.props.state !== GAME.State.IDLE) return;
		switch(type)
		{
			default:
				return <GameConfig value={this.props.config} />;
		}
	}

	/**
	 * Render the running game panel
	 */
	renderRun() {
		if(this.props.state === GAME.State.IDLE) return;
		switch(this.props.type)
		{
			default:
				return <GameRun value={this.props.config}/>;
		}
	}

	/**
	 * Render the pause/play Controls
	 */
	renderControls() {
		return <Controls state={this.props.state} onChange={this.props.stateHandler}/>;
	}

}
export default Game;
