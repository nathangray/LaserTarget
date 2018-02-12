/**
 * Base component for rendering a Game while playing
 */
import React from 'react';
var FontAwesome = require('react-fontawesome');

class GameRun extends React.Component {
	static propTypes = {
	}

	render() {
		return (<div className="Game_play">
			<FontAwesome name='flag' size='4x'/>
		</div>);
	}
}

export default GameRun;
