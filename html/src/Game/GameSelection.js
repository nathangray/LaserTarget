/**
 * Component for rendering game controls
 */
import React from 'react';
import * as GAME from './constants.js';
var FontAwesome = require('react-fontawesome');

class GameSelection extends React.Component {
	propTypes: {
		state:      React.PropTypes.number,
		value:      React.PropTypes.string,
		onChange:   React.PropTypes.func
	}

	constructor(props) {
    super(props);

		this.handleChange = this.handleChange.bind(this);
  }

	render() {
		if((this.props.state !== GAME.State.IDLE) && this.props.value)
		{
			return GAME.TypeInfo[this.props.value].name;
		}
		return (
			<select id='game_type' value={this.props.value}
				onChange={this.handleChange}
			>
		 	{this.buildOptions()}
			</select>
		);
	}
	buildOptions() {
		var options = [];
		if(!this.props.value || !GAME.Type[this.props.value])
		{
			options.push(<option key='select' value=''>Select</option>);
		}
		for(var type in GAME.Type)
		{
			var info = GAME.TypeInfo[type];
			options.push(<option key={type} value={type} title={info.description}>{info.name}</option>)
		}
		return options;
	}
	handleChange(event) {
		this.setState({value: event.target.value});
		if (typeof this.props.onChange === 'function') {
	      this.props.onChange(event.target.value);
	  }
	}
};

export default GameSelection;
