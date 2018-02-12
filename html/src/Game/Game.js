/**
 * Component for rendering a Game
 */
import React, { Component } from 'react';
import './Game.css';
import GameConfig from './GameConfig.js';
import GameRun from './GameRun.js';
import Controls from './Controls.js';

class Game extends Component {

	render() {
		let state = Game.StateName[this.props.state] || 'Unknown';
		let className = "Game STATE_" + state.toUpperCase();

		return (<div className={className}>
			{state}
			{this.renderConfig()}
			{this.renderRun()}
			{this.renderControls()}
		</div>);
	}

	renderConfig() {
		if(this.props.state !== Game.State.IDLE) return;
		switch(this.props.type)
		{
			default:
				return <GameConfig value={this.props.config} />;
		}
	}

	renderRun() {
		if(this.props.state === Game.State.IDLE) return;
		switch(this.props.type)
		{
			default:
				return <GameRun/>;
		}
	}

	renderControls() {
		return <Controls state={this.props.state}/>;
	}
}

Game.State = {
	IDLE: 0,
	STARTING: 1,
	PLAY: 2,
	ENDING: 10,
	END: 11
}
Game.StateName = {
	0: 'Idle',
	1: 'Starting',
	2: 'Play',
	10: 'Ending',
	11: 'End'
}

export default Game;
