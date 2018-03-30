/**
 * Component for rendering game controls
 */
import React from 'react';
import * as GAME from './constants.js';
var FontAwesome = require('react-fontawesome');

class Controls extends React.Component {
	propTypes: {
      state:      React.PropTypes.number,
      onChange:   React.PropTypes.func
  }

	constructor(props) {
    super(props);

    // This binding is necessary to make `this` work in the callback
    this.changeHandler = this.changeHandler.bind(this);
  }

	render() {
		switch (this.props.state)	{
			case GAME.State.IDLE:
				return <div className="controls">
						<FontAwesome name='play' data-state={GAME.State.PLAY} onClick={this.changeHandler}/>
					</div>;
			case GAME.State.STARTING:
			case GAME.State.PLAY:
				return <div className="controls">
						<FontAwesome name='stop' data-state={GAME.State.END}  onClick={this.changeHandler}/>
						<FontAwesome name='repeat' data-state={GAME.State.IDLE} onClick={this.changeHandler}/>
					</div>;
			case GAME.State.END:
				return <div className="controls">
					<FontAwesome name='cogs' data-state={GAME.State.IDLE} onClick={this.changeHandler} />
					<FontAwesome name='repeat' data-state={GAME.State.PLAY} onClick={this.changeHandler}/>
				</div>;
			default:
				return <div className="controls">
					<FontAwesome name='add' data-state={GAME.State.IDLE} onClick={this.changeHandler} />
				</div>;
		}
	}

	changeHandler(e) {
		if (typeof this.props.onChange === 'function') {
        this.props.onChange(e.target.dataset.state);
    }
	}
}

export default Controls;
