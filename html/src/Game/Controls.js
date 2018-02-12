/**
 * Component for rendering game controls
 */
import React from 'react';
import Game from './Game.js';
var FontAwesome = require('react-fontawesome');

class Controls extends React.Component {

		render() {
			switch (this.props.state)	{
				case Game.State.IDLE:
				case Game.State.END:
					return <div className="controls"><FontAwesome name='play' /></div>;
				case Game.State.STARTING:
				case Game.State.PLAY:
					return <div className="controls">
							<FontAwesome name='stop' />
							<FontAwesome name='repeat' />
						</div>;
				default:
					return <div className="controls"><FontAwesome name='add' /></div>;
			}
		}
	}

	export default Controls;
