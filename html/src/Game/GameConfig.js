/**
 * Base Component for rendering a Game Configuration
 */
import React, { Component } from 'react';
var FontAwesome = require('react-fontawesome');

class GameConfig extends Component {

	render() {
		return (<div className="Game_config">
			<FontAwesome name='cogs' size='4x'/>
		</div>);
	}
}

export default GameConfig;
