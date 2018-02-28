/**
 * Component for rendering game controls
 */
import React from 'react';
import Game from './Game.js';
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
			case Game.State.IDLE:
			case Game.State.END:
				return <div className="controls"><FontAwesome name='play' data-state={Game.State.PLAY} onClick={this.changeHandler}/></div>;
			case Game.State.STARTING:
			case Game.State.PLAY:
				return <div className="controls">
						<FontAwesome name='stop' data-state={Game.State.END} />
						<FontAwesome name='repeat' data-state={Game.State.IDLE}/>
					</div>;
			default:
				return <div className="controls"><FontAwesome name='add' data-state={Game.State.IDLE} /></div>;
		}
	}

	changeHandler(e) {
		if (typeof this.props.onChange === 'function') {
        this.props.onChange(e.target.dataset.state);
    }
	}
}

export default Controls;
