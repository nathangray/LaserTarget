import React from 'react';
import ReactDOM from 'react-dom';
import App from './App';
import Game from './Game/Game.js';
import {shallow, mount, render} from 'enzyme'

it('renders without crashing', () => {
  const div = document.createElement('div');
  ReactDOM.render(<App />, div);
});

it("Fetches state on #componentDidMount", async () => {

	fetch.mockResponse(JSON.stringify({state: 0}), {status: 200})

	const app = shallow(<App />)

  await app.instance().componentDidMount()

	// Starts in idle
	expect(app.state()).toHaveProperty('state', Game.State.IDLE);
});
