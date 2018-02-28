import React from 'react';
import ReactDOM from 'react-dom';
import {shallow, mount, render} from 'enzyme'

import Game from './Game.js';
import GameConfig from './GameConfig.js';
import GameRun from './GameRun.js';


it("Shows config when idle", async () => {
	const wrapper = mount(<Game state={Game.State.IDLE}/>);

	//console.log(wrapper.html());

	// Idle should show config, not run
	expect(wrapper.find('GameConfig').length).toBe(1);
	expect(wrapper.find(GameRun).length).toBe(0);
});

it("Shows status when running", async () => {
	const wrapper = mount(<Game state={Game.State.PLAY}/>)

	// Running should show run, not config
	expect(wrapper.find(GameConfig).length).toBe(0);
	expect(wrapper.find(GameRun).length).toBe(1);
});
